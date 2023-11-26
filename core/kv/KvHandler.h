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


namespace nemesis { namespace core { namespace kv {


class KvHandler
{
public:
  KvHandler(const std::size_t nPools, const std::size_t coreOffset, const njson& config) :
    m_createSessionPoolId(nPools == 1U ? SessionIndexers[1U] : SessionIndexers[0U]),
    m_config(config)
  {
    for (std::size_t pool = 0, core = coreOffset ; pool < nPools ; ++pool, ++core)
      m_pools.emplace_back(new KvPoolWorker{core, pool});
  }

  ~KvHandler()
  {
    for (auto& pool : m_pools)
      delete pool;
  }

private:
  
  // CAREFUL: these have to be in the order of KvQueryType enum
  const std::array<std::function<void(KvWebSocket *, njson&&)>, static_cast<std::size_t>(KvQueryType::MAX)> Handlers = 
  {
    std::bind(&KvHandler::sessionNew,       std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionEnd,       std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionOpen,      std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionInfo,      std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionInfoAll,   std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionSave,      std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionLoad,      std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::set,              std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::setQ,             std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::get,              std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::add,              std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::addQ,             std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::remove,           std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::clear,            std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::count,            std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::contains,         std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::find,             std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::update,           std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::keys,             std::ref(*this), std::placeholders::_1, std::placeholders::_2)    
  };


public:

  
  RequestStatus handle(KvWebSocket * ws, const std::string_view& command, njson&& json)
  {
    RequestStatus status = RequestStatus::Ok;
    
    if (auto itType = QueryNameToType.find(command) ; itType != QueryNameToType.cend())
    {
      // TODO QueryNameToType.second is tuple from previous version, keep for now but doesn't actually need to be
      auto [queryType] = itType->second;  
      auto handler = Handlers[static_cast<std::size_t>(queryType)];

      try
      {
        handler(ws, std::move(json));
      }
      catch (const std::exception& kex)
      {
        status = RequestStatus::Unknown;
      }
    }
    else
      status = RequestStatus::CommandNotExist;

    return status;
  }


  void monitor ()
  {
    for(auto& pool : m_pools)
      pool->execute(KvCommand{.type = KvQueryType::InternalSessionMonitor});
  }


  LoadResult load(const fs::path& dataSetsRoot/*, const SaveType saveType*/)
  {
    std::cout << "Loading from " << dataSetsRoot << '\n';

    const auto hostPools = m_pools.size();
    const auto sourcePools = countFiles(dataSetsRoot);
    
    LoadResult loadResult { .status = RequestStatus::LoadComplete };

    auto start = NemesisClock::now();

    if (hostPools == 1U)
    {
      //std::cout << "Everything to pool 0\n";

      njson cmd;
      cmd["dirs"] = njson::array();

      for(auto& poolDataDir : fs::directory_iterator(dataSetsRoot))
        cmd["dirs"].emplace_back(poolDataDir.path().string());

      auto result = submitSync(0, KvQueryType::InternalLoad, std::move(cmd));
      loadResult = std::any_cast<LoadResult> (result);
    }
    else if (hostPools == sourcePools)
    {
      //std::cout << "Direct\n";

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
                                                                      .type = KvQueryType::InternalLoad,
                                                                      .syncResponseHandler = onResult});
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
      //std::cout << "Re-map\n";      
      loadResult = loadRemap(dataSetsRoot);
    }


    auto end = NemesisClock::now();
    loadResult.loadTime = end - start;

    return loadResult;
  }


private:
  
  
  LoadResult loadRemap (const fs::path& dataSetsRoot)
  {
    LoadResult loadResult { .status = RequestStatus::LoadComplete};
    std::atomic_size_t nSessions{0}, nKeys{0};
    std::atomic_bool error{false};


    for(auto& poolDir : fs::directory_iterator(dataSetsRoot))
    {
      const auto& poolPath = poolDir.path();      

      std::cout << "Loading pool data from: " << poolPath << '\n';

      // open each file, get token, create pool id, send to pool
      for(auto& file : fs::directory_iterator(poolPath))
      {
        std::ifstream seshStream {file.path()};
        njson sessions = njson::parse(seshStream);

        //std::cout << "File " << file.path() << " has " << sessions.size() << " session" << '\n';

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
                                              .type = KvQueryType::InternalLoad,
                                              .syncResponseHandler = onResult});
        }

