#ifndef NDB_CORE_KVHANDLERS_H
#define NDB_CORE_KVHANDLERS_H


#include <functional>
#include <tuple>
#include <ankerl/unordered_dense.h>
#include <core/NemesisCommon.h>
#include <core/Persistance.h>
#include <core/NemesisConfig.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvExecutor.h>
#include <core/kv/KvCommandValidate.h>
#include <core/sh/ShExecutor.h>
#include <core/sh/ShSessions.h>


namespace nemesis { namespace core { namespace kv {

using namespace nemesis::core::sh;


/*
KvHandler receives a command:
  - checks if the command exists and is enabled
  - calls a handler function

The class handles commands for when sessions are enabled or disabled. To avoid
separate functions or passing tokens around unneccessarily, the class uses a
template param HaveSessions:

  - if true, m_container is a Sessions object (which contains active sessions, each with their map)
  - if false, m_container is a CacheMap object (a single map for all keys)
  - All KV_ commands use executeKvCommand(), which has overloads for sessions being enabled/disabled

HaveSessions is true:
  - executeKvCommand() gets the session's map and passes the map to callKvHandler()
  - handleKvExecuteResult() sets the session token in the response before sending

HaveSessions is false:
  - Session related functions are disabled or never called
  - executeKvCommand() calls callKvHandler() with the same map (since all keys are stored in a single map)
  - handleKvExecuteResult() just sends the response
*/
template<bool HaveSessions>
class KvHandler
{
public:
  KvHandler(const njson& config, std::shared_ptr<Sessions> sessions) :
    m_config(config),
    m_sessions(sessions)
    
  {
  }
  

private:

  struct Handler
  {
    using Handle = std::function<Response(njson&)>;

    Handler(Handle&& h) : handler(std::move(h))
    {

    }

    Handler(const Handler&) = default;
    Handler(Handler&&) = default;
    Handler& operator= (Handler&&) = default;

    Response operator()(njson& request)
    {
      return handler(request);
    }

    Handle handler;
  };


  using HandlerPmrMap = ankerl::unordered_dense::pmr::map<KvQueryType, Handler>;
  using QueryTypePmrMap = ankerl::unordered_dense::pmr::map<std::string_view, KvQueryType>;


  template<class Alloc>
  auto createHandlers (Alloc& alloc)
  {
    // initialise with 1 bucket and pmr allocator
    HandlerPmrMap h (
    {
      {KvQueryType::KvSet,          Handler{std::bind_front(&KvHandler<HaveSessions>::set,        std::ref(*this))}},
      {KvQueryType::KvSetQ,         Handler{std::bind_front(&KvHandler<HaveSessions>::setQ,       std::ref(*this))}},
      {KvQueryType::KvGet,          Handler{std::bind_front(&KvHandler<HaveSessions>::get,        std::ref(*this))}},
      {KvQueryType::KvAdd,          Handler{std::bind_front(&KvHandler<HaveSessions>::add,        std::ref(*this))}},
      {KvQueryType::KvAddQ,         Handler{std::bind_front(&KvHandler<HaveSessions>::addQ,       std::ref(*this))}},
      {KvQueryType::KvRemove,       Handler{std::bind_front(&KvHandler<HaveSessions>::remove,     std::ref(*this))}},
      {KvQueryType::KvClear,        Handler{std::bind_front(&KvHandler<HaveSessions>::clear,      std::ref(*this))}},
      {KvQueryType::KvCount,        Handler{std::bind_front(&KvHandler<HaveSessions>::count,      std::ref(*this))}},
      {KvQueryType::KvContains,     Handler{std::bind_front(&KvHandler<HaveSessions>::contains,   std::ref(*this))}},
      {KvQueryType::KvFind,         Handler{std::bind_front(&KvHandler<HaveSessions>::find,       std::ref(*this))}},
      {KvQueryType::KvUpdate,       Handler{std::bind_front(&KvHandler<HaveSessions>::update,     std::ref(*this))}},
      {KvQueryType::KvKeys,         Handler{std::bind_front(&KvHandler<HaveSessions>::keys,       std::ref(*this))}},
      {KvQueryType::KvClearSet,     Handler{std::bind_front(&KvHandler<HaveSessions>::clearSet,   std::ref(*this))}},
      {KvQueryType::KvArrayAppend,  Handler{std::bind_front(&KvHandler<HaveSessions>::arrayAppend, std::ref(*this))}}
      
    }, 1, alloc);

    
    if constexpr (!HaveSessions)
    {
      // KV_SAVE/KV_LOAD are only enabled when sessions are disabled
      // SH_SAVE/SH_LOAD are used when sessions are enabled 
      h.emplace(KvQueryType::KvSave,  Handler{std::bind_front(&KvHandler<HaveSessions>::save, std::ref(*this))});
      h.emplace(KvQueryType::KvLoad,  Handler{std::bind_front(&KvHandler<HaveSessions>::load, std::ref(*this))});
    }

    return h;
  }


