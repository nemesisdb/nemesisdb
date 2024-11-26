#ifndef NDB_CORE_KVHANDLERS_H
#define NDB_CORE_KVHANDLERS_H


#include <functional>
#include <vector>
#include <tuple>
#include <latch>
#include <ranges>
#include <ankerl/unordered_dense.h>
#include <core/NemesisCommon.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvExecutor.h>
#include <core/kv/ShExecutor.h>
#include <core/kv/KvSessions.h>


namespace nemesis { namespace core { namespace kv {

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
    using Handle = std::function<void(KvWebSocket *, njson&)>;

    Handler(Handle&& h) : handler(std::move(h))
    {

    }

    Handler(const Handler&) = default;
    Handler(Handler&&) = default;
    Handler& operator= (Handler&&) = default;

    void operator()(KvWebSocket * ws, njson& request)
    {
      handler(ws, request);
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
      // KV_SAVE and KV_LOAD are only enabled when sessions are disabled, when sessions are enabled SH_SAVE/SH_LOAD are used
      h.emplace(KvQueryType::KvSave,  Handler{std::bind_front(&KvHandler<HaveSessions>::kvSave, std::ref(*this))});
      h.emplace(KvQueryType::KvLoad,  Handler{std::bind_front(&KvHandler<HaveSessions>::kvLoad, std::ref(*this))});
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
   

  RequestStatus handle(KvWebSocket * ws, const std::string_view& command, njson& request)
  {      
    static PmrResource<typename HandlerPmrMap::value_type, 1024U> handlerPmrResource; // TODO buffer size
    static PmrResource<typename HandlerPmrMap::value_type, 1024U> queryTypeNamePmrResource; // TODO buffer size
    static HandlerPmrMap MsgHandlers{createHandlers(handlerPmrResource.getAlloc())}; 
    static QueryTypePmrMap QueryNameToType{createQueryTypeNameMap(queryTypeNamePmrResource.getAlloc())};
    

    RequestStatus status = RequestStatus::Ok;
    
    if (const auto itType = QueryNameToType.find(command) ; itType == QueryNameToType.cend())
      status = RequestStatus::CommandNotExist;
    else if (const auto handlerIt = MsgHandlers.find(itType->second) ; handlerIt == MsgHandlers.cend())
      status = RequestStatus::CommandDisabled;
    else
    {
      try
      {
        auto& handler = handlerIt->second;
        handler(ws, request);
      }
      catch (const std::exception& kex)
      {
        PLOGF << kex.what() ;
        status = RequestStatus::Unknown;
      }
    }

    return status;
  }


  // TODO should this be moved to ShHandler?
  void monitor () requires(HaveSessions)
  {
    SessionExecutor::sessionMonitor(m_sessions);
  }
    

  LoadResult internalLoad(const std::string& loadName, const fs::path& dataSetsRoot)
  {
    const auto start = NemesisClock::now();
    
    // hijack the response and report to caller rather than sending response over WebSocket
    const njson rsp = doLoad(loadName, dataSetsRoot);

    LoadResult loadResult;
    loadResult.duration = NemesisClock::now() - start;
    loadResult.status = static_cast<RequestStatus>(rsp["SH_LOAD_RSP"]["st"].as<std::uint64_t>());
    loadResult.nSessions = rsp["SH_LOAD_RSP"]["sessions"].as<std::size_t>(); // this is 0 when loading RawKv (no sessions)
    loadResult.nKeys = rsp["SH_LOAD_RSP"]["keys"].as<std::size_t>();

    return loadResult;
  }


  // This can be called when we receive a KV_LOAD, SH_LOAD or when loading on startup 
  njson doLoad (const std::string& loadName, const fs::path& dataSetsRoot)
  {
    PLOGI << "Loading from " << dataSetsRoot;
    
    njson rsp;

    if constexpr (HaveSessions)
      rsp = SessionExecutor::loadSessions (loadName, getContainer(), dataSetsRoot);   
    else
      rsp = KvExecutor::loadKv (loadName, getContainer(), dataSetsRoot);   

    PLOGI << "Loading complete";

    return rsp;
  }


private:
    
  bool getSessionToken(KvWebSocket * ws, const std::string_view queryRspName, const njson& cmdRoot, SessionToken& tkn)
    requires(HaveSessions)
  {
    if (cmdRoot.contains("tkn") && cmdRoot.at("tkn").is<SessionToken>())
    {
      tkn = cmdRoot.at("tkn").as<SessionToken>();
      return true;
    }
    else
    {
      send(ws, createErrorResponse(queryRspName, RequestStatus::SessionTokenInvalid));
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

 
  std::optional<std::reference_wrapper<Sessions::Session>> getSession (KvWebSocket * ws, const std::string_view cmdRspName, const njson& cmd, SessionToken& token)
    requires(HaveSessions)
  {
    if (getSessionToken(ws, cmdRspName, cmd, token))
    {
      if (auto sessionOpt = m_sessions->get(token); sessionOpt)
      {
        if (sessionOpt->get().expires)
          m_sessions->updateExpiry(sessionOpt->get());

        return sessionOpt;
      }
      else
        send(ws, createErrorResponse(cmdRspName, RequestStatus::SessionNotExist, token));
    }

    return {};
  }


  template<typename F>
  void callKvHandler(KvWebSocket * ws, CacheMap& map, const SessionToken& token, const njson& cmdRoot, F&& handler)
    requires(HaveSessions && std::is_invocable_v<F, CacheMap&, njson&>)
  {
    auto rsp = std::invoke(handler, map, cmdRoot);
    if (!rsp.empty())
    {
      auto& rspBody = rsp.object_range().begin()->value();
      rspBody["tkn"] = token;

      send(ws, rsp);
    } 
  }


  template<typename F>
  void callKvHandler(KvWebSocket * ws, CacheMap& map,const njson& cmdRoot, F&& handler)
    requires(!HaveSessions && std::is_invocable_v<F, CacheMap&, njson&>)
  {
    const auto rsp = std::invoke(handler, map, cmdRoot);
    if (!rsp.empty())
      send(ws, rsp);
  }


  template<typename F>
  void executeKvCommand(const std::string_view cmdRspName, KvWebSocket * ws, const njson& cmd, F&& handler)
    requires((HaveSessions && std::is_invocable_v<F, CacheMap&, SessionToken&, njson&>) || std::is_invocable_v<F, CacheMap&, njson&>)
  {
    const auto& cmdRoot = cmd.object_range().cbegin()->value();

    if constexpr (HaveSessions)
    {
      SessionToken token;
      if (auto session = getSession(ws, cmdRspName, cmdRoot, token); session)
        callKvHandler(ws, session->get().map, token, cmdRoot, std::forward<F>(handler));
    }
    else
      callKvHandler(ws, m_map, cmdRoot, std::forward<F>(handler));
  }


  ndb_always_inline void set(KvWebSocket * ws, njson& request)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };
    
    if (isValid(SetRsp, ws, request, {{Param::required("keys", JsonObject)}}, validate))
      executeKvCommand(SetRsp, ws, request, KvExecutor::set);
  }

  
  ndb_always_inline void setQ(KvWebSocket * ws, njson& request)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };

