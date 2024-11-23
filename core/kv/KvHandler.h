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

  // KvHandler(const njson& config) requires (HaveSessions) :
  //   m_config(config),
  //   m_container() // Sessions sets the initial bucket count
    
  // {
  // }


  // KvHandler(const njson& config) requires (!HaveSessions) :
  //   m_config(config),
  //   m_container(50'000) // Adds ~2Mb to initial memory use
    
  // {
  // }
  

private:

  using Handle = std::function<void(const std::string_view, const std::string_view, KvWebSocket *, njson&)>;

  struct Handler
  {
    Handler(Handle&& h, const std::string_view name, const std::string_view rsp) :
      handler(std::move(h)),
      qryName(name),
      qryRspName(rsp)
    {

    }

    Handler(const Handler&) = default;
    Handler(Handler&&) = default;
    Handler& operator= (Handler&&) = default;

    void operator()(KvWebSocket * ws, njson& request)
    {
      handler(qryName, qryRspName, ws, request);
    }

    Handle handler;
    std::string_view qryName;
    std::string_view qryRspName;
  };


  using HandlerPmrMap = ankerl::unordered_dense::pmr::map<KvQueryType, Handler>;
  using QueryTypePmrMap = ankerl::unordered_dense::pmr::map<std::string_view, KvQueryType>;


  template<class Alloc>
  auto createHandlers (Alloc& alloc)
  {
    // initialise with 1 bucket and pmr allocator
    HandlerPmrMap h (
    {
      {KvQueryType::KvSet,          Handler{std::bind_front(&KvHandler<HaveSessions>::set,        std::ref(*this)), "KV_SET",   "KV_SET_RSP"}},
      {KvQueryType::KvSetQ,         Handler{std::bind_front(&KvHandler<HaveSessions>::setQ,       std::ref(*this)), "KV_SETQ",  "KV_SETQ_RSP"}},
      {KvQueryType::KvGet,          Handler{std::bind_front(&KvHandler<HaveSessions>::get,        std::ref(*this)), "KV_GET",   "KV_GET_RSP"}},
      {KvQueryType::KvAdd,          Handler{std::bind_front(&KvHandler<HaveSessions>::add,        std::ref(*this)), "KV_ADD",   "KV_ADD_RSP"}},
      {KvQueryType::KvAddQ,         Handler{std::bind_front(&KvHandler<HaveSessions>::addQ,       std::ref(*this)), "KV_ADDQ",  "KV_ADDQ_RSP"}},
      {KvQueryType::KvRemove,       Handler{std::bind_front(&KvHandler<HaveSessions>::remove,     std::ref(*this)), "KV_RMV",   "KV_RMV_RSP"}},
      {KvQueryType::KvClear,        Handler{std::bind_front(&KvHandler<HaveSessions>::clear,      std::ref(*this)), "KV_CLEAR", "KV_CLEAR_RSP"}},
      {KvQueryType::KvCount,        Handler{std::bind_front(&KvHandler<HaveSessions>::count,      std::ref(*this)), "KV_COUNT", "KV_COUNT_RSP"}},
      {KvQueryType::KvContains,     Handler{std::bind_front(&KvHandler<HaveSessions>::contains,   std::ref(*this)), "KV_CONTAINS",  "KV_CONTAINS_RSP"}},
      {KvQueryType::KvFind,         Handler{std::bind_front(&KvHandler<HaveSessions>::find,       std::ref(*this)), "KV_FIND",      "KV_FIND_RSP"}},
      {KvQueryType::KvUpdate,       Handler{std::bind_front(&KvHandler<HaveSessions>::update,     std::ref(*this)), "KV_UPDATE",    "KV_UPDATE_RSP"}},
      {KvQueryType::KvKeys,         Handler{std::bind_front(&KvHandler<HaveSessions>::keys,       std::ref(*this)), "KV_KEYS",      "KV_KEYS_RSP"}},
      {KvQueryType::KvClearSet,     Handler{std::bind_front(&KvHandler<HaveSessions>::clearSet,   std::ref(*this)), "KV_CLEAR_SET", "KV_CLEAR_SET_RSP"}},
      {KvQueryType::KvArrayAppend,  Handler{std::bind_front(&KvHandler<HaveSessions>::arrayAppend, std::ref(*this)), "KV_ARR_APPEND", "KV_ARR_APPEND_RSP"}}
      
    }, 1, alloc);

    
    if constexpr (!HaveSessions)
    {
      // KV_SAVE and KV_LOAD are only enabled when sessions are disabled, when sessions are enabled SH_SAVE/SH_LOAD are used
      h.emplace(KvQueryType::KvSave,  Handler{std::bind_front(&KvHandler<HaveSessions>::kvSave, std::ref(*this)), "KV_SAVE",  "KV_SAVE_RSP"});
      h.emplace(KvQueryType::KvLoad,  Handler{std::bind_front(&KvHandler<HaveSessions>::kvLoad, std::ref(*this)), "KV_LOAD",  "KV_LOAD_RSP"});
    }

    return h;
  }


  template<class Alloc>
  auto createQueryTypeNameMap (Alloc& alloc)
  {
    QueryTypePmrMap map ( 
    {  
      {"KV_SET",          KvQueryType::KvSet},
      {"KV_SETQ",         KvQueryType::KvSetQ},
      {"KV_GET",          KvQueryType::KvGet},
      {"KV_ADD",          KvQueryType::KvAdd},
      {"KV_ADDQ",         KvQueryType::KvAddQ},
      {"KV_RMV",          KvQueryType::KvRemove},
      {"KV_CLEAR",        KvQueryType::KvClear},
      {"KV_COUNT",        KvQueryType::KvCount},
      {"KV_CONTAINS",     KvQueryType::KvContains},
      {"KV_FIND",         KvQueryType::KvFind},
      {"KV_UPDATE",       KvQueryType::KvUpdate},
      {"KV_KEYS",         KvQueryType::KvKeys},
      {"KV_CLEAR_SET",    KvQueryType::KvClearSet},
      {"KV_ARR_APPEND",   KvQueryType::KvArrayAppend},
      {"KV_SAVE",         KvQueryType::KvSave},
      {"KV_LOAD",         KvQueryType::KvLoad}
    }, 1, alloc); 

    return map;
  }