  template<class Alloc>
  auto createQueryTypeNameMap (Alloc& alloc)
  {
    QueryTypePmrMap map ( 
    {  
      {SetReq,          KvQueryType::KvSet},
      {SetQReq,         KvQueryType::KvSetQ},
      {GetReq,          KvQueryType::KvGet},
      {AddReq,          KvQueryType::KvAdd},
      {AddQReq,         KvQueryType::KvAddQ},
      {RmvReq,          KvQueryType::KvRemove},
      {ClearReq,        KvQueryType::KvClear},
      {CountReq,        KvQueryType::KvCount},
      {ContainsReq,     KvQueryType::KvContains},
      {FindReq,         KvQueryType::KvFind},
      {UpdateReq,       KvQueryType::KvUpdate},
      {KeysReq,         KvQueryType::KvKeys},
      {ClearSetReq,     KvQueryType::KvClearSet},
      {ArrAppendReq,    KvQueryType::KvArrayAppend},
      {SaveReq,         KvQueryType::KvSave},
      {LoadReq,         KvQueryType::KvLoad}
    }, 1, alloc); 

    return map;
  }


public:
   

  Response handle(const std::string_view& command, njson& request)
  {      
    static PmrResource<typename HandlerPmrMap::value_type, 1024U> handlerPmrResource; // TODO buffer size
    static PmrResource<typename HandlerPmrMap::value_type, 1024U> queryTypeNamePmrResource; // TODO buffer size
    static HandlerPmrMap MsgHandlers{createHandlers(handlerPmrResource.getAlloc())}; 
    static QueryTypePmrMap QueryNameToType{createQueryTypeNameMap(queryTypeNamePmrResource.getAlloc())};
    

    if (const auto itType = QueryNameToType.find(command) ; itType == QueryNameToType.cend())
      return Response {.rsp = createErrorResponse(RequestStatus::CommandNotExist)};
    else if (const auto handlerIt = MsgHandlers.find(itType->second) ; handlerIt == MsgHandlers.cend())
      return Response {.rsp = createErrorResponse(RequestStatus::CommandDisabled)};
    else
    {
      try
      {
        auto& handler = handlerIt->second;
        return handler(request);
      }
      catch (const std::exception& kex)
      {
        PLOGF << kex.what() ;
      }

      return Response {.rsp = createErrorResponse(RequestStatus::Unknown)};
    }
  }


  // TODO should this be moved to ShHandler?
  void monitor () requires(HaveSessions)
  {
    SessionExecutor::sessionMonitor(m_sessions);
  }
    

  // Called when loading at startup, so can be loading with sessions enabled or disabled.
  LoadResult internalLoad(const std::string& loadName, const fs::path& dataSetsRoot)
  {
    const auto start = NemesisClock::now();
    
    // call doLoad(), which is used by KV_LOAD, grabbing from the rsp
    const njson rsp = doLoad(loadName, dataSetsRoot);

    LoadResult loadResult;
    loadResult.duration = NemesisClock::now() - start;
    loadResult.status = static_cast<RequestStatus>(rsp[kv::LoadRsp]["st"].as<std::uint64_t>());
    loadResult.nKeys = rsp[kv::LoadRsp]["keys"].as<std::size_t>();
    loadResult.nSessions = 0;
    
    return loadResult;
  }


private:
    
