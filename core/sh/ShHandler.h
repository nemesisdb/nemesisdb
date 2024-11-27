#ifndef NDB_CORE_SHHANDLER_H
#define NDB_CORE_SHHANDLER_H


#include <functional>
#include <vector>
#include <tuple>
#include <ankerl/unordered_dense.h>
#include <core/NemesisCommon.h>
#include <core/Persistance.h>
#include <core/sh/ShCommon.h>
#include <core/sh/ShSessions.h>
#include <core/sh/ShCommands.h>
#include <core/sh/ShCommandValidate.h>


namespace nemesis { namespace core { namespace sh {

namespace sh = nemesis::core::sh;


class ShHandler
{
  struct Handler
  {
    using HandleFunc = std::function<Response(njson&)>;

    Handler(HandleFunc&& h) : handler(std::move(h))
    {

    }

    Handler(const Handler&) = default;
    Handler(Handler&&) = default;
    Handler& operator= (Handler&&) = default;

    Response operator()(njson& request)
    {
      return handler(request);
    }

    HandleFunc handler;
  };

  
  using QueryTypePmrMap = ankerl::unordered_dense::pmr::map<std::string_view, ShQueryType>;
  using HandlerPmrMap = ankerl::unordered_dense::pmr::map<ShQueryType, Handler>;


  template<class Alloc>
  auto createHandlers (Alloc& alloc)
  {
    // initialise with 1 bucket and pmr allocator
    HandlerPmrMap h (
    {
      {ShQueryType::ShNew,     Handler{std::bind_front(&ShHandler::sessionNew,      std::ref(*this))}},
      {ShQueryType::ShEnd,     Handler{std::bind_front(&ShHandler::sessionEnd,      std::ref(*this))}},
      {ShQueryType::ShEndAll,  Handler{std::bind_front(&ShHandler::sessionEndAll,   std::ref(*this))}},
      {ShQueryType::ShExists,  Handler{std::bind_front(&ShHandler::sessionExists,   std::ref(*this))}},
      {ShQueryType::ShInfo,    Handler{std::bind_front(&ShHandler::sessionInfo,     std::ref(*this))}},
      {ShQueryType::ShInfoAll, Handler{std::bind_front(&ShHandler::sessionInfoAll,  std::ref(*this))}},
      {ShQueryType::ShLoad,    Handler{std::bind_front(&ShHandler::sessionLoad,     std::ref(*this))}},
      {ShQueryType::ShSave,    Handler{std::bind_front(&ShHandler::sessionSave,     std::ref(*this))}},
      //{KvQueryType::ShOpen,    Handler{std::bind_front(&ShHandler::sessionOpen,     std::ref(*this))}}
      
    }, 1, alloc);

    return h;
  }


  template<class Alloc>
  auto createQueryTypeNameMap (Alloc& alloc)
  {
    QueryTypePmrMap map ( 
    { 
      {sh::NewReq,      ShQueryType::ShNew},
      {sh::EndReq,      ShQueryType::ShEnd},      
      {sh::InfoReq,     ShQueryType::ShInfo},
      {sh::InfoAllReq,  ShQueryType::ShInfoAll},
      {sh::SaveReq,     ShQueryType::ShSave},
      {sh::LoadReq,     ShQueryType::ShLoad},
      {sh::EndAllReq,   ShQueryType::ShEndAll},
      {sh::ExistsReq,   ShQueryType::ShExists},
      //{"SH_OPEN",     ShQueryType::ShOpen},
    }, 1, alloc); 

    return map;
  }



public:
  ShHandler(const njson& config, std::shared_ptr<Sessions> sessions) : m_config(config), m_sessions(sessions)
    
  {
  }