public:
  
  template<typename T, std::size_t Size>
  struct PmrResource
  {
    PmrResource() : mbr(std::data(buffer), std::size(buffer)), alloc(&mbr)
    {

    }

    std::pmr::polymorphic_allocator<T>& getAlloc ()
    {
      return alloc;
    }

    private:
      std::array<std::byte, Size> buffer;
      std::pmr::monotonic_buffer_resource mbr;
      std::pmr::polymorphic_allocator<T> alloc;
  };
  

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
  

  void load(const std::string& loadName, const fs::path& dataSetsRoot, KvWebSocket * ws)
  {
    send (ws, doLoad(loadName, dataSetsRoot));
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

 
  template<typename Json>
  std::optional<std::reference_wrapper<Sessions::Session>> getSession (KvWebSocket * ws, const std::string_view cmdRspName, const Json& cmd, SessionToken& token)
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


  template<typename Json>
  void handleKvExecuteResult(KvWebSocket * ws, const Json& rsp)
    requires(!HaveSessions)
  {
    if (!rsp.empty())
      send(ws, rsp);
  }


  template<typename Json>
  void handleKvExecuteResult(KvWebSocket * ws, Json& rsp, const SessionToken& tkn)
    requires(HaveSessions)
  {
    if (!rsp.empty())
    {
      //setToken(rsp.object_range().begin()->value(), tkn);
      auto& rspBody = rsp.object_range().begin()->value();
      rspBody["tkn"] = tkn;

      send(ws, rsp);
    }      
  }


  template<typename Json, typename F>
  void callKvHandler(KvWebSocket * ws, CacheMap& map, const SessionToken& token, const Json& cmdRoot, F&& handler)
    requires(HaveSessions && std::is_invocable_v<F, CacheMap&, Json&>)
  {
    auto rsp = std::invoke(handler, map, cmdRoot);
    handleKvExecuteResult(ws, rsp, token);
  }


  template<typename Json, typename F>
  void callKvHandler(KvWebSocket * ws, CacheMap& map,const Json& cmdRoot, F&& handler)
    requires(!HaveSessions && std::is_invocable_v<F, CacheMap&, Json&>)
  {
    const auto rsp = std::invoke(handler, map, cmdRoot);
    handleKvExecuteResult(ws, rsp);
  }


  template<typename Json, typename F>
  void executeKvCommand(const std::string_view cmdRspName, KvWebSocket * ws, const Json& cmd, F&& handler)
    requires((HaveSessions && std::is_invocable_v<F, CacheMap&, SessionToken&, Json&>) || std::is_invocable_v<F, CacheMap&, Json&>)
  {
    const auto& cmdRoot = cmd.object_range().cbegin()->value();

    if constexpr (HaveSessions)
    {
      SessionToken token;
      if (auto session = getSession(ws, cmdRspName, cmdRoot, token); session)
        callKvHandler(ws, session->get().map, token, cmdRoot, std::forward<F>(handler));
    }
    else
      callKvHandler(ws, getContainer(), cmdRoot, std::forward<F>(handler));
  }


  ndb_always_inline void set(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& request)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };
    
    if (isValid(queryRspName, ws, request, {{Param::required("keys", JsonObject)}}, validate))
      executeKvCommand(queryRspName, ws, request, KvExecutor::set);
  }

  
  ndb_always_inline void setQ(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& request)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };

    if (isValid(queryRspName, ws, request, {{Param::required("keys", JsonObject)}}, validate))
      executeKvCommand(queryRspName, ws, request, KvExecutor::setQ);
  }

  
  ndb_always_inline void get(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& request)
  {
    if (isValid(queryRspName, ws, request, {{Param::required("keys", JsonArray)}}))
      executeKvCommand(queryRspName, ws, request, KvExecutor::get);
  }

  
  ndb_always_inline void add(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    if (isValid(queryRspName, ws, json, {{Param::required("keys", JsonObject)}}))
      executeKvCommand(queryRspName, ws, json, KvExecutor::add);
  }


  ndb_always_inline void addQ(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };

    if (isValid(queryRspName, ws, json, {{Param::required("keys", JsonObject)}}, validate))
      executeKvCommand(queryRspName, ws, json, KvExecutor::addQ);
  }


  ndb_always_inline void remove(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };

    if (isValid(queryRspName, ws, json, {{Param::required("keys", JsonArray)}}, validate))
      executeKvCommand(queryRspName, ws, json, KvExecutor::remove);
  }

  
  ndb_always_inline void clear(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    executeKvCommand(queryRspName, ws, json, KvExecutor::clear);
  }


  ndb_always_inline void count(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    executeKvCommand(queryRspName, ws, json, KvExecutor::count);
  }


  ndb_always_inline void contains(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    if (isValid(queryRspName, ws, json, {{Param::required("keys", JsonArray)}}))
      executeKvCommand(queryRspName, ws, json, KvExecutor::contains);
  }


  ndb_always_inline void find(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      if (cmd.at("rsp") != "paths" && cmd.at("rsp") != "kv" && cmd.at("rsp") != "keys")
        return {RequestStatus::CommandSyntax, "'rsp' invalid value"};
      else if (const auto& path = cmd.at("path").as_string() ; path.empty())
        return {RequestStatus::ValueSize, "'path' is empty"};

      return {RequestStatus::Ok, ""};
    };

    if (isValid(queryRspName, ws, json, {{Param::required("path", JsonString)}, {Param::required("rsp", JsonString)}, {Param::optional("keys", JsonArray)}}, validate))
    {
      auto& cmd = json.at(queryName);

      if (!cmd.contains("keys"))
        cmd["keys"] = njson::array(); // executor expects "keys"

      executeKvCommand(queryRspName, ws, json, KvExecutor::find);
    }
  }


  ndb_always_inline void update(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      // 'value' can be any valid JSON type, so just check it's present here
      if (!cmd.contains("value"))
        return {RequestStatus::ParamMissing, "Missing parameter"};
      
      return {RequestStatus::Ok, ""};
    };

    if (isValid(queryRspName, ws, json, {{Param::required("key", JsonString)}, {Param::required("path", JsonString)}}, validate))
      executeKvCommand(queryRspName, ws, json, KvExecutor::update);
  }


  ndb_always_inline void keys(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    executeKvCommand(queryRspName, ws, json, KvExecutor::keys);
  }


  ndb_always_inline void clearSet(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    if (isValid(queryRspName, ws, json, {{Param::required("keys", JsonObject)}}))
      executeKvCommand(queryRspName, ws, json, KvExecutor::clearSet);
  }
  

  ndb_always_inline void arrayAppend(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    if (isValid(queryRspName, ws, json, {{Param::required("key", JsonString)}, {Param::required("items", JsonArray)}, {Param::optional("name", JsonString)}}))
      executeKvCommand(queryRspName, ws, json, KvExecutor::arrayAppend);
  }


  void kvSave(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    if (!NemesisConfig::persistEnabled(m_config))
      send(ws, createErrorResponseNoTkn(queryRspName, RequestStatus::CommandDisabled));
    else
    {
      auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
      {
        if (cmd.at("name").empty())
          return {RequestStatus::ValueSize, "name empty"};
        else
          return {RequestStatus::Ok, ""};
      };
      
      if (isValid(queryRspName, ws, json, {{Param::required("name", JsonString)}}, validate))
      {
        doSave(queryName, queryRspName, ws, json.at(queryName));
      }
    }
  }


  void kvLoad(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    if (isValid(queryRspName, ws, json, {{Param::required("name", JsonString)}}))
    {
      const auto& loadName = json.at(queryName).at("name").as_string();

      if (const auto [valid, msg] = validatePreLoad(loadName, fs::path{NemesisConfig::savePath(m_config)}, false); !valid)
      {
        PLOGE << msg;

        njson rsp;
        rsp[queryRspName]["st"] = toUnderlying(RequestStatus::LoadError);
        rsp[queryRspName]["name"] = loadName;
        send (ws, rsp);
      }
      else
      {
        const auto [root, md, data, pathsValid] = getLoadPaths(fs::path{NemesisConfig::savePath(m_config)} / loadName);
        load(loadName, data, ws);
      }
    }
  }

  
  void doSave (const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& cmd)
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
        rsp = KvExecutor::saveKv(getContainer(), dataPath, name);
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
