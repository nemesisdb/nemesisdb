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
        map.set(cmd.contents);
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
        send(cmd, PoolRequestResponse::keyAddQ(std::move(key)).dump()); 
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

      if (!positions.is_array())
        send(cmd, PoolRequestResponse::arrayMove(RequestStatus::ValueTypeInvalid, key).dump());
      else
      {
        const auto status = map.arrayMove(cmd.contents);
        send(cmd, PoolRequestResponse::arrayMove(status, key).dump());
      }
    };


    auto find = [this](CacheMap& map, KvCommand& cmd)
    {
      auto keys = map.find(cmd.contents, cmd.find);
      cmd.cordinatedResponseHandler(std::make_any<std::vector<cachedkey>>(std::move(keys)));
    };


    auto sessionNew = [this](CacheMap& map, KvCommand& cmd)
    {
      send(cmd, PoolRequestResponse::sessionNew(RequestStatus::Ok, cmd.shtk, cmd.contents.at("name")).dump()); 
    };


    auto sessionEnd = [this](CacheMap& map, KvCommand& cmd)
    {
      // handled below 
    };


    auto sessionOpen = [this](CacheMap& map, KvCommand& cmd)
    {
      
    };


    auto sessionSet = [this](CacheMap& map, KvCommand& cmd)
    {
      fcjson rsp;
      rsp["SH_SET_RSP"]["tkn"] = cmd.shtk;

      for(auto& kv : cmd.contents.items())
      {
        auto [it, inserted] = map.set(kv.key(), std::move(kv.value()));
        rsp["SH_SET_RSP"]["keys"][kv.key()] = inserted ? RequestStatus::KeySet : RequestStatus::KeyUpdated;
      }

      send(cmd, rsp.dump());
    };


    auto sessionSetQ = [this](CacheMap& map, KvCommand& cmd)
    {
      fcjson rsp;
      
      for(auto& kv : cmd.contents.items())
      {
        try
        {
          map.set(kv.key(), std::move(kv.value()));
        }
        catch(const std::exception& e)
        {
          rsp["SH_SETQ_RSP"]["keys"][kv.key()] = RequestStatus::Unknown;
        }
      }

      if (!rsp.is_null())
      {
        rsp["SH_SETQ_RSP"]["tkn"] = cmd.shtk;
        send(cmd, rsp.dump());
      }      
    };


    auto sessionGet = [this, &get](CacheMap& map, KvCommand& cmd)
    {
      fcjson rsp;
      rsp["SH_GET_RSP"]["tkn"] = cmd.shtk;

      for(auto& item : cmd.contents.items())
      {
        if (auto [exists, pair] = map.get(item.value()); exists)
          rsp["SH_GET_RSP"].emplace(std::move(pair.begin().key()), std::move(pair.begin().value()));
        else
          rsp["SH_GET_RSP"][item.value()] = fcjson{}; //null
      }

      send(cmd, rsp.dump());
    };


    auto sessionAdd = [this](CacheMap& map, KvCommand& cmd)
    {
      fcjson rsp;
      rsp["SH_ADD_RSP"]["tkn"] = cmd.shtk;

      for(auto& kv : cmd.contents.items())
      {
        const auto inserted = map.add(kv.key(), std::move(kv.value()));
        rsp["SH_ADD_RSP"]["keys"][kv.key()] = inserted ? RequestStatus::KeySet : RequestStatus::KeyExists;
      }

      send(cmd, rsp.dump());
    };


    auto sessionAddQ = [this](CacheMap& map, KvCommand& cmd)
    {
      fcjson rsp;

      for(auto& kv : cmd.contents.items())
      {
        if (const auto inserted = map.add(kv.key(), std::move(kv.value())); !inserted)
          rsp["SH_ADD_RSP"]["keys"][kv.key()] = RequestStatus::KeyExists;
      }

      if (!rsp.is_null())
      {
        rsp["SH_ADD_RSP"]["tkn"] = cmd.shtk;
        send(cmd, rsp.dump());
      }
    };


    auto sessionRemove = [this](CacheMap& map, KvCommand& cmd)
    {
      const auto& key = cmd.contents.get_ref<const std::string&>();
      const auto removed = map.remove(key);
      send(cmd, PoolRequestResponse::sessionRemove(cmd.shtk, removed, std::move(key)).dump());
    };


    auto sessionClear = [this](CacheMap& map, KvCommand& cmd)
    {
      const auto[valid, size] = map.clear();
      send(cmd, PoolRequestResponse::sessionClear(cmd.shtk, valid, size).dump());
    };


    auto sessionCount = [this](CacheMap& map, KvCommand& cmd)
    {
      send(cmd, PoolRequestResponse::sessionCount(cmd.shtk, map.count()).dump());
    };


    auto sessionAppend = [this](CacheMap& map, KvCommand& cmd)
    {
      RequestStatus status = map.append(cmd.contents);
      auto& key = cmd.contents.begin().key();

      send(cmd, PoolRequestResponse::sessionAppend(cmd.shtk, status, std::move(key)).dump());
    };


    auto sessionContains = [this](CacheMap& map, KvCommand& cmd)
    {
      fcjson rsp;
      rsp["SH_CONTAINS_RSP"]["st"] = RequestStatus::Ok;
      rsp["SH_CONTAINS_RSP"]["tkn"] = cmd.shtk;

      for (auto& item : cmd.contents.items())
        rsp["SH_CONTAINS_RSP"]["keys"][item.value()] = map.contains(item.value());

      send(cmd, rsp.dump());
    };


    auto sessionArrayMove = [this](CacheMap& map, KvCommand& cmd)
    {
      fcjson rsp;
      rsp["SH_ARRAY_MOVE_RSP"]["tkn"] = cmd.shtk;

      for (auto& item : cmd.contents.items())
      {
        if (!item.value().is_array())
          rsp["SH_ARRAY_MOVE_RSP"][item.key()] = RequestStatus::ValueTypeInvalid;
        else
        {
          const auto status = map.arrayMove(fcjson {{item.key(), std::move(item.value())}});
          rsp["SH_ARRAY_MOVE_RSP"][item.key()] = status;
        }
      }

      send(cmd, rsp.dump());
    };


    auto sessionFind = [this](CacheMap& map, KvCommand& cmd)
    {
      fcjson rsp;
      rsp["SH_FIND_RSP"]["tkn"] = cmd.shtk;
      rsp["SH_FIND_RSP"]["keys"] = fcjson::array();
      
      map.findNoRegEx(cmd.contents, cmd.find, rsp["SH_FIND_RSP"]["keys"]);

      send(cmd, rsp.dump());
    };


    auto sessionUpdate = [this](CacheMap& map, KvCommand& cmd)
    {
      fcjson rsp;
      rsp["SH_UPDATE_RSP"]["tkn"] = cmd.shtk;
            
      fcjson::iterator  itKey = cmd.contents.begin(),
                        itPath = std::next(itKey, 1);

      if (itKey.key() != "key")
        std::swap(itKey, itPath);

      bool pathValid = true;
      fcjson::json_pointer path;

      try
      {
        path = std::move(fcjson::json_pointer{itPath.key()});
      }
      catch (...)
      {
        pathValid = false;
      }

      RequestStatus status;

      if (pathValid)
        status = map.updateByPath(itKey.value(), path, std::move(itPath.value()));
      else
        status = RequestStatus::PathInvalid;  // rename to PathInvalid

      rsp["SH_UPDATE_RSP"]["st"] = status;

      send(cmd, rsp.dump());
    };


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
      sessionNew,
      sessionEnd,
      sessionOpen,
      sessionSet,
      sessionSetQ,
      sessionGet,
      sessionAdd,
      sessionAddQ,
      sessionRemove,
      sessionClear,
      sessionCount,
      sessionAppend,
      sessionContains,
      sessionArrayMove,
      sessionFind,
      sessionUpdate
    };


    KvCommand cmd;
    Sessions sessions;

    // create default session
    sessions.start(defaultSessionToken);

    while (m_run)
    {
      try
      {
        cmd = std::move(m_channel.value_pop());

        if (cmd.type == KvQueryType::SessionNew)
        {
          auto cache = sessions.start(cmd.shtk);
          handlers[static_cast<const std::size_t>(cmd.type)](cache->get(), cmd);
        }
        else if (cmd.type == KvQueryType::SessionEnd)
        {
          auto status = sessions.end(cmd.shtk) ? RequestStatus::Ok : RequestStatus::SessionNotExist;
          send(cmd, PoolRequestResponse::sessionEnd(status, cmd.shtk).dump());
        }
        else if (cmd.type == KvQueryType::SessionOpen)
        {
          const auto haveSession = sessions.contains(cmd.shtk);
          cmd.cordinatedResponseHandler(std::any{haveSession});
        }
        else if (auto cache = sessions.get(cmd.shtk); cache)
          handlers[static_cast<const std::size_t>(cmd.type)](cache->get(), cmd);
        else
          send(cmd, createMessageResponse(QueryTypeToName.at(cmd.type) + "_RSP", RequestStatus::SessionNotExist).dump());
      
        // TODO command = KvCommand{};
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
