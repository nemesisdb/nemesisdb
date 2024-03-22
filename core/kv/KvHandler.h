#ifndef NDB_CORE_KVHANDLERS_H
#define NDB_CORE_KVHANDLERS_H


#include <functional>
#include <vector>
#include <tuple>
#include <latch>
#include <ranges>
#include <core/NemesisCommon.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvSessionExecutor.h>
#include <core/kv/KvSessions.h>


namespace nemesis { namespace core { namespace kv {


class KvHandler
{
public:
  KvHandler(const njson& config) :  m_config(config)
  {
  }
  

  ~KvHandler()
  {
  }


private:
  
  // CAREFUL: these have to be in the order of KvQueryType enum
  const std::array<std::function<void(KvWebSocket *, njson_pmr&)>, static_cast<std::size_t>(KvQueryType::MAX)> MsgHandlers = 
  {
    std::bind_front(&KvHandler::sessionNew,       std::ref(*this)),
    std::bind_front(&KvHandler::sessionEnd,       std::ref(*this)),
    std::bind_front(&KvHandler::sessionOpen,      std::ref(*this)),
    std::bind_front(&KvHandler::sessionInfo,      std::ref(*this)),
    std::bind_front(&KvHandler::sessionInfoAll,   std::ref(*this)),
    std::bind_front(&KvHandler::sessionSave,      std::ref(*this)),
    std::bind_front(&KvHandler::sessionLoad,      std::ref(*this)),
    std::bind_front(&KvHandler::sessionEndAll,    std::ref(*this)),
    std::bind_front(&KvHandler::sessionExists,    std::ref(*this)),
    std::bind_front(&KvHandler::set,              std::ref(*this)),
    std::bind_front(&KvHandler::setQ,             std::ref(*this)),
    std::bind_front(&KvHandler::get,              std::ref(*this)),
    std::bind_front(&KvHandler::add,              std::ref(*this)),
    std::bind_front(&KvHandler::addQ,             std::ref(*this)),
    std::bind_front(&KvHandler::remove,           std::ref(*this)),
    std::bind_front(&KvHandler::clear,            std::ref(*this)),
    std::bind_front(&KvHandler::count,            std::ref(*this)),
    std::bind_front(&KvHandler::contains,         std::ref(*this)),
    std::bind_front(&KvHandler::find,             std::ref(*this)),
    std::bind_front(&KvHandler::update,           std::ref(*this)),
    std::bind_front(&KvHandler::keys,             std::ref(*this)),
    std::bind_front(&KvHandler::clearSet,         std::ref(*this))
  };
  

public:

  
  RequestStatus handle(KvWebSocket * ws, const std::string_view& command, njson_pmr& request)
  {
    RequestStatus status = RequestStatus::Ok;
    
    if (const auto itType = QueryNameToType.find(command) ; itType == QueryNameToType.cend())
      status = RequestStatus::CommandNotExist;
    else
    {
      // TODO QueryNameToType.second is tuple from previous version, keep for now but doesn't actually need to be
      const auto [queryType] = itType->second;  
      const auto handler = MsgHandlers[static_cast<std::size_t>(queryType)];

      try
      {
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
  {
    SessionExecutor::sessionMonitor(m_sessions);    
  }

  
  void load(const std::string& loadName, KvWebSocket * ws, const fs::path& dataSetsRoot)
  {
    PLOGI << "Loading from " << dataSetsRoot;

    send(ws, SessionExecutor::loadSessions (loadName, m_sessions, dataSetsRoot));

    PLOGI << "Loading finished";
  }
 

  LoadResult internalLoad(const std::string& loadName, const fs::path& dataSetsRoot)
  {
    PLOGI << "Loading from " << dataSetsRoot;

    const auto start = NemesisClock::now();
    const auto rsp = SessionExecutor::loadSessions (loadName, m_sessions, dataSetsRoot);

    PLOGI << "Loading finished";

    LoadResult loadResult { .loadTime = NemesisClock::now() - start };
    loadResult.status = static_cast<RequestStatus>(rsp["SH_LOAD_RSP"]["st"].as<std::uint64_t>());
    loadResult.nSessions = rsp["SH_LOAD_RSP"]["sessions"].as<std::size_t>();
    loadResult.nKeys = rsp["SH_LOAD_RSP"]["keys"].as<std::size_t>();

    return loadResult;
  }


private:
  
  // Wraps a PMR allocator for njson_pmr, ensuring the stack buffer is in scope and
  // the JSON object is created correctly.
  struct PmrRsp
  {
    // Initialise PMR allocator and JSON
    PmrRsp() :
      pool(std::data(buffer), std::size(buffer)),
      alloc(&pool)
    {
     
    }


    // F is passed an initialised njson_pmr, where the body of the response is stored. The body
    // is emplace into a 'cmdName' parent object.
    template<typename F>
    const void operator()(const std::string_view cmdName, njson_pmr& response, F&& fillBody) requires(std::is_invocable_v<F, njson_pmr&>)
    {
      response.try_emplace(cmdName, njson_pmr{jsoncons::json_object_arg, response.get_allocator()});
      fillBody(response.at(cmdName));
    }


    private:
      char buffer[1024U]; // TODO
      std::pmr::monotonic_buffer_resource pool;
      std::pmr::polymorphic_allocator<char> alloc;
  };

    
  bool getSessionToken(KvWebSocket * ws, const std::string_view queryRspName, const njson_pmr& cmdRoot, SessionToken& tkn)
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


  ndb_always_inline void send (KvWebSocket * ws, const njson_pmr& msg)
  {
    ws->send(msg.to_string(), WsSendOpCode);
  }


  ndb_always_inline void send (KvWebSocket * ws, const njson& msg)
  {
    ws->send(msg.to_string(), WsSendOpCode);
  }


  template<typename Json>
  bool doIsValid (const std::string& queryRspName, KvWebSocket * ws, 
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


  bool isValid (const std::string& queryRspName, KvWebSocket * ws, 
                const njson_pmr& cmd, const std::map<const std::string_view, const Param>& params,
                typename std::function<std::tuple<RequestStatus, const std::string_view>(const njson_pmr&)> onPostValidate = nullptr)
  {
    const auto& cmdRoot = cmd.object_range().cbegin()->value();
    return doIsValid(queryRspName, ws, cmdRoot, params, onPostValidate);
  }


  bool isValid (const std::string& queryRspName, const std::string_view child, KvWebSocket * ws, 
                const njson_pmr& cmd, const std::map<const std::string_view, const Param>& params,
                std::function<std::tuple<RequestStatus, const std::string_view>(const njson_pmr&)> onPostValidate = nullptr)
  {
    const auto& childObject = cmd.object_range().cbegin()->value()[child];
    return doIsValid(queryRspName, ws, childObject, params, onPostValidate);
  }
  


  // SESSION
  ndb_always_inline void sessionNew(KvWebSocket * ws, njson_pmr& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::ShNew);
    static const std::string queryRspName   = queryName + "_RSP";
    PmrRsp pmr;
    
    
    auto validate = [](const njson_pmr& cmd) -> std::tuple<RequestStatus, const std::string_view>
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

        char rspBuffer[256U] = {}; // TODO size
        std::pmr::monotonic_buffer_resource rspPool{std::data(rspBuffer), std::size(rspBuffer)};
        std::pmr::polymorphic_allocator<char> rspAlloc(&rspPool);

        njson_pmr response {jsoncons::json_object_arg, rspAlloc};
        response.try_emplace(queryRspName, njson_pmr{jsoncons::json_object_arg, rspAlloc});
        
        SessionExecutor::newSession(m_sessions, name, createSessionToken(name, shared), shared, duration, deleteOnExpire, response.at(queryRspName));

        send(ws, response);
      }
    }
  }

  
  ndb_always_inline void sessionEnd(KvWebSocket * ws, njson_pmr& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::ShEnd);
    static const std::string queryRspName   = queryName + "_RSP";

    SessionToken token;
    
    if (getSessionToken(ws, queryRspName, json.at(queryName), token))
      send(ws, SessionExecutor::endSession(m_sessions, token));
  }

  
  ndb_always_inline void sessionOpen(KvWebSocket * ws, njson_pmr& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::ShOpen);
    static const std::string queryRspName   = queryName +"_RSP";