  // move to NemesisCommon
  bool getSessionToken(const std::string_view queryRspName, const njson& cmdRoot, SessionToken& tkn)
    requires(HaveSessions)
  {
    if (cmdRoot.contains("tkn") && cmdRoot.at("tkn").is<SessionToken>())
    {
      tkn = cmdRoot.at("tkn").as<SessionToken>();
      return true;
    }
    else  [[unlikely]]
    {
      return false;
    }
  }


  std::shared_ptr<Sessions> getContainer() requires(HaveSessions)
  {
    return m_sessions;
  }


  CacheMap& getContainer() requires(!HaveSessions)
  {
    return m_map;
  }

 
  std::optional<std::reference_wrapper<Sessions::Session>> getSession (const std::string_view cmdRspName, const njson& cmd, SessionToken& token)
    requires(HaveSessions)
  {
    if (getSessionToken(cmdRspName, cmd, token))
    {
      if (auto sessionOpt = m_sessions->get(token); sessionOpt)
      {
        if (sessionOpt->get().expires)
          m_sessions->updateExpiry(sessionOpt->get());

        return sessionOpt;
      }
    }

    return {};
  }


  template<typename F>
  Response callKvHandler(CacheMap& map, const SessionToken& token, const njson& cmdRoot, F&& handler)
    requires( HaveSessions &&
              std::is_invocable_v<F, CacheMap&, njson&> &&
              std::is_same_v<Response, std::invoke_result_t<F, CacheMap&, njson&>>)
  {
    if (Response response = std::invoke(handler, map, cmdRoot); !response.rsp.empty())
    {
      auto& rspBody = response.rsp.object_range().begin()->value();
      rspBody["tkn"] = token;
      return response;
    }
    else
      return Response{};
  }


  template<typename F>
  Response callKvHandler(CacheMap& map,const njson& cmdRoot, F&& handler)
    requires( !HaveSessions &&
              std::is_invocable_v<F, CacheMap&, njson&> &&
              std::is_same_v<Response, std::invoke_result_t<F, CacheMap&, njson&>>)
  {
    return std::invoke(handler, map, cmdRoot);
  }


  template<typename F>
  Response executeKvCommand(const std::string_view cmdRspName, const njson& cmd, F&& handler)
    requires((HaveSessions && std::is_invocable_v<F, CacheMap&, SessionToken&, njson&>) || std::is_invocable_v<F, CacheMap&, njson&>)
  {
    const auto& cmdRoot = cmd.object_range().cbegin()->value();

    if constexpr (HaveSessions)
    {
      SessionToken token;
      if (auto session = getSession(cmdRspName, cmdRoot, token); session)
        return callKvHandler(session->get().map, token, cmdRoot, std::forward<F>(handler));
      else
        return Response{.rsp = createErrorResponse(cmdRspName, RequestStatus::SessionNotExist)};
    }
    else
      return callKvHandler(m_map, cmdRoot, std::forward<F>(handler));
  }


