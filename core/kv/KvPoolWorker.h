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
        rsp["KV_SET_RSP"]["keys"][kv.key()] = static_cast<int>(inserted ? RequestStatus::KeySet : RequestStatus::KeyUpdated);
      }

      send(cmd, rsp.to_string());
    };

    /*
    auto setQ = [this](CacheMap& map, KvCommand& cmd)
    {
      njson rsp;
      
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
        send(cmd, rsp.to_string());
      }      
    };


    auto get = [this](CacheMap& map, KvCommand& cmd)
    {
      njson rsp;
      rsp["KV_GET_RSP"]["tkn"] = cmd.shtk;

      for(auto& item : cmd.contents.items())
      {
        if (item.value().is_string()) [[likely]]
        {
          if (auto [exists, pair] = map.get(item.value().get_ref<const cachedkey&>()); exists)
            rsp["KV_GET_RSP"]["keys"].emplace(std::move(pair.begin().key()), std::move(pair.begin().value()));
          else
            rsp["KV_GET_RSP"]["keys"][item.value()] = njson{}; //null
        }        
      }

      send(cmd, rsp.to_string());
    };


    auto add = [this](CacheMap& map, KvCommand& cmd)
    {
      njson rsp;
      rsp["KV_ADD_RSP"]["tkn"] = cmd.shtk;

      if (cmd.contents.empty())
        rsp["KV_ADD_RSP"]["keys"] = njson::object();

      for(auto& kv : cmd.contents.items())
      {
        const auto inserted = map.add(kv.key(), std::move(kv.value()));
        rsp["KV_ADD_RSP"]["keys"][kv.key()] = inserted ? RequestStatus::KeySet : RequestStatus::KeyExists;
      }

      send(cmd, rsp.to_string());
    };


    auto addQ = [this](CacheMap& map, KvCommand& cmd)
    {
      njson rsp;

      if (cmd.contents.empty())
      {
        rsp["KV_ADDQ_RSP"]["keys"] = njson::object();
        send(cmd, rsp.to_string());
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
          send(cmd, rsp.to_string());
        }
      }
    };


    auto remove = [this](CacheMap& map, KvCommand& cmd)
    {
      njson rsp;
      rsp["KV_RMV_RSP"]["tkn"] = cmd.shtk;

      for(auto& key : cmd.contents.items())
      {
        if (key.value().is_string())
        {
          const auto removed = map.remove(key.value().get_ref<const cachedkey&>());
          rsp["KV_RMV_RSP"][key.value()] = removed ? RequestStatus::KeyRemoved : RequestStatus::KeyNotExist;
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


    auto append = [this](CacheMap& map, KvCommand& cmd)
    {
      njson rsp;
      rsp["KV_APPEND_RSP"]["tkn"] = std::move(cmd.shtk);

      for(auto& kv : cmd.contents.items())
        rsp["KV_APPEND_RSP"]["keys"][kv.key()] = map.append(kv.key(), std::move(kv.value()));

      send(cmd, rsp.to_string());
    };


    auto contains = [this](CacheMap& map, KvCommand& cmd)
    {
      njson rsp;
      rsp["KV_CONTAINS_RSP"]["st"] = RequestStatus::Ok;
      rsp["KV_CONTAINS_RSP"]["tkn"] = cmd.shtk;

      for (auto& item : cmd.contents.items())
        rsp["KV_CONTAINS_RSP"]["keys"][item.value()] = map.contains(item.value());

      send(cmd, rsp.to_string());
    };


    auto find = [this](CacheMap& map, KvCommand& cmd)
    {
      const bool paths = cmd.contents.at("rsp") == "paths";

      njson result = njson::array();

      map.find(cmd.contents, paths, result);

      njson rsp;
      rsp["KV_FIND_RSP"]["tkn"] = cmd.shtk;

      if (cmd.contents.at("rsp") != "kv")
        rsp["KV_FIND_RSP"][paths ? "paths" : "keys"] = std::move(result);
      else
      {
        rsp["KV_FIND_RSP"]["keys"] = njson{};

        for(auto& item : result.items())
        {
          if (auto [exists, pair] = map.get(item.value().get_ref<const cachedkey&>()); exists)
            rsp["KV_FIND_RSP"]["keys"].emplace(std::move(pair.begin().key()), std::move(pair.begin().value()));
          else
            rsp["KV_FIND_RSP"]["keys"][item.value()] = njson{}; //null
        } 
      }

      send(cmd, rsp.to_string());
    };
    


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
      /*setQ,
      get,
      add,
      addQ,
      remove,
      clear,
      count,
      append,
      contains,
      find,
      update*/
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