  Response handle(const std::string& command, njson& request)
  {
    static PmrResource<typename HandlerPmrMap::value_type, 1024U> handlerPmrResource; // TODO buffer size
    static PmrResource<typename HandlerPmrMap::value_type, 1024U> queryTypeNamePmrResource; // TODO buffer size
    static HandlerPmrMap MsgHandlers{createHandlers(handlerPmrResource.getAlloc())}; 
    static QueryTypePmrMap QueryNameToType{createQueryTypeNameMap(queryTypeNamePmrResource.getAlloc())};
    
    if (const auto itType = QueryNameToType.find(command) ; itType == QueryNameToType.cend())
      return Response{.rsp = createErrorResponse(command+"_RSP", RequestStatus::CommandNotExist)};
    else if (const auto handlerIt = MsgHandlers.find(itType->second) ; handlerIt == MsgHandlers.cend())
      return Response{.rsp = createErrorResponse(command+"_RSP", RequestStatus::CommandDisabled)};
    else
    {
      try
      {
        auto& handler = handlerIt->second;
        return handler(request);
      }
      catch (const std::exception& kex)
      {
        PLOGF << kex.what();
        return Response{.rsp = createErrorResponse(command+"_RSP", RequestStatus::Unknown)};
      }
    }
  }


  LoadResult internalLoad(const std::string& loadName, const fs::path& dataSetsRoot)
  {
    const auto start = NemesisClock::now();
    
    const njson rsp = doLoad(loadName, dataSetsRoot);

    LoadResult loadResult;
    loadResult.duration = NemesisClock::now() - start;
    loadResult.status = static_cast<RequestStatus>(rsp[sh::LoadRsp]["st"].as<std::uint64_t>());
    loadResult.nSessions = rsp[sh::LoadRsp]["sessions"].as<std::size_t>();
    loadResult.nKeys = rsp[sh::LoadRsp]["keys"].as<std::size_t>();

    return loadResult;
  }


private:
  ndb_always_inline Response sessionNew(njson& req)
  {
    if (auto [valid, rsp] = validateNew(req); !valid)
    {
      return Response{.rsp = std::move(rsp)};
    }
    else
    {
      bool deleteOnExpire = false;
      SessionDuration duration{SessionDuration::rep{0}};

      if (req.at(sh::NewReq).contains("expiry"))
      {
        const auto& expiry = req.at(sh::NewReq).at("expiry");

        duration = SessionDuration{expiry.at("duration").as<SessionDuration::rep>()};
        deleteOnExpire = expiry.at("deleteSession").as_bool();
      }
      
      return SessionExecutor::newSession(m_sessions, createSessionToken(), duration, deleteOnExpire);
    }
  }


