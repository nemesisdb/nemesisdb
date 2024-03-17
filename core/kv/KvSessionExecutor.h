#ifndef _FC_CORESESSIONEXECUTOR_
#define _FC_CORESESSIONEXECUTOR_

#include <functional>
#include <core/NemesisCommon.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvSessions.h>


namespace nemesis { namespace core { namespace kv {

using namespace nemesis::core;
using namespace nemesis::core::kv;


enum class SessionExecutorType 
{
  ShNew,
  ShEnd
};

enum class KvExecutorType
{
  KvSetQ,
  KvGet
};


class SessionExecutor
{
public:

  static njson newSession (Sessions& sessions, const SessionToken& tkn, const njson& cmd)
  {
    const SessionDuration duration {cmd.at("expiry").at("duration").as<SessionDuration::rep>()};
    const bool deleteOnExpire = cmd.at("expiry").at("deleteSession").as_bool();

    if (const auto cache = sessions.start(tkn, cmd.at("shared") == true, duration, deleteOnExpire); cache)
      return PoolRequestResponse::sessionNew(RequestStatus::Ok, tkn, cmd.at("name").as_string());
    else
      return PoolRequestResponse::sessionNew(RequestStatus::SessionNewFail, tkn, cmd.at("name").as_string());
  }


  static njson endSession (Sessions& sessions, const SessionToken& tkn)
  {
    const auto status = sessions.end(tkn) ? RequestStatus::Ok : RequestStatus::SessionNotExist;
    return PoolRequestResponse::sessionEnd(status, tkn);
  }


  static bool openSession (Sessions& sessions, const SessionToken& tkn)
  {
    const auto [exists, shared] = sessions.openShared(tkn);
    return exists && shared;
  }


  static njson sessionInfo (Sessions& sessions, const SessionToken& tkn)
  {
    if (const auto session = sessions.get(tkn); session)
    {
      const auto& sesh = session->get();
      const auto& expiryInfo = sesh.expireInfo;
      const auto keyCount = sesh.map.count();
      const auto remaining = sesh.expires ? std::chrono::duration_cast<std::chrono::seconds>(expiryInfo.time - SessionClock::now()) :
                                            std::chrono::seconds{0};

      return PoolRequestResponse::sessionInfo(RequestStatus::Ok, tkn, sesh.shared, sesh.expires,
                                              expiryInfo.deleteOnExpire, expiryInfo.duration, remaining, keyCount);
    }
    else
      return PoolRequestResponse::sessionInfo(RequestStatus::SessionNotExist, tkn);
  }

  
  static njson sessionInfoAll (const Sessions& sessions)
  {
    njson rsp;
    rsp["SH_INFO_ALL_RSP"]["st"] = toUnderlying(RequestStatus::Ok);
    rsp["SH_INFO_ALL_RSP"]["totalSessions"] = sessions.countSessions();
    rsp["SH_INFO_ALL_RSP"]["totalKeys"] = sessions.countKeys();
    return rsp;
  }


  static njson sessionExists (const Sessions& sessions, const njson& tkns)
  {
    njson rsp;
    rsp["SH_EXISTS_RSP"]["st"] = toUnderlying(RequestStatus::Ok);
    rsp["SH_EXISTS_RSP"]["exist"] = njson::make_array();

    for (const auto& item : tkns.array_range())
    {
      if (item.is<SessionToken>())
      {
        const auto& tkn = item.as<SessionToken>();
        if (sessions.contains(tkn))
          rsp["SH_EXISTS_RSP"]["exist"].push_back(tkn);
      }      
    }

    return rsp;
  }


  static njson sessionEndAll (Sessions& sessions)
  {
    njson rsp;
    rsp["SH_END_ALL_RSP"]["st"] = toUnderlying(RequestStatus::Ok);
    rsp["SH_END_ALL_RSP"]["cnt"] = sessions.clear();
    return rsp;
  }


  static void sessionMonitor(Sessions& sessions)
  {
    sessions.handleExpired();
  }


  static njson saveSessions (const Sessions& sessions, const njson& cmd)
  {
    const std::size_t MaxDataFileSize = 10 * 1024 * 1024;
    const auto root = cmd["poolDataRoot"].as_string_view();
    const auto path = fs::path{root};
    const auto start = NemesisClock::now();

    RequestStatus status = RequestStatus::SaveComplete;

    njson rsp;
    rsp["SH_SAVE_RSP"]["st"] = toUnderlying(status); // keeps 'st' as first member, but 
    rsp["SH_SAVE_RSP"]["name"] = cmd["name"].as_string();

    
    try
    {
      if (!fs::create_directories(path))
        status = RequestStatus::SaveError;
      else
      {
        const auto& sessionsMap = sessions.getSessions();

        if (cmd.contains("tkns"))
          saveSelection(cmd, sessionsMap, path, MaxDataFileSize);
        else
          saveAll(sessionsMap, path, MaxDataFileSize);
      }
    }
    catch (const std::exception& ex)
    {
      PLOGE << ex.what();
      status = RequestStatus::SaveError;
    }

    rsp["SH_SAVE_RSP"]["st"] = toUnderlying(status);
    rsp["SH_SAVE_RSP"]["duration"] = chrono::duration_cast<chrono::milliseconds>(NemesisClock::now() - start).count();

    return rsp;
  }


