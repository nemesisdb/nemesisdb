#ifndef NDB_CORE_KVHANDLERS_H
#define NDB_CORE_KVHANDLERS_H


#include <functional>
#include <vector>
#include <tuple>
#include <latch>
#include <ranges>
#include <core/NemesisCommon.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvPoolWorker.h>
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
  const std::array<std::function<void(KvWebSocket *, njson&&)>, static_cast<std::size_t>(KvQueryType::MAX)> MsgHandlers = 
  {
    std::bind_front(&KvHandler::sessionNew,       std::ref(*this)),
    std::bind_front(&KvHandler::sessionEnd,       std::ref(*this)),
    std::bind_front(&KvHandler::sessionOpen,      std::ref(*this)),
    std::bind_front(&KvHandler::sessionInfo,      std::ref(*this)),
    std::bind_front(&KvHandler::sessionInfoAll,   std::ref(*this)),
    nullptr, //std::bind_front(&KvHandler::sessionSave,      std::ref(*this)),
    nullptr, //std::bind_front(&KvHandler::sessionLoad,      std::ref(*this)),
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

  
  RequestStatus handle(KvWebSocket * ws, const std::string_view& command, njson&& json)
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
        handler(ws, std::move(json));
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
    // for(auto& pool : m_pools)
    //   pool->execute(KvCommand{.type = KvQueryType::InternalSessionMonitor});
  }

  /* 
  LoadResult load(const fs::path& dataSetsRoot)
  {
    PLOGI << "Loading from " << dataSetsRoot;

    const auto start = NemesisClock::now();
    const auto hostPools = m_pools.size();
    const auto sourcePools = countFiles(dataSetsRoot);
    
    LoadResult loadResult { .status = RequestStatus::LoadComplete };

    if (hostPools == 1U)
    {
      PLOGD << "Everything to pool 0";

      njson cmd;
      cmd["dirs"] = njson::array();

      for(auto& poolDataDir : fs::directory_iterator(dataSetsRoot))
        cmd["dirs"].emplace_back(poolDataDir.path().string());

      auto result = submitSync(0, KvQueryType::InternalLoad, std::move(cmd));
      loadResult = std::any_cast<LoadResult> (result);
    }
    else if (hostPools == sourcePools)
    {
      PLOGD << "Direct";

      std::latch latch{static_cast<std::ptrdiff_t>(hostPools)};
      std::mutex resultsMux;
      std::vector<std::any> results;

      auto onResult = [&latch, &results, &resultsMux](auto r)
      {
        {
          std::scoped_lock lck{resultsMux};
          results.emplace_back(std::move(r));
        }
        latch.count_down();
      };
      
      for(auto& poolDataDir : fs::directory_iterator(dataSetsRoot))
      {
        const auto& poolPath = poolDataDir.path();

        njson cmd;
        cmd["dirs"] = njson::make_array(1, poolPath.string());

        m_pools[std::stoul(poolPath.filename())]->execute(KvCommand{  .contents = std::move(cmd),
                                                                      .syncResponseHandler = onResult,
                                                                      .type = KvQueryType::InternalLoad});
      }
      

      latch.wait();

      // collate results
      for(auto& result : results)
        loadResult += std::any_cast<LoadResult> (result);
    }    
    else
    {
      // there's no shortcut to this because source and host pools can't be inferred
      // have to recalculate each source session's pool from its token
      PLOGD << "Re-map";      
      loadResult = loadRemap(dataSetsRoot);
    }


    loadResult.loadTime = NemesisClock::now() - start;

    return loadResult;
  }
 */