    auto validate = [](const njson_pmr& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      if (cmd.at("name").empty())
        return {RequestStatus::ValueSize, "Session name empty"};

      return {RequestStatus::Ok, ""};
    };


    const auto& cmd = json.at(queryName);

    if (isValid(queryRspName, ws, cmd, {{Param::required("name", JsonString)}}, validate))
    {
      njson rsp;
      
      const auto token = createSessionToken(cmd.at("name").as_string(), true);
      
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

  
  ndb_always_inline void sessionInfo(KvWebSocket * ws, njson_pmr& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::ShInfo);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);

    if (cmd.size() != 1U)
      send(ws, createErrorResponse(queryRspName, RequestStatus::CommandSyntax));
    else
    {
      SessionToken token;
    
      if (getSessionToken(ws, queryName, cmd, token))
        send(ws, SessionExecutor::sessionInfo(m_sessions, token));
    }
  }

  
  ndb_always_inline void sessionInfoAll(KvWebSocket * ws, njson_pmr& json)
  {
    send(ws, SessionExecutor::sessionInfoAll(m_sessions));
  }
  

  ndb_always_inline void sessionExists(KvWebSocket * ws, njson_pmr& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::ShExists);
    static const std::string queryRspName   = queryName + "_RSP";

    auto& cmd = json.at(queryName);

