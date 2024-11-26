#ifndef NDB_CORE_SHHANDLER_H
#define NDB_CORE_SHHANDLER_H


#include <functional>
#include <vector>
#include <tuple>
#include <ankerl/unordered_dense.h>
#include <core/NemesisCommon.h>
#include <core/Persistance.h>
#include <core/sh/ShCommon.h>
#include <core/sh/KvSessions.h>
#include <core/sh/ShCommands.h>


namespace nemesis { namespace core { namespace sh {

namespace sh = nemesis::core::sh;


class ShHandler
{
  struct Handler
  {
    using HandleFunc = std::function<Response(KvWebSocket *, njson&)>;

    Handler(HandleFunc&& h) : handler(std::move(h))
    {

    }

    Handler(const Handler&) = default;
    Handler(Handler&&) = default;
    Handler& operator= (Handler&&) = default;

    Response operator()(KvWebSocket * ws, njson& request)
    {
      return handler(ws, request);
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


  Response handle(KvWebSocket * ws, const std::string& command, njson& request)
  {
    static PmrResource<typename HandlerPmrMap::value_type, 1024U> handlerPmrResource; // TODO buffer size
    static PmrResource<typename HandlerPmrMap::value_type, 1024U> queryTypeNamePmrResource; // TODO buffer size
    static HandlerPmrMap MsgHandlers{createHandlers(handlerPmrResource.getAlloc())}; 
    static QueryTypePmrMap QueryNameToType{createQueryTypeNameMap(queryTypeNamePmrResource.getAlloc())};
    
    if (const auto itType = QueryNameToType.find(command) ; itType == QueryNameToType.cend())
      //ws->send(createErrorResponse(command+"_RSP", RequestStatus::CommandNotExist).to_string(), WsSendOpCode);
      return Response{.rsp = createErrorResponse(command+"_RSP", RequestStatus::CommandNotExist)};
    else if (const auto handlerIt = MsgHandlers.find(itType->second) ; handlerIt == MsgHandlers.cend())
      //ws->send(createErrorResponse(command+"_RSP", RequestStatus::CommandDisabled).to_string(), WsSendOpCode);
      return Response{.rsp = createErrorResponse(command+"_RSP", RequestStatus::CommandDisabled)};
    else
    {
      try
      {
        auto& handler = handlerIt->second;
        return handler(ws, request);
      }
      catch (const std::exception& kex)
      {
        PLOGF << kex.what();
        //ws->send(createErrorResponse(command+"_RSP", RequestStatus::Unknown).to_string(), WsSendOpCode);
        return Response{.rsp = createErrorResponse(command+"_RSP", RequestStatus::Unknown)};
      }
    }
  }


private:
  ndb_always_inline Response sessionNew(KvWebSocket * ws, njson& req)
  {
    if (auto [topLevelValid, rsp] = isValid(sh::NewRsp, ws, req, {{Param::optional("expiry", JsonObject)}}); !topLevelValid)
      return Response{.rsp = std::move(rsp)};
    else
    {
      const auto& cmd = req.at(sh::NewReq);

      SessionDuration duration{SessionDuration::rep{0}};
      
      bool deleteOnExpire = false;

      if (cmd.contains("expiry"))
      {
        auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
        {
          if (cmd.at("duration") < 0)
            return {RequestStatus::ValueSize, "expiry must be >= 0"};
          else
            return {RequestStatus::Ok, ""};
        };

        
        auto [expiryValid, rsp] = isValid(sh::NewRsp, "expiry", ws, req, {{Param::required("duration", JsonUInt)},
                                                                    {Param::required("deleteSession", JsonBool)}}, validate);
        if (!expiryValid)
          return Response{.rsp = std::move(rsp)};
        else
        {
          duration = SessionDuration{cmd.at("expiry").at("duration").as<SessionDuration::rep>()};
          deleteOnExpire = cmd.at("expiry").at("deleteSession").as_bool();
        }
      }

      // here if no expiry set, or expiry is set and it is valid
      return SessionExecutor::newSession(m_sessions, createSessionToken(), duration, deleteOnExpire);
    }
  }


  ndb_always_inline Response sessionEnd(KvWebSocket * ws, njson& json)
  {
    SessionToken token;
    
    if (getSessionToken(ws, sh::EndRsp, json.at(sh::EndReq), token))
      return SessionExecutor::endSession(m_sessions, token);
    else
      return Response{.rsp = createErrorResponse(sh::EndRsp, RequestStatus::SessionNotExist)};
  }

  
  /*
  ndb_always_inline Response sessionOpen(KvWebSocket * ws, njson& json)
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

  
  ndb_always_inline Response sessionInfo(KvWebSocket * ws, njson& json)
  {
    auto& cmd = json.at(sh::InfoReq);

    if (cmd.size() != 1U)
      return Response{.rsp = createErrorResponse(sh::InfoRsp, RequestStatus::CommandSyntax)};
    else
    {
      SessionToken token;
    
      if (getSessionToken(ws, sh::InfoReq, cmd, token))
        return SessionExecutor::sessionInfo(m_sessions, token);
      else
        return Response{.rsp = createErrorResponse(sh::InfoRsp, RequestStatus::SessionNotExist)};
    }
  }
  
  
  ndb_always_inline Response sessionInfoAll(KvWebSocket * ws, njson& json)
  {
    return SessionExecutor::sessionInfoAll(m_sessions);
  }
  
  
  ndb_always_inline Response sessionExists(KvWebSocket * ws, njson& json)
  {
    const auto& cmd = json.at(sh::ExistsReq);

    if (auto [valid, rsp] = isValid(sh::ExistsRsp, ws, json, {{Param::required("tkns", JsonArray)}}); valid)
      return SessionExecutor::sessionExists(m_sessions, cmd.at("tkns"));
    else
      return Response{.rsp = std::move(rsp)};
  }


  ndb_always_inline Response sessionEndAll(KvWebSocket * ws, njson& json)
  {
    return SessionExecutor::sessionEndAll(m_sessions);
  }

  
  ndb_always_inline Response sessionSave(KvWebSocket * ws, njson& json)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      if (cmd.contains("tkns"))
      {
        if (cmd.at("tkns").empty())
          return {RequestStatus::ValueSize, "'tkns' empty"};
        else
        {
          for (const auto& item : cmd.at("tkns").array_range())
          {
            if (!item.is_uint64())
              return {RequestStatus::ValueTypeInvalid, "'tkns' contains invalid token"};
          }
        }
      }

      return {RequestStatus::Ok, ""};
    };

    
    if (!NemesisConfig::persistEnabled(m_config))
      return Response{.rsp = createErrorResponseNoTkn(sh::SaveRsp, RequestStatus::CommandDisabled)};
    else 
    {
      auto [valid, rsp] = isValid(sh::SaveRsp, ws, json, {{Param::required("name", JsonString)}, {Param::optional("tkns", JsonArray)}}, validate);

      if (valid)
        return doSave(sh::SaveReq, sh::SaveRsp, ws, json.at(sh::SaveReq));
      else
        return Response{.rsp = std::move(rsp)};
    }
  } 

  
  ndb_always_inline Response sessionLoad(KvWebSocket * ws, njson& json)
  {
    if (auto [valid, rsp] = isValid(sh::LoadRsp, ws, json, {{Param::required("name", JsonString)}}); !valid)
      return Response{.rsp = std::move(rsp)};
    else
    {
      const auto& loadName = json.at(sh::LoadReq).at("name").as_string();

      if (const auto [valid, msg] = validatePreLoad(loadName, fs::path{NemesisConfig::savePath(m_config)}, true); !valid)
      {
        PLOGE << msg;

        Response response;
        response.rsp[sh::LoadRsp]["st"] = toUnderlying(RequestStatus::LoadError);
        response.rsp[sh::LoadRsp]["name"] = loadName;
        return response;
      }
      else
      {
        // ignore pathsValid here because validatePreLoad() fails if not paths not valid
        const auto [root, md, data, pathsValid] = getLoadPaths(fs::path{NemesisConfig::savePath(m_config)} / loadName);
        return doLoad(loadName, data);
      }
    }
  }

  
private:
  // move to NemesisCommon
  bool getSessionToken(KvWebSocket * ws, const std::string_view queryRspName, const njson& cmdRoot, SessionToken& tkn)
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


  Response doSave (const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& cmd)
  {
    const auto& name = cmd.at("name").as_string();
    const auto rootDirName = std::to_string(KvSaveClock::now().time_since_epoch().count());
    const auto root = fs::path {NemesisConfig::savePath(m_config)} / name / rootDirName;
    const auto metaPath = root / "md";
    const auto dataPath = root / "data";

    RequestStatus status = RequestStatus::Ok;

    std::ofstream metaStream;

    if (!fs::create_directories(metaPath))
      status = RequestStatus::SaveDirWriteFail;
    else if (metaStream.open(metaPath / "md.json", std::ios_base::trunc | std::ios_base::out); !metaStream.is_open())
      status = RequestStatus::SaveDirWriteFail;
        
    if (status != RequestStatus::Ok)
    {
      njson rsp;
      rsp[queryRspName]["name"] = name;
      rsp[queryRspName]["st"] = toUnderlying(status);
      //send(ws, rsp);
     return Response{.rsp = std::move(rsp)};
    }
    else
    {
      auto metaData = createInitialSaveMetaData(name, true);
      
      metaData.dump(metaStream);
      
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
      completeSaveMetaData(metaData, metaDataStatus);
      
      metaStream.seekp(0);
      metaData.dump(metaStream);

      return response;
    }
  }


  Response doLoad (const std::string& loadName, const fs::path& dataSetsRoot)
  {
    PLOGI << "Loading from " << dataSetsRoot;
    
    Response response = SessionExecutor::loadSessions (loadName, m_sessions, dataSetsRoot);  

    PLOGI << "Loading complete";

    return response;
  }

private:
  njson m_config;
  std::shared_ptr<Sessions> m_sessions;
};


}
}
}


#endif