  ndb_always_inline Response sessionEnd(njson& req)
  {
    SessionToken token;
    
    if (getSessionToken(sh::EndRsp, req.at(sh::EndReq), token))
      return SessionExecutor::endSession(m_sessions, token);
    else
      return Response{.rsp = createErrorResponse(sh::EndRsp, RequestStatus::SessionNotExist)};
  }

  
  /*
  ndb_always_inline Response sessionOpen(njson& json)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      if (cmd.at("name").empty())
        return {RequestStatus::ValueSize, "Session name empty"};

      return {RequestStatus::Ok, ""};
    };
    

    if (auto [valid, rsp] = isValid(queryRspName, ws, json, {{Param::required("name", JsonString)}}, validate))
    {
      njson rsp;

      const auto& cmd = json.at(queryName);
      const auto token = createSessionToken(cmd.at("name").as_string());
      
      // generate a shared token from the name. If a session with the same name was created but isn't shared, 
      // the tokens will be completely different
      if (SessionExecutor::openSession(m_sessions, token))
      {
        rsp[queryRspName]["tkn"] = token;
        rsp[queryRspName]["st"] = toUnderlying(RequestStatus::Ok);
      }
      else
      {
        rsp[queryRspName]["tkn"] = njson::null();
        rsp[queryRspName]["st"] = toUnderlying(RequestStatus::SessionOpenFail);
      }        

      send(ws, rsp);
    }
  }
  */

  
  ndb_always_inline Response sessionInfo(njson& req)
  {
    auto& cmd = req.at(sh::InfoReq);

    if (cmd.size() != 1U)
      return Response{.rsp = createErrorResponse(sh::InfoRsp, RequestStatus::CommandSyntax)};
    else
    {
      SessionToken token;
    
      if (getSessionToken(sh::InfoReq, cmd, token))
        return SessionExecutor::sessionInfo(m_sessions, token);
      else
        return Response{.rsp = createErrorResponse(sh::InfoRsp, RequestStatus::SessionNotExist)};
    }
  }
  
  
  ndb_always_inline Response sessionInfoAll(njson& req)
  {
    return SessionExecutor::sessionInfoAll(m_sessions);
  }
  
  
  ndb_always_inline Response sessionExists(njson& req)
  {
    if (auto [valid, rsp] = validateExists(req); !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return SessionExecutor::sessionExists(m_sessions, req.at(sh::ExistsReq).at("tkns"));
  }


  ndb_always_inline Response sessionEndAll(njson& req)
  {
    return SessionExecutor::sessionEndAll(m_sessions);
  }

  
  ndb_always_inline Response sessionSave(njson& req)
  {    
    // TODO  add this check in handle() so the error can be caught earlier
    if (!NemesisConfig::persistEnabled(m_config))
      return Response{.rsp = createErrorResponseNoTkn(sh::SaveRsp, RequestStatus::CommandDisabled)};
    else 
    {
      if (auto [valid, rsp] = validateSave(req); !valid)
        return Response{.rsp = std::move(rsp)};
      else
        return doSave(sh::SaveReq, sh::SaveRsp, req.at(sh::SaveReq));
    }
  } 

  
  ndb_always_inline Response sessionLoad(njson& req)
  {
    if (auto [valid, rsp] = validateLoad(req); !valid)
        return Response{.rsp = std::move(rsp)};
    else
    {
      const auto& loadName = req.at(sh::LoadReq).at("name").as_string();

      if (const auto [preLoadValid, msg] = validatePreLoad(loadName, fs::path{NemesisConfig::savePath(m_config)}, true); !preLoadValid)
      {
        PLOGE << msg;

        Response response;
        response.rsp[sh::LoadRsp]["st"] = toUnderlying(RequestStatus::LoadError);
        response.rsp[sh::LoadRsp]["name"] = loadName;
        return response;
      }
      else
      {
        // can ignore pathsValid here because if paths are invalid, validatePreLoad() returns false
        const auto [root, md, data, pathsValid] = getLoadPaths(fs::path{NemesisConfig::savePath(m_config)} / loadName);
        return Response{.rsp = doLoad(loadName, data)};
      }
    }    
  }

  
private:
  // move to NemesisCommon
  bool getSessionToken(const std::string_view queryRspName, const njson& cmdRoot, SessionToken& tkn)
  {
    if (cmdRoot.contains("tkn") && cmdRoot.at("tkn").is<SessionToken>())
    {
      tkn = cmdRoot.at("tkn").as<SessionToken>();
      return true;
    }
    else
    {
      return false;
    }
  }


  Response doSave (const std::string_view queryName, const std::string_view queryRspName, njson& cmd)
  {
    const auto& name = cmd.at("name").as_string();
    const auto dataSetDir = std::to_string(KvSaveClock::now().time_since_epoch().count());
    const auto root = fs::path {NemesisConfig::savePath(m_config)} / name / dataSetDir;
    const auto metaPath = root / "md";
    const auto dataPath = root / "data";
    

    if (auto [preparedStatus, metaStream] = prepareSave(cmd, root); preparedStatus != RequestStatus::Ok)
    {
      njson rsp;
      rsp[queryRspName]["name"] = name;
      rsp[queryRspName]["st"] = toUnderlying(preparedStatus);
      return Response{.rsp = std::move(rsp)};
    }
    else
    {
      auto metaData = createInitialSaveMetaData(metaStream, name, true);
      
      KvSaveStatus metaDataStatus = KvSaveStatus::Pending;
      Response response;

      try
      {
        response = SessionExecutor::saveSessions(m_sessions, cmd, name, dataPath);
        metaDataStatus = KvSaveStatus::Complete;
      }
      catch(const std::exception& e)
      {
        PLOGE << e.what();
        metaDataStatus = KvSaveStatus::Error;
        response.rsp[queryRspName]["st"] = toUnderlying(RequestStatus::SaveError);
      }
      
      // update metdata
      completeSaveMetaData(metaStream, metaData, metaDataStatus);

      return response;
    }
  }


  njson doLoad (const std::string& loadName, const fs::path& dataSetsRoot)
  {
    PLOGI << "Loading from " << dataSetsRoot;
    
    Response response = SessionExecutor::loadSessions (loadName, m_sessions, dataSetsRoot); 

    PLOGI << "Loading complete";

    return response.rsp;
  }

private:
  njson m_config;
  std::shared_ptr<Sessions> m_sessions;
};


}
}
}


#endif
