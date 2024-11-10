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
#include <core/kv/KvSessionExecutor.h>
#include <core/kv/KvSessions.h>


namespace nemesis { namespace core { namespace kv {

/*
KvHandler receives a command:
  - checks if the command exists and is enabled
  - calls a handler function

The class handles commands for when sessions are enabled or disabled. To avoid
code such as this for each command:
  
  if(sessionsEnabled)
  {
    auto session = getSession();
    runCommand(session, xyz);
  }
  else
    runCommand(xyz);

The template argument HaveSessions is used:
  - if true, m_container is a Sessions object (which contains sessions with a map per session)
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
  KvHandler(const njson& config) requires (HaveSessions) :
    m_config(config),
    m_container() // Sessions sets the initial bucket count
    
  {
  }


  KvHandler(const njson& config) requires (!HaveSessions) :
    m_config(config),
    m_container(50'000) // Adds ~2Mb to initial memory use
    
  {
  }
  

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
      {KvQueryType::KvSet,        Handler{std::bind_front(&KvHandler<HaveSessions>::set,        std::ref(*this)), "KV_SET",   "KV_SET_RSP"}},
      {KvQueryType::KvSetQ,       Handler{std::bind_front(&KvHandler<HaveSessions>::setQ,       std::ref(*this)), "KV_SETQ",  "KV_SETQ_RSP"}},
      {KvQueryType::KvGet,        Handler{std::bind_front(&KvHandler<HaveSessions>::get,        std::ref(*this)), "KV_GET",   "KV_GET_RSP"}},
      {KvQueryType::KvAdd,        Handler{std::bind_front(&KvHandler<HaveSessions>::add,        std::ref(*this)), "KV_ADD",   "KV_ADD_RSP"}},
      {KvQueryType::KvAddQ,       Handler{std::bind_front(&KvHandler<HaveSessions>::addQ,       std::ref(*this)), "KV_ADDQ",  "KV_ADDQ_RSP"}},
      {KvQueryType::KvRemove,     Handler{std::bind_front(&KvHandler<HaveSessions>::remove,     std::ref(*this)), "KV_RMV",   "KV_RMV_RSP"}},
      {KvQueryType::KvClear,      Handler{std::bind_front(&KvHandler<HaveSessions>::clear,      std::ref(*this)), "KV_CLEAR", "KV_CLEAR_RSP"}},
      {KvQueryType::KvCount,      Handler{std::bind_front(&KvHandler<HaveSessions>::count,      std::ref(*this)), "KV_COUNT", "KV_COUNT_RSP"}},
      {KvQueryType::KvContains,   Handler{std::bind_front(&KvHandler<HaveSessions>::contains,   std::ref(*this)), "KV_CONTAINS",  "KV_CONTAINS_RSP"}},
      {KvQueryType::KvFind,       Handler{std::bind_front(&KvHandler<HaveSessions>::find,       std::ref(*this)), "KV_FIND",      "KV_FIND_RSP"}},
      {KvQueryType::KvUpdate,     Handler{std::bind_front(&KvHandler<HaveSessions>::update,     std::ref(*this)), "KV_UPDATE",    "KV_UPDATE_RSP"}},
      {KvQueryType::KvKeys,       Handler{std::bind_front(&KvHandler<HaveSessions>::keys,       std::ref(*this)), "KV_KEYS",      "KV_KEYS_RSP"}},
      {KvQueryType::KvClearSet,   Handler{std::bind_front(&KvHandler<HaveSessions>::clearSet,   std::ref(*this)), "KV_CLEAR_SET", "KV_CLEAR_SET_RSP"}},
      {KvQueryType::KvArrayAppend, Handler{std::bind_front(&KvHandler<HaveSessions>::arrayAppend, std::ref(*this)), "KV_ARR_APPEND", "KV_ARR_APPEND_RSP"}}
      
    }, 1, alloc);

    
    if constexpr (HaveSessions)
    {
      h.try_emplace(KvQueryType::ShNew,     Handler{std::bind_front(&KvHandler<HaveSessions>::sessionNew,      std::ref(*this)), "SH_NEW",       "SH_NEW_RSP"});
      h.try_emplace(KvQueryType::ShEnd,     Handler{std::bind_front(&KvHandler<HaveSessions>::sessionEnd,      std::ref(*this)), "SH_END",       "SH_END_RSP"});
      h.try_emplace(KvQueryType::ShEndAll,  Handler{std::bind_front(&KvHandler<HaveSessions>::sessionEndAll,   std::ref(*this)), "SH_END_ALL",   "SH_END_ALL_RSP"});
      h.try_emplace(KvQueryType::ShExists,  Handler{std::bind_front(&KvHandler<HaveSessions>::sessionExists,   std::ref(*this)), "SH_EXISTS",    "SH_EXISTS_RSP"});
      h.try_emplace(KvQueryType::ShInfo,    Handler{std::bind_front(&KvHandler<HaveSessions>::sessionInfo,     std::ref(*this)), "SH_INFO",      "SH_INFO_RSP"});
      h.try_emplace(KvQueryType::ShInfoAll, Handler{std::bind_front(&KvHandler<HaveSessions>::sessionInfoAll,  std::ref(*this)), "SH_INFO_ALL",  "SH_INFO_ALL_RSP"});
      h.try_emplace(KvQueryType::ShLoad,    Handler{std::bind_front(&KvHandler<HaveSessions>::sessionLoad,     std::ref(*this)), "SH_LOAD",      "SH_LOAD_RSP"});
      h.try_emplace(KvQueryType::ShSave,    Handler{std::bind_front(&KvHandler<HaveSessions>::sessionSave,     std::ref(*this)), "SH_SAVE",      "SH_SAVE_RSP"});
      h.try_emplace(KvQueryType::ShOpen,    Handler{std::bind_front(&KvHandler<HaveSessions>::sessionOpen,     std::ref(*this)), "SH_OPEN",      "SH_OPEN_RSP"});
    }
    else
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
      // kv commands required irrespective of sessions
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
      {"KV_ARR_APPEND",   KvQueryType::KvArrayAppend}
    }, 1, alloc); 


    if constexpr (HaveSessions)
    {
      map.try_emplace("SH_NEW",      KvQueryType::ShNew);
      map.try_emplace("SH_END",      KvQueryType::ShEnd);
      map.try_emplace("SH_OPEN",     KvQueryType::ShOpen);
      map.try_emplace("SH_INFO",     KvQueryType::ShInfo);
      map.try_emplace("SH_INFO_ALL", KvQueryType::ShInfoAll);
      map.try_emplace("SH_SAVE",     KvQueryType::ShSave);
      map.try_emplace("SH_LOAD",     KvQueryType::ShLoad);
      map.try_emplace("SH_END_ALL",  KvQueryType::ShEndAll);
      map.try_emplace("SH_EXISTS",   KvQueryType::ShExists);
    }
    else
    {
      map.try_emplace("KV_SAVE",     KvQueryType::KvSave);
      map.try_emplace("KV_LOAD",     KvQueryType::KvLoad);
    }

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


  void monitor ()
    requires(HaveSessions)
  {
    SessionExecutor<true>::sessionMonitor(getContainer());
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
      rsp = SessionExecutor<true>::loadSessions (loadName, getContainer(), dataSetsRoot);   
    else
      rsp = KvExecutor<false>::loadKv (loadName, getContainer(), dataSetsRoot);   

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


  std::conditional_t<HaveSessions, Sessions&, CacheMap&> getContainer()
  {
    return m_container;
  }


  ndb_always_inline void send (KvWebSocket * ws, const njson& msg)
  {
    ws->send(msg.to_string(), WsSendOpCode);
  }


  template<typename Json>
  bool doIsValid (const std::string_view queryRspName, KvWebSocket * ws, 
                  const Json& cmd, const std::map<const std::string_view, const Param>& params,
                  std::function<std::tuple<RequestStatus, const std::string_view>(const Json&)> onPostValidate = nullptr)
  {
    const auto [stat, msg] = isCmdValid<Json, RequestStatus,
                                              RequestStatus::Ok,
                                              RequestStatus::ParamMissing,
                                              RequestStatus::ValueTypeInvalid>(cmd, params, onPostValidate);
    
    if (stat != RequestStatus::Ok)
    {
      PLOGD << msg;
      send(ws, createErrorResponse(queryRspName, stat, msg));
    }
      
    return stat == RequestStatus::Ok;
  }


  bool isValid (const std::string_view queryRspName, KvWebSocket * ws, 
                const njson& cmd, const std::map<const std::string_view, const Param>& params,
                typename std::function<std::tuple<RequestStatus, const std::string_view>(const njson&)> onPostValidate = nullptr)
  {
    const auto& cmdRoot = cmd.object_range().cbegin()->value();
    return doIsValid(queryRspName, ws, cmdRoot, params, onPostValidate);
  }


  bool isValid (const std::string_view queryRspName, const std::string_view child, KvWebSocket * ws, 
                const njson& cmd, const std::map<const std::string_view, const Param>& params,
                std::function<std::tuple<RequestStatus, const std::string_view>(const njson&)> onPostValidate = nullptr)
  {
    const auto& childObject = cmd.object_range().cbegin()->value()[child];
    return doIsValid(queryRspName, ws, childObject, params, onPostValidate);
  }
  


  // SESSION
  ndb_always_inline void sessionNew(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      if (cmd.at("name").empty())
        return {RequestStatus::ValueSize, "Session name empty"};

      return {RequestStatus::Ok, ""};
    };

    if (isValid(queryRspName, ws, json, {{Param::required("name", JsonString)}, {Param::optional("expiry", JsonObject)}, {Param::optional("shared", JsonBool)}}, validate))
    {
      const auto& cmd = json.at(queryName);

      SessionDuration duration{SessionDuration::rep{0}};
      bool deleteOnExpire = false;

      bool valid = true;

      if (cmd.contains("expiry"))
      {
        valid = isValid(queryRspName, "expiry", ws, json, {{Param::required("duration", JsonUInt)}, {Param::required("deleteSession", JsonBool)}});
        if (valid)
        {
          duration = SessionDuration{cmd.at("expiry").at("duration").as<SessionDuration::rep>()};
          deleteOnExpire = cmd.at("expiry").at("deleteSession").as_bool();
        }
      }
      
      
      if (valid)
      {
        const std::string name = cmd.at("name").as_string();
        const bool shared = cmd.get_value_or<bool>("shared", false);
        send(ws, SessionExecutor<HaveSessions>::newSession(getContainer(), name, createSessionToken(name, shared), shared, duration, deleteOnExpire));
      }
    }
  }

  
  ndb_always_inline void sessionEnd(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    SessionToken token;
    
    if (getSessionToken(ws, queryRspName, json.at(queryName), token))
      send(ws, SessionExecutor<HaveSessions>::endSession(getContainer(), token));
  }

  
  ndb_always_inline void sessionOpen(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      if (cmd.at("name").empty())
        return {RequestStatus::ValueSize, "Session name empty"};

      return {RequestStatus::Ok, ""};
    };
    

    if (isValid(queryRspName, ws, json, {{Param::required("name", JsonString)}}, validate))
    {
      njson rsp;

      const auto& cmd = json.at(queryName);
      const auto token = createSessionToken(cmd.at("name").as_string(), true);
      
      // generate a shared token from the name. If a session with the same name was created but isn't shared, 
      // the tokens will be completely different
      if (SessionExecutor<HaveSessions>::openSession(getContainer(), token))
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

  
  ndb_always_inline void sessionInfo(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    auto& cmd = json.at(queryName);

    if (cmd.size() != 1U)
      send(ws, createErrorResponse(queryRspName, RequestStatus::CommandSyntax));
    else
    {
      SessionToken token;
    
      if (getSessionToken(ws, queryName, cmd, token))
        send(ws, SessionExecutor<HaveSessions>::sessionInfo(getContainer(), token));
    }
  }
  
  
  ndb_always_inline void sessionInfoAll(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    send(ws, SessionExecutor<HaveSessions>::sessionInfoAll(getContainer()));
  }
  
  
  ndb_always_inline void sessionExists(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    const auto& cmd = json.at(queryName);

    if (isValid(queryRspName, ws, json, {{Param::required("tkns", JsonArray)}}))
      send(ws, SessionExecutor<HaveSessions>::sessionExists(getContainer(), cmd.at("tkns")));
  }


  ndb_always_inline void sessionEndAll(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    send(ws, SessionExecutor<HaveSessions>::sessionEndAll(getContainer()));
  }

  
  ndb_always_inline void sessionSave(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      const bool haveTkns = cmd.contains("tkns");

      if (haveTkns)
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
    
    if (!NemesisConfig::saveEnabled(m_config))
      send(ws, createErrorResponseNoTkn(queryRspName, RequestStatus::CommandDisabled));
    else if (isValid(queryRspName, ws, json, {{Param::required("name", JsonString)}, {Param::optional("tkns", JsonArray)}}, validate))
    {
      doSave(queryName, queryRspName, ws, json.at(queryName));
    }
  } 

  
  ndb_always_inline void sessionLoad(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    if (isValid(queryRspName, ws, json, {{Param::required("name", JsonString)}}))
    {
      const auto& loadName = json.at(queryName).at("name").as_string();

      if (const auto [valid, msg] = validatePreLoad(loadName, fs::path{NemesisConfig::savePath(m_config)}, true); !valid)
      {
        PLOGE << msg;

        njson rsp;
        rsp[queryRspName]["st"] = toUnderlying(RequestStatus::LoadError);
        rsp[queryRspName]["name"] = loadName;
        send (ws, rsp);
      }
      else
      {
        // ignore pathsValid here because validatePreLoad() fails if not paths not valid
        const auto [root, md, data, pathsValid] = getLoadPaths(fs::path{NemesisConfig::savePath(m_config)} / loadName);
        load(loadName, data, ws);
      }
    }
  }

  

  // KV
  template<typename Json>
  std::optional<std::reference_wrapper<Sessions::Session>> getSession (KvWebSocket * ws, const std::string_view cmdRspName, const Json& cmd, SessionToken& token)
    requires(HaveSessions)
  {
    if (getSessionToken(ws, cmdRspName, cmd, token))
    {
      if (auto session = getContainer().get(token); session)
      {
        if (session->get().expires)
          getContainer().updateExpiry(session->get());

        return session;
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
      setToken(rsp.object_range().begin()->value(), tkn);
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
      executeKvCommand(queryRspName, ws, request, KvExecutor<HaveSessions>::set);
  }

  
  ndb_always_inline void setQ(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& request)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };

    if (isValid(queryRspName, ws, request, {{Param::required("keys", JsonObject)}}, validate))
      executeKvCommand(queryRspName, ws, request, KvExecutor<HaveSessions>::setQ);
  }

  
  ndb_always_inline void get(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& request)
  {
    if (isValid(queryRspName, ws, request, {{Param::required("keys", JsonArray)}}))
      executeKvCommand(queryRspName, ws, request, KvExecutor<HaveSessions>::get);
  }

  
  ndb_always_inline void add(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    if (isValid(queryRspName, ws, json, {{Param::required("keys", JsonObject)}}))
      executeKvCommand(queryRspName, ws, json, KvExecutor<HaveSessions>::add);
  }


  ndb_always_inline void addQ(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };

    if (isValid(queryRspName, ws, json, {{Param::required("keys", JsonObject)}}, validate))
      executeKvCommand(queryRspName, ws, json, KvExecutor<HaveSessions>::addQ);
  }


  ndb_always_inline void remove(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };

    if (isValid(queryRspName, ws, json, {{Param::required("keys", JsonArray)}}, validate))
      executeKvCommand(queryRspName, ws, json, KvExecutor<HaveSessions>::remove);
  }

  
  ndb_always_inline void clear(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    executeKvCommand(queryRspName, ws, json, KvExecutor<HaveSessions>::clear);
  }


  ndb_always_inline void count(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    executeKvCommand(queryRspName, ws, json, KvExecutor<HaveSessions>::count);
  }


  ndb_always_inline void contains(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    if (isValid(queryRspName, ws, json, {{Param::required("keys", JsonArray)}}))
      executeKvCommand(queryRspName, ws, json, KvExecutor<HaveSessions>::contains);
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

      executeKvCommand(queryRspName, ws, json, KvExecutor<HaveSessions>::find);
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
      executeKvCommand(queryRspName, ws, json, KvExecutor<HaveSessions>::update);
  }


  ndb_always_inline void keys(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    executeKvCommand(queryRspName, ws, json, KvExecutor<HaveSessions>::keys);
  }


  ndb_always_inline void clearSet(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    if (isValid(queryRspName, ws, json, {{Param::required("keys", JsonObject)}}))
      executeKvCommand(queryRspName, ws, json, KvExecutor<HaveSessions>::clearSet);
  }
  

  ndb_always_inline void arrayAppend(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    if (isValid(queryRspName, ws, json, {{Param::required("key", JsonString)}, {Param::required("items", JsonArray)}, {Param::optional("name", JsonString)}}))
      executeKvCommand(queryRspName, ws, json, KvExecutor<HaveSessions>::arrayAppend);
  }


  void kvSave(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
  {
    if (!NemesisConfig::saveEnabled(m_config))
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


  ndb_always_inline void kvLoad(const std::string_view queryName, const std::string_view queryRspName, KvWebSocket * ws, njson& json)
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

    njson rsp;
    rsp[queryRspName]["name"] = name;
    
    const auto setPath = std::to_string(KvSaveClock::now().time_since_epoch().count());
    const auto root = fs::path {NemesisConfig::savePath(m_config)} / name / setPath;
    const auto metaPath = fs::path{root} / "md";
    const auto dataPath = fs::path{root} / "data";

    std::ofstream metaStream;

    if (!fs::create_directories(metaPath))
      rsp[queryRspName]["st"] = toUnderlying(RequestStatus::SaveDirWriteFail);
    else if (metaStream.open(metaPath / "md.json", std::ios_base::trunc | std::ios_base::out); !metaStream.is_open())
      rsp[queryRspName]["st"] = toUnderlying(RequestStatus::SaveDirWriteFail);
    else
    {
      rsp[queryRspName]["st"] = toUnderlying(RequestStatus::SaveStart);
      send(ws, rsp);
      
      const auto start = KvSaveClock::now();
      KvSaveStatus metaDataStatus = KvSaveStatus::Pending;

      // write metadata now incase we're interrupted mid-save
      njson metadata;
      metadata["name"] = name;
      metadata["version"] = METADATA_VERSION;
      metadata["status"] = toUnderlying(metaDataStatus);        
      metadata["start"] = chrono::time_point_cast<KvSaveMetaDataUnit>(start).time_since_epoch().count();
      metadata["complete"] = 0; // time completed

      if constexpr(HaveSessions)
        metadata["saveDataType"] = toUnderlying(SaveDataType::SessionKv);
      else
        metadata["saveDataType"] = toUnderlying(SaveDataType::RawKv);
      
      metadata.dump(metaStream);

              
      // create save command and call executor
      njson saveCmd;
      saveCmd["poolDataRoot"] = dataPath.string();
      saveCmd["name"] = name;
      
      try
      {
        if constexpr (HaveSessions)
        {
          if (cmd.contains("tkns"))
            saveCmd["tkns"] = std::move(cmd.at("tkns"));

          rsp = SessionExecutor<true>::saveSessions(getContainer(), saveCmd);
        }
        else
          rsp = KvExecutor<false>::saveKv(getContainer(), dataPath, name);


        metaDataStatus = KvSaveStatus::Complete;
      }
      catch(const std::exception& e)
      {
        PLOGE << e.what();
        metaDataStatus = KvSaveStatus::Error;
      }
      
      // update metdata
      metadata["status"] = toUnderlying(metaDataStatus);
      metadata["complete"] = chrono::time_point_cast<KvSaveMetaDataUnit>(KvSaveClock::now()).time_since_epoch().count();
      metaStream.seekp(0);

      metadata.dump(metaStream);
    }

    send(ws, rsp);
  }
  

private:
  njson m_config;
  std::conditional_t<HaveSessions, Sessions, CacheMap> m_container;
};

}
}
}

#endif
