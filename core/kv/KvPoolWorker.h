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


// A number of KvPoolWorker are created (MaxPools), which is hardware_concurrency() minus the number of IO threads.
// A pool worker runs in a thread which is assigned to a core. Each pool worker has a dedicated map which stores the key/values. 
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
      // handled below
    };


    auto sessionInfo = [this](CacheMap& map, KvCommand& cmd)
    {
      // handled below
    };


    auto sessionInfoAll = [this](CacheMap& map, KvCommand& cmd)
    {
      // handled below
    };


    auto set = [this](CacheMap& map, KvCommand& cmd)
    {
      fcjson rsp;
      rsp["KV_SET_RSP"]["tkn"] = cmd.shtk;

      for(auto& kv : cmd.contents.items())
      {
        auto [it, inserted] = map.set(kv.key(), std::move(kv.value()));
        rsp["KV_SET_RSP"]["keys"][kv.key()] = inserted ? RequestStatus::KeySet : RequestStatus::KeyUpdated;
      }

      send(cmd, rsp.dump());
    };


    auto setQ = [this](CacheMap& map, KvCommand& cmd)
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
          rsp["KV_SETQ_RSP"]["keys"][kv.key()] = RequestStatus::Unknown;
        }
      }

      if (!rsp.is_null())
      {
        rsp["KV_SETQ_RSP"]["tkn"] = cmd.shtk;
        send(cmd, rsp.dump());
      }      
    };


    auto get = [this](CacheMap& map, KvCommand& cmd)
    {
      fcjson rsp;
      rsp["KV_GET_RSP"]["tkn"] = cmd.shtk;

      for(auto& item : cmd.contents.items())
      {
        if (item.value().is_string()) [[likely]]
        {
          if (auto [exists, pair] = map.get(item.value()); exists)
            rsp["KV_GET_RSP"].emplace(std::move(pair.begin().key()), std::move(pair.begin().value()));
          else
            rsp["KV_GET_RSP"][item.value()] = fcjson{}; //null
        }        
      }

      send(cmd, rsp.dump());
    };


    auto add = [this](CacheMap& map, KvCommand& cmd)
    {
      fcjson rsp;
      rsp["KV_ADD_RSP"]["tkn"] = cmd.shtk;

      if (cmd.contents.empty())
        rsp["KV_ADD_RSP"]["keys"] = fcjson::object();

      for(auto& kv : cmd.contents.items())
      {
        const auto inserted = map.add(kv.key(), std::move(kv.value()));
        rsp["KV_ADD_RSP"]["keys"][kv.key()] = inserted ? RequestStatus::KeySet : RequestStatus::KeyExists;
      }

      send(cmd, rsp.dump());
    };


    auto addQ = [this](CacheMap& map, KvCommand& cmd)
    {
      fcjson rsp;

      if (cmd.contents.empty())
      {
        rsp["KV_ADDQ_RSP"]["keys"] = fcjson::object();
        send(cmd, rsp.dump());
      }
      else
      {
        for(auto& kv : cmd.contents.items())
        {
          if (const auto inserted = map.add(kv.key(), std::move(kv.value())); !inserted)
            rsp["KV_ADDQ_RSP"]["keys"][kv.key()] = RequestStatus::KeyExists;
        }

        if (!rsp.is_null())
        {
          rsp["KV_ADDQ_RSP"]["tkn"] = cmd.shtk;
          send(cmd, rsp.dump());
        }
      }
    };


    auto remove = [this](CacheMap& map, KvCommand& cmd)
    {
      fcjson rsp;
      rsp["KV_RMV_RSP"]["tkn"] = cmd.shtk;

      for(auto& key : cmd.contents.items())
      {
        if (key.value().is_string())
        {
          const auto removed = map.remove(key.value().get_ref<const cachedkey&>());
          rsp["KV_RMV_RSP"][key.value()] = removed ? RequestStatus::KeyRemoved : RequestStatus::KeyNotExist;
        }
      }

      send(cmd, rsp.dump());
    };


    auto clear = [this](CacheMap& map, KvCommand& cmd)
    {
      const auto[valid, size] = map.clear();
      send(cmd, PoolRequestResponse::sessionClear(cmd.shtk, valid, size).dump());
    };


    auto count = [this](CacheMap& map, KvCommand& cmd)
    {
      send(cmd, PoolRequestResponse::sessionCount(cmd.shtk, map.count()).dump());
    };


    auto append = [this](CacheMap& map, KvCommand& cmd)
    {
      RequestStatus status = map.append(cmd.contents);
      auto& key = cmd.contents.begin().key();

      send(cmd, PoolRequestResponse::sessionAppend(cmd.shtk, status, std::move(key)).dump());
    };


    auto contains = [this](CacheMap& map, KvCommand& cmd)
    {
      fcjson rsp;
      rsp["KV_CONTAINS_RSP"]["st"] = RequestStatus::Ok;
      rsp["KV_CONTAINS_RSP"]["tkn"] = cmd.shtk;

      for (auto& item : cmd.contents.items())
        rsp["KV_CONTAINS_RSP"]["keys"][item.value()] = map.contains(item.value());

      send(cmd, rsp.dump());
    };


    /*
    auto sessionArrayMove = [this](CacheMap& map, KvCommand& cmd)
    {
      fcjson rsp;
      rsp["KV_ARRAY_MOVE_RSP"]["tkn"] = cmd.shtk;

      for (auto& item : cmd.contents.items())
      {
        if (!item.value().is_array())
          rsp["KV_ARRAY_MOVE_RSP"][item.key()] = RequestStatus::ValueTypeInvalid;
        else
        {
          const auto status = map.arrayMove(fcjson {{item.key(), std::move(item.value())}});
          rsp["KV_ARRAY_MOVE_RSP"][item.key()] = status;
        }
      }

      send(cmd, rsp.dump());
    };
    */


    auto find = [this](CacheMap& map, KvCommand& cmd)
    {
      fcjson rsp;
      rsp["KV_FIND_RSP"]["tkn"] = cmd.shtk;
      rsp["KV_FIND_RSP"]["keys"] = fcjson::array();
      
      map.findNoRegEx(cmd.contents, cmd.find, rsp["KV_FIND_RSP"]["keys"]);

      send(cmd, rsp.dump());
    };


    auto update = [this](CacheMap& map, KvCommand& cmd)
    {
      fcjson rsp;
      rsp["KV_UPDATE_RSP"]["tkn"] = cmd.shtk;
            
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

      rsp["KV_UPDATE_RSP"]["st"] = status;

      send(cmd, rsp.dump());
    };


    // CAREFUL: these have to be in the order of KvQueryType enum
    static const std::array<std::function<void(CacheMap&, KvCommand&)>, static_cast<std::size_t>(KvQueryType::Max)> handlers = 
    {    
      sessionNew,
      sessionEnd,
      sessionOpen,
      sessionInfo,
      sessionInfoAll,
      set,
      setQ,
      get,
      add,
      addQ,
      remove,
      clear,
      count,
      append,
      contains,
      find,
      update
    };


    KvCommand cmd;
    Sessions sessions;

    while (m_run)
    {
      try
      {
        cmd = std::move(m_channel.value_pop());

        if (cmd.type == KvQueryType::SessionNew)
        {
          const SessionDuration duration {cmd.contents.at("expiry").at("duration").get<SessionDuration::rep>()};
          const bool deleteOnExpire = cmd.contents.at("expiry").at("deleteSession");

          auto cache = sessions.start(cmd.shtk, cmd.contents.at("shared") == true, duration, deleteOnExpire);
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
          cmd.syncResponseHandler(std::any{haveSession});
        }
        else if (cmd.type == KvQueryType::SessionInfo)
        {
          if (auto sesh = sessions.get(cmd.shtk); sesh)
          {
            const auto shared = sesh->get().shared;
            const auto expires = sesh->get().expires;
            const auto deleteSesion = sesh->get().expireInfo.deleteOnExpire;
            const auto expireTime = std::chrono::time_point_cast<SessionExpireTimeUnit>(sesh->get().expireInfo.time).time_since_epoch();
            const auto duration = sesh->get().expireInfo.duration;
            const auto keyCount = sesh->get().map.count();
            
            send(cmd, PoolRequestResponse::sessionInfo(RequestStatus::Ok, cmd.shtk, shared, expires, deleteSesion, duration, expireTime, keyCount).dump());
          }
          else
            send(cmd, PoolRequestResponse::sessionInfo(RequestStatus::SessionNotExist, cmd.shtk).dump());
        }
        else if (cmd.type == KvQueryType::SessionInfoAll)
        {
          std::tuple<const std::size_t, const std::size_t> rsp {std::make_tuple(sessions.countSessions(), sessions.countKeys())};
          cmd.syncResponseHandler(std::any{std::move(rsp)});
        }
        else if (cmd.type == KvQueryType::InternalSessionMonitor)
        {
          sessions.removeExpired();
        }
        else  [[likely]]
        {
          if (auto map = sessions.getMap(cmd.shtk); map)
            handlers[static_cast<const std::size_t>(cmd.type)](map->get(), cmd);
          else
            send(cmd, createErrorResponse(QueryTypeToName.at(cmd.type) + "_RSP", RequestStatus::SessionNotExist, cmd.shtk).dump());
        }
      
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