        latch.wait();
      }
    }

    return LoadResult{ .status = error ? RequestStatus::LoadError : RequestStatus::LoadComplete,
                              .nSessions = nSessions.load(),
                              .nKeys = nKeys.load()};
  }


  fc_always_inline PoolId getPoolId (const SessionToken& shtk)
  {
    PoolId id;
    m_createSessionPoolId(shtk, id);
    return id;
  }


  bool getSessionToken(KvWebSocket * ws, const std::string_view queryRspName, njson& cmd, SessionToken& t)
  {
    bool valid = false;
    if (cmd.contains("tkn") && cmd.at("tkn").is_uint64())
    {
      t = cmd.at("tkn").as<SessionToken>();
      cmd.erase("tkn");
      valid = true;
      // if (const auto& value = cmd.at("tkn").as_string(); value.empty())
      //   ws->send(createErrorResponse(queryRspName, RequestStatus::SessionTokenInvalid).to_string(), WsSendOpCode);
      // else
      // {
      //   //t = std::move(cmd.at("tkn").as_string());
      //   t = cmd.at("tkn").as<SessionToken>();
      //   cmd.erase("tkn");
      //   valid = true;
      // }
    }
    else
      ws->send(createErrorResponse(queryRspName, RequestStatus::SessionTokenInvalid).to_string(), WsSendOpCode);

    return valid;
  }


  // Submit asynchronously with just token
  fc_always_inline void submit(KvWebSocket * ws, const SessionToken& token, const KvQueryType queryType, const std::string_view command, const std::string_view rspName, njson&& cmd = "")
  {
    submit(ws, getPoolId(token), token, queryType, command, rspName, std::move(cmd));
  } 


  // Submit asynchronously with pool id
  fc_always_inline void submit(KvWebSocket * ws, const PoolId& poolId, const SessionToken& token, const KvQueryType queryType, const std::string_view command, const std::string_view rspName, njson&& cmd = "")
  {
    m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                        .loop = uWS::Loop::get(),
                                        .contents = std::move(cmd),
                                        .type = queryType,
                                        .shtk = token});
  } 

  
  // Submit to a pool synchronously, when not triggered from a WebSocket client (i.e. load on startup)
  fc_always_inline std::any submitSync(const PoolId pool, const KvQueryType queryType, njson&& cmd = "")
  {
    std::latch latch{1U};
    std::any result;

    auto onResult = [&latch, &result](auto r)
    {
      result = std::move(r);
      latch.count_down();
    };

    m_pools[pool]->execute(KvCommand{ .ws = nullptr,
                                      .loop = nullptr,
                                      .contents = std::move(cmd),
                                      .type = queryType,
                                      .syncResponseHandler = onResult});

    latch.wait();
    return result;
  }

  
  // Submit to a pool synchronously
  fc_always_inline std::any submitSync(KvWebSocket * ws, const SessionToken& token, const KvQueryType queryType, const std::string_view command, const std::string_view rspName, njson&& cmd = "")
  {
    std::latch latch{1U};
    std::any result;

    auto onResult = [&latch, &result](auto r)
    {
      result = std::move(r);
      latch.count_down();
    };


    const auto poolId = getPoolId(token);
    m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                        .loop = uWS::Loop::get(),
                                        .contents = std::move(cmd),
                                        .type = queryType,
                                        .syncResponseHandler = onResult,
                                        .shtk = token});

    latch.wait();
    return result;
  }


  // Submit to all pools synchronously
  fc_always_inline std::vector<std::any> submitSync(KvWebSocket * ws, const KvQueryType queryType, const std::string_view command, const std::string_view rspName, njson&& cmd = "")
  {
    std::latch latch{static_cast<std::ptrdiff_t>(m_pools.size())};
    std::vector<std::any> results;
    std::mutex resultsMux;

    auto onResult = [&latch, &results, &resultsMux](auto r)
    {
      {
        std::scoped_lock lck{resultsMux};
        results.emplace_back(std::move(r));
      }
      
      latch.count_down();
    };


    for (auto& pool : m_pools)
    {
      pool->execute(KvCommand{  .ws = ws,
                                .loop = uWS::Loop::get(),
                                .contents = cmd,
                                .type = queryType,
                                .syncResponseHandler = onResult});
    }
    
    latch.wait();
    return results;
  }

  
  // SESSION
  
  fc_always_inline void sessionNew(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType      = KvQueryType::ShNew;
    static const std::string queryName      = QueryTypeToName.at(queryType);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);

    if (cmd.size() < 1U || cmd.size() > 3U)
      ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).to_string(), WsSendOpCode);
    else if (!cmd.contains("name"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing, "name").to_string(), WsSendOpCode);
    else if (!cmd.at("name").is_string())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "name").to_string(), WsSendOpCode);
    else if (const auto value = cmd.at("name").as_string(); value.empty())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueSize, "name").to_string(), WsSendOpCode);
    else
    {
      bool expiryValid = true, sharedValid = true;

      if (cmd.contains("expiry"))
      {
        const auto& expiry = cmd.at("expiry");
        expiryValid = expiry.contains("duration") && expiry.at("duration").is_uint64() &&
                      expiry.contains("deleteSession") && expiry.at("deleteSession").is_bool();
      }        
      else
      {
        cmd["expiry"]["duration"] = 0U;
        cmd["expiry"]["deleteSession"] = true;
      }
        

      if (cmd.contains("shared"))
        sharedValid = cmd.at("shared").is_bool();
      else
        cmd["shared"] = false;

      
      if (!expiryValid)
        ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax, "expiry").to_string(), WsSendOpCode);
      else if (!sharedValid)
        ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax, "shared").to_string(), WsSendOpCode);
      else
      {
        const auto token = createSessionToken(cmd.at("name").as_string(), cmd["shared"] == true);
        
        PoolId poolId = getPoolId(token);
        submit(ws, token, queryType, queryName, queryRspName, std::move(cmd));
      }
    } 
  }

  
  fc_always_inline void sessionOpen(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType      = KvQueryType::ShOpen;
    static const std::string queryName      = QueryTypeToName.at(queryType);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);

    if (cmd.size() != 1U)
      ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).to_string(), WsSendOpCode);
    else if (!cmd.contains("name"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing, "name").to_string(), WsSendOpCode);
    else if (!cmd.at("name").is_string())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "name").to_string(), WsSendOpCode);
    else if (const auto value = cmd.at("name").as_string(); value.empty())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueSize, "name").to_string(), WsSendOpCode);
    else
    {
      // ask the pool if the session token exists

      // we don't need to check if the pool is shareable because if it isn't, the token will be completely different,
      // so it'll either go to the wrong pool or not exist in the correct pool
      const auto token = createSessionToken(cmd.at("name").as_string(), true);
      
      const PoolId pool = getPoolId(token);
      const auto result = submitSync(ws, token, queryType, queryName, queryRspName);
      
      const auto [exists, shared] = std::any_cast<std::tuple<bool, bool>>(result) ;

      // if they attempt to open a sesh that isn't shared, we can't accurately report if the 
      // session exists because the session will have a completely different token, so it either
      // completely works or a general failure response 
      njson rsp;
      if (exists && shared)
      {
        rsp[queryRspName]["tkn"] = token;
        rsp[queryRspName]["st"] = static_cast<int>(RequestStatus::Ok);
      }
      else
      {
        rsp[queryRspName]["tkn"] = njson::null();
        rsp[queryRspName]["st"] = static_cast<int>(RequestStatus::SessionOpenFail);
      }        

      ws->send(rsp.to_string(), WsSendOpCode);
    }
  }

  
  fc_always_inline void sessionInfo(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType      = KvQueryType::ShInfo;
    static const std::string queryName      = QueryTypeToName.at(queryType);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);

    if (cmd.size() != 1U)
      ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).to_string(), WsSendOpCode);
    else
    {
      SessionToken token;
    
      if (getSessionToken(ws, queryName, json.at(queryName), token))
        submit(ws, token, queryType, queryName, queryRspName);
    }
  }


  fc_always_inline void sessionInfoAll(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType      = KvQueryType::ShInfoAll;
    static const std::string queryName      = QueryTypeToName.at(queryType);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);

    if (!cmd.empty())
      ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).to_string(), WsSendOpCode);
    else
    {
      auto results = submitSync(ws, queryType, queryName, queryRspName);
      
      std::size_t totalSesh{0}, totalKeys{0};
      for(auto& result : results)
      {
        auto [sessions, keys] = std::any_cast<std::tuple<const std::size_t, const std::size_t>>(result);
        totalSesh += sessions;
        totalKeys += keys;
      }

      njson rsp;
      rsp[queryRspName]["totalSessions"] = totalSesh;
      rsp[queryRspName]["totalKeys"] = totalKeys;

      ws->send(rsp.to_string(), WsSendOpCode);
    }
  }


  fc_always_inline void sessionEnd(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType = KvQueryType::ShEnd;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    SessionToken token;
    
    if (getSessionToken(ws, queryName, json.at(queryName), token))
      submit(ws, token, queryType, queryName, queryRspName);
  }
  
  
  fc_always_inline void sessionSave(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType      = KvQueryType::ShSave;
    static const std::string queryName      = QueryTypeToName.at(queryType);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    const bool haveTkns = cmd.contains("tkns");

    if (!NemesisConfig::kvSaveEnabled(m_config))
      ws->send(createErrorResponseNoTkn(queryRspName, RequestStatus::CommandDisabled).to_string(), WsSendOpCode);
    else if (!(cmd.contains("name") && cmd.at("name").is_string()))
      ws->send(createErrorResponseNoTkn(queryRspName, RequestStatus::CommandSyntax).to_string(), WsSendOpCode);
    else if (haveTkns && !cmd.at("tkns").is_array())
      ws->send(createErrorResponseNoTkn(queryRspName, RequestStatus::ValueTypeInvalid, "tkns").to_string(), WsSendOpCode);
    else if (haveTkns && cmd.at("tkns").empty())
      ws->send(createErrorResponseNoTkn(queryRspName, RequestStatus::ValueSize, "tkns").to_string(), WsSendOpCode);
    else
    {
      //validate tkns array, if present, before continuing
      bool tknsValid = true;
      if (haveTkns)
      {
        for (const auto& item : cmd.at("tkns").array_range())
        {
          if (tknsValid = item.is_uint64(); !tknsValid)
            break;
        }
      }

      if (!tknsValid)
        ws->send(createErrorResponseNoTkn(queryRspName, RequestStatus::ValueTypeInvalid, "tkns").to_string(), WsSendOpCode);
      else
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
          ws->send(rsp.to_string(), WsSendOpCode);
        }
        else if (metaStream.open(metaPath/"md.json", std::ios_base::trunc | std::ios_base::out); !metaStream.is_open())
        {
          rsp[queryRspName]["st"] = toUnderlying(RequestStatus::SaveDirWriteFail);
          ws->send(rsp.to_string(), WsSendOpCode);
        }
        else
        {
          rsp[queryRspName]["st"] = toUnderlying(RequestStatus::SaveStart);
          ws->send(rsp.to_string(), WsSendOpCode);

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

          auto results = submitSync(ws, queryType, queryName, queryRspName, std::move(saveCmd));

          RequestStatus st = RequestStatus::Ok;

          for (auto& result : results)
            st = (std::any_cast<RequestStatus>(result) == RequestStatus::SaveComplete ? RequestStatus::SaveComplete : RequestStatus::SaveError);
          
          auto end = KvSaveClock::now();

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

          ws->send(rsp.to_string(), WsSendOpCode);
        }
      }
    }
  }  


  fc_always_inline void sessionLoad(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType      = KvQueryType::ShLoad;
    static const std::string queryName      = QueryTypeToName.at(queryType);
    static const std::string queryRspName   = queryName +"_RSP";

    const auto& loadPath = fs::path{NemesisConfig::kvSavePath(m_config)};

    auto& cmd = json.at(queryName);

    if (!(cmd.contains("names") && cmd.at("names").is_array()))
      ws->send(createErrorResponseNoTkn(queryRspName, RequestStatus::CommandSyntax).to_string(), WsSendOpCode);
    else if (!fs::exists(loadPath))
      ws->send(createErrorResponseNoTkn(queryRspName, RequestStatus::LoadError, "load path not exist").to_string(), WsSendOpCode);
    else
    {
      bool namesValid = true;
      RequestStatus checkStatus = RequestStatus::Ok;
      std::string msg;
      
      for (const auto& item : cmd.at("names").array_range())
      {
        if (namesValid = item.is_string(); !namesValid)
          checkStatus = RequestStatus::ValueTypeInvalid;
        else
        {
          // confirm load dir exists, i.e. :  <save_path>/<load_name>
          const fs::path loadRoot = loadPath / item.as_string();

          if (namesValid = fs::exists(loadRoot); !namesValid)
          {
            checkStatus = RequestStatus::LoadError;
            msg = item.as_string() + " does not exist";
          }
        } 
      }

      if (!namesValid)
        ws->send(createErrorResponseNoTkn(queryRspName, checkStatus, msg).to_string(), WsSendOpCode);
      else
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
        
        ws->send(rsp.to_string(), WsSendOpCode);
      }      
    }
  }

  
  // DATA

  fc_always_inline void set(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType    = KvQueryType::KvSet;
    static const std::string queryName    = QueryTypeToName.at(queryType);
    static const std::string queryRspName = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing, "keys").to_string(), WsSendOpCode);
    else if (!cmd.at("keys").is_object())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").to_string(), WsSendOpCode);
    else if (getSessionToken(ws, queryRspName, cmd, token))
      submit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }

  
  fc_always_inline void setQ(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType    = KvQueryType::KvSetQ;
    static const std::string queryName    = QueryTypeToName.at(queryType);
    static const std::string queryRspName = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing, "keys").to_string(), WsSendOpCode);
    else if (!cmd.at("keys").is_object())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").to_string(), WsSendOpCode);
    else if (getSessionToken(ws, queryRspName, cmd, token))
      submit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }

  
  fc_always_inline void get(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType      = KvQueryType::KvGet;
    static const std::string queryName      = QueryTypeToName.at(queryType);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing, "keys").to_string(), WsSendOpCode);
    else if (!cmd.at("keys").is_array())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").to_string(), WsSendOpCode);
    else if (getSessionToken(ws, queryRspName, cmd, token))
      submit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }

  
  fc_always_inline void add(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType      = KvQueryType::KvAdd;
    static const std::string queryName      = QueryTypeToName.at(queryType);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing, "keys").to_string(), WsSendOpCode);
    else if (!cmd.at("keys").is_object())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").to_string(), WsSendOpCode);
    else if (getSessionToken(ws, queryRspName, cmd, token))
      submit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }


  fc_always_inline void addQ(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType      = KvQueryType::KvAddQ;
    static const std::string queryName      = QueryTypeToName.at(queryType);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing, "keys").to_string(), WsSendOpCode);
    else if (!cmd.at("keys").is_object())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").to_string(), WsSendOpCode);
    else if (getSessionToken(ws, queryRspName, cmd, token))
      submit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }


  fc_always_inline void remove(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType = KvQueryType::KvRemove;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing, "keys").to_string(), WsSendOpCode);
    else if (!cmd.at("keys").is_array())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").to_string(), WsSendOpCode);
    else if (getSessionToken(ws, queryRspName, cmd, token))
      submit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }

  
  fc_always_inline void clear(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType = KvQueryType::KvClear;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    SessionToken token;

    if (getSessionToken(ws, queryName, json.at(queryName), token))
      submit(ws, token, queryType, queryName, queryRspName);
  }


  fc_always_inline void count(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType = KvQueryType::KvCount;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    SessionToken token;

    if (getSessionToken(ws, queryName, json.at(queryName), token))
      submit(ws, token, queryType, queryName, queryRspName);
  }


  fc_always_inline void contains(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType      = KvQueryType::KvContains;
    static const std::string queryName      = QueryTypeToName.at(queryType);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing).to_string(), WsSendOpCode);
    else if (!cmd.at("keys").is_array())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").to_string(), WsSendOpCode);
    else if (getSessionToken(ws, queryName, cmd, token))
      submit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }

  
  fc_always_inline void find(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType      = KvQueryType::KvFind;
    static const std::string queryName      = QueryTypeToName.at(queryType);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    
    SessionToken token;
    if (getSessionToken(ws, queryRspName, cmd, token))
    {
      // getSessionToken() deletes the "tkn" so only 'path' and 'rsp' must remain, with optional 'keys'
      if (cmd.size() < 2U || cmd.size() > 3U)
        ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).to_string(), WsSendOpCode);
      else if (!cmd.contains("path"))
        ws->send(createErrorResponse(queryRspName, RequestStatus::NoPath).to_string(), WsSendOpCode);
      else if (!cmd.at("path").is_string())
        ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "path").to_string(), WsSendOpCode);
      else if (const auto& path = cmd.at("path").as_string() ; path.empty())
        ws->send(createErrorResponse(queryRspName, RequestStatus::ValueSize, "path").to_string(), WsSendOpCode);      
      else if (!cmd.contains("rsp"))
        ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing, "rsp").to_string(), WsSendOpCode);
      else if (!cmd.at("rsp").is_string())
        ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "rsp").to_string(), WsSendOpCode);
      else if (cmd.at("rsp") != "paths" && cmd.at("rsp") != "kv" && cmd.at("rsp") != "keys")
        ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax, "rsp").to_string(), WsSendOpCode);
      else if (cmd.contains("keys") && !cmd.at("keys").is_array())
        ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").to_string(), WsSendOpCode);
      else
      {
        if (!cmd.contains("keys"))
          cmd["keys"] = njson::array();

        const auto poolId = getPoolId(token);
        
        m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                            .loop = uWS::Loop::get(),
                                            .contents = std::move(cmd),
                                            .type = queryType,
                                            .shtk = token});
      }
    }
  }
  
  
  fc_always_inline void update(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType      = KvQueryType::KvUpdate;
    static const std::string queryName      = QueryTypeToName.at(queryType);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    
    SessionToken token;
    if (getSessionToken(ws, queryRspName, cmd, token))
    {
      if (cmd.size() != 3U) // after tkn removed
        ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).to_string(), WsSendOpCode);
      else if (!cmd.contains("key"))
        ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing, "key").to_string(), WsSendOpCode);
      else if (!cmd.at("key").is_string())
        ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "key").to_string(), WsSendOpCode);
      else if (!cmd.contains("path"))
        ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing, "path").to_string(), WsSendOpCode);
      else if (!cmd.at("path").is_string())
        ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "path").to_string(), WsSendOpCode);
      else if (!cmd.contains("value"))
        ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing, "value").to_string(), WsSendOpCode);
      else
        submit(ws, token, queryType, queryName, queryRspName, std::move(cmd));
    }
  }


  fc_always_inline void keys(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType      = KvQueryType::KvKeys;
    static const std::string queryName      = QueryTypeToName.at(queryType);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    
    SessionToken token;
    if (getSessionToken(ws, queryRspName, cmd, token))
    {
      if (!cmd.empty()) // after tkn removed
        ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).to_string(), WsSendOpCode);
      else
        submit(ws, token, queryType, queryName, queryRspName, std::move(cmd));
    }
  }
  

private:
  std::vector<KvPoolWorker *> m_pools;
  std::function<void(const SessionToken&, PoolId&)> m_createSessionPoolId;
  njson m_config;
};

}
}
}

#endif
