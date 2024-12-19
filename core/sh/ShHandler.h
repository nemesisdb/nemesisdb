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
#include <core/sh/ShExecutor.h>
#include <core/sh/ShCommands.h>
#include <core/sh/ShCommandValidate.h>
#include <core/kv/KvExecutor.h>
#include <core/kv/KvCommandValidate.h>


namespace nemesis { namespace sh {

namespace kv = nemesis::kv;


class ShHandler
{  
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
      //
      {ShQueryType::Set,        Handler{std::bind_front(&ShHandler::set,        std::ref(*this))}},
      {ShQueryType::Get,        Handler{std::bind_front(&ShHandler::get,        std::ref(*this))}},
      {ShQueryType::Count,      Handler{std::bind_front(&ShHandler::count,      std::ref(*this))}},
      {ShQueryType::Add,        Handler{std::bind_front(&ShHandler::add,        std::ref(*this))}},
      {ShQueryType::Rmv,        Handler{std::bind_front(&ShHandler::remove,     std::ref(*this))}},
      {ShQueryType::Clear,      Handler{std::bind_front(&ShHandler::clear,      std::ref(*this))}},
      {ShQueryType::Contains,   Handler{std::bind_front(&ShHandler::contains,   std::ref(*this))}},
      {ShQueryType::Keys,       Handler{std::bind_front(&ShHandler::keys,       std::ref(*this))}},
      {ShQueryType::ClearSet,   Handler{std::bind_front(&ShHandler::clearSet,   std::ref(*this))}}      
    }, 1, alloc);

    return h;
  }


  template<class Alloc>
  auto createQueryTypeNameMap (Alloc& alloc)
  {
    QueryTypePmrMap map ( 
    { 
      {cmds::NewReq,        ShQueryType::ShNew},
      {cmds::EndReq,        ShQueryType::ShEnd},      
      {cmds::InfoReq,       ShQueryType::ShInfo},
      {cmds::InfoAllReq,    ShQueryType::ShInfoAll},
      {cmds::SaveReq,       ShQueryType::ShSave},
      {cmds::LoadReq,       ShQueryType::ShLoad},
      {cmds::EndAllReq,     ShQueryType::ShEndAll},
      {cmds::ExistsReq,     ShQueryType::ShExists},
      //{"SH_OPEN",       ShQueryType::ShOpen},
      {cmds::SetReq,        ShQueryType::Set},
      {cmds::GetReq,        ShQueryType::Get},
      {cmds::CountReq,      ShQueryType::Count},
      {cmds::AddReq,        ShQueryType::Add},
      {cmds::RmvReq,        ShQueryType::Rmv},
      {cmds::ClearReq,      ShQueryType::Clear},
      {cmds::ContainsReq,   ShQueryType::Contains},
      {cmds::KeysReq,       ShQueryType::Keys},
      {cmds::ClearSetReq,   ShQueryType::ClearSet}
    }, 1, alloc); 

    return map;
  }



