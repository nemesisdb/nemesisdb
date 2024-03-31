#ifndef NDB_CORE_SESSIONEXECUTOR_H
#define NDB_CORE_SESSIONEXECUTOR_H

#include <functional>
#include <scoped_allocator>
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




template<bool HaveSessions>
class SessionExecutor
{
  using enum RequestStatus;

public:
  
  static njson newSession (Sessions& sessions, const std::string& name, const SessionToken tkn, const bool shared, const SessionDuration duration, const bool deleteOnExpire)
  {    
    njson rsp {jsoncons::json_object_arg, {{"SH_NEW_RSP", njson{jsoncons::json_object_arg}}}};

    auto& body = rsp.at("SH_NEW_RSP");
    
    body["tkn"] = tkn;
    body["name"] = name;    
    body["st"] = toUnderlying(Ok);

    if (const auto cache = sessions.start(tkn, shared, duration, deleteOnExpire); !cache) [[unlikely]]
      body["st"] = toUnderlying(SessionNewFail);

    return rsp;
  }

  
  static njson endSession (Sessions& sessions, const SessionToken& tkn)
  {
    const auto status = sessions.end(tkn) ? RequestStatus::Ok : RequestStatus::SessionNotExist;
    
    njson rsp;
    rsp["SH_END_RSP"]["st"] = toUnderlying(status);
    rsp["SH_END_RSP"]["tkn"] = tkn;
    return rsp;
  }


  static bool openSession (Sessions& sessions, const SessionToken& tkn)
  {
    const auto [exists, shared] = sessions.openShared(tkn);
    return exists && shared;
  }


  static njson sessionInfo (Sessions& sessions, const SessionToken& tkn)
  {
    njson rsp {jsoncons::json_object_arg, {{"SH_INFO_RSP", njson{jsoncons::json_object_arg}}}};

    auto& body = rsp.at("SH_INFO_RSP");

    body["tkn"] = tkn;
    body["shared"] = njson::null();
    body["keyCnt"] = njson::null();

    if (const auto session = sessions.get(tkn); !session)
      body["st"] = toUnderlying(RequestStatus::SessionNotExist);
    else
    {
      const auto& sesh = session->get();
      const auto& expiryInfo = sesh.expireInfo;
      const auto keyCount = sesh.map.count();
      const auto remaining = sesh.expires ? std::chrono::duration_cast<std::chrono::seconds>(expiryInfo.time - SessionClock::now()) :
                                            std::chrono::seconds{0};

      body["st"] = toUnderlying(RequestStatus::Ok);
      body["shared"] = sesh.shared;
      body["keyCnt"] = keyCount;
      body["expires"] = sesh.expires;

      if (sesh.expires)
      {
        body["expiry"]["duration"] = expiryInfo.duration.count();
        body["expiry"]["remaining"] = remaining.count();
        body["expiry"]["deleteSession"] = expiryInfo.deleteOnExpire;
      }
    }

    return rsp;
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
    rsp["SH_EXISTS_RSP"]["exist"].reserve(std::min<std::size_t>(tkns.size(), 100U));

    for (const auto& item : tkns.array_range())
    {
      if (item.is<SessionToken>())
      {
        const auto& tkn = item.as<SessionToken>();
        if (sessions.contains(tkn))
          rsp["SH_EXISTS_RSP"]["exist"].push_back(tkn);
      }
    }

    rsp["SH_EXISTS_RSP"]["exist"].shrink_to_fit();

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
    rsp["SH_SAVE_RSP"]["st"] = toUnderlying(status);
    rsp["SH_SAVE_RSP"]["name"] = cmd["name"].as_string();

    
    try
    {
      if (!fs::create_directories(path))
        status = RequestStatus::SaveError;
      else
      {
        const auto& sessionsMap = sessions.getSessions();

        if (cmd.contains("tkns"))
          saveSelectSessions(cmd, sessionsMap, path, MaxDataFileSize);
        else
          saveAllSessions(sessionsMap, path, MaxDataFileSize);
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

  // SAVE DATA

  static void saveSelectSessions (const njson& cmd, const Sessions::SessionsMap& allSessions, const fs::path& path, const std::size_t MaxDataFileSize)
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
          flushSessionBuffer(path / std::to_string(nFiles), sstream);
          ++nFiles;
          first = true;
        }
      }
    }