    if (isValid(queryRspName, ws, cmd, {{Param::required("tkns", JsonArray)}}))
      send(ws, SessionExecutor::sessionExists(m_sessions, cmd.at("tkns")));
  }


  ndb_always_inline void sessionEndAll(KvWebSocket * ws, njson_pmr& json)
  {
    send(ws, SessionExecutor::sessionEndAll(m_sessions));
  }

  
  ndb_always_inline void sessionSave(KvWebSocket * ws, njson_pmr& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::ShSave);
    static const std::string queryRspName   = queryName +"_RSP";
    

    auto validate = [](const njson_pmr& cmd) -> std::tuple<RequestStatus, const std::string_view>
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
    
    if (!NemesisConfig::kvSaveEnabled(m_config))
      send(ws, createErrorResponseNoTkn(queryRspName, RequestStatus::CommandDisabled));
    else if (isValid(queryRspName, ws, json, {{Param::required("name", JsonString)}, {Param::optional("tkns", JsonArray)}}, validate))
    {
      auto& cmd = json.at(queryName);
      const auto& name = cmd.at("name").as_string();

      njson rsp;
      rsp[queryRspName]["name"] = name;
      
      const auto setPath = std::to_string(KvSaveClock::now().time_since_epoch().count());
      const auto root = fs::path {NemesisConfig::kvSavePath(m_config)} / name / setPath;
      const auto metaPath = fs::path{root} / "md";
      const auto dataPath = fs::path{root} / "data";

      std::ofstream metaStream;

      if (!fs::create_directories(metaPath))
        rsp[queryRspName]["st"] = toUnderlying(RequestStatus::SaveDirWriteFail);
      else if (metaStream.open(metaPath / "md.json", std::ios_base::trunc | std::ios_base::out); !metaStream.is_open())
        rsp[queryRspName]["st"] = toUnderlying(RequestStatus::SaveDirWriteFail);
      else
      {
        const bool haveTkns = cmd.contains("tkns");

        rsp[queryRspName]["st"] = toUnderlying(RequestStatus::SaveStart);
        send(ws, rsp);

        // write metadata before we start incase we're interrupted mid-save
        auto start = KvSaveClock::now();
        KvSaveStatus metaDataStatus = KvSaveStatus::Pending;

        njson metadata;
        metadata["name"] = name;
        metadata["version"] = METADATA_VERSION;
        metadata["status"] = toUnderlying(metaDataStatus);        
        metadata["start"] = chrono::time_point_cast<KvSaveMetaDataUnit>(start).time_since_epoch().count();
        metadata["saveType"] = haveTkns ? toUnderlying(SaveType::SelectSessions) : toUnderlying(SaveType::AllSessions);
        metadata["complete"] = 0;
        
        metadata.dump(metaStream);

        // TODO TODO 
        
        // create save command and call executor
        // njson saveCmd;
        // saveCmd["poolDataRoot"] = dataPath.string();
        // saveCmd["name"] = name;

        // if (haveTkns)
        //   saveCmd["tkns"] = std::move(cmd.at("tkns"));

        
        // try
        // {
        //   rsp = SessionExecutor::saveSessions(m_sessions, saveCmd);
        //   metaDataStatus = KvSaveStatus::Complete;
        // }
        // catch(const std::exception& e)
        // {
        //   PLOGE << e.what();
        //   metaDataStatus = KvSaveStatus::Error;
        // }

        // // update metdata
        // metadata["status"] = toUnderlying(metaDataStatus);
        // metadata["complete"] = chrono::time_point_cast<KvSaveMetaDataUnit>(KvSaveClock::now()).time_since_epoch().count();
        // metaStream.seekp(0);

        // metadata.dump(metaStream);
      }

      send(ws, rsp);
    }
  } 

  
  ndb_always_inline void sessionLoad(KvWebSocket * ws, njson_pmr& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::ShLoad);
    static const std::string queryRspName   = queryName +"_RSP";

