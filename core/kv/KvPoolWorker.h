#ifndef _FC_KVPOOLWORKER_
#define _FC_KVPOOLWORKER_

#include <string_view>
#include <thread>
#include <condition_variable>
#include <regex>
#include <boost/fiber/fiber.hpp>
#include <boost/fiber/buffered_channel.hpp>
#include <ankerl/unordered_dense.h>
#include <core/NemesisCommon.h>
#include <core/CacheMap.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvSessions.h>



namespace nemesis { namespace core { namespace kv {

using namespace std::string_view_literals;


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
      // handled below
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
      njson2 rsp;
      rsp["KV_SET_RSP"]["tkn"] = cmd.shtk;

      for(auto& kv : cmd.contents.object_range())
      {
        auto [it, inserted] = map.set(kv.key(), std::move(kv.value()));
        rsp["KV_SET_RSP"]["keys"][kv.key()] = toUnderlying(inserted ? RequestStatus::KeySet : RequestStatus::KeyUpdated);
      }

      send(cmd, rsp.to_string());
    };

    
    auto setQ = [this](CacheMap& map, KvCommand& cmd)
    {
      njson2 rsp ;
      
      for(auto& kv : cmd.contents.object_range())
      {
        try
        {
          map.set(kv.key(), std::move(kv.value()));
        }
        catch(const std::exception& e)
        {
          rsp["KV_SETQ_RSP"]["keys"][kv.key()] = toUnderlying(RequestStatus::Unknown);
        }
      }

      if (!rsp.empty())
      {
        rsp["KV_SETQ_RSP"]["tkn"] = cmd.shtk;
        send(cmd, rsp.to_string());
      }      
    };

    
    auto get = [this](CacheMap& map, KvCommand& cmd)
    {
      njson2 rsp;
      rsp["KV_GET_RSP"]["tkn"] = cmd.shtk;

      for(auto& item : cmd.contents.array_range())
      {
        if (item.is_string()) [[likely]]
        {
          const auto& key = item.as_string();
          if (auto [exists, value] = map.get(key); exists)
            rsp["KV_GET_RSP"]["keys"][key] = std::move(value);
          else
            rsp["KV_GET_RSP"]["keys"][key] = njson2::null();
        }        
      }

      send(cmd, rsp.to_string());
    };

    
    auto add = [this](CacheMap& map, KvCommand& cmd)
    {
      njson2 rsp;
      rsp["KV_ADD_RSP"]["tkn"] = cmd.shtk;

      if (cmd.contents.empty())
      { 
        rsp["KV_ADD_RSP"]["keys"] = njson2::object();
        send(cmd, rsp.to_string());
      }
      else
      {
        for(auto& kv : cmd.contents.object_range())
        {
          const auto inserted = map.add(kv.key(), std::move(kv.value()));
          rsp["KV_ADD_RSP"]["keys"][kv.key()] = toUnderlying(inserted ? RequestStatus::KeySet : RequestStatus::KeyExists);
        }

        send(cmd, rsp.to_string());
      }
    };


    auto addQ = [this](CacheMap& map, KvCommand& cmd)
    {
      if (!cmd.contents.empty())
      {
        njson2 rsp;

        for(auto& kv : cmd.contents.object_range())
        {
          if (const auto inserted = map.add(kv.key(), std::move(kv.value())); !inserted)
            rsp["KV_ADDQ_RSP"]["keys"][kv.key()] = toUnderlying(RequestStatus::KeyExists);
        }

        if (!rsp.empty())
        {
          rsp["KV_ADDQ_RSP"]["tkn"] = cmd.shtk;
          send(cmd, rsp.to_string());
        }
      }
    };

    
    auto remove = [this](CacheMap& map, KvCommand& cmd)
    {
      njson2 rsp;
      rsp["KV_RMV_RSP"]["tkn"] = cmd.shtk;

      for(auto& value : cmd.contents.array_range())
      {
        if (value.is_string())
        {
          const auto& key = value.as_string();
          const auto removed = map.remove(key);
          rsp["KV_RMV_RSP"][key] = toUnderlying(removed ? RequestStatus::KeyRemoved : RequestStatus::KeyNotExist);
        }
      }

      send(cmd, rsp.to_string());
    };


    auto clear = [this](CacheMap& map, KvCommand& cmd)
    {
      const auto[valid, size] = map.clear();
      send(cmd, PoolRequestResponse::sessionClear(cmd.shtk, valid, size).to_string());
    };


    auto count = [this](CacheMap& map, KvCommand& cmd)
    {
      send(cmd, PoolRequestResponse::sessionCount(cmd.shtk, map.count()).to_string());
    };

    
    auto contains = [this](CacheMap& map, KvCommand& cmd)
    {
      njson2 rsp;
      rsp["KV_CONTAINS_RSP"]["st"] = toUnderlying(RequestStatus::Ok);
      rsp["KV_CONTAINS_RSP"]["tkn"] = cmd.shtk;

      for (auto& item : cmd.contents.array_range())
      {
        if (item.is_string())
        {
          const cachedkey& key = item.as_string();
          rsp["KV_CONTAINS_RSP"]["keys"][key] = map.contains(key);
        }
      }

      send(cmd, rsp.to_string());
    };

    
    auto find = [this](CacheMap& map, KvCommand& cmd)
    {
      const bool paths = cmd.contents.at("rsp") == "paths";

      njson2 result = njson::array();

      map.find(cmd.contents, paths, result);

      njson2 rsp;
      rsp["KV_FIND_RSP"]["tkn"] = cmd.shtk;

      if (cmd.contents.at("rsp") != "kv")
        rsp["KV_FIND_RSP"][paths ? "paths" : "keys"] = std::move(result);
      else
      {
        // return key-values as if doing a KV_GET
        rsp["KV_FIND_RSP"]["keys"] = njson2::object();

        for(auto& item : result.array_range())
        {
          const auto& key = item.as_string();

          if (auto [exists, value] = map.get(key); exists)
            rsp["KV_FIND_RSP"]["keys"][key] = std::move(value);
          else
            rsp["KV_FIND_RSP"]["keys"][key] = njson2::null();
        } 
      }

      send(cmd, rsp.to_string());
    };
    