    // leftovers (didn't reach the file size)
    if (sstream.tellp() > 0)
    {
      flushSessionBuffer(path / std::to_string(nFiles), sstream);
    }
  }


  static void saveAllSessions (const Sessions::SessionsMap& allSessions, const fs::path& path, const std::size_t MaxDataFileSize)
  {    
    std::string buffer; // vector<char> ?
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
        flushSessionBuffer(path / std::to_string(nFiles), sstream);
        ++nFiles;
        first = true;
      }
    }

    // leftovers that didn't reach the file size
    if (sstream.tellp() > 0)
      flushSessionBuffer(path / std::to_string(nFiles), sstream);
  }


  static void flushSessionBuffer (const fs::path& path, std::stringstream& sstream)
  {
    std::ofstream dataStream {path};
    dataStream << '[' << sstream.str() << ']';

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


  // LOAD DATA
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
        if (seshData.contains("keys") && seshData.at("keys").is_object())
        {
          for (auto& items : seshData.at("keys").object_range())
          {
            session->get().set(items.key(), std::move(items.value()));
            ++nKeys;
          }
        }
        ++nSessions;
      }      
    }

    return status;
  }

};



template<>
class SessionExecutor<false>
{
  // No sessions to execute
};


template<bool HaveSessions>
class KvExecutor
{
public:

