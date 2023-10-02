#ifndef _FC_KVPOOL_
#define _FC_KVPOOL_

#include <string_view>
#include <thread>
#include <condition_variable>
#include <algorithm> // for std::move(start, end, target)
#include <boost/fiber/fiber.hpp>
#include <boost/fiber/buffered_channel.hpp>
#include <ankerl/unordered_dense.h>
#include <core/kv/KvCommon.h>
#include <concurrentqueue/concurrentqueue.h>


namespace fusion { namespace core { namespace kv {

using CacheMap = ankerl::unordered_dense::segmented_map<cachedkey, cachedvalue>;


// A number of KvPoolWorker are created (MaxPools), which is typically hardware_concurrency() minus the number of IO threads.
// A pool worker runs in a thread which is assigned to a core. Each pool worker has a dedicated map which stores the key/values. 
// A key is hashed and then modulode with MaxPools which determines which pool worker handles that key.
// The hash is just a simple addition of the first N characters in the key (rather than std::hash, which is a bit heavy for this usecase).
class KvPoolWorker
{ 

public:

  KvPoolWorker(const std::size_t core, const PoolId id) noexcept : m_poolId(id), m_run(true), m_channel(8192U), m_thread(&KvPoolWorker::run, this) 
  {
    if (!setThreadAffinity(m_thread.native_handle(), core))
      std::cout << "Failed to assign KvPoolWorker thread: " << core << '\n';
    //else
      //std::cout << "Assigned KvPoolWorker thread: " << core << '\n';

    //setThreadRealtime(m_thread.native_handle(), 25);
  }

  ~KvPoolWorker()
  {
    m_run = false;
    m_channel.close();
  }

  KvPoolWorker& operator=(KvPoolWorker&&) noexcept = default;


  fc_always_inline void execute(KvCommand&& cmd)
  {
    m_channel.push(std::move(cmd));
  }

private:
    
  fc_always_inline void send (KvCommand& cmd, std::string&& msg)
  {
    cmd.loop->defer([cmd, msg = std::move(msg)] () mutable
    {
      if (cmd.ws->getUserData()->connected)
        cmd.ws->send(msg, kv::WsSendOpCode);
    });
  }
  
