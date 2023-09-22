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

  KvPool(const std::size_t core, const PoolId id) noexcept : m_core(core), m_poolId(id), m_run(true), m_channel(8192U), m_thread(&KvPool::run, this) 
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
    auto set = [](CacheMap& map, KvCommand& command)
    {
      const auto& key = command.contents.begin().key(); 
      auto& value = command.contents.begin().value(); 
      const auto [ignore, inserted] = map.insert_or_assign(key, std::move(value));
      
      // TODO problem here, probably because calling from thread outside of uWS event loop
      command.loop->defer([ws = command.ws, key = std::move(key), inserted](){ws->send(PoolRequestResponse::keySet(inserted, std::move(key)).dump(), uWS::OpCode::TEXT); });
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
        command.ws->send(unknownErrRsp.dump(), uWS::OpCode::TEXT);
      }
    };

    auto get = [](CacheMap& map, KvCommand& command)
    {
      if (const auto it = map.find(command.contents) ; it != map.cend())
      {
        cachedpair pair = {{it->first, it->second}};
        command.loop->defer([ws = command.ws, pair = std::move(pair)]{ ws->send(PoolRequestResponse::getFound(std::move(pair)).dump(), uWS::OpCode::TEXT);});
      }
      else
        command.ws->send(PoolRequestResponse::getNotFound(std::move(command.contents)).dump(), uWS::OpCode::TEXT);
    };


    /*
    auto setQ = [](CacheMap& map, KvQuery& query)
    {
      const auto& key = query.contents.items().begin().key(); 
      auto& value = query.contents.items().begin().value(); 
      map.insert_or_assign(key, std::move(value));
    };

    auto add = [](CacheMap& map, KvQuery& query)
    {
      const auto& key = query.contents.items().begin().key(); 
      auto& value = query.contents.items().begin().value(); 

      const auto [ignore, added] = map.emplace(key, std::move(value));

      //query.metrics.poolHandlerComplete = KvClock::now();
      //query.rspHandler(PoolRequestResponse::keyAdd(added, key, //query.metrics));
    };

    auto addQ = add;  // they are the same, the handler decides if a response is sent.

    auto remove = [](CacheMap& map, KvQuery& query)
    {
      const auto& key = query.contents.get_ref<const std::string&>();
      const auto nRemoved = map.erase(key);

      //query.metrics.poolHandlerComplete = KvClock::now();
      //query.rspHandler(PoolRequestResponse::keyRemoved(nRemoved != 0U, key, //query.metrics));
    };

    auto clear = [](CacheMap& map, KvQuery& query)
    {
      const auto size = map.size();
      map.clear();
      //query.metrics.poolHandlerComplete = KvClock::now();

      //query.rspHandler(PoolRequestResponse::clear(size, //query.metrics));
    };

    auto serverInfo = [](CacheMap& map, KvQuery& query)
    {
      const auto size = map.size();
      //query.metrics.poolHandlerComplete = KvClock::now();

      //query.rspHandler(PoolRequestResponse::count(size, //query.metrics));
    };

    auto renameKey = [](CacheMap& map, KvQuery& query)
    {
      auto& existingKey = query.contents.begin().key();
      auto newKey = query.contents.begin().value().get<std::string_view>();
      
      if (auto it = map.find(existingKey) ; it != map.end())
      {
        const auto [ignore, inserted] = map.emplace(newKey, std::move(it->second));

        if (inserted)
          map.erase(existingKey);
        
        //query.metrics.poolHandlerComplete = KvClock::now();
        inserted ?  //query.rspHandler(PoolRequestResponse::renameKey(query.contents, //query.metrics)) :
                    //query.rspHandler(PoolRequestResponse::renameKeyFail(QueryStatus::KeyExists, query.contents, //query.metrics));
      }
      else
      {
        //query.metrics.poolHandlerComplete = KvClock::now();
        //query.rspHandler(PoolRequestResponse::renameKeyFail(QueryStatus::KeyNotExist, query.contents, //query.metrics));
      }
    };
    */
    
    static const std::array<std::function<void(CacheMap&, KvCommand&)>, static_cast<std::size_t>(3U/*KvQueryType::Max*/)> handlers = 
    {      
      set,
      setQ,
      get/*,
      add,
      addQ,
      remove,
      clear,
      serverInfo,
      renameKey*/
    };

    CacheMap map;
    KvCommand command;

    try
    {
      while (m_run)
      {
        command = m_channel.value_pop();
        // //query.metrics.poolHandlerCalled = KvClock::now();

        handlers[static_cast<const std::size_t>(command.type)](map, command);
        
        // TODO command = KvCommand{};
      }
    }
    catch (const boost::fibers::fiber_error fex)
    {
      if (!m_channel.is_closed())
      {
        std::cout << "Pool Exception: " << fex.what() << '\n';
        // if (//query.rspHandler)
        //   //query.rspHandler(PoolRequestResponse::unknownError());
      } 
    }
    catch (const std::exception& ex)
    {
      std::cout << "Pool Exception: " << ex.what() << '\n';
      // if (//query.rspHandler)
      //   //query.rspHandler(PoolRequestResponse::unknownError());
    }
  }


private:
  std::size_t m_core;
  PoolId m_poolId;
  std::atomic_bool m_run;
  std::jthread m_thread;
  boost::fibers::buffered_channel<KvCommand> m_channel;  
  
};

}
}
}

#endif