    /*
    auto update = [this](CacheMap& map, KvCommand& cmd)
    {
      njson rsp;
      rsp["KV_UPDATE_RSP"]["tkn"] = cmd.shtk;
            
      njson::iterator  itKey = cmd.contents.begin(),
                        itPath = std::next(itKey, 1);

      if (itKey.key() != "key")
        std::swap(itKey, itPath);

      bool pathValid = true;
      njson::json_pointer path;

      try
      {
        path = std::move(njson::json_pointer{itPath.key()});
      }
      catch (...)
      {
        pathValid = false;
      }

      RequestStatus status;

      if (pathValid)
        rsp["KV_UPDATE_RSP"]["keys"][itKey.value()] = map.updateByPath(itKey.value(), path, std::move(itPath.value()));
      else
        rsp["KV_UPDATE_RSP"]["keys"][itKey.value()] = RequestStatus::PathInvalid;

      send(cmd, rsp.to_string());
    };
    */

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
      contains,
      find,
      /*update*/
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
          const SessionDuration duration {cmd.contents.at("expiry").at("duration").as<SessionDuration::rep>()};
          const bool deleteOnExpire = cmd.contents.at("expiry").at("deleteSession").as_bool();

          if (const auto cache = sessions.start(cmd.shtk, cmd.contents.at("shared") == true, duration, deleteOnExpire); cache)
            send(cmd, PoolRequestResponse::sessionNew(RequestStatus::Ok, cmd.shtk, cmd.contents.at("name").as_string()).to_string()); 
          else
            send(cmd, PoolRequestResponse::sessionNew(RequestStatus::SessionNewFail, cmd.shtk, cmd.contents.at("name").as_string()).to_string()); 
        }
        else if (cmd.type == KvQueryType::SessionEnd)
        {
          auto status = sessions.end(cmd.shtk) ? RequestStatus::Ok : RequestStatus::SessionNotExist;
          send(cmd, PoolRequestResponse::sessionEnd(status, cmd.shtk).to_string());
        }
        else if (cmd.type == KvQueryType::SessionOpen)
        {
          const auto existsSharedTuple = sessions.openShared(cmd.shtk);
          cmd.syncResponseHandler(std::any{existsSharedTuple});
        }
        else if (cmd.type == KvQueryType::SessionInfo)
        {
          if (auto session = sessions.get(cmd.shtk); session)
          {
            const auto& sesh = session->get();
            const auto& expiryInfo = sesh.expireInfo;

            const auto keyCount = sesh.map.count();
            const auto remaining = sesh.expires ?   std::chrono::duration_cast<std::chrono::seconds>(expiryInfo.time - SessionClock::now()) :
                                                    std::chrono::seconds{0};

            send(cmd, PoolRequestResponse::sessionInfo( RequestStatus::Ok, cmd.shtk, sesh.shared, sesh.expires, 
                                                        expiryInfo.deleteOnExpire, expiryInfo.duration, remaining, keyCount).to_string());
          }
          else
            send(cmd, PoolRequestResponse::sessionInfo(RequestStatus::SessionNotExist, cmd.shtk).to_string());
        }
        else if (cmd.type == KvQueryType::SessionInfoAll)
        {
          std::tuple<const std::size_t, const std::size_t> rsp {std::make_tuple(sessions.countSessions(), sessions.countKeys())};
          cmd.syncResponseHandler(std::any{std::move(rsp)});
        }
        else if (cmd.type == KvQueryType::InternalSessionMonitor)
        {
          sessions.handleExpired();
        }
        else  [[likely]]
        {
          if (auto map = sessions.getMap(cmd.shtk); map)
            handlers[static_cast<const std::size_t>(cmd.type)](map->get(), cmd);
          else
            send(cmd, createErrorResponse(QueryTypeToName.at(cmd.type) + "_RSP", RequestStatus::SessionNotExist, cmd.shtk).to_string());
        }
      
        // TODO command = KvCommand{};
      }
      catch (const boost::fibers::fiber_error& fex)
      {
        if (!m_channel.is_closed())
        {
          std::cout << "Pool Fiber Exception: " << fex.what() << '\n';
          
          // cmd::ws can be null if the exception is thrown during internal monitor check
          if (cmd.ws)
            send(cmd, createErrorResponse(RequestStatus::Unknown).to_string());
        } 
      }
      catch (const std::exception& ex)
      {
        std::cout << "Pool Exception: " << ex.what() << '\n';

        // cmd::ws can be null if the exception is thrown during internal monitor check
        if (cmd.ws)
          send(cmd, createErrorResponse(RequestStatus::Unknown).to_string());
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