public:
  ShHandler(std::shared_ptr<Sessions> sessions) : m_settings(Settings::get()), m_sessions(sessions)
    
  {
  }


  ndb_always_inline Response handle(const std::string& command, njson& request)
  {
    static PmrResource<typename HandlerPmrMap::value_type, 1024U> handlerPmrResource; // TODO buffer size
    static PmrResource<typename HandlerPmrMap::value_type, 1024U> queryTypeNamePmrResource; // TODO buffer size
    static const HandlerPmrMap MsgHandlers{createHandlers(handlerPmrResource.getAlloc())}; 
    static const QueryTypePmrMap QueryNameToType{createQueryTypeNameMap(queryTypeNamePmrResource.getAlloc())};
    
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


  void monitor ()
  {
    SessionExecutor::sessionMonitor(m_sessions);
  }


private:

  /* TODO decide plan for shared sessions
  ndb_always_inline Response sessionOpen(njson& json)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      if (cmd.at("name").empty())
        return {RequestStatus::ValueSize, "Session name empty"};

      return {RequestStatus::Ok, ""};
    };
    

    if (const auto status = isValid(queryRspName, ws, json, {{Param::required("name", JsonString)}}, validate))
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


  ndb_always_inline Response sessionNew(njson& req)
  {
    if (const auto status = validateNew(req); status != RequestStatus::Ok)
    {
      return Response{.rsp = createErrorResponse(cmds::NewRsp, status)};
    }
    else
    {
      Sessions::ExpireInfo expiryInfo{};  // defaults all 0

      if (req.at(cmds::NewReq).contains("expiry"))
      {
        const auto& expiry = req.at(cmds::NewReq).at("expiry");

        expiryInfo.duration = SessionDuration{expiry.at("duration").as<SessionDuration::rep>()};
        expiryInfo.deleteOnExpire = expiry.get_value_or<bool>("deleteSession", false);
        expiryInfo.extendOnSetAdd = expiry.get_value_or<bool>("extendOnSetAdd", false);
        expiryInfo.extendOnGet = expiry.get_value_or<bool>("extendOnGet", false);
      }
      
      return SessionExecutor::newSession(m_sessions, createSessionToken(), expiryInfo);
    }
  }


  ndb_always_inline Response sessionEnd(njson& req)
  {
    SessionToken token;
    
    if (getSessionToken(req.at(cmds::EndReq), token))
      return SessionExecutor::endSession(m_sessions, token);
    else
      return Response{.rsp = createErrorResponse(cmds::EndRsp, RequestStatus::SessionNotExist)};
  }


  ndb_always_inline Response sessionInfo(njson& req)
  {
    auto& cmd = req.at(cmds::InfoReq);

    if (cmd.size() != 1U)
      return Response{.rsp = createErrorResponse(cmds::InfoRsp, RequestStatus::CommandSyntax)};
    else
    {
      SessionToken token;
    
      if (getSessionToken(cmd, token))
        return SessionExecutor::sessionInfo(m_sessions, token);
      else
        return Response{.rsp = createErrorResponse(cmds::InfoRsp, RequestStatus::SessionNotExist)};
    }
  }
  
  
  ndb_always_inline Response sessionInfoAll(njson& req)
  {
    return SessionExecutor::sessionInfoAll(m_sessions);
  }
  
  
  ndb_always_inline Response sessionExists(njson& req)
  {
    if (const auto status = validateExists(req); status != RequestStatus::Ok)
      return Response{.rsp = createErrorResponse(cmds::ExistsRsp, status)};
    else
      return SessionExecutor::sessionExists(m_sessions, req.at(cmds::ExistsReq).at("tkns"));
  }


  ndb_always_inline Response sessionEndAll(njson& req)
  {
    return SessionExecutor::sessionEndAll(m_sessions);
  }

  
  ndb_always_inline Response sessionSave(njson& req)
  {    
    // TODO  add this check in handle() so the error can be caught earlier
    if (!m_settings.persistEnabled)
      return Response{.rsp = createErrorResponse(cmds::SaveRsp, RequestStatus::CommandDisabled)};
    else 
    {
      if (const auto status = validateSave(req); status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(cmds::SaveRsp, status)};
      else
        return doSave(cmds::SaveReq, cmds::SaveRsp, req.at(cmds::SaveReq));
    }
  } 

  
  ndb_always_inline Response sessionLoad(njson& req)
  {
    if (const auto status = validateLoad(req); status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(sh::LoadRsp, status)};
    else
    {
      const auto& loadName = req.at(sh::LoadReq).at("name").as_string();

      if (const PreLoadInfo info = validatePreLoad(loadName, fs::path{m_settings.persistPath}); !info.valid)
      {
        PLOGE << info.err;

        Response response;
        response.rsp[sh::LoadRsp]["st"] = toUnderlying(RequestStatus::LoadError);
        response.rsp[sh::LoadRsp]["name"] = loadName;
        return response;
      }
      else
      {
        // can ignore pathsValid here because if paths are invalid, validatePreLoad() returns false
        const auto [root, md, data, pathsValid] = getLoadPaths(fs::path{m_settings.persistPath} / loadName);
        return Response{.rsp = doLoad(loadName, data)};
      }
    }    
  }

 
  template<typename F>
  Response executeKvCommand(const std::string_view cmdRspName, const njson& cmd, F&& handler)
    requires(std::is_invocable_v<F, CacheMap&, njson&>)
  {
    const auto& cmdRoot = cmd.object_range().cbegin()->value();

    SessionToken token;

    if (auto sessionOpt = getSession(cmdRoot, token); sessionOpt)
      return std::invoke(handler, sessionOpt->get().map, cmdRoot);
    else
      return Response{.rsp = createErrorResponse(cmdRspName, RequestStatus::SessionNotExist)};
  }


  template<typename F>
  Response executeExtendableKvCommand(const std::string_view cmdRspName, const njson& request, const ShQueryType queryType, F&& handler)
    requires(std::is_invocable_v<F, CacheMap&, njson&>)
  {
    const auto& cmdRoot = request.object_range().cbegin()->value();

    SessionToken token;

    if (auto sessionOpt = getExtendableSession(cmdRoot, token, queryType); sessionOpt)
      return std::invoke(handler, sessionOpt->get().map, cmdRoot);
    else
      return Response{.rsp = createErrorResponse(cmdRspName, RequestStatus::SessionNotExist)};
  }


  ndb_always_inline Response set(njson& request)
  {
    if (const auto status = kv::validateSet(cmds::SetReq, cmds::SetRsp, request); status != RequestStatus::Ok)
      return Response{.rsp = createErrorResponse(cmds::SetRsp, status)};
    else
      return executeExtendableKvCommand(cmds::SetRsp, request, ShQueryType::Set, kv::KvExecutor<true>::set);
  }


  ndb_always_inline Response get(njson& request)
  {
    if (const auto status = kv::validateGet(cmds::GetReq, cmds::GetRsp, request); status != RequestStatus::Ok)
      return Response{.rsp = createErrorResponse(cmds::GetRsp, status)};
    else
      return executeExtendableKvCommand(cmds::GetRsp, request, ShQueryType::Get, kv::KvExecutor<true>::get);
  }


  ndb_always_inline Response count(njson& request)
  {
    return executeKvCommand(cmds::CountRsp, request, kv::KvExecutor<true>::count);
  }


  ndb_always_inline Response add(njson& request)
  {
    if (const auto status = kv::validateAdd(cmds::AddReq, cmds::AddRsp, request); status != RequestStatus::Ok)
      return Response{.rsp = createErrorResponse(cmds::AddRsp, status)};
    else
      return executeExtendableKvCommand(cmds::AddRsp, request, ShQueryType::Add, kv::KvExecutor<true>::add);
  }


  ndb_always_inline Response remove(njson& request)
  {
    if (const auto status = kv::validateRemove(cmds::RmvReq, cmds::RmvRsp, request); status != RequestStatus::Ok)
      return Response{.rsp = createErrorResponse(cmds::RmvRsp, status)};
    else
      return executeKvCommand(cmds::RmvRsp, request, kv::KvExecutor<true>::remove);
  }


  ndb_always_inline Response clear(njson& request)
  {
    return executeKvCommand(cmds::ClearRsp, request, kv::KvExecutor<true>::clear);
  }


  ndb_always_inline Response contains(njson& request)
  {
    if (const auto status = kv::validateContains(cmds::ContainsReq, cmds::ContainsRsp, request); status != RequestStatus::Ok)
      return Response{.rsp = createErrorResponse(cmds::ContainsRsp, status)};
    else
      return executeKvCommand(cmds::ContainsRsp, request, kv::KvExecutor<true>::contains);
  }


  ndb_always_inline Response keys(njson& request)
  {
    return executeKvCommand(cmds::KeysRsp, request, kv::KvExecutor<true>::keys);
  }


  ndb_always_inline Response clearSet(njson& request)
  {
    if (const auto status = kv::validateClearSet(cmds::ClearSetReq, cmds::ClearSetRsp, request); status != RequestStatus::Ok)
      return Response{.rsp = createErrorResponse(cmds::ClearSetRsp, status)};
    else
      return executeKvCommand(cmds::ClearSetRsp, request, kv::KvExecutor<true>::clearSet);
  }


private:
    
  bool getSessionToken(const njson& cmdRoot, SessionToken& tkn)
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


  ndb_always_inline std::optional<std::reference_wrapper<Sessions::Session>> getExtendableSession (const njson& cmd, SessionToken& token, const ShQueryType extendableQuery)
  {
    if (auto sessionOpt = getSession(cmd, token); sessionOpt)
    {
      bool extend = false;
      switch (extendableQuery)
      {
        case ShQueryType::Add:
        case ShQueryType::Set:
          extend = sessionOpt->get().expireInfo.extendOnSetAdd;
        break;

        case ShQueryType::Get:
          extend = sessionOpt->get().expireInfo.extendOnGet;
        break;

        default:
          // ignore
        break;
      }

      if (extend)
        m_sessions->updateExpiry(sessionOpt->get());

      return sessionOpt;
    }

    return {};
  }


  ndb_always_inline std::optional<std::reference_wrapper<Sessions::Session>> getSession (const njson& cmd, SessionToken& token)
  {
    if (getSessionToken(cmd, token))
    {
      if (auto sessionOpt = m_sessions->get(token); sessionOpt) [[likely]]
        return sessionOpt;
    }

    return {};
  }



  Response doSave (const std::string_view queryName, const std::string_view queryRspName, njson& cmd)
  {
    const auto& name = cmd.at("name").as_string();
    const auto dataSetDir = std::to_string(KvSaveClock::now().time_since_epoch().count());
    const auto root = fs::path {m_settings.persistPath} / name / dataSetDir;
    const auto metaPath = root / "md";
    const auto dataPath = root / "data";
    

    if (auto [preparedStatus, metaStream] = prepareSave(cmd, root); preparedStatus != RequestStatus::Ok)
    {
      njson rsp;
      rsp[queryRspName]["name"] = name;
      rsp[queryRspName]["st"] = toUnderlying(preparedStatus);
      return Response{.rsp = createErrorResponse(queryRspName, preparedStatus)};
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
  const Settings& m_settings;
  std::shared_ptr<Sessions> m_sessions;
};


}
}

#endif