  void run ()
  {
    auto doAdd = [](CacheMap& map, KvCommand& cmd) -> std::tuple<bool, std::string>
    {
      const auto& key = cmd.contents.items().begin().key();
      auto& value = cmd.contents.items().begin().value(); 

      const auto [ignore, added] = map.emplace(key, std::move(value));
      return std::make_tuple(added, key);
    };

    auto set = [this](CacheMap& map, KvCommand& cmd)
    {
      const auto& key = cmd.contents.begin().key(); 
      auto& value = cmd.contents.begin().value(); 
      const auto [ignore, inserted] = map.insert_or_assign(key, std::move(value));
      
      send(cmd, PoolRequestResponse::keySet(inserted, key).dump());
    };

    auto setQ = [this](CacheMap& map, KvCommand& cmd)
    {
      const auto& key = cmd.contents.begin().key(); 
      try
      {
        auto& value = cmd.contents.begin().value(); 
        map.insert_or_assign(key, std::move(value));  
      }
      catch(const std::exception& e)
      {
        kvjson unknownErrRsp{{"KV_SETQ_RSP", {{"st", KvRequestStatus::Unknown}, {"k", std::move(key)}}}};
        send(cmd, unknownErrRsp.dump());
      }
    };

    auto get = [this](CacheMap& map, KvCommand& cmd)
    {
      if (const auto it = map.find(cmd.contents) ; it != map.cend())
      {
        cachedpair pair = {{it->first, it->second}};
        send(cmd, PoolRequestResponse::getFound(std::move(pair)).dump());
      }
      else
        send(cmd, PoolRequestResponse::getNotFound(std::move(cmd.contents)).dump()); 
    };

    auto add = [doAdd, this](CacheMap& map, KvCommand& cmd)
    {
      auto [added, key] = doAdd(map, cmd);
      send(cmd, PoolRequestResponse::keyAdd(added, std::move(key)).dump()); 
    };

    auto addQ = [doAdd, this](CacheMap& map, KvCommand& cmd)
    {
      if (auto [added, key] = doAdd(map, cmd); !added)  // only respond if key not added
        send(cmd, PoolRequestResponse::keyAddQ(added, std::move(key)).dump()); 
    };

    auto remove = [this](CacheMap& map, KvCommand& cmd)
    {
      const auto& key = cmd.contents.get_ref<const std::string&>();
      const auto nRemoved = map.erase(key);

      send(cmd, PoolRequestResponse::keyRemoved(nRemoved, std::move(key)).dump()); 
    };

    auto clear = [this](CacheMap& map, KvCommand& cmd)
    {
      const std::size_t size = map.size();
      bool valid = true;

      try
      {
        map.clear();
      }
      catch (...)
      {
        valid = false;
      }
      
      cmd.cordinatedResponseHandler(std::make_any<std::tuple<bool, std::size_t>>(std::make_tuple(valid, size)));
    };

    auto serverInfo = [this](CacheMap& map, KvCommand& cmd)
    {
      // does nothing, never called, but required for handlers array below
    };

    auto count = [this](CacheMap& map, KvCommand& cmd)
    {
      cmd.cordinatedResponseHandler(std::make_any<std::size_t>(map.size()));
    };

    auto append = [this](CacheMap& map, KvCommand& cmd)
    {
      KvRequestStatus status = KvRequestStatus::Ok;

      auto& key = cmd.contents.begin().key();
      if (const auto it = map.find(key) ; it != map.cend())
      {
        const auto type = cmd.contents.begin().value().type();
        switch (type)
        {
          case kvjson::value_t::array:
          {
            for (auto& item : cmd.contents.at(key))
              it->second.insert(it->second.end(), std::move(item));
          }
          break;

          case kvjson::value_t::object:
            it->second.insert(cmd.contents.begin().value().begin(), cmd.contents.begin().value().end());
          break;

          case kvjson::value_t::string:
            it->second.get_ref<kvjson::string_t&>().append(cmd.contents.begin().value());
          break;

          default:
            status = KvRequestStatus::ValueTypeInvalid;
          break;
        }
      }
      else
        status = KvRequestStatus::KeyNotExist;

      send(cmd, PoolRequestResponse::append(status, std::move(key)).dump());
    };

    auto contains = [this](CacheMap& map, KvCommand& cmd)
    {
      if (map.contains(cmd.contents))
        send(cmd, PoolRequestResponse::contains(KvRequestStatus::KeyExists, cmd.contents.get<std::string_view>()).dump());
      else
        send(cmd, PoolRequestResponse::contains(KvRequestStatus::KeyNotExist, cmd.contents.get<std::string_view>()).dump());
    };

    auto arrayMove = [this](CacheMap& map, KvCommand& cmd)
    {
      auto& key = cmd.contents.begin().key();
      auto& positions = cmd.contents.begin().value();
      
      if (positions.size() != 2U && positions.size() != 1U)
        send(cmd, PoolRequestResponse::arrayMove(KvRequestStatus::ValueSize, key).dump());
      else
      { 
        if (auto it = map.find(key) ; it == map.cend())
          send(cmd, PoolRequestResponse::arrayMove(KvRequestStatus::KeyNotExist, key).dump()); 
        else  [[likely]]
        {
          if (auto& array = it->second; !array.is_array())
            send(cmd, PoolRequestResponse::arrayMove(KvRequestStatus::ValueTypeInvalid, key).dump()); 
          else if (array.empty())
            send(cmd, PoolRequestResponse::arrayMove(KvRequestStatus::OutOfBounds, key).dump()); 
          else
          {
            auto isIndexValidType = [](const json& array, const std::size_t index)
            {
              return array[index].is_number_unsigned();
            };

            if (positions.size() == 1U && !isIndexValidType(positions, 0U))
              send(cmd, PoolRequestResponse::arrayMove(KvRequestStatus::ValueTypeInvalid, key).dump());
            else if (positions.size() == 2U && (!isIndexValidType(positions, 0U) || !isIndexValidType(positions, 1U)))
              send(cmd, PoolRequestResponse::arrayMove(KvRequestStatus::ValueTypeInvalid, key).dump());
            else
            {
              const std::int64_t currPos = positions[0U];
              const std::int64_t newPos = positions.size() == 1U ? array.size() : positions[1U].get<std::int64_t>(); // at the end if no position supplied

              if (currPos < 0 || currPos > array.size() - 1 || newPos < 0)
                send(cmd, PoolRequestResponse::arrayMove(KvRequestStatus::OutOfBounds, key).dump());
              else if (currPos == newPos)
                send(cmd, PoolRequestResponse::arrayMove(KvRequestStatus::Ok, key).dump());
              else
              {
                array.insert(std::next(array.cbegin(), newPos), std::move(array[currPos]));
                array.erase(currPos > newPos ? currPos+1 : currPos);

                send(cmd, PoolRequestResponse::arrayMove(KvRequestStatus::Ok, key).dump());
              }
            }
          }
        }
      }        
    };


    /*
    auto renameKey = [](CacheMap& map, KvCommand& cmd)
    {
      auto& existingKey = command->contents.begin().key();
      auto newKey = command->contents.begin().value().get<std::string_view>();
      
      if (auto it = map.find(existingKey) ; it != map.end())
      {
        const auto [ignore, inserted] = map.emplace(newKey, std::move(it->second));

        if (inserted)
        {
          map.erase(existingKey);
          command.loop->defer([ws = command.ws, contents = std::move(command->contents)]{ ws->send(PoolRequestResponse::renameKey(std::move(contents)).dump(), kv::WsSendOpCode);});
        }
        else
          command.loop->defer([ws = command.ws, contents = std::move(command->contents)]{ ws->send(PoolRequestResponse::renameKeyFail(KvRequestStatus::KeyExists, std::move(contents)).dump(), kv::WsSendOpCode);});
      }
      else
      {
        command.loop->defer([ws = command.ws, contents = std::move(command->contents)]{ ws->send(PoolRequestResponse::renameKeyFail(KvRequestStatus::KeyNotExist, std::move(contents)).dump(), kv::WsSendOpCode);}) ;
      }
    };
    */


    // CAREFUL: these have to be in the order of KvQueryType enum
    static const std::array<std::function<void(CacheMap&, KvCommand&)>, static_cast<std::size_t>(KvQueryType::Max)> handlers = 
    {      
      set,
      setQ,
      get,
      add,
      addQ,
      remove,
      clear,
      serverInfo,
      count,
      append,
      contains,
      arrayMove
      /*renameKey*/
    };

    CacheMap map;
    KvCommand cmd;

    try
    {
      while (m_run)
      {
        cmd = std::move(m_channel.value_pop());
        handlers[static_cast<const std::size_t>(cmd.type)](map, cmd);
        // TODO command = KvCommand{};
      }
    }
    catch (const boost::fibers::fiber_error& fex)
    {
      if (!m_channel.is_closed())
      {
        std::cout << "Pool Fiber Exception: " << fex.what() << '\n';
        send(cmd, createErrorResponse(KvRequestStatus::Unknown).dump());;
      } 
    }
    catch (const std::exception& ex)
    {
      std::cout << "Pool Exception: " << ex.what() << '\n';
      send(cmd, createErrorResponse(KvRequestStatus::Unknown).dump());
    }
  }


private:
  PoolId m_poolId;
  std::atomic_bool m_run; // TODO this doesn't need to be atomic, stop() just closes channel?
  std::jthread m_thread;
  boost::fibers::buffered_channel<KvCommand> m_channel;  
  
};

}
}
}

#endif