private:
  
  /* 
  LoadResult loadRemap (const fs::path& dataSetsRoot)
  {
    LoadResult loadResult { .status = RequestStatus::LoadComplete};
    std::atomic_size_t nSessions{0}, nKeys{0};
    std::atomic_bool error{false};


    for(auto& poolDir : fs::directory_iterator(dataSetsRoot))
    {
      const auto& poolPath = poolDir.path();      

      PLOGD << "Loading pool data from: " << poolPath;

      // open each file, get token, create pool id, send to pool
      for(auto& file : fs::directory_iterator(poolPath))
      {
        std::ifstream seshStream {file.path()};
        njson sessions = njson::parse(seshStream);

        PLOGD << "File " << file.path() << " has " << sessions.size() << " session";

        std::latch latch{static_cast<std::ptrdiff_t>(sessions.size())};

        auto onResult = [&latch, &nSessions, &nKeys, &error](auto r)
        {
          LoadResult lr = std::any_cast<LoadResult> (r);
          nSessions += lr.nSessions;
          nKeys += lr.nKeys;

          if (!error) // if not already in an error condition
            error = !LoadResult::statusSuccess(lr);

          latch.count_down();
        };

        for(auto& sesh : sessions.array_range())
        {
          const SessionToken& token = sesh["sh"]["tkn"].as<SessionToken>();

          njson cmd;
          cmd["sesh"] = std::move(sesh);
          
          const auto poolId = getPoolId(token);
          m_pools[poolId]->execute(KvCommand{ .contents = std::move(cmd),
                                              .syncResponseHandler = onResult,
                                              .type = KvQueryType::InternalLoad});
        }

        latch.wait();
      }
    }

    return LoadResult{ .status = error ? RequestStatus::LoadError : RequestStatus::LoadComplete,
                              .nSessions = nSessions.load(),
                              .nKeys = nKeys.load()};
  }
 */

  


  bool getSessionToken(KvWebSocket * ws, const std::string_view queryRspName, const njson& cmd, SessionToken& tkn)
  {
    // use cmd.object_range().cbegin()->value() instead being called with getSessionToken (ws, "blah", cmd.at(queryName), tkn)?
    if (cmd.contains("tkn") && cmd.at("tkn").is<SessionToken>())
    {
      tkn = cmd.at("tkn").as<SessionToken>();
      return true;
    }
    else
    {
      send(ws, createErrorResponse(queryRspName, RequestStatus::SessionTokenInvalid).to_string());
      return false;
    }
  }


  // Send response to client: must only be called from the originating I/O thread
  ndb_always_inline void send (KvWebSocket * ws, const std::string& msg)
  {
    ws->send(msg, WsSendOpCode);
  }


  ndb_always_inline void send (KvWebSocket * ws, const njson& msg)
  {
    ws->send(msg.to_string(), WsSendOpCode);
  }


  bool isValid (const std::string& queryRspName, KvWebSocket * ws, 
                const njson& cmd, const std::map<const std::string_view, const Param>& params,
                std::function<std::tuple<RequestStatus, const std::string_view>(const njson&)> onPostValidate = nullptr)
  {
    const auto [stat, msg] = isCmdValid<RequestStatus, RequestStatus::Ok, RequestStatus::ParamMissing, RequestStatus::ValueTypeInvalid>(cmd, params, onPostValidate);
    
    if (stat != RequestStatus::Ok)
    {
      PLOGD << msg;
      send(ws, createErrorResponse(queryRspName, stat, msg).to_string());
    }
      
    return stat == RequestStatus::Ok;
  }


  // SESSION
  ndb_always_inline void sessionNew(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType      = KvQueryType::ShNew;
    static const std::string queryName      = QueryTypeToName.at(queryType);
    static const std::string queryRspName   = queryName + "_RSP";
    
    
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      if (cmd.at("name").empty())
        return {RequestStatus::ValueSize, "Session name empty"};

      return {RequestStatus::Ok, ""};
    };


    auto& cmd = json.at(queryName);

    if (isValid(queryRspName, ws, cmd, {{Param::required("name", JsonString)}, {Param::optional("expiry", JsonObject)}, {Param::optional("shared", JsonBool)}}, validate))
    {
      bool valid = true;

      if (cmd.contains("expiry"))
        valid = isValid(queryRspName, ws, cmd.at("expiry"), {{Param::required("duration", JsonUInt)}, {Param::required("deleteSession", JsonBool)}});
      else
      {
        cmd["expiry"]["duration"] = 0U; // defaults to never expire
        cmd["expiry"]["deleteSession"] = true;
      }

      if (valid)
      {
        cmd["shared"] = cmd.get_value_or<bool>("shared", false);

        const auto token = createSessionToken(cmd.at("name").as_string(), cmd["shared"] == true);
        send(ws, SessionExecutor::newSession(m_sessions, token, cmd));
      }
    }
  }

  
  ndb_always_inline void sessionEnd(KvWebSocket * ws, njson&& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::ShEnd);
    static const std::string queryRspName   = queryName + "_RSP";

    SessionToken token;
    
    if (getSessionToken(ws, queryRspName, json.at(queryName), token))
      send(ws, SessionExecutor::endSession(m_sessions, token));
  }

  
  ndb_always_inline void sessionOpen(KvWebSocket * ws, njson&& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::ShOpen);
    static const std::string queryRspName   = queryName +"_RSP";


    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
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

  
  ndb_always_inline void sessionInfo(KvWebSocket * ws, njson&& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::ShInfo);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);

    if (cmd.size() != 1U)
      send(ws, createErrorResponse(queryRspName, RequestStatus::CommandSyntax).to_string());
    else
    {
      SessionToken token;
    
      if (getSessionToken(ws, queryName, cmd, token))
        send(ws, SessionExecutor::sessionInfo(m_sessions, token));
    }
  }

  
  ndb_always_inline void sessionInfoAll(KvWebSocket * ws, njson&& json)
  {
    send(ws, SessionExecutor::sessionInfoAll(m_sessions));
  }
  

  ndb_always_inline void sessionExists(KvWebSocket * ws, njson&& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::ShExists);
    static const std::string queryRspName   = queryName + "_RSP";

    auto& cmd = json.at(queryName);

    if (isValid(queryRspName, ws, cmd, {{Param::required("tkns", JsonArray)}}))
      send(ws, SessionExecutor::sessionExists(m_sessions, cmd.at("tkns")));
  }


  ndb_always_inline void sessionEndAll(KvWebSocket * ws, njson&& json)
  {
    send(ws, SessionExecutor::sessionEndAll(m_sessions));
  }

  
  /*
  ndb_always_inline void sessionSave(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType      = KvQueryType::ShSave;
    static const std::string queryName      = QueryTypeToName.at(queryType);
    static const std::string queryRspName   = queryName +"_RSP";

    
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
    

    auto& cmd = json.at(queryName);

    if (!NemesisConfig::kvSaveEnabled(m_config))
      send(ws, createErrorResponseNoTkn(queryRspName, RequestStatus::CommandDisabled).to_string());
    else if (isValid(queryRspName, ws, cmd, {{Param::required("name", JsonString)}, {Param::optional("tkns", JsonArray)}}, validate))
    {
      const auto& name = cmd.at("name").as_string();

      njson rsp;
      rsp[queryRspName]["name"] = name;
      
      const auto timestampDir = std::to_string(KvSaveClock::now().time_since_epoch().count());
      const auto root = fs::path {NemesisConfig::kvSavePath(m_config)} / name / timestampDir;
      const auto metaPath = fs::path{root} / "md";
      const auto dataPath = fs::path{root} / "data";

      std::ofstream metaStream;

      if (!fs::create_directories(metaPath))
      {
        rsp[queryRspName]["st"] = toUnderlying(RequestStatus::SaveDirWriteFail);
        send(ws, rsp.to_string());
      }
      else if (metaStream.open(metaPath/"md.json", std::ios_base::trunc | std::ios_base::out); !metaStream.is_open())
      {
        rsp[queryRspName]["st"] = toUnderlying(RequestStatus::SaveDirWriteFail);
        send(ws, rsp.to_string());
      }
      else
      {
        const bool haveTkns = cmd.contains("tkns");

        rsp[queryRspName]["st"] = toUnderlying(RequestStatus::SaveStart);
        send(ws, rsp.to_string());

        // write metadata before we start incase we're interrupted mid-save
        auto start = KvSaveClock::now();
        njson metadata;
        metadata["name"] = name;
        metadata["status"] = toUnderlying(KvSaveStatus::Pending);        
        metadata["pools"] = m_pools.size();
        metadata["start"] = std::chrono::time_point_cast<KvSaveMetaDataUnit>(start).time_since_epoch().count();
        metadata["saveType"] = haveTkns ? toUnderlying(SaveType::SelectSessions) : toUnderlying(SaveType::AllSessions);
        metadata["complete"] = 0;
        
        metadata.dump(metaStream);

        // build and send save to pools
        njson saveCmd;
        saveCmd["poolDataRoot"] = dataPath.string();

        if (haveTkns)
          saveCmd["tkns"] = std::move(cmd.at("tkns"));

        
        const auto results = submitSync(ws, queryType, queryName, queryRspName, std::move(saveCmd));

        RequestStatus st = RequestStatus::Ok;

        for (const auto& result : results)
          st = (std::any_cast<RequestStatus>(result) == RequestStatus::SaveComplete ? RequestStatus::SaveComplete : RequestStatus::SaveError);
        
        const auto end = KvSaveClock::now();
        
        // update metdata
        metadata["status"] = toUnderlying(KvSaveStatus::Complete);
        metadata["complete"] = std::chrono::time_point_cast<KvSaveMetaDataUnit>(end).time_since_epoch().count();
        metaStream.seekp(0);
        metadata.dump(metaStream);

        // send command rsp
        rsp[queryRspName]["st"] = toUnderlying(st);

        // duration is unpredictable, making testing responses a PITA
        #ifndef NDB_UNIT_TEST
        rsp[queryRspName]["duration"] = std::chrono::duration_cast<KvSaveMetaDataUnit>(end-start).count();
        #endif

        send(ws, rsp.to_string());
      }
    }
  }  


  ndb_always_inline void sessionLoad(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType      = KvQueryType::ShLoad;
    static const std::string queryName      = QueryTypeToName.at(queryType);
    static const std::string queryRspName   = queryName +"_RSP";

    const auto& loadPath = fs::path{NemesisConfig::kvSavePath(m_config)};

    auto validate = [this, loadPath](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      for (const auto& item : cmd.at("names").array_range())
      {
        if (!item.is_string())
          return {RequestStatus::ValueTypeInvalid, "names contains incorrect data type"};
        else
        {
          // confirm load dir exists, i.e. :  <loadPath>/<loadName>
          if (!fs::exists(loadPath / item.as_string()))
            return {RequestStatus::LoadError, "Load path does not exist"};
        } 
      }

      return {RequestStatus::Ok, ""};
    };


    auto& cmd = json.at(queryName);

    if (isValid(queryRspName, ws, cmd, {{Param::required("names", JsonArray)}}, validate))
    {
      njson rsp;
        
      for (const auto& item : cmd.at("names").array_range())
      {
        const auto loadName = item.as_string();
        const auto loadRoot = loadPath / loadName;

        rsp[queryRspName][loadName]["m"] = "";
        rsp[queryRspName][loadName]["sessions"] = 0;
        rsp[queryRspName][loadName]["keys"] = 0;

        if (auto dataSetPath = getDefaultDataSetPath(loadRoot); dataSetPath.empty())
          rsp[queryRspName][loadName]["st"] = toUnderlying(RequestStatus::LoadComplete);  // no datasets, not an error
        else
        {
          const fs::path datasets = dataSetPath / "data";

          if (!fs::exists(datasets))
          {
            rsp[queryRspName][loadName]["st"] = toUnderlying(RequestStatus::LoadError);
            rsp[queryRspName][loadName]["m"] = "Data directory does not exist";
          }
          else
          {            
            const auto result = load(datasets);
            rsp[queryRspName][loadName]["st"] = toUnderlying(result.status);              
            rsp[queryRspName][loadName]["sessions"] = result.nSessions;
            rsp[queryRspName][loadName]["keys"] = result.nKeys;
          }
        }
      }
      
      send(ws, rsp.to_string());
    }
  }

  
 
  */


  // DATA
  template<typename F>
  void handleKvExecuteResult(KvWebSocket * ws, const njson& rsp, F&& handler)
    requires(std::is_invocable_v<F, CacheMap&, SessionToken&, njson&>)
  {
    using R = std::invoke_result_t<F, CacheMap&, SessionToken&, njson&>;

    if constexpr (std::is_same_v<R, njson>)
    {
      // don't send null response, i.e. setQ and addQ only respond on error
      if (!rsp.is_null())
        send(ws, rsp);
    }
    else
      send(ws, rsp);
  }


  template<typename F>
  void callKvHandler(KvWebSocket * ws, CacheMap& map, const SessionToken& token, njson& cmd, F&& handler)
    requires(std::is_invocable_v<F, CacheMap&, SessionToken&, njson&>)
  {
    const auto rsp = std::invoke(handler, map, token, cmd);
    handleKvExecuteResult(ws, rsp, handler);
  }


  template<typename F>
  void callKvHandler(KvWebSocket * ws, CacheMap& map, const SessionToken& token, const njson& cmd, F&& handler)
    requires(std::is_invocable_v<F, CacheMap&, SessionToken&, njson&>)
  {
    const auto rsp = std::invoke(handler, map, token, cmd);
    handleKvExecuteResult(ws, rsp, handler);
  }


  std::optional<std::reference_wrapper<Sessions::Session>> getSession (KvWebSocket * ws, const std::string_view cmdRspName, const njson& cmd, SessionToken& token)
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

  
  template<typename F>
  void executeKvCommand(const std::string_view cmdRspName, KvWebSocket * ws, njson& cmd, F&& handler)
    requires(std::is_invocable_v<F, CacheMap&, SessionToken&, njson&>)
  {
    SessionToken token;
    if (auto session = getSession(ws, cmdRspName, cmd, token); session)
      callKvHandler(ws, session->get().map, token, cmd, handler);
  }


  template<typename F>
  void executeKvCommand(const std::string_view cmdRspName, KvWebSocket * ws, const njson& cmd, F&& handler)
    requires(std::is_invocable_v<F, CacheMap&, SessionToken&, njson&>)
  {
    SessionToken token;
    if (auto session = getSession(ws, cmdRspName, cmd, token); session)
      callKvHandler(ws, session->get().map, token, cmd, handler);
  }


  ndb_always_inline void set(KvWebSocket * ws, njson&& json)
  {
    static const std::string queryName    = QueryTypeToName.at(KvQueryType::KvSet);
    static const std::string queryRspName = queryName +"_RSP";

    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };

    auto& cmd = json.at(queryName);
    
    if (isValid(queryRspName, ws, cmd, {{Param::required("keys", JsonObject)}}, validate))
      executeKvCommand(queryRspName, ws, cmd, KvExecutor::set);
  }


  ndb_always_inline void setQ(KvWebSocket * ws, njson&& json)
  {
    static const std::string queryName    = QueryTypeToName.at(KvQueryType::KvSetQ);
    static const std::string queryRspName = queryName +"_RSP";

    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };

    auto& cmd = json.at(queryName);
    
    if (isValid(queryRspName, ws, cmd, {{Param::required("keys", JsonObject)}}, validate))
      executeKvCommand(queryRspName, ws, cmd, KvExecutor::setQ);
  }

  
  ndb_always_inline void get(KvWebSocket * ws, njson&& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvGet);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    
    if (isValid(queryRspName, ws, cmd, {{Param::required("keys", JsonArray)}}))
      executeKvCommand(queryRspName, ws, cmd, KvExecutor::get);
  }

  
  ndb_always_inline void add(KvWebSocket * ws, njson&& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvAdd);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);

    if (isValid(queryRspName, ws, cmd, {{Param::required("keys", JsonObject)}}))
      executeKvCommand(queryRspName, ws, cmd, KvExecutor::add);
  }


  ndb_always_inline void addQ(KvWebSocket * ws, njson&& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvAddQ);
    static const std::string queryRspName   = queryName +"_RSP";

    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };

    auto& cmd = json.at(queryName);

    if (isValid(queryRspName, ws, cmd, {{Param::required("keys", JsonObject)}}, validate))
      executeKvCommand(queryRspName, ws, cmd, KvExecutor::addQ);
  }


  ndb_always_inline void remove(KvWebSocket * ws, njson&& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvRemove);
    static const std::string queryRspName   = queryName +"_RSP";

    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };

    auto& cmd = json.at(queryName);

    if (isValid(queryRspName, ws, cmd, {{Param::required("keys", JsonArray)}}, validate))
      executeKvCommand(queryRspName, ws, cmd, KvExecutor::remove);
  }

  
  ndb_always_inline void clear(KvWebSocket * ws, njson&& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvClear);
    static const std::string queryRspName   = queryName +"_RSP";
    
    executeKvCommand(queryRspName, ws, json.at(queryName), KvExecutor::clear);
  }


  ndb_always_inline void count(KvWebSocket * ws, njson&& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvCount);
    static const std::string queryRspName   = queryName +"_RSP";
    
    executeKvCommand(queryRspName, ws, json.at(queryName), KvExecutor::count);
  }


  ndb_always_inline void contains(KvWebSocket * ws, njson&& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvContains);
    static const std::string queryRspName   = queryName +"_RSP";
    
    const auto& cmd = json.at(queryName);
    if (isValid(queryRspName, ws, cmd, {{Param::required("keys", JsonArray)}}))
      executeKvCommand(queryRspName, ws, cmd, KvExecutor::contains);
  }


  ndb_always_inline void find(KvWebSocket * ws, njson&& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvFind);
    static const std::string queryRspName   = queryName +"_RSP";
    
    
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      if (cmd.at("rsp") != "paths" && cmd.at("rsp") != "kv" && cmd.at("rsp") != "keys")
        return {RequestStatus::CommandSyntax, "'rsp' invalid value"};
      else if (const auto& path = cmd.at("path").as_string() ; path.empty())
        return {RequestStatus::ValueSize, "'path' is empty"};

      return {RequestStatus::Ok, ""};
    };


    auto& cmd = json.at(queryName);
    
    if (isValid(queryRspName, ws, cmd, {{Param::required("path", JsonString)}, {Param::required("rsp", JsonString)}, {Param::optional("keys", JsonArray)}}, validate))
    {
      if (!cmd.contains("keys"))
        cmd["keys"] = njson::array(); // executor expects "keys"

      executeKvCommand(queryRspName, ws, cmd, KvExecutor::find);
    }
  }


  ndb_always_inline void update(KvWebSocket * ws, njson&& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvUpdate);
    static const std::string queryRspName   = queryName +"_RSP";

    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      // 'value' can be any valid JSON type, so just check it's present here
      if (!cmd.contains("value"))
        return {RequestStatus::ParamMissing, "Missing parameter"};
      
      return {RequestStatus::Ok, ""};
    };

    auto& cmd = json.at(queryName);
    
    if (isValid(queryRspName, ws, cmd, {{Param::required("key", JsonString)}, {Param::required("path", JsonString)}}, validate))
      executeKvCommand(queryRspName, ws, cmd, KvExecutor::update);
  }


  ndb_always_inline void keys(KvWebSocket * ws, njson&& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvKeys);
    static const std::string queryRspName   = queryName +"_RSP";

    executeKvCommand(queryRspName, ws, json.at(queryName), KvExecutor::keys);
  }


  ndb_always_inline void clearSet(KvWebSocket * ws, njson&& json)
  {
    static const std::string queryName      = QueryTypeToName.at(KvQueryType::KvClearSet);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    if (isValid(queryRspName, ws, cmd, {{Param::required("keys", JsonObject)}}))
      executeKvCommand(queryRspName, ws, json.at(queryName), KvExecutor::clearSet);
  }


private:
  njson m_config;
  Sessions m_sessions;
};

}
}
}

#endif
