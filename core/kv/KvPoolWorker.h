#ifndef _FC_KVPOOLWORKER_
#define _FC_KVPOOLWORKER_

#include <string_view>
#include <thread>
#include <condition_variable>
#include <sstream>
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

namespace fs = std::filesystem;
namespace chrono = std::chrono;


// A number of KvPoolWorker are created (MaxPools), which is hardware_concurrency() minus the number of IO threads.
// A pool worker runs in a thread which is assigned to a core. Each pool worker has a dedicated map which stores the key/values. 
class KvPoolWorker
{ 
public:

  KvPoolWorker(const std::size_t core, const PoolId id) noexcept :
    m_poolId(id),
    m_run(true),
    m_channel(16384U),  // must be a power of 2
    m_thread(&KvPoolWorker::run, this) 
  {
    if (!setThreadAffinity(m_thread.native_handle(), core))
      PLOGE << "Failed to assign KvPoolWorker thread: " << core;
    
    //setThreadRealtime(m_thread.native_handle(), 25);
  }

  ~KvPoolWorker()
  {
    m_run = false;
    m_channel.close();
  }

  KvPoolWorker& operator=(KvPoolWorker&&) noexcept = default;


  ndb_always_inline void execute(KvCommand&& cmd)
  {
    m_channel.push(std::move(cmd));
  }


private:
    
  ndb_always_inline void send (KvCommand& cmd, std::string&& msg)
  {
    cmd.loop->defer([cmd, msg = std::move(msg)] () mutable
    {
      if (cmd.ws->getUserData()->connected)
        cmd.ws->send(msg, kv::WsSendOpCode);
    });
  }
  
  
  njson doSet (CacheMap& map, njson& keys)
  {
    njson rsp;

    for(auto& kv : keys.object_range())
    {
      auto [it, inserted] = map.set(kv.key(), std::move(kv.value()));
      rsp[kv.key()] = toUnderlying(inserted ? RequestStatus::KeySet : RequestStatus::KeyUpdated);
    }

    return rsp;
  }


