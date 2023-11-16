#ifndef _FC_KVPOOLWORKER_
#define _FC_KVPOOLWORKER_

#include <string_view>
#include <thread>
#include <condition_variable>
#include <regex>
#include <boost/fiber/fiber.hpp>
#include <boost/fiber/buffered_channel.hpp>
#include <ankerl/unordered_dense.h>
#include <jsoncons/json_encoder.hpp>
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
    auto placeholder = [this](CacheMap& map, KvCommand& cmd)
    {
    };


    auto set = [this](CacheMap& map, KvCommand& cmd)
    {
      njson rsp;
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
      njson rsp ;
      
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
      njson rsp;
      rsp["KV_GET_RSP"]["tkn"] = cmd.shtk;

      for(auto& item : cmd.contents.array_range())
      {
        if (item.is_string()) [[likely]]
        {
          const auto& key = item.as_string();
          if (auto [exists, value] = map.get(key); exists)
            rsp["KV_GET_RSP"]["keys"][key] = std::move(value);
          else
            rsp["KV_GET_RSP"]["keys"][key] = njson::null();
        }        
      }

      send(cmd, rsp.to_string());
    };

    
    auto add = [this](CacheMap& map, KvCommand& cmd)
    {
      njson rsp;
      rsp["KV_ADD_RSP"]["tkn"] = cmd.shtk;

      if (cmd.contents.empty())
      { 
        rsp["KV_ADD_RSP"]["keys"] = njson::object();
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
        njson rsp;

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
      njson rsp;
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
      njson rsp;
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

      njson rsp;
      rsp["KV_FIND_RSP"]["tkn"] = cmd.shtk;

      njson result = njson::array();

      if (!map.find(cmd.contents, paths, result))
        rsp["KV_FIND_RSP"]["st"] = toUnderlying(RequestStatus::PathInvalid);
      else
      {       
        rsp["KV_FIND_RSP"]["st"] = toUnderlying(RequestStatus::Ok);

        if (cmd.contents.at("rsp") != "kv")
          rsp["KV_FIND_RSP"][paths ? "paths" : "keys"] = std::move(result);
        else
        {
          // return key-values the same as KV_GET
          rsp["KV_FIND_RSP"]["keys"] = njson::object();

          for(auto& item : result.array_range())
          {
            const auto& key = item.as_string();

            if (auto [exists, value] = map.get(key); exists)
              rsp["KV_FIND_RSP"]["keys"][key] = std::move(value);
            else
              rsp["KV_FIND_RSP"]["keys"][key] = njson::null();
          } 
        }
      }

      send(cmd, rsp.to_string());
    };
    
    
    auto update = [this](CacheMap& map, KvCommand& cmd)
    {
      const auto& key = cmd.contents.at("key").as_string();
      const auto& path = cmd.contents.at("path").as_string();

      njson rsp;
      rsp["KV_UPDATE_RSP"]["tkn"] = cmd.shtk;

      const auto [keyExists, count] = map.update(key, path, std::move(cmd.contents.at("value")));

      rsp["KV_UPDATE_RSP"]["st"] = keyExists ? toUnderlying(RequestStatus::Ok) : toUnderlying(RequestStatus::KeyNotExist);
      rsp["KV_UPDATE_RSP"]["cnt"] = count;

      send(cmd, rsp.to_string());
    };


    auto keys = [this](CacheMap& map, KvCommand& cmd)
    {
      send(cmd, PoolRequestResponse::sessionKeys(cmd.shtk, map.keys()).to_string());
    };

    

    // CAREFUL: these have to be in the order of KvQueryType enum
    static const std::array<std::function<void(CacheMap&, KvCommand&)>, static_cast<std::size_t>(KvQueryType::MAX)> handlers = 
    {    
      placeholder,  // SessionNew
      placeholder,  // SessionEnd
      placeholder,  // SessionOpen
      placeholder,  // SessionInfo
      placeholder,  // SessionInfoAll
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
      update,
      keys,
      placeholder   // kvSave
    };


    KvCommand cmd;
    Sessions sessions;

    while (m_run)
    {
      try
      {
        cmd = std::move(m_channel.value_pop());

        // TODO can this be tidied by converting query type to int and using relative positions?

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
        else if (cmd.type == KvQueryType::KvSave)
        {
          save(cmd, sessions);
        }
        else if (cmd.type == KvQueryType::InternalSessionMonitor)
        {
          sessions.handleExpired();
        }
        else if (cmd.type == KvQueryType::InternalLoad)
        {
          load(cmd, sessions);
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



  void save(KvCommand& cmd, const Sessions& sessions)
  {
    const auto root = cmd.contents["poolDataRoot"].as_string_view();
    const auto path = std::filesystem::path{root} / std::to_string(m_poolId);

    RequestStatus status = RequestStatus::Ok;

    try
    {
      std::cout << "Pool " << m_poolId << " creating: " << path << '\n';

      if (!std::filesystem::create_directories(path))
        status = RequestStatus::SaveError;
      else
      {
        for(const auto& [token, sesh] : sessions.getSessions())
        {
          std::cout << "Pool " << m_poolId << " creating: " << (path/token) << '\n';

          std::ofstream out{path / token};
          out << "[";
          
          bool first = true;
          for(const auto& [k, v] : sesh.map.map())
          {
            out << (first ? "{" : ",{");
            out << "\"" << k << "\":";
            v.dump(out);
            out << "}";

            first = false;
          }

          out << "]";
        }
      }
    }
    catch (...)
    {
      status = RequestStatus::SaveError;
    } 

    cmd.syncResponseHandler(std::any{status});   
  }


  void load(KvCommand& cmd, Sessions& sessions)
  {
    RequestStatus status = RequestStatus::Loading;
    std::size_t nSessions{0}, nKeys{0};

    try
    {
      std::cout << cmd.contents << "\n";

      for (auto& dataDir : cmd.contents.at("dirs").array_range())
      {
        std::filesystem::path dir {dataDir.as_string_view()};
        for (const auto seshFile : std::filesystem::directory_iterator{dir})
        {
          if (seshFile.is_regular_file())
          {
            // TODO session info
            if (auto session = sessions.start(seshFile.path().filename(), false, SessionDuration{0}, false); session)
            {
              std::ifstream seshStream{seshFile.path()};
              auto seshData = njson::parse(seshStream);

              for (auto& items : seshData.array_range())
              {
                for (auto& kv : items.object_range())
                {
                  session->get().set(kv.key(), std::move(kv.value()));
                  ++nKeys;
                }
              }

              ++nSessions;
            }
          }
        }
      }

      status = RequestStatus::LoadComplete;
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
      status = RequestStatus::LoadError;
    }

    cmd.syncResponseHandler(std::any{StartupLoadResult{.status = status, .nSessions = nSessions, .nKeys = nKeys}});
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
