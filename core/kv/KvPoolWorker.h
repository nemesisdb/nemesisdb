#ifndef _FC_KVPOOL_
#define _FC_KVPOOL_

#include <string_view>
#include <thread>
#include <condition_variable>
#include <boost/fiber/fiber.hpp>
#include <boost/fiber/buffered_channel.hpp>
#include <ankerl/unordered_dense.h>
#include <core/kv/KvCommon.h>
#include <concurrentqueue/concurrentqueue.h>


namespace fusion { namespace core { namespace kv {

using CacheMap = ankerl::unordered_dense::segmented_map<cachedkey, cachedvalue>;


// A pool runs in a thread which is assigned to a core.
// It is assigned based on MaxPools, which is the hardware_concurrency().
class KvPool
{ 

public:

  KvPool(const std::size_t core, const PoolId id) noexcept : m_poolId(id), m_run(true), m_channel(8192U), m_thread(&KvPool::run, this) 
  {
    if (!setThreadAffinity(m_thread.native_handle(), core))
      std::cout << "Failed to assign KvPool thread: " << core << '\n';

    //setThreadRealtime(m_thread.native_handle(), 25);
  }

  ~KvPool()
  {
    m_run = false;
    m_channel.close();
  }

  KvPool& operator=(KvPool&&) noexcept = default;

  fc_always_inline void execute(KvCommand&& command)
  {
    //query.metrics.poolReceived = KvClock::now();
    //query.metrics.poolId = m_poolId;
    
    m_channel.push(std::move(command));
  }

private:
    
  void run ()
  {
    auto doAdd = [](CacheMap& map, KvCommand& command) -> std::tuple<bool, std::string_view>
    {
      const auto& key = command.contents.items().begin().key(); 
      auto& value = command.contents.items().begin().value(); 

      const auto [ignore, added] = map.emplace(key, std::move(value));
      return std::make_tuple(added, key);
    };

    auto set = [](CacheMap& map, KvCommand& command)
    {
      const auto& key = command.contents.begin().key(); 
      auto& value = command.contents.begin().value(); 
      const auto [ignore, inserted] = map.insert_or_assign(key, std::move(value));
      
      command.loop->defer([ws = command.ws, key = std::move(key), inserted](){ws->send(PoolRequestResponse::keySet(inserted, std::move(key)).dump(), kv::WsSendOpCode); });
    };

    auto setQ = [](CacheMap& map, KvCommand& command)
    {
      const auto& key = command.contents.begin().key(); 
      try
      {
        auto& value = command.contents.begin().value(); 
        map.insert_or_assign(key, std::move(value));  
      }
      catch(const std::exception& e)
      {
        kvjson unknownErrRsp{{"SETQ_RSP", {{"st", KvRequestStatus::Unknown}, {"k", std::move(key)}}}};
        command.ws->send(unknownErrRsp.dump(), kv::WsSendOpCode);
      }
    };

    auto get = [](CacheMap& map, KvCommand& command)
    {
      if (const auto it = map.find(command.contents) ; it != map.cend())
      {
        cachedpair pair = {{it->first, it->second}};
        command.loop->defer([ws = command.ws, pair = std::move(pair)]{ ws->send(PoolRequestResponse::getFound(std::move(pair)).dump(), kv::WsSendOpCode);});
      }
      else
        command.loop->defer([ws = command.ws, key = std::move(command.contents)]{ ws->send(PoolRequestResponse::getNotFound(std::move(key)).dump(), kv::WsSendOpCode);});
    };

    auto add = [doAdd](CacheMap& map, KvCommand& command)
    {
      auto [added, key] = doAdd(map, command);
      command.loop->defer([ws = command.ws, added, key = std::move(key)]{ ws->send(PoolRequestResponse::keyAdd(added, std::move(key)).dump(), kv::WsSendOpCode);});
    };

    auto addQ = [doAdd](CacheMap& map, KvCommand& command)
    {
      if (auto [added, key] = doAdd(map, command); !added)  // only respond if key not added
        command.loop->defer([ws = command.ws, added = false, key = std::move(key)]{ ws->send(PoolRequestResponse::keyAdd(added, std::move(key)).dump(), kv::WsSendOpCode);});
    };

    auto remove = [](CacheMap& map, KvCommand& command)
    {
      const auto& key = command.contents.get_ref<const std::string&>();
      const auto nRemoved = map.erase(key);

      command.loop->defer([ws = command.ws, removed = nRemoved != 0U, key = std::move(key)]{ ws->send(PoolRequestResponse::keyRemoved(removed, std::move(key)).dump(), kv::WsSendOpCode);});
    };

    auto clear = [](CacheMap& map, KvCommand& command)
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
      
      command.cordinatedResponseHandler(std::make_any<std::tuple<bool, std::size_t>>(std::make_tuple(valid, size)));
    };

    auto serverInfo = [](CacheMap& map, KvCommand& command)
    {
      // does nothing, never called, but required for handlers array below
    };

    auto count = [](CacheMap& map, KvCommand& command)
    {
      command.cordinatedResponseHandler(std::make_any<std::size_t>(map.size()));
    };

    // auto renameKey = [](CacheMap& map, KvCommand& command)
    // {
    //   auto& existingKey = command.contents.begin().key();
    //   auto newKey = command.contents.begin().value().get<std::string_view>();
      
    //   if (auto it = map.find(existingKey) ; it != map.end())
    //   {
    //     const auto [ignore, inserted] = map.emplace(newKey, std::move(it->second));

    //     if (inserted)
    //     {
    //       map.erase(existingKey);
    //       command.loop->defer([ws = command.ws, contents = std::move(command.contents)]{ ws->send(PoolRequestResponse::renameKey(std::move(contents)).dump(), kv::WsSendOpCode);});
    //     }
    //     else
    //       command.loop->defer([ws = command.ws, contents = std::move(command.contents)]{ ws->send(PoolRequestResponse::renameKeyFail(KvRequestStatus::KeyExists, std::move(contents)).dump(), kv::WsSendOpCode);});
    //   }
    //   else
    //   {
    //     command.loop->defer([ws = command.ws, contents = std::move(command.contents)]{ ws->send(PoolRequestResponse::renameKeyFail(KvRequestStatus::KeyNotExist, std::move(contents)).dump(), kv::WsSendOpCode);}) ;
    //   }
    // };


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
      count/*,
      renameKey*/
    };

    CacheMap map;
    KvCommand command;

    try
    {
      while (m_run)
      {
        command = m_channel.value_pop();
        handlers[static_cast<const std::size_t>(command.type)](map, command);
        // TODO command = KvCommand{};
      }
    }
    catch (const boost::fibers::fiber_error& fex)
    {
      if (!m_channel.is_closed())
      {
        std::cout << "Pool Exception: " << fex.what() << '\n';
        command.loop->defer([ws = command.ws]{ ws->send(createErrorResponse(KvRequestStatus::Unknown).dump(), kv::WsSendOpCode);});
      } 
    }
    catch (const std::exception& ex)
    {
      std::cout << "Pool Exception: " << ex.what() << '\n';
      command.loop->defer([ws = command.ws]{ ws->send(createErrorResponse(KvRequestStatus::Unknown).dump(), kv::WsSendOpCode);});
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
