#ifndef NDB_CORE_SHEXECUTOR_H
#define NDB_CORE_SHEXECUTOR_H

#include <functional>
#include <fstream>
#include <core/NemesisCommon.h>
#include <core/kv/KvCommon.h>
#include <core/sh/ShSessions.h>
#include <core/sh/ShCommands.h>

namespace nemesis { namespace sh {

using namespace nemesis::core;
using namespace nemesis::sh;


class SessionExecutor
{
  using enum RequestStatus;

  

public:
  
  static Response newSession (std::shared_ptr<Sessions> sessions, const SessionToken tkn, const Sessions::ExpireInfo& expiry)
  {    
    Response response;
    response.rsp = njson {jsoncons::json_object_arg, {{cmds::NewRsp, njson::object()}}};

    auto& body = response.rsp.at(cmds::NewRsp);
    
    body["tkn"] = tkn; 
    body["st"] = toUnderlying(Ok);

    if (const auto cache = sessions->start(tkn, false, expiry); !cache) [[unlikely]]
      body["st"] = toUnderlying(SessionNewFail);

    return response;
  }


  static Response endSession (std::shared_ptr<Sessions> sessions, const SessionToken& tkn)
  {
    const auto status = sessions->end(tkn) ? RequestStatus::Ok : RequestStatus::SessionNotExist;
    
    Response response;
    response.rsp[cmds::EndRsp]["st"] = toUnderlying(status);
    response.rsp[cmds::EndRsp]["tkn"] = tkn;
    return response;
  }


  // static bool openSession (std::shared_ptr<Sessions> sessions, const SessionToken& tkn)
  // {
  //   const auto [exists, shared] = sessions->openShared(tkn);
  //   return exists && shared;
  // }


  static Response sessionInfo (std::shared_ptr<Sessions> sessions, const SessionToken& tkn)
  {
    Response response;
    response.rsp = njson {jsoncons::json_object_arg, {{cmds::InfoRsp, njson::object()}}}; 

    auto& body = response.rsp.at(cmds::InfoRsp);

    body["tkn"] = tkn;
    body["shared"] = njson::null();
    body["keyCnt"] = njson::null();

    if (const auto session = sessions->get(tkn); !session)
      body["st"] = toUnderlying(RequestStatus::SessionNotExist);
    else
    {
      const auto& sesh = session->get();
      const auto& expireInfo = sesh.expireInfo;
      const auto keyCount = sesh.map.count();
      const auto remaining = sesh.expires ? std::chrono::duration_cast<std::chrono::seconds>(expireInfo.time - SessionClock::now()) :
                                            std::chrono::seconds{0};

      body["st"] = toUnderlying(RequestStatus::Ok);
      body["shared"] = sesh.shared;
      body["keyCnt"] = keyCount;
      body["expires"] = sesh.expires;

      if (sesh.expires)
      {
        body["expiry"]["duration"] = expireInfo.duration.count();
        body["expiry"]["remaining"] = remaining.count();
        body["expiry"]["deleteSession"] = expireInfo.deleteOnExpire;
        body["expiry"]["extendOnSetAdd"] = expireInfo.extendOnSetAdd;
        body["expiry"]["extendOnGet"] = expireInfo.extendOnGet;
      }
    }

    return response;
  }


  static Response sessionInfoAll (std::shared_ptr<Sessions> sessions)
  {
    Response response;
    response.rsp[cmds::InfoAllRsp]["st"] = toUnderlying(RequestStatus::Ok);
    response.rsp[cmds::InfoAllRsp]["totalSessions"] = sessions->countSessions();
    response.rsp[cmds::InfoAllRsp]["totalKeys"] = sessions->countKeys();
    return response;
  }


  static Response sessionExists (std::shared_ptr<Sessions> sessions, const njson& tkns)
  {
    Response response;
    response.rsp[cmds::ExistsRsp]["st"] = toUnderlying(RequestStatus::Ok);
    response.rsp[cmds::ExistsRsp]["exist"] = njson::make_array();
    response.rsp[cmds::ExistsRsp]["exist"].reserve(std::min<std::size_t>(tkns.size(), 100U));

    for (const auto& item : tkns.array_range())
    {
      if (item.is<SessionToken>())
      {
        const auto& tkn = item.as<SessionToken>();
        if (sessions->contains(tkn))
          response.rsp[cmds::ExistsRsp]["exist"].push_back(tkn);
      }
    }

    response.rsp[cmds::ExistsRsp]["exist"].shrink_to_fit();

    return response;
  }


  static Response sessionEndAll (std::shared_ptr<Sessions> sessions)
  {
    Response response;
    response.rsp[cmds::EndAllRsp]["st"] = toUnderlying(RequestStatus::Ok);
    response.rsp[cmds::EndAllRsp]["cnt"] = sessions->clear();
    return response;
  }


  static void sessionMonitor(std::shared_ptr<Sessions> sessions)
  {
    sessions->handleExpired();
  }


  static Response saveSessions (std::shared_ptr<Sessions> sessions, const njson& cmd, const std::string_view name, const fs::path path)
  {
    const std::size_t MaxDataFileSize = 10 * 1024 * 1024;
    const auto start = NemesisClock::now();

    RequestStatus status = RequestStatus::SaveComplete;

    Response response;
    response.rsp[cmds::SaveRsp]["st"] = toUnderlying(status);
    response.rsp[cmds::SaveRsp]["name"] = name;

    
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

    response.rsp[cmds::SaveRsp]["st"] = toUnderlying(status);
    response.rsp[cmds::SaveRsp]["duration"] = chrono::duration_cast<chrono::milliseconds>(NemesisClock::now() - start).count();

    return response;
  }


  static Response loadSessions (const std::string& loadName, std::shared_ptr<Sessions> sessions, const fs::path& dataRoot)
  {
    RequestStatus status{RequestStatus::Loading};
    std::size_t nSessions{0}, nKeys{0};
    Response response;

    if (!fs::exists(dataRoot))
    {
      status = RequestStatus::LoadError;
      response.rsp[cmds::LoadRsp]["m"] = "Dataset does not exist";
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
        
    response.rsp[cmds::LoadRsp]["name"] = loadName;
    response.rsp[cmds::LoadRsp]["st"] = status == RequestStatus::LoadComplete ? toUnderlying(RequestStatus::LoadComplete) :
                                                                                toUnderlying(RequestStatus::LoadError);
    response.rsp[cmds::LoadRsp]["sessions"] = nSessions;
    response.rsp[cmds::LoadRsp]["keys"] = nKeys;

    return response;
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
    seshData["sh"]["expiry"]["extendOnSetAdd"] = sesh.expireInfo.extendOnSetAdd;
    seshData["sh"]["expiry"]["extendOnGet"] = sesh.expireInfo.extendOnGet;
    
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

      Sessions::ExpireInfo expireInfo;
      expireInfo.duration = SessionDuration{seshData["sh"]["expiry"]["duration"].as<std::size_t>()};
      expireInfo.deleteOnExpire = seshData["sh"]["expiry"]["deleteSession"].as_bool();
      expireInfo.extendOnSetAdd = seshData["sh"]["expiry"]["extendOnSetAdd"].as_bool();
      expireInfo.extendOnGet = seshData["sh"]["expiry"]["extendOnGet"].as_bool();
      
      if (auto session = sessions->start(token, isShared, expireInfo); session)
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

#endif