  void run ()
  {
    auto placeholder = [this](CacheMap& map, KvCommand& cmd)
    {
    };


    auto set = [this](CacheMap& map, KvCommand& cmd)
    {
      njson rsp;
      rsp["KV_SET_RSP"]["tkn"] = cmd.tkn;
      rsp["KV_SET_RSP"]["keys"] = doSet(map, cmd.contents);
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
        rsp["KV_SETQ_RSP"]["tkn"] = cmd.tkn;
        send(cmd, rsp.to_string());
      }      
    };

    
    auto get = [this](CacheMap& map, KvCommand& cmd)
    {
      njson rsp;
      rsp["KV_GET_RSP"]["tkn"] = cmd.tkn;

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
      rsp["KV_ADD_RSP"]["tkn"] = cmd.tkn;

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
          rsp["KV_ADDQ_RSP"]["tkn"] = cmd.tkn;
          send(cmd, rsp.to_string());
        }
      }
    };

    
    auto remove = [this](CacheMap& map, KvCommand& cmd)
    {
      njson rsp;
      rsp["KV_RMV_RSP"]["tkn"] = cmd.tkn;

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
      send(cmd, PoolRequestResponse::sessionClear(cmd.tkn, valid, size).to_string());
    };


    auto clearSet = [this](CacheMap& map, KvCommand& cmd)
    {
      njson rsp;
      rsp["KV_CLEAR_SET_RSP"]["tkn"] = cmd.tkn;

      if (const auto[valid, size] = map.clear(); valid)
      {
        rsp["KV_CLEAR_SET_RSP"]["st"] = toUnderlying(RequestStatus::Ok);
        rsp["KV_CLEAR_SET_RSP"]["cnt"] = size;
        rsp["KV_CLEAR_SET_RSP"]["keys"] = doSet(map, cmd.contents);        
      }
      else
      {
        rsp["KV_CLEAR_SET_RSP"]["st"] = toUnderlying(RequestStatus::Unknown);
        rsp["KV_CLEAR_SET_RSP"]["cnt"] = 0U;
        rsp["KV_CLEAR_SET_RSP"]["keys"] = njson::object();
      }

      send(cmd, rsp.to_string());
    };


    auto count = [this](CacheMap& map, KvCommand& cmd)
    {
      send(cmd, PoolRequestResponse::sessionCount(cmd.tkn, map.count()).to_string());
    };

    
    auto contains = [this](CacheMap& map, KvCommand& cmd)
    {
      njson rsp;
      rsp["KV_CONTAINS_RSP"]["st"] = toUnderlying(RequestStatus::Ok);
      rsp["KV_CONTAINS_RSP"]["tkn"] = cmd.tkn;

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
      rsp["KV_FIND_RSP"]["tkn"] = cmd.tkn;

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
      rsp["KV_UPDATE_RSP"]["tkn"] = cmd.tkn;

      const auto [keyExists, count] = map.update(key, path, std::move(cmd.contents.at("value")));

      rsp["KV_UPDATE_RSP"]["st"] = keyExists ? toUnderlying(RequestStatus::Ok) : toUnderlying(RequestStatus::KeyNotExist);
      rsp["KV_UPDATE_RSP"]["cnt"] = count;

      send(cmd, rsp.to_string());
    };


    auto keys = [this](CacheMap& map, KvCommand& cmd)
    {
      send(cmd, PoolRequestResponse::sessionKeys(cmd.tkn, map.keys()).to_string());
    };

    

    // CAREFUL: these have to be in the order of KvQueryType enum
    static const std::array<std::function<void(CacheMap&, KvCommand&)>, static_cast<std::size_t>(KvQueryType::MAX)> handlers = 
    {    
      placeholder,  // ShNew
      placeholder,  // ShEnd
      placeholder,  // ShOpen
      placeholder,  // ShInfo
      placeholder,  // ShInfoAll
      placeholder,  // ShSave
      placeholder,  // ShLoad (set as InternalLoad)
      placeholder,  // ShEndAll
      placeholder,  // ShExists
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
      clearSet
    };


    KvCommand cmd;
    Sessions sessions;

    while (m_run)
    {
      try
      {
        cmd = std::move(m_channel.value_pop());

        // TODO can this be tidied by converting query type to int and using relative positions?

        if (cmd.type == KvQueryType::ShNew)
        {
          const SessionDuration duration {cmd.contents.at("expiry").at("duration").as<SessionDuration::rep>()};
          const bool deleteOnExpire = cmd.contents.at("expiry").at("deleteSession").as_bool();

          if (const auto cache = sessions.start(cmd.tkn, cmd.contents.at("shared") == true, duration, deleteOnExpire); cache)
            send(cmd, PoolRequestResponse::sessionNew(RequestStatus::Ok, cmd.tkn, cmd.contents.at("name").as_string()).to_string()); 
          else
            send(cmd, PoolRequestResponse::sessionNew(RequestStatus::SessionNewFail, cmd.tkn, cmd.contents.at("name").as_string()).to_string()); 
        }
        else if (cmd.type == KvQueryType::ShEnd)
        {
          auto status = sessions.end(cmd.tkn) ? RequestStatus::Ok : RequestStatus::SessionNotExist;
          send(cmd, PoolRequestResponse::sessionEnd(status, cmd.tkn).to_string());
        }
        else if (cmd.type == KvQueryType::ShOpen)
        {
          const auto existsSharedTuple = sessions.openShared(cmd.tkn);
          cmd.syncResponseHandler(std::any{existsSharedTuple});
        }
        else if (cmd.type == KvQueryType::ShInfo)
        {
          if (auto session = sessions.get(cmd.tkn); session)
          {
            const auto& sesh = session->get();
            const auto& expiryInfo = sesh.expireInfo;

            const auto keyCount = sesh.map.count();
            const auto remaining = sesh.expires ?   std::chrono::duration_cast<std::chrono::seconds>(expiryInfo.time - SessionClock::now()) :
                                                    std::chrono::seconds{0};

            send(cmd, PoolRequestResponse::sessionInfo( RequestStatus::Ok, cmd.tkn, sesh.shared, sesh.expires, 
                                                        expiryInfo.deleteOnExpire, expiryInfo.duration, remaining, keyCount).to_string());
          }
          else
            send(cmd, PoolRequestResponse::sessionInfo(RequestStatus::SessionNotExist, cmd.tkn).to_string());
        }
        else if (cmd.type == KvQueryType::ShInfoAll)
        {
          std::tuple<const std::size_t, const std::size_t> rsp {std::make_tuple(sessions.countSessions(), sessions.countKeys())};
          cmd.syncResponseHandler(std::any{std::move(rsp)});
        }
        else if (cmd.type == KvQueryType::ShSave)
        {
          save(cmd, sessions);
        }
        else if (cmd.type == KvQueryType::ShEndAll)
        {
          const auto count = sessions.clear();
          cmd.syncResponseHandler(std::any{std::move(count)});
        }
        else if (cmd.type == KvQueryType::ShExists)
        {
          auto result = std::make_tuple(cmd.tkn, sessions.contains(cmd.tkn));
          cmd.syncResponseHandler(result);
        }
        else if (cmd.type == KvQueryType::InternalSessionMonitor)
        {
          sessions.handleExpired();
        }
        else if (cmd.type == KvQueryType::InternalLoad || cmd.type == KvQueryType::ShLoad)
        {
          load(cmd, sessions);
        }
        else  [[likely]]
        {
          if (auto sesh = sessions.get(cmd.tkn); sesh)
          {
            handlers[static_cast<const std::size_t>(cmd.type)](sesh->get().map, cmd);
            
            if (sesh->get().expires)
              sessions.updateExpiry(sesh->get());
          }
          else
            send(cmd, createErrorResponse(QueryTypeToName.at(cmd.type) + "_RSP", RequestStatus::SessionNotExist, cmd.tkn).to_string());
        }
      }
      catch (const boost::fibers::fiber_error& fex)
      {
        if (!m_channel.is_closed())
        {
          PLOGE << "Pool Fiber Exception: " << fex.what();
          
          // cmd::ws can be null if the exception is thrown during internal monitor check
          if (cmd.ws)
            send(cmd, createErrorResponse(RequestStatus::Unknown).to_string());
        } 
      }
      catch (const std::exception& ex)
      {
        PLOGE << "Pool Exception: " << ex.what();

        // cmd::ws can be null if the exception is thrown during internal monitor check
        if (cmd.ws)
          send(cmd, createErrorResponse(RequestStatus::Unknown).to_string());
      }
    }
  }



  void save(KvCommand& cmd, const Sessions& sessions)
  {    
    const std::size_t MaxDataFileSize = 10 * 1024 * 1024;
    const auto root = cmd.contents["poolDataRoot"].as_string_view();
    const auto path = fs::path{root} / std::to_string(m_poolId);

    RequestStatus status = RequestStatus::SaveComplete;

    try
    {
      if (!fs::create_directories(path))
        status = RequestStatus::SaveError;
      else
      {
        const bool haveTkns = cmd.contents.contains("tkns");
        const auto& sessionsMap = sessions.getSessions();

        if (haveTkns)
        {
          saveSelection(cmd, sessionsMap, path, MaxDataFileSize);
        }
        else
        {
          saveAll(sessionsMap, path, MaxDataFileSize);
        }
      }
    }
    catch (const std::exception& ex)
    {
      std::cout << ex.what() << '\n';
      status = RequestStatus::SaveError;
    }

    cmd.syncResponseHandler(std::any{status});
  }


  void saveSelection (KvCommand& cmd, const Sessions::SessionsMap& allSessions, const fs::path& path, const std::size_t MaxDataFileSize)
  {
    std::string buffer;
    buffer.reserve(MaxDataFileSize);

    std::stringstream sstream{buffer};
    std::size_t nFiles = 0;

    const auto& tkns = cmd.contents.at("tkns");
    bool first = true;

    for(const auto& item : tkns.array_range())
    {
      const auto token = item.as<SessionToken>();

      if (auto it = allSessions.find(token); it != allSessions.cend())
      {
        writeSesh(first, it->second, token, sstream);
        first = false;

        if (sstream.tellp() >= MaxDataFileSize)
        {
          flushSaveBuffer(path / std::to_string(nFiles), sstream);
          ++nFiles;
          first = true;
        }
      }
    }

    // leftovers (didn't reach the file size)
    if (sstream.tellp() > 0)
    {
      flushSaveBuffer(path / std::to_string(nFiles), sstream);
    }
  }


  void saveAll (const Sessions::SessionsMap& allSessions, const fs::path& path, const std::size_t MaxDataFileSize)
  {    
    std::string buffer;
    buffer.reserve(MaxDataFileSize);

    std::stringstream sstream{buffer};
    std::size_t nFiles = 0;

    bool first = true;
    for(const auto& [token, sesh] : allSessions)
    {
      writeSesh(first, sesh, token, sstream);
      first = false;

      if (sstream.tellp() >= MaxDataFileSize)
      {
        flushSaveBuffer(path / std::to_string(nFiles), sstream);
        ++nFiles;
        first = true;
      }
    }

    // leftovers (didn't reach the file size)
    if (sstream.tellp() > 0)
    {
      flushSaveBuffer(path / std::to_string(nFiles), sstream);
    }
  }


  void flushSaveBuffer (const fs::path& path, std::stringstream& sstream)
  {
    std::ofstream dataStream {path};
    dataStream << '[' << sstream.str() << ']';  // no difference with sstream.rdbuf()

    sstream.clear();
    sstream.str(std::string{});
  }


  void writeSesh (const bool first, const Sessions::SessionType& sesh, const SessionToken token, std::stringstream& buffer)
  {
    njson seshData;
    seshData["sh"]["tkn"] = token;
    seshData["sh"]["shared"] = sesh.shared;
    seshData["sh"]["expiry"]["duration"] = chrono::duration_cast<SessionDuration>(sesh.expireInfo.duration).count();
    seshData["sh"]["expiry"]["deleteSession"] = sesh.expireInfo.deleteOnExpire;
    
    seshData["keys"] = njson::object();

    for(const auto& [k, v] : sesh.map.map())
      seshData["keys"][k] = v;

    if (!first)
      buffer << ',';

    seshData.dump(buffer);
  }


  void load(KvCommand& cmd, Sessions& sessions)
  {
    LoadResult result { .status = RequestStatus::LoadComplete };

    auto updateStatus = [&result](const RequestStatus st)
    {
      if (LoadResult::statusSuccess(result))
        result.status = st; // only overwrite status if we're not in an error condition
    };


    try
    {      
      if (cmd.contents.contains("dirs"))
      {
        // loading everything from multiple pool data dirs
        for (auto& dataDir : cmd.contents.at("dirs").array_range())
        {
          fs::path dir {dataDir.as_string_view()};
          for (const auto seshFile : fs::directory_iterator{dir})
          {
            auto status = readSeshFile(sessions, seshFile.path(), result.nSessions, result.nKeys);
            updateStatus(status);
          }
        }
      }
      else
      {
        // load an individual session
        auto status = loadSession(sessions, std::move(cmd.contents.at("sesh")), result.nSessions, result.nKeys);
        updateStatus(status);
      }      
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
      result.status = RequestStatus::LoadError;
    }

    cmd.syncResponseHandler(std::any{std::move(result)});
  }


