#ifndef NDB_CORE_SHEXECUTOR_H
#define NDB_CORE_SHEXECUTOR_H

#include <functional>
#include <fstream>
#include <core/NemesisCommon.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvSessions.h>


namespace nemesis { namespace core { namespace kv {

using namespace nemesis::core;
using namespace nemesis::core::kv;



class SessionExecutor
{
  using enum RequestStatus;

public:
  
  static njson newSession (std::shared_ptr<Sessions> sessions, const SessionToken tkn, const SessionDuration duration, const bool deleteOnExpire)
  {    
    njson rsp {jsoncons::json_object_arg, {{"SH_NEW_RSP", njson{jsoncons::json_object_arg}}}};

    auto& body = rsp.at("SH_NEW_RSP");
    
    body["tkn"] = tkn; 
    body["st"] = toUnderlying(Ok);

    if (const auto cache = sessions->start(tkn, false, duration, deleteOnExpire); !cache) [[unlikely]]
      body["st"] = toUnderlying(SessionNewFail);

    return rsp;
  }


  static njson endSession (std::shared_ptr<Sessions> sessions, const SessionToken& tkn)
  {
    const auto status = sessions->end(tkn) ? RequestStatus::Ok : RequestStatus::SessionNotExist;
    
    njson rsp;
    rsp["SH_END_RSP"]["st"] = toUnderlying(status);
    rsp["SH_END_RSP"]["tkn"] = tkn;
    return rsp;
  }


  static bool openSession (std::shared_ptr<Sessions> sessions, const SessionToken& tkn)
  {
    const auto [exists, shared] = sessions->openShared(tkn);
    return exists && shared;
  }


  static njson sessionInfo (std::shared_ptr<Sessions> sessions, const SessionToken& tkn)
  {
    njson rsp {jsoncons::json_object_arg, {{"SH_INFO_RSP", njson{jsoncons::json_object_arg}}}};

    auto& body = rsp.at("SH_INFO_RSP");

    body["tkn"] = tkn;
    body["shared"] = njson::null();
    body["keyCnt"] = njson::null();

    if (const auto session = sessions->get(tkn); !session)
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


  static njson sessionInfoAll (std::shared_ptr<Sessions> sessions)
  {
    njson rsp;
    rsp["SH_INFO_ALL_RSP"]["st"] = toUnderlying(RequestStatus::Ok);
    rsp["SH_INFO_ALL_RSP"]["totalSessions"] = sessions->countSessions();
    rsp["SH_INFO_ALL_RSP"]["totalKeys"] = sessions->countKeys();
    return rsp;
  }


  static njson sessionExists (std::shared_ptr<Sessions> sessions, const njson& tkns)
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
        if (sessions->contains(tkn))
          rsp["SH_EXISTS_RSP"]["exist"].push_back(tkn);
      }
    }

    rsp["SH_EXISTS_RSP"]["exist"].shrink_to_fit();

    return rsp;
  }


  static njson sessionEndAll (std::shared_ptr<Sessions> sessions)
  {
    njson rsp;
    rsp["SH_END_ALL_RSP"]["st"] = toUnderlying(RequestStatus::Ok);
    rsp["SH_END_ALL_RSP"]["cnt"] = sessions->clear();
    return rsp;
  }


  static void sessionMonitor(std::shared_ptr<Sessions> sessions)
  {
    sessions->handleExpired();
  }


  static njson saveSessions (std::shared_ptr<Sessions> sessions, const njson& cmd, const std::string_view name, const fs::path path)
  {
    const std::size_t MaxDataFileSize = 10 * 1024 * 1024;
    const auto start = NemesisClock::now();

    RequestStatus status = RequestStatus::SaveComplete;

    njson rsp;
    rsp["SH_SAVE_RSP"]["st"] = toUnderlying(status);
    rsp["SH_SAVE_RSP"]["name"] = name;

    
    try
    {
      if (!fs::create_directories(path))
        status = RequestStatus::SaveError;
      else
      {
        const auto& sessionsMap = sessions->getSessions();

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


  static njson loadSessions (const std::string& loadName, std::shared_ptr<Sessions> sessions, const fs::path& dataRoot)
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

  static void saveSelectSessions (const njson& cmd, const Sessions::SessionsMap& allSessions, const fs::path& path, const std::streamoff MaxDataFileSize)
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
        writeSession(first, it->second, token, sstream);
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


  static void saveAllSessions (const Sessions::SessionsMap& allSessions, const fs::path& path, const std::streamoff MaxDataFileSize)
  {    
    std::string buffer; // vector<char> ?
    buffer.reserve(MaxDataFileSize);

    std::stringstream sstream{buffer};
    std::size_t nFiles = 0;

    bool first = true;
    for(const auto& [token, sesh] : allSessions)
    {
      writeSession(first, sesh, token, sstream);
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

  
  static void writeSession (const bool first, const Sessions::SessionType& sesh, const SessionToken token, std::stringstream& buffer)
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
  static RequestStatus readSeshFile (std::shared_ptr<Sessions> sessions, const fs::path path, std::size_t& nSessions, std::size_t& nKeys)
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


  static RequestStatus loadSession (std::shared_ptr<Sessions> sessions, njson&& seshData, std::size_t& nSessions, std::size_t& nKeys)
  {
    RequestStatus status = RequestStatus::LoadComplete;

    if (seshData.contains("sh"))  // can be empty object because sesh file is created even if pool contains no sessions
    {
      const auto token = seshData["sh"]["tkn"].as<SessionToken>();
      const auto isShared = seshData["sh"]["shared"].as_bool();
      const auto deleteOnExpire = seshData["sh"]["expiry"]["deleteSession"].as_bool();
      const auto duration = SessionDuration{seshData["sh"]["expiry"]["duration"].as<std::size_t>()};

      if (auto session = sessions->start(token, isShared, duration, deleteOnExpire); session)
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

}
}
}

#endif