  static njson loadSessions (const std::string& loadName, Sessions& sessions, const fs::path& dataRoot)
  {
    RequestStatus status{RequestStatus::Loading};
    std::size_t nSessions{0}, nKeys{0};
    njson rsp;

    if (!fs::exists(dataRoot))
    {
      status = RequestStatus::LoadError;
      rsp["SH_LOAD_RSP"]["m"] = "Dataset does not exist";
    }
    else
    {
      try
      {      
        for (const auto& seshFile : fs::directory_iterator{dataRoot})
        {
          status = readSeshFile(sessions, seshFile.path(), nSessions, nKeys);
          
          if (status == RequestStatus::LoadError)
            break;
        }
      }
      catch(const std::exception& e)
      {
        PLOGE << e.what();
        status = RequestStatus::LoadError;
      }
    }
        
    rsp["SH_LOAD_RSP"]["name"] = loadName;
    rsp["SH_LOAD_RSP"]["st"] = status == RequestStatus::LoadComplete ?  toUnderlying(RequestStatus::LoadComplete) :
                                                                        toUnderlying(RequestStatus::LoadError);
    rsp["SH_LOAD_RSP"]["sessions"] = nSessions;
    rsp["SH_LOAD_RSP"]["keys"] = nKeys;

    return rsp;
  }


  private:


    static void saveSelection (const njson& cmd, const Sessions::SessionsMap& allSessions, const fs::path& path, const std::size_t MaxDataFileSize)
    {
      std::string buffer;
      buffer.reserve(MaxDataFileSize);

      std::stringstream sstream{buffer};
      std::size_t nFiles = 0;

      const auto& tkns = cmd.at("tkns");
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


    static void saveAll (const Sessions::SessionsMap& allSessions, const fs::path& path, const std::size_t MaxDataFileSize)
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


    static void flushSaveBuffer (const fs::path& path, std::stringstream& sstream)
    {
      std::ofstream dataStream {path};
      dataStream << '[' << sstream.str() << ']';  // no difference with sstream.rdbuf()

      sstream.clear();
      sstream.str(std::string{});
    }


    static void writeSesh (const bool first, const Sessions::SessionType& sesh, const SessionToken token, std::stringstream& buffer)
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

    
    static RequestStatus readSeshFile (Sessions& sessions, const fs::path path, std::size_t& nSessions, std::size_t& nKeys)
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


    static RequestStatus loadSession (Sessions& sessions, njson&& seshData, std::size_t& nSessions, std::size_t& nKeys)
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
        // else duplicate, removed from v0.5
      }

      return status;
    }

};


class KvExecutor
{
public:
  
  static njson set (CacheMap& map, const SessionToken& tkn, njson& cmd)
  {
    njson rsp ;
    rsp["KV_SET_RSP"]["tkn"] = tkn;

    for(auto& kv : cmd["keys"].object_range())
    {
      auto [it, inserted] = map.set(kv.key(), std::move(kv.value()));
      rsp["KV_SET_RSP"]["keys"][kv.key()] = toUnderlying(inserted ? RequestStatus::KeySet : RequestStatus::KeyUpdated);
    }

    return rsp;
  }


  static njson setQ (CacheMap& map, const SessionToken& tkn, njson& cmd)
  {
    njson rsp;
      
    for(auto& kv : cmd["keys"].object_range())
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
      rsp["KV_SETQ_RSP"]["tkn"] = tkn;

    return rsp.empty() ? njson::null() : rsp;
  }


  static njson get (CacheMap& map, const SessionToken& tkn, const njson& cmd)
  {
    njson rsp;
    rsp["KV_GET_RSP"]["tkn"] = tkn;

    for(auto& item : cmd["keys"].array_range())
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

    return rsp;
  }


  static njson add (CacheMap& map, const SessionToken& tkn, njson& cmd)
  {
    njson rsp;
    rsp["KV_ADD_RSP"]["tkn"] = tkn;
    rsp["KV_ADD_RSP"]["keys"] = njson::object();

    for(auto& kv : cmd["keys"].object_range())
    {
      const auto inserted = map.add(kv.key(), std::move(kv.value()));
      rsp["KV_ADD_RSP"]["keys"][kv.key()] = toUnderlying(inserted ? RequestStatus::KeySet : RequestStatus::KeyExists);
    }

    return rsp;
  }