  ndb_always_inline Response set(njson& request)
  {
    if (auto [valid, rsp] = validateSet(request); !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(SetRsp, request, KvExecutor::set);
  }

  
  ndb_always_inline Response setQ(njson& request)
  {
    if (auto [valid, rsp] = validateSetQ(request); !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(SetQRsp, request, KvExecutor::setQ);
  }

  
  ndb_always_inline Response get(njson& request)
  {
    if (auto [valid, rsp] = validateGet(request); !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(GetRsp, request, KvExecutor::get);
  }

  
  ndb_always_inline Response add(njson& request)
  {
    if (auto [valid, rsp] = validateAdd(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(AddRsp, request, KvExecutor::add);
  }


  ndb_always_inline Response addQ(njson& request)
  {
    if (auto [valid, rsp] = validateAddQ(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(AddQRsp, request, KvExecutor::addQ);
  }


  ndb_always_inline Response remove(njson& request)
  {
    if (auto [valid, rsp] = validateRemove(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(RmvRsp, request, KvExecutor::remove);
  }

  
  ndb_always_inline Response clear(njson& request)
  {
    return executeKvCommand(ClearRsp, request, KvExecutor::clear);
  }


  ndb_always_inline Response count(njson& request)
  {
    return executeKvCommand(CountRsp, request, KvExecutor::count);
  }


  ndb_always_inline Response contains(njson& request)
  {
    if (auto [valid, rsp] = validateContains(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(ContainsRsp, request, KvExecutor::contains);
  }


  ndb_always_inline Response find(njson& request)
  {
    if (auto [valid, rsp] = validateFind(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
    {
      if (!request.at(FindReq).contains("keys"))
        request.at(FindReq)["keys"] = njson::array(); // executor expects "keys"

      return executeKvCommand(FindRsp, request, KvExecutor::find);
    }
  }


  ndb_always_inline Response update(njson& request)
  {
    if (auto [valid, rsp] = validateUpdate(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(UpdateRsp, request, KvExecutor::update);
  }


  ndb_always_inline Response keys(njson& request)
  {
    return executeKvCommand(KeysRsp, request, KvExecutor::keys);
  }


  ndb_always_inline Response clearSet(njson& request)
  {
    if (auto [valid, rsp] = validateClearSet(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(ClearSetRsp, request, KvExecutor::clearSet);
  }
  

  ndb_always_inline Response arrayAppend(njson& request)
  {
    if (auto [valid, rsp] = validateArrayAppend(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(ArrAppendRsp, request, KvExecutor::arrayAppend);
  }

  
  Response save(njson& request)  requires (!HaveSessions)
  {
    if (!NemesisConfig::persistEnabled(m_config))
      return Response{.rsp = createErrorResponseNoTkn(SaveRsp, RequestStatus::CommandDisabled)};
    else
    {
      if (auto [valid, rsp] = kv::validateSave(request) ; !valid)
        return Response{.rsp = std::move(rsp)};
      else
        return doSave(SaveRsp, request.at(SaveReq));
    }
  }


  Response load(njson& request) requires (!HaveSessions)
  {
    if (auto [valid, rsp] = kv::validateLoad(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
    {
      const auto& loadName = request.at(LoadReq).at("name").as_string();

      if (const auto [valid, msg] = validatePreLoad(loadName, fs::path{NemesisConfig::savePath(m_config)}, false); !valid)
      {
        PLOGE << msg;

        Response response;
        response.rsp[LoadRsp]["st"] = toUnderlying(RequestStatus::LoadError);
        response.rsp[LoadRsp]["name"] = loadName;
        return response;
      }
      else
      {
        const auto [root, md, data, pathsValid] = getLoadPaths(fs::path{NemesisConfig::savePath(m_config)} / loadName);        
        return Response {.rsp = doLoad(loadName, data)};
      }
    }
  }

  
  // TODO check load/save , it's confusing after code splitting KvHandler into ShHandler
  Response doSave (const std::string_view queryRspName, njson& cmd) requires (!HaveSessions)
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
      auto metaData = createInitialSaveMetaData(metaStream, name, false);
      
      KvSaveStatus metaDataStatus = KvSaveStatus::Pending;
      Response response;

      try
      {
        response = KvExecutor::saveKv(m_map, dataPath, name);
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
    
    Response response = KvExecutor::loadKv (loadName, getContainer(), dataSetsRoot);

    PLOGI << "Loading complete";

    return response.rsp;
  }


private:
  njson m_config;
  std::shared_ptr<Sessions> m_sessions;
  CacheMap m_map;
};

}
}
}

#endif