  static njson set (CacheMap& map,  const njson& cmd)
  {
    njson rsp {jsoncons::json_object_arg, {{"KV_SET_RSP", jsoncons::json_object_arg_t{}}}};
    auto& body = rsp.at("KV_SET_RSP");

    body["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      for(const auto& kv : cmd["keys"].object_range())
        map.set(kv.key(), kv.value());
    }
    catch(const std::exception& ex)
    {
      PLOGE << ex.what();
      body["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return rsp;
  }


  static njson setQ (CacheMap& map,  const njson& cmd)
  {
    njson rsp;

    try
    {         
      for(const auto& kv : cmd["keys"].object_range())
        map.set(kv.key(), kv.value());     
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      rsp = njson {jsoncons::json_object_arg, {{"KV_SETQ_RSP", jsoncons::json_object_arg_t{}}}};
      rsp["KV_SETQ_RSP"]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return rsp;
  }


  static njson get (CacheMap& map,  const njson& cmd)
  {
    static njson Prepared {jsoncons::json_object_arg, {{"KV_GET_RSP", {jsoncons::json_object_arg,
                                                                      {
                                                                        {"st", toUnderlying(RequestStatus::Ok)},
                                                                        {"keys", njson{}} // initialise as an empty object
                                                                      }}}}};
    
    njson rsp = Prepared;
    auto& body = rsp.at("KV_GET_RSP");
    auto& keys = body.at("keys");

    try
    {
      for(const auto& item : cmd["keys"].array_range())
      {
        if (item.is_string()) [[likely]]
        {
          const auto& key = item.as_string();

          if (const auto value = map.get(key) ; value)
            keys.try_emplace(key, (*value).get());
        }
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      body["st"] = toUnderlying(RequestStatus::Unknown);
    }
    
    return rsp;
  }


  static njson add (CacheMap& map,  const njson& cmd)
  {
    njson rsp {jsoncons::json_object_arg, {{"KV_ADD_RSP", jsoncons::json_object_arg_t{}}}};
    
    rsp["KV_ADD_RSP"]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      for(const auto& kv : cmd["keys"].object_range())
        map.add(kv.key(), kv.value());
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      rsp["KV_ADD_RSP"]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return rsp;
  }


  static njson addQ (CacheMap& map,  const njson& cmd)
  {
    njson rsp;

    try
    {
      for(auto& kv : cmd["keys"].object_range())
        map.add(kv.key(), kv.value());
    }
    catch (const std::exception& ex)
    {
      PLOGE << ex.what();
      rsp = njson {jsoncons::json_object_arg, {{"KV_ADDQ_RSP", jsoncons::json_object_arg_t{}}}};
      rsp["KV_ADDQ_RSP"]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return rsp;
  }


  static njson remove (CacheMap& map,  const njson& cmd)
  {
    njson rsp;
    rsp["KV_RMV_RSP"]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      for(const auto& value : cmd["keys"].array_range())
      {
        if (value.is_string())
        {
          const auto& key = value.as_string();
          map.remove(key);
        }
      }  
    }
    catch(const std::exception& e)
    {
      rsp["KV_RMV_RSP"]["st"] = toUnderlying(RequestStatus::Unknown);
      PLOGE << e.what();
    }
    
    return rsp;
  }


  static njson clear (CacheMap& map,  const njson& cmd)
  {
    const auto[ok, count] = map.clear();

    njson rsp;
    rsp["KV_CLEAR_RSP"]["st"] = ok ? toUnderlying(RequestStatus::Ok) : toUnderlying(RequestStatus::Unknown);
    rsp["KV_CLEAR_RSP"]["cnt"] = count;
    
    return rsp;
  }


  static njson contains (CacheMap& map,  const njson& cmd)
  {
    njson rsp;
    rsp["KV_CONTAINS_RSP"]["st"] = toUnderlying(RequestStatus::Ok);
    rsp["KV_CONTAINS_RSP"]["contains"] = njson::array{};

    auto& contains = rsp.at("KV_CONTAINS_RSP").at("contains");

    for (auto&& item : cmd["keys"].array_range())
    {
      if (item.is_string())
      {
        if (cachedkey key = item.as_string(); map.contains(key))
          contains.emplace_back(std::move(key));
      }
    }

    return rsp;
  }

  
  static njson find (CacheMap& map,  const njson& cmd)
  {
    const bool paths = cmd.at("rsp") == "paths";

    njson rsp;

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

          if (const auto value = map.get(key); value)
            rsp["KV_FIND_RSP"]["keys"][key] = (*value).get();
        } 
      }
    }

    return rsp;
  }


  static njson update (CacheMap& map,  const njson& cmd)
  {
    const auto& key = cmd.at("key").as_string();
    const auto& path = cmd.at("path").as_string();

    const auto [keyExists, count] = map.update(key, path, cachedvalue::parse(cmd.at("value").to_string()));
    
    njson rsp;
    rsp["KV_UPDATE_RSP"]["st"] = keyExists ? toUnderlying(RequestStatus::Ok) : toUnderlying(RequestStatus::KeyNotExist);
    rsp["KV_UPDATE_RSP"]["cnt"] = count;

    return rsp;
  }


  static njson keys (CacheMap& map,  const njson& cmd)
  {
    njson rsp;
    rsp["KV_KEYS_RSP"]["st"] = toUnderlying(RequestStatus::Ok);
    rsp["KV_KEYS_RSP"]["keys"] = std::move(map.keys());
    return rsp;
  }


  static njson clearSet (CacheMap& map,  const njson& cmd)
  {
    njson rsp;
    rsp["KV_CLEAR_SET_RSP"]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      if (const auto[valid, size] = map.clear(); valid)
      {
        rsp["KV_CLEAR_SET_RSP"]["cnt"] = size;

        for(const auto& kv : cmd["keys"].object_range())
          map.set(kv.key(), kv.value());
      }
      else
      {
        PLOGE << "KV_CLEAR_SET failed to clear";
        rsp["KV_CLEAR_SET_RSP"]["st"] = toUnderlying(RequestStatus::Unknown);
        rsp["KV_CLEAR_SET_RSP"]["cnt"] = 0U;
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      rsp["KV_CLEAR_SET_RSP"]["st"] = toUnderlying(RequestStatus::Unknown);
      rsp["KV_CLEAR_SET_RSP"]["cnt"] = 0U;
    }

    return rsp;
  }


  static njson count (CacheMap& map,  const njson& cmd)
  {
    njson rsp;
    rsp["KV_COUNT_RSP"]["st"] = toUnderlying(RequestStatus::Ok);
    rsp["KV_COUNT_RSP"]["cnt"] = map.count();
    return rsp;
  }


  static njson saveKv (const CacheMap& map, const fs::path& path, const std::string_view name)
  {
    static const std::size_t MaxDataFileSize = 10U * 1024U * 1024U;

    auto start = NemesisClock::now();

    RequestStatus status = RequestStatus::SaveComplete;

    njson rsp;
    rsp["KV_SAVE_RSP"]["name"] = name;

    try
    {
      if (!fs::create_directories(path))
        status = RequestStatus::SaveError;
      else
      {
        std::string buffer; // vector<char> ?
        buffer.reserve(MaxDataFileSize);

        std::stringstream sstream{buffer};
        std::size_t nFiles = 0;
        
        const std::size_t totalKeys = map.count();
        bool first = true;

        for(const auto& [k, v] : map.map())
        {   
          if (!first)
            sstream << ',';

          first = false;

          sstream << '"' << k << "\":" << v.to_string();
          
          if (sstream.tellp() >= MaxDataFileSize)
          {
            flushKvBuffer(path / std::to_string(nFiles), sstream);
            ++nFiles;
            first = true;
          }         
        }

        // leftovers that didn't reach the file size
        if (sstream.tellp() > 0)
          flushKvBuffer(path / std::to_string(nFiles), sstream);
      }
    }
    catch(const std::exception& e)
    {
      status = RequestStatus::SaveError;
    }
    

    rsp["KV_SAVE_RSP"]["st"] = toUnderlying(status);
    rsp["KV_SAVE_RSP"]["duration"] = chrono::duration_cast<chrono::milliseconds>(NemesisClock::now() - start).count();

    return rsp;
  }


  static njson loadKv (const std::string& loadName, CacheMap& map, const fs::path& dataRoot)
  {
    RequestStatus status{RequestStatus::Loading};
    std::size_t nSessions{0}, nKeys{0};
    
    njson rsp;
    rsp["SH_LOAD_RSP"]["st"] = toUnderlying(status);
    rsp["SH_LOAD_RSP"]["name"] = loadName;
    rsp["SH_LOAD_RSP"]["duration"] = 0; // updated below
    rsp["SH_LOAD_RSP"]["sessions"] = 0; // always 0 when sessions disabled
    rsp["SH_LOAD_RSP"]["keys"] = 0; // updated below

    try
    {      
      const auto start = NemesisClock::now();

      for (const auto& kvFile : fs::directory_iterator{dataRoot})
      {
        status = readKvFile(map, kvFile.path(), nKeys);
        
        if (status == RequestStatus::LoadError)
          break;
      }

      rsp["SH_LOAD_RSP"]["duration"] = chrono::duration_cast<chrono::milliseconds>(NemesisClock::now() - start).count();
      rsp["SH_LOAD_RSP"]["keys"] = nKeys;      
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      status = RequestStatus::LoadError;
    }

    rsp["SH_LOAD_RSP"]["st"] = toUnderlying(status);
    
    return rsp;
  }


private:

  static void flushKvBuffer (const fs::path& path, std::stringstream& sstream)
  {
    std::ofstream dataStream {path};
    dataStream << "{ \"keys\":{" << sstream.str() << "}}";

    sstream.clear();
    sstream.str(std::string{});
  }


  static RequestStatus readKvFile (CacheMap& map, const fs::path path, std::size_t& nKeys)
  {
    RequestStatus status = RequestStatus::LoadComplete;

    try
    {
      std::ifstream dataStream{path};
      
      if (const auto root = njson::parse(dataStream); root.is_object() && root.contains("keys") && root.at("keys").is_object())
      {
        for (const auto& item : root.at("keys").object_range())
        {
          map.set(item.key(), item.value());
          ++nKeys;
        }
      }      
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      status = RequestStatus::LoadError;
    }
    
    return status;
  }

};


}
}
}

#endif