public:

  ndb_always_inline RequestStatus readSeshFile (Sessions& sessions, const fs::path path, std::size_t& nSessions, std::size_t& nKeys)
  {
    RequestStatus status = RequestStatus::LoadComplete;

    try
    {
      std::ifstream seshStream{path};
      auto root = njson::parse(seshStream);

      for (auto& item : root.array_range())
      {
        if (auto st = loadSession(sessions, std::move(item), nSessions, nKeys); status != RequestStatus::LoadError)
          status = st;
      }
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
      status = RequestStatus::LoadError;
    }
    return status;
  };


  ndb_always_inline RequestStatus loadSession (Sessions& sessions, njson&& seshData, std::size_t& nSessions, std::size_t& nKeys)
  {
    RequestStatus status = RequestStatus::LoadComplete;

    if (seshData.contains("sh"))  // can be empty object because sesh file is created even if pool contains no sessions
    {
      const auto token = seshData["sh"]["tkn"].as<SessionToken>();
      const auto isShared = seshData["sh"]["shared"].as_bool();
      const auto deleteOnExpire = seshData["sh"]["expiry"]["deleteSession"].as_bool();
      const auto duration = SessionDuration{seshData["sh"]["expiry"]["duration"].as<std::size_t>()};

      if (auto session = sessions.start(token, isShared, duration, deleteOnExpire); session)
      {
        if (seshData.contains("keys"))
        {
          for (auto& items : seshData.at("keys").object_range())
          {
            session->get().set(items.key(), std::move(items.value()));
            ++nKeys;
          }
        }
        ++nSessions;
      }
      else 
      {
        status = RequestStatus::LoadDuplicate;
      }
    }

    return status;
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
