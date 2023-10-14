#ifndef _FC_KVPOOL_
#define _FC_KVPOOL_

#include <string_view>
#include <thread>
#include <condition_variable>
#include <regex>
#include <boost/fiber/fiber.hpp>
#include <boost/fiber/buffered_channel.hpp>
#include <ankerl/unordered_dense.h>
#include <concurrentqueue/concurrentqueue.h>
#include <core/CacheMap.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvSessions.h>


namespace fusion { namespace core { namespace kv {

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
    auto set = [this](CacheMap& map, KvCommand& cmd)
    {
      auto [it, inserted] = map.set(cmd.contents);
      send(cmd, PoolRequestResponse::keySet(inserted, it->first).dump());
    };


    auto setQ = [this](CacheMap& map, KvCommand& cmd)
    {
      try
      {
        map.setQ(cmd.contents);
      }
      catch(const std::exception& e)
      {
        const auto& key = cmd.contents.begin().key();
        fcjson unknownErrRsp{{"KV_SETQ_RSP", {{"st", RequestStatus::Unknown}, {"k", std::move(key)}}}};
        send(cmd, unknownErrRsp.dump());
      }
    };


    auto get = [this](CacheMap& map, KvCommand& cmd)
    {
      if (auto [exists, pair] = map.get(cmd.contents); exists)
        send(cmd, PoolRequestResponse::getFound(std::move(pair)).dump());
      else
        send(cmd, PoolRequestResponse::getNotFound(std::move(cmd.contents)).dump()); 
    };


    auto add = [this](CacheMap& map, KvCommand& cmd)
    {
      auto [added, key] = map.add(cmd.contents);
      send(cmd, PoolRequestResponse::keyAdd(added, std::move(key)).dump()); 
    };


    auto addQ = [this](CacheMap& map, KvCommand& cmd)
    {
      if (auto [added, key] = map.add(cmd.contents); !added)  // only respond if key not added
        send(cmd, PoolRequestResponse::keyAddQ(added, std::move(key)).dump()); 
    };


    auto remove = [this](CacheMap& map, KvCommand& cmd)
    {
      const auto& key = cmd.contents.get_ref<const std::string&>();
      const auto removed = map.remove(key);
      send(cmd, PoolRequestResponse::keyRemoved(removed, std::move(key)).dump()); 
    };


    auto clear = [this](CacheMap& map, KvCommand& cmd)
    {
      const auto[valid, size] = map.clear();
      cmd.cordinatedResponseHandler(std::make_any<std::tuple<bool, std::size_t>>(std::make_tuple(valid, size)));
    };


    auto serverInfo = [this](CacheMap& map, KvCommand& cmd)
    {
      // does nothing, dealt with by ShHandler/KvHandler, but required for handlers array below
    };


    auto count = [this](CacheMap& map, KvCommand& cmd)
    {
      cmd.cordinatedResponseHandler(std::make_any<std::size_t>(map.count()));
    };

    
    auto append = [this](CacheMap& map, KvCommand& cmd)
    {
      RequestStatus status = map.append(cmd.contents);
      auto& key = cmd.contents.begin().key();

      send(cmd, PoolRequestResponse::append(status, std::move(key)).dump());
    };

    
    auto contains = [this](CacheMap& map, KvCommand& cmd)
    {
      if (map.contains(cmd.contents))
        send(cmd, PoolRequestResponse::contains(RequestStatus::KeyExists, cmd.contents.get<std::string_view>()).dump());
      else
        send(cmd, PoolRequestResponse::contains(RequestStatus::KeyNotExist, cmd.contents.get<std::string_view>()).dump());
    };

    
    auto arrayMove = [this](CacheMap& map, KvCommand& cmd)
    {
      auto& key = cmd.contents.begin().key();
      auto& positions = cmd.contents.begin().value();
      
      const auto status = map.arrayMove(cmd.contents);
      send(cmd, PoolRequestResponse::arrayMove(status, key).dump());       
    };


    auto find = [this](CacheMap& map, KvCommand& cmd)
    {
      auto keys = map.find(cmd.contents, cmd.find);
      cmd.cordinatedResponseHandler(std::make_any<std::vector<cachedkey>>(std::move(keys)));
    };


    auto sessionNew = [this](CacheMap& map, KvCommand& cmd)
    {
      send(cmd, PoolRequestResponse::sessionStart(RequestStatus::Ok, cmd.shtk, cmd.contents.at("name")).dump()); 
    };


    /*
    auto renameKey = [](CacheMap2& map, KvCommand& cmd)
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
          command.loop->defer([ws = command.ws, contents = std::move(command->contents)]{ ws->send(PoolRequestResponse::renameKeyFail(RequestStatus::KeyExists, std::move(contents)).dump(), kv::WsSendOpCode);});
      }
      else
      {
        command.loop->defer([ws = command.ws, contents = std::move(command->contents)]{ ws->send(PoolRequestResponse::renameKeyFail(RequestStatus::KeyNotExist, std::move(contents)).dump(), kv::WsSendOpCode);}) ;
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
      arrayMove,
      find,
      sessionNew
    };


    KvCommand cmd;
    Sessions sessions;

    try
    {
      // create default session
      sessions.start(defaultSessionToken);

      while (m_run)
      {
        cmd = std::move(m_channel.value_pop());

        if (cmd.type == KvQueryType::SessionNew)
        {
          auto cache = sessions.start(cmd.shtk);
          handlers[static_cast<const std::size_t>(cmd.type)](cache->get(), cmd);
        }
        else if (auto cache = sessions.get(cmd.shtk); cache)
          handlers[static_cast<const std::size_t>(cmd.type)](cache->get(), cmd);
        else
          send(cmd, createMessageResponse(QueryTypeToName.at(cmd.type) + "_RSP", RequestStatus::SessionNotExist).dump());
      
        // TODO command = KvCommand{};
      }
    }
    catch (const boost::fibers::fiber_error& fex)
    {
      if (!m_channel.is_closed())
      {
        std::cout << "Pool Fiber Exception: " << fex.what() << '\n';
        send(cmd, createErrorResponse(RequestStatus::Unknown).dump());
      } 
    }
    catch (const std::exception& ex)
    {
      std::cout << "Pool Exception: " << ex.what() << '\n';
      send(cmd, createErrorResponse(RequestStatus::Unknown).dump());
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