    if (isValid(SetQRsp, ws, request, {{Param::required("keys", JsonObject)}}, validate))
      executeKvCommand(SetQRsp, ws, request, KvExecutor::setQ);
  }

  
  ndb_always_inline void get(KvWebSocket * ws, njson& request)
  {
    if (isValid(GetReq, ws, request, {{Param::required("keys", JsonArray)}}))
      executeKvCommand(GetRsp, ws, request, KvExecutor::get);
  }

  
  ndb_always_inline void add(KvWebSocket * ws, njson& json)
  {
    if (isValid(AddRsp, ws, json, {{Param::required("keys", JsonObject)}}))
      executeKvCommand(AddRsp, ws, json, KvExecutor::add);
  }


  ndb_always_inline void addQ(KvWebSocket * ws, njson& json)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };

    if (isValid(AddQRsp, ws, json, {{Param::required("keys", JsonObject)}}, validate))
      executeKvCommand(AddQRsp, ws, json, KvExecutor::addQ);
  }


  ndb_always_inline void remove(KvWebSocket * ws, njson& json)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };

    if (isValid(RmvRsp, ws, json, {{Param::required("keys", JsonArray)}}, validate))
      executeKvCommand(RmvRsp, ws, json, KvExecutor::remove);
  }

  
  ndb_always_inline void clear(KvWebSocket * ws, njson& json)
  {
    executeKvCommand(ClearRsp, ws, json, KvExecutor::clear);
  }


  ndb_always_inline void count(KvWebSocket * ws, njson& json)
  {
    executeKvCommand(CountRsp, ws, json, KvExecutor::count);
  }


  ndb_always_inline void contains(KvWebSocket * ws, njson& json)
  {
    if (isValid(ContainsRsp, ws, json, {{Param::required("keys", JsonArray)}}))
      executeKvCommand(ContainsRsp, ws, json, KvExecutor::contains);
  }


  ndb_always_inline void find(KvWebSocket * ws, njson& json)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      if (cmd.at("rsp") != "paths" && cmd.at("rsp") != "kv" && cmd.at("rsp") != "keys")
        return {RequestStatus::CommandSyntax, "'rsp' invalid value"};
      else if (const auto& path = cmd.at("path").as_string() ; path.empty())
        return {RequestStatus::ValueSize, "'path' is empty"};

      return {RequestStatus::Ok, ""};
    };

    if (isValid(FindRsp, ws, json, {{Param::required("path", JsonString)}, {Param::required("rsp", JsonString)}, {Param::optional("keys", JsonArray)}}, validate))
    {
      auto& cmd = json.at(FindReq);

      if (!cmd.contains("keys"))
        cmd["keys"] = njson::array(); // executor expects "keys"

      executeKvCommand(FindRsp, ws, json, KvExecutor::find);
    }
  }


  ndb_always_inline void update(KvWebSocket * ws, njson& json)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      // 'value' can be any valid JSON type, so just check it's present here
      if (!cmd.contains("value"))
        return {RequestStatus::ParamMissing, "Missing parameter"};
      
      return {RequestStatus::Ok, ""};
    };

    if (isValid(UpdateRsp, ws, json, {{Param::required("key", JsonString)}, {Param::required("path", JsonString)}}, validate))
      executeKvCommand(UpdateRsp, ws, json, KvExecutor::update);
  }


  ndb_always_inline void keys(KvWebSocket * ws, njson& json)
  {
    executeKvCommand(KeysRsp, ws, json, KvExecutor::keys);
  }


  ndb_always_inline void clearSet(KvWebSocket * ws, njson& json)
  {
    if (isValid(ClearSetRsp, ws, json, {{Param::required("keys", JsonObject)}}))
      executeKvCommand(ClearSetRsp, ws, json, KvExecutor::clearSet);
  }
  

  ndb_always_inline void arrayAppend(KvWebSocket * ws, njson& json)
  {
    if (isValid(ArrAppendRsp, ws, json, {{Param::required("key", JsonString)}, {Param::required("items", JsonArray)}, {Param::optional("name", JsonString)}}))
      executeKvCommand(ArrAppendRsp, ws, json, KvExecutor::arrayAppend);
  }


  void kvSave(KvWebSocket * ws, njson& json)  requires (!HaveSessions)
  {
    if (!NemesisConfig::persistEnabled(m_config))
      send(ws, createErrorResponseNoTkn(SaveRsp, RequestStatus::CommandDisabled));
    else
    {
      auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
      {
        if (cmd.at("name").empty())
          return {RequestStatus::ValueSize, "name empty"};
        else
          return {RequestStatus::Ok, ""};
      };
      
      if (isValid(SaveRsp, ws, json, {{Param::required("name", JsonString)}}, validate))
      {
        doSave(SaveRsp, ws, json.at(SaveReq));
      }
    }
  }


  void kvLoad(KvWebSocket * ws, njson& json) requires (!HaveSessions)
  {
    if (isValid(LoadRsp, ws, json, {{Param::required("name", JsonString)}}))
    {
      const auto& loadName = json.at(LoadReq).at("name").as_string();

      if (const auto [valid, msg] = validatePreLoad(loadName, fs::path{NemesisConfig::savePath(m_config)}, false); !valid)
      {
        PLOGE << msg;

        njson rsp;
        rsp[LoadRsp]["st"] = toUnderlying(RequestStatus::LoadError);
        rsp[LoadRsp]["name"] = loadName;
        send (ws, rsp);
      }
      else
      {
        const auto [root, md, data, pathsValid] = getLoadPaths(fs::path{NemesisConfig::savePath(m_config)} / loadName);
        send(ws, doLoad(loadName, data));
      }
    }
  }

  
  void doSave (const std::string_view queryRspName, KvWebSocket * ws, njson& cmd) requires (!HaveSessions)
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
      send(ws, rsp);
    }
    else
    {
      auto metaData = createInitialSaveMetaData(name, false);
      
      metaData.dump(metaStream);
      
      KvSaveStatus metaDataStatus = KvSaveStatus::Pending;
      njson rsp;

      try
      {
        rsp = KvExecutor::saveKv(m_map, dataPath, name);
        metaDataStatus = KvSaveStatus::Complete;
      }
      catch(const std::exception& e)
      {
        PLOGE << e.what();
        metaDataStatus = KvSaveStatus::Error;
      }
      
      // update metdata
      completeSaveMetaData(metaData, metaDataStatus);
      
      metaStream.seekp(0);
      metaData.dump(metaStream);

      send(ws, rsp);
    }
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