    if (isValid(queryRspName, ws, json, {{Param::required("name", JsonString)}}))
    {
      const auto& loadName = json.at(queryName).at("name").as_string();
      const auto& loadPath = fs::path{NemesisConfig::kvSavePath(m_config)};
      const auto loadRoot = loadPath / loadName;

      if (!fs::exists(loadRoot))
      {
        njson rsp;
        rsp[queryRspName]["st"] = toUnderlying(RequestStatus::LoadError);
        rsp[queryRspName]["name"] = loadName;
        send (ws, rsp);
      }
      else
      {
        // loadRoot may contain several saves (i.e. SH_SAVE used multiple times with the same 'name'),
        // so use getDefaultDataSetPath() to get the most recent
        const auto dataSetPath = getDefaultDataSetPath(loadRoot);    
        load(loadName, ws, dataSetPath / "data");
      }
    }
  }


  // KV
  template<typename Json>
  std::optional<std::reference_wrapper<Sessions::Session>> getSession (KvWebSocket * ws, const std::string_view cmdRspName, const Json& cmd, SessionToken& token)
  {
    if (getSessionToken(ws, cmdRspName, cmd, token))
    {
      if (auto session = m_sessions.get(token); session)
      {
        if (session->get().expires)
          m_sessions.updateExpiry(session->get());

        return session;
      }
      else
        send(ws, createErrorResponse(cmdRspName, RequestStatus::SessionNotExist, token));
    }

    return {};
  }


  template<typename Json>
  void handleKvExecuteResult(KvWebSocket * ws, const Json& rsp)
  {
    if (!rsp.empty())
      send(ws, rsp);
  }


  template<typename Json, typename F>
  void callKvHandler(KvWebSocket * ws, CacheMap& map, const SessionToken& token, Json& cmdRoot, F&& handler)
    requires(std::is_invocable_v<F, CacheMap&, SessionToken&, Json&>)
  {
    const auto rsp = std::invoke(handler, map, token, cmdRoot);
    handleKvExecuteResult(ws, rsp);
  }


  template<typename Json, typename F>
  void callKvHandler(KvWebSocket * ws, CacheMap& map, const SessionToken& token, const Json& cmdRoot, F&& handler)
    requires(std::is_invocable_v<F, CacheMap&, SessionToken&, Json&>)
  {
    const auto rsp = std::invoke(handler, map, token, cmdRoot);
    handleKvExecuteResult(ws, rsp);
  }


  template<typename Json, typename F>
  void executeKvCommand(const std::string_view cmdRspName, KvWebSocket * ws, Json& cmd, F&& handler)
    requires(std::is_invocable_v<F, CacheMap&, SessionToken&, Json&>)
  {
    auto& cmdRoot = cmd.object_range().begin()->value();

    SessionToken token;
    if (auto session = getSession(ws, cmdRspName, cmdRoot, token); session)
      callKvHandler(ws, session->get().map, token, cmdRoot, std::forward<F>(handler));
  }


  template<typename Json, typename F>
  void executeKvCommand(const std::string_view cmdRspName, KvWebSocket * ws, const Json& cmd, F&& handler)
    requires(std::is_invocable_v<F, CacheMap&, SessionToken&, Json&>)
  {
    const auto& cmdRoot = cmd.object_range().cbegin()->value();

    SessionToken token;
    if (auto session = getSession(ws, cmdRspName, cmdRoot, token); session)
      callKvHandler(ws, session->get().map, token, cmdRoot, std::forward<F>(handler));
  }


  /*
  template<typename F>
  void executePmrKvCommand(const std::string_view cmdRspName, KvWebSocket * ws, const njson_pmr& request, njson_pmr& response, F&& handler)
    requires(std::is_invocable_v<F, CacheMap&, SessionToken&, njson_pmr&, njson_pmr&>)
  {
    const auto& cmdBody = request.object_range().cbegin()->value();

    SessionToken token;
    if (auto session = getSession(ws, cmdRspName, cmdBody, token); session)
    {      
      response.try_emplace(cmdRspName, njson_pmr{jsoncons::json_object_arg, response.get_allocator()});
      std::invoke(handler, session->get().map, token, cmdBody, response.at(cmdRspName));

      if (!response.at(cmdRspName).empty())
        send(ws, response);
    }      
  }
  */


  ndb_always_inline void set(KvWebSocket * ws, njson_pmr& request)
  {
    static const std::string queryName    = QueryTypeToName.at(KvQueryType::KvSet);
    static const std::string queryRspName = queryName +"_RSP";
    

    auto validate = [](const njson_pmr& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };
    
    if (isValid(queryRspName, ws, request, {{Param::required("keys", JsonObject)}}, validate))
      executeKvCommand(queryRspName, ws, request, KvExecutor::set);
  }


  ndb_always_inline void setQ(KvWebSocket * ws, njson_pmr& request)
  {
    static const std::string queryName    = QueryTypeToName.at(KvQueryType::KvSetQ);
    static const std::string queryRspName = queryName +"_RSP";
    
    auto validate = [](const njson_pmr& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };

    if (isValid(queryRspName, ws, request, {{Param::required("keys", JsonObject)}}, validate))
      executeKvCommand(queryRspName, ws, request, KvExecutor::setQ);
  }

  
  ndb_always_inline void get(KvWebSocket * ws, njson_pmr& request)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvGet);
    static const std::string queryRspName   = queryName +"_RSP";

    if (isValid(queryRspName, ws, request, {{Param::required("keys", JsonArray)}}))
      executeKvCommand(queryRspName, ws, request, KvExecutor::get);
  }

  
  ndb_always_inline void add(KvWebSocket * ws, njson_pmr& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvAdd);
    static const std::string queryRspName   = queryName +"_RSP";

    if (isValid(queryRspName, ws, json, {{Param::required("keys", JsonObject)}}))
      executeKvCommand(queryRspName, ws, json, KvExecutor::add);
  }


  ndb_always_inline void addQ(KvWebSocket * ws, njson_pmr& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvAddQ);
    static const std::string queryRspName   = queryName +"_RSP";

    auto validate = [](const njson_pmr& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };

    if (isValid(queryRspName, ws, json, {{Param::required("keys", JsonObject)}}, validate))
      executeKvCommand(queryRspName, ws, json, KvExecutor::addQ);
  }


  ndb_always_inline void remove(KvWebSocket * ws, njson_pmr& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvRemove);
    static const std::string queryRspName   = queryName +"_RSP";

    auto validate = [](const njson_pmr& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };

    if (isValid(queryRspName, ws, json, {{Param::required("keys", JsonArray)}}, validate))
      executeKvCommand(queryRspName, ws, json, KvExecutor::remove);
  }

  
  ndb_always_inline void clear(KvWebSocket * ws, njson_pmr& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvClear);
    static const std::string queryRspName   = queryName +"_RSP";
    
    executeKvCommand(queryRspName, ws, json, KvExecutor::clear);
  }


  ndb_always_inline void count(KvWebSocket * ws, njson_pmr& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvCount);
    static const std::string queryRspName   = queryName +"_RSP";
    
    executeKvCommand(queryRspName, ws, json, KvExecutor::count);
  }


  ndb_always_inline void contains(KvWebSocket * ws, njson_pmr& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvContains);
    static const std::string queryRspName   = queryName +"_RSP";
    
    if (isValid(queryRspName, ws, json, {{Param::required("keys", JsonArray)}}))
      executeKvCommand(queryRspName, ws, json, KvExecutor::contains);
  }


  ndb_always_inline void find(KvWebSocket * ws, njson_pmr& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvFind);
    static const std::string queryRspName   = queryName +"_RSP";


    auto validate = [](const njson_pmr& cmd) -> std::tuple<RequestStatus, const std::string_view>
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
        cmd["keys"] = njson_pmr::array(); // executor expects "keys"

      executeKvCommand(queryRspName, ws, json, KvExecutor::find);
    }
  }


  ndb_always_inline void update(KvWebSocket * ws, njson_pmr& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvUpdate);
    static const std::string queryRspName   = queryName +"_RSP";

    auto validate = [](const njson_pmr& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      // 'value' can be any valid JSON type, so just check it's present here
      if (!cmd.contains("value"))
        return {RequestStatus::ParamMissing, "Missing parameter"};
      
      return {RequestStatus::Ok, ""};
    };

    if (isValid(queryRspName, ws, json, {{Param::required("key", JsonString)}, {Param::required("path", JsonString)}}, validate))
      executeKvCommand(queryRspName, ws, json, KvExecutor::update);
  }


  ndb_always_inline void keys(KvWebSocket * ws, njson_pmr& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvKeys);
    static const std::string queryRspName   = queryName +"_RSP";

    executeKvCommand(queryRspName, ws, json, KvExecutor::keys);
  }


  ndb_always_inline void clearSet(KvWebSocket * ws, njson_pmr& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvClearSet);
    static const std::string queryRspName   = queryName +"_RSP";

    if (isValid(queryRspName, ws, json, {{Param::required("keys", JsonObject)}}))
      executeKvCommand(queryRspName, ws, json, KvExecutor::clearSet);
  }


private:
  njson m_config;
  Sessions m_sessions;
};

}
}
}

#endif