  static njson addQ (CacheMap& map, const SessionToken& tkn, njson& cmd)
  {
    njson rsp;

    for(auto& kv : cmd["keys"].object_range())
    {
      if (const auto inserted = map.add(kv.key(), std::move(kv.value())); !inserted)
        rsp["KV_ADDQ_RSP"]["keys"][kv.key()] = toUnderlying(RequestStatus::KeyExists);
    }

    if (!rsp.empty()) // if key was not added
      rsp["KV_ADDQ_RSP"]["tkn"] = tkn;

    return rsp.empty() ? njson::null() : rsp;
  }


  static njson remove (CacheMap& map, const SessionToken& tkn, const njson& cmd)
  {
    njson rsp;
    rsp["KV_RMV_RSP"]["tkn"] = tkn;

    for(auto& value : cmd["keys"].array_range())
    {
      if (value.is_string())
      {
        const auto& key = value.as_string();
        const auto removed = map.remove(key);
        rsp["KV_RMV_RSP"][key] = toUnderlying(removed ? RequestStatus::KeyRemoved : RequestStatus::KeyNotExist);
      }
    }

    return rsp;
  }


  static njson clear (CacheMap& map, const SessionToken& tkn, njson& cmd)
  {
    const auto[valid, size] = map.clear();
    return PoolRequestResponse::sessionClear(tkn, valid, size);
  }

  
  static njson count (CacheMap& map, const SessionToken& tkn, const njson& cmd)
  {
    return PoolRequestResponse::sessionCount(tkn, map.count());
  }


  static njson contains (CacheMap& map, const SessionToken& tkn, const njson& cmd)
  {
    njson rsp;
    rsp["KV_CONTAINS_RSP"]["st"] = toUnderlying(RequestStatus::Ok);
    rsp["KV_CONTAINS_RSP"]["tkn"] = tkn;

    for (auto& item : cmd["keys"].array_range())
    {
      if (item.is_string())
      {
        const cachedkey& key = item.as_string();
        rsp["KV_CONTAINS_RSP"]["keys"][key] = map.contains(key);
      }
    }

    return rsp;
  }

  
  static njson find (CacheMap& map, const SessionToken& tkn, const njson& cmd)
  {
    const bool paths = cmd.at("rsp") == "paths";

    njson rsp;
    rsp["KV_FIND_RSP"]["tkn"] = tkn;

    njson result = njson::array();

    if (!map.find(cmd, paths, result))
      rsp["KV_FIND_RSP"]["st"] = toUnderlying(RequestStatus::PathInvalid);
    else
    {       
      rsp["KV_FIND_RSP"]["st"] = toUnderlying(RequestStatus::Ok);

      if (cmd.at("rsp") != "kv")
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

    return rsp;
  }


  static njson update (CacheMap& map, const SessionToken& tkn, njson& cmd)
  {
    const auto& key = cmd.at("key").as_string();
    const auto& path = cmd.at("path").as_string();

    njson rsp;
    rsp["KV_UPDATE_RSP"]["tkn"] = tkn;

    const auto [keyExists, count] = map.update(key, path, std::move(cmd.at("value")));

    rsp["KV_UPDATE_RSP"]["st"] = keyExists ? toUnderlying(RequestStatus::Ok) : toUnderlying(RequestStatus::KeyNotExist);
    rsp["KV_UPDATE_RSP"]["cnt"] = count;

    return rsp;
  }


  static njson keys (CacheMap& map, const SessionToken& tkn, njson& cmd)
  {
    return PoolRequestResponse::sessionKeys(tkn, map.keys());
  }


  static njson clearSet (CacheMap& map, const SessionToken& tkn, njson& cmd)
  {
    njson rsp;
    rsp["KV_CLEAR_SET_RSP"]["tkn"] = tkn;
    rsp["KV_CLEAR_SET_RSP"]["keys"] = njson::object();

    if (const auto[valid, size] = map.clear(); valid)
    {
      rsp["KV_CLEAR_SET_RSP"]["st"] = toUnderlying(RequestStatus::Ok);
      rsp["KV_CLEAR_SET_RSP"]["cnt"] = size;

      for(auto& kv : cmd["keys"].object_range())
      {
        auto [it, inserted] = map.set(kv.key(), std::move(kv.value()));
        rsp["KV_CLEAR_SET_RSP"]["keys"][kv.key()] = toUnderlying(RequestStatus::KeySet);  // just cleared, so nothing to update
      }
    }
    else
    {
      rsp["KV_CLEAR_SET_RSP"]["st"] = toUnderlying(RequestStatus::Unknown);
      rsp["KV_CLEAR_SET_RSP"]["cnt"] = 0U;
      rsp["KV_CLEAR_SET_RSP"]["keys"] = njson::object();
    }

    return rsp;
  }
};


}
}
}

#endif

