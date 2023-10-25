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
#include <core/ks/KsSets.h>


namespace nemesis { namespace core { namespace kv {


class KvHandler
{
public:
  KvHandler(const std::size_t nPools, const std::size_t coreOffset, ks::Sets * ks) :
    m_createSessionPoolId(nPools == 1U ? SessionIndexers[1U] : SessionIndexers[0U]),
    m_ks(ks)
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
  const std::array<std::function<void(KvWebSocket *, njson&&)>, static_cast<std::size_t>(KvQueryType::Max)> Handlers = 
  {
    std::bind(&KvHandler::sessionNew,       std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionEnd,       std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionOpen,      std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionInfo,      std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionInfoAll,   std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::set,              std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::setQ,             std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::get,              std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::add,              std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::addQ,             std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::remove,           std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::clear,            std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::count,            std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::append,           std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::contains,         std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    //std::bind(&KvHandler::arrayMove,        std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    //std::bind(&KvHandler::find,             std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::update,           std::ref(*this), std::placeholders::_1, std::placeholders::_2)
  };


public:

  //std::tuple<RequestStatus,std::string> handle(KvWebSocket * ws, njson&& json)
  RequestStatus handle(KvWebSocket * ws, njson&& json)
  {
    RequestStatus status = RequestStatus::Ok;

    const std::string& queryName = json.cbegin().key();
    
    if (auto itType = QueryNameToType.find(queryName) ; itType != QueryNameToType.cend())
    {
      auto [queryType, commandType] = itType->second;

      if (commandType == json.at(queryName).type())
      {
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
        status = RequestStatus::CommandType;
    }
    else
      status = RequestStatus::CommandNotExist;

    return status;//std::make_tuple(status, queryName);
  }


  void monitor ()
  {
    for(auto& pool : m_pools)
      pool->execute(KvCommand{.type = KvQueryType::InternalSessionMonitor});
  }


private:
  

  fc_always_inline PoolId getPoolId (const SessionToken& shtk)
  {
    PoolId id;
    m_createSessionPoolId(shtk, id);
    return id;
  }


  bool getSessionToken(KvWebSocket * ws, const std::string_view queryRspName, njson& cmd, SessionToken& t)
  {
    if ((!cmd.contains ("tkn")) || !cmd.at("tkn").is_string() || cmd.at("tkn").get_ref<const std::string&>().empty())
    {
      ws->send(createErrorResponse(queryRspName, RequestStatus::SessionTokenInvalid).dump(), WsSendOpCode);
      return false;
    }
    else
    {
      t = std::move(cmd.at("tkn"));
      cmd.erase("tkn");
      return true;
    } 
  }


  fc_always_inline void sessionSubmit(KvWebSocket * ws, const SessionToken& token, const KvQueryType queryType, const std::string_view command, const std::string_view rspName, njson&& cmd = "")
  {
    const auto poolId = getPoolId(token);
    m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                        .loop = uWS::Loop::get(),
                                        .contents = std::move(cmd),
                                        .type = queryType,
                                        .shtk = token});
  } 


  fc_always_inline std::any sessionSubmitSync(KvWebSocket * ws, const SessionToken& token, const KvQueryType queryType, const std::string_view command, const std::string_view rspName, njson&& cmd = "")
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


  fc_always_inline std::vector<std::any> sessionSubmitSync(KvWebSocket * ws, const KvQueryType queryType, const std::string_view command, const std::string_view rspName, njson&& cmd = "")
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
                                .contents = std::move(cmd),
                                .type = queryType,
                                .syncResponseHandler = onResult});
    }
    
    latch.wait();
    return results;
  }


  fc_always_inline void sessionSubmitPairs(KvWebSocket * ws, njson&& cmd, const SessionToken& token, const KvQueryType queryType, const std::string_view command, const std::string_view rspName)
  {
    const auto poolId = getPoolId(token);

    for (auto& pair : cmd.items())
    {
      auto& value = pair.value(); 

      if (valueTypeValid(value))    [[likely]]
      {
        m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                            .loop = uWS::Loop::get(),
                                            .contents = std::move(pair),
                                            .type = queryType,
                                            .shtk = token});
      }
      else
        ws->send(createErrorResponse(rspName, RequestStatus::ValueTypeInvalid, pair.key()).dump(), WsSendOpCode);
    }
  }


  fc_always_inline void sessionSubmitKeys(KvWebSocket * ws, njson&& cmd, const SessionToken& token, const KvQueryType queryType, const std::string_view command, const std::string_view rspName)
  {
    const auto poolId = getPoolId(token);

    for (auto& key : cmd)
    {
      if (key.is_string())
      {        
        m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                            .loop = uWS::Loop::get(),
                                            .contents = std::move(key.get<std::string_view>()),
                                            .type = queryType,
                                            .shtk = token});
      }
      else
        ws->send(createErrorResponse(rspName, RequestStatus::KeyTypeInvalid, token).dump(), WsSendOpCode);
    }
  }


  // SESSION
  
  fc_always_inline void sessionNew(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionNew;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    auto& cmd = json.at(queryName);

    if (cmd.size() < 1U || cmd.size() > 3U)
      ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).dump(), WsSendOpCode);
    else if (!cmd.contains("name"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing, "name").dump(), WsSendOpCode);
    else if (!cmd.at("name").is_string())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "name").dump(), WsSendOpCode);
    else if (cmd.at("name").get_ref<const std::string&>().empty())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueSize, "name").dump(), WsSendOpCode);
    else
    {
      bool expiryValid = true, sharedValid = true;

      if (cmd.contains("expiry"))
      {
        const auto& expiry = cmd.at("expiry");
        expiryValid = expiry.contains("duration") && expiry.at("duration").is_number_unsigned() &&
                      expiry.contains("deleteSession") && expiry.at("deleteSession").is_boolean();
      }        
      else
      {
        cmd["expiry"]["duration"] = 0U;
        cmd["expiry"]["deleteSession"] = true;
      }
        

      if (cmd.contains("shared"))
        sharedValid = cmd.at("shared").is_boolean();
      else
        cmd["shared"] = false;

      
      if (!expiryValid)
        ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax, "expiry").dump(), WsSendOpCode);
      else if (!sharedValid)
        ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax, "shared").dump(), WsSendOpCode);
      else
      {
        const auto token = createSessionToken(cmd.at("name"), cmd["shared"] == true);
        
        PoolId poolId = getPoolId(token);
        sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd));
      }
    } 
  }


  fc_always_inline void sessionOpen(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType      = KvQueryType::SessionOpen;
    static const std::string queryName      = QueryTypeToName.at(queryType);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);

    if (cmd.size() != 1U)
      ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).dump(), WsSendOpCode);
    else if (!cmd.contains("name"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing, "name").dump(), WsSendOpCode);
    else if (!cmd.at("name").is_string())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "name").dump(), WsSendOpCode);
    else if (cmd.at("name").get_ref<const std::string&>().empty())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueSize, "name").dump(), WsSendOpCode);
    else
    {
      // ask the pool if the session token exists

      // we don't need to check if the pool is shareable because if it isn't, the token will be completely different,
      // so it'll either go to the wrong pool or not exist in the correct pool
      const auto token = createSessionToken(cmd.at("name"), true);
      
      const PoolId pool = getPoolId(token);
      const auto result = sessionSubmitSync(ws, token, queryType, queryName, queryRspName);
      
      njson rsp;
      
      const auto [exists, shared] = std::any_cast<std::tuple<bool, bool>>(result) ;

      // if they attempt to open a sesh that isn't shared, we can't accurately report if the 
      // session exists because the session will have a completely different token, so it either
      // completely works or a general failure response 
      if (exists && shared)
      {
        rsp[queryRspName]["tkn"] = token;
        rsp[queryRspName]["st"] = RequestStatus::Ok;
      }
      else
      {
        rsp[queryRspName]["tkn"] = njson{}; // null
        rsp[queryRspName]["st"] = RequestStatus::SessionOpenFail;
      }        

      ws->send(rsp.dump(), WsSendOpCode);
    }
  }


  fc_always_inline void sessionInfo(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType      = KvQueryType::SessionInfo;
    static const std::string queryName      = QueryTypeToName.at(queryType);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);

    if (cmd.size() != 1U)
      ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).dump(), WsSendOpCode);
    else
    {
      SessionToken token;
    
      if (getSessionToken(ws, queryName, json.at(queryName), token))
        sessionSubmit(ws, token, queryType, queryName, queryRspName);
    }
  }


  fc_always_inline void sessionInfoAll(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType      = KvQueryType::SessionInfoAll;
    static const std::string queryName      = QueryTypeToName.at(queryType);
    static const std::string queryRspName   = queryName +"_RSP";

    auto& cmd = json.at(queryName);

    if (!cmd.empty())
      ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).dump(), WsSendOpCode);
    else
    {
      auto results = sessionSubmitSync(ws, queryType, queryName, queryRspName);
      
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

      ws->send(rsp.dump(), WsSendOpCode);
    }
  }


  fc_always_inline void sessionEnd(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionEnd;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    SessionToken token;
    
    if (getSessionToken(ws, queryName, json.at(queryName), token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName);
  }

  
  // DATA

  fc_always_inline void set(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionSet;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing, "keys").dump(), WsSendOpCode);
    else if (!cmd.at("keys").is_object())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").dump(), WsSendOpCode);
    else if (getSessionToken(ws, queryRspName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }


  fc_always_inline void setQ(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionSetQ;
    static const std::string_view queryName     = "KV_SETQ";
    static const std::string_view queryRspName  = "KV_SETQ_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing, "keys").dump(), WsSendOpCode);
    else if (!cmd.at("keys").is_object())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").dump(), WsSendOpCode);
    else if (getSessionToken(ws, queryRspName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }


  fc_always_inline void get(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionGet;
    static const std::string_view queryName     = "KV_GET";
    static const std::string_view queryRspName  = "KV_GET_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing, "keys").dump(), WsSendOpCode);
    else if (!cmd.at("keys").is_array())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").dump(), WsSendOpCode);
    else if (getSessionToken(ws, queryRspName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }


  fc_always_inline void add(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionAdd;
    static const std::string_view queryName     = "KV_ADD";
    static const std::string_view queryRspName  = "KV_ADD_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing, "keys").dump(), WsSendOpCode);
    else if (!cmd.at("keys").is_object())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").dump(), WsSendOpCode);
    else if (getSessionToken(ws, queryRspName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }


  fc_always_inline void addQ(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionAddQ;
    static const std::string_view queryName     = "KV_ADDQ";
    static const std::string_view queryRspName  = "KV_ADDQ_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing, "keys").dump(), WsSendOpCode);
    else if (!cmd.at("keys").is_object())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").dump(), WsSendOpCode);
    else if (getSessionToken(ws, queryRspName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }


  fc_always_inline void remove(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionRemove;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing, "keys").dump(), WsSendOpCode);
    else if (!cmd.at("keys").is_array())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").dump(), WsSendOpCode);
    else if (getSessionToken(ws, queryRspName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }


  fc_always_inline void clear(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionClear;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    SessionToken token;

    if (getSessionToken(ws, queryName, json.at(queryName), token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName);
  }


  fc_always_inline void count(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionCount;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    SessionToken token;

    if (getSessionToken(ws, queryName, json.at(queryName), token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName);
  }


  fc_always_inline void append(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionAppend;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing, "keys").dump(), WsSendOpCode);
    else if (!cmd.at("keys").is_object())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").dump(), WsSendOpCode);
    else if (getSessionToken(ws, queryName, json.at(queryName), token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }


  fc_always_inline void contains(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionContains;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing).dump(), WsSendOpCode);
    else if (!cmd.at("keys").is_array())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").dump(), WsSendOpCode);
    else if (getSessionToken(ws, queryName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }


  // fc_always_inline void arrayMove(KvWebSocket * ws, njson&& json)
  // {
  //   static const KvQueryType queryType = KvQueryType::SessionArrayMove;
  //   static const std::string queryName     = QueryTypeToName.at(queryType);
  //   static const std::string queryRspName  = queryName +"_RSP";

  //   auto& cmd = json.at(queryName);
  //   SessionToken token;

  //   if (!cmd.contains("keys"))
  //     ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing).dump(), WsSendOpCode);
  //   else if (!cmd.at("keys").is_object())
  //     ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").dump(), WsSendOpCode);
  //   else if (getSessionToken(ws, queryName, cmd, token))
  //     sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  // }


  // fc_always_inline void find(KvWebSocket * ws, njson&& json)
  // {
  //   static const KvQueryType queryType = KvQueryType::SessionFind;
  //   static const std::string queryName     = QueryTypeToName.at(queryType);
  //   static const std::string queryRspName  = queryName +"_RSP";

  //   auto& cmd = json.at(queryName);
    
  //   SessionToken token;
  //   if (getSessionToken(ws, queryRspName, cmd, token))
  //   {
  //     // getSessionToken() deletes the "tkn" so only path and operator should remain
  //     if (cmd.size() != 2U)
  //       ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).dump(), WsSendOpCode);
  //     else if (!cmd.contains("path"))
  //       ws->send(createErrorResponse(queryRspName, RequestStatus::FindNoPath).dump(), WsSendOpCode);
  //     else if (!cmd.at("path").is_string())
  //       ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "path").dump(), WsSendOpCode);
  //     else
  //     {
  //       njson::const_iterator  itPath = cmd.cbegin(),
  //                               itOp = std::next(cmd.cbegin(), 1);

  //       if (itPath.key() != "path")
  //         std::swap(itOp, itPath);

  //       if (!findConditions.isValidOperator(itOp.key()))
  //         ws->send(createErrorResponse(queryRspName, RequestStatus::FindInvalidOperator).dump(), WsSendOpCode);
  //       else
  //       {
  //         const auto poolId = getPoolId(token);
  //         KvFind findData { .condition = findConditions.getOperator(itOp.key())};
          
  //         m_pools[poolId]->execute(KvCommand{ .ws = ws,
  //                                             .loop = uWS::Loop::get(),
  //                                             .contents = std::move(cmd),
  //                                             .type = queryType,
  //                                             .find = findData,
  //                                             .shtk = token});
  //       }          
  //     }
  //   }    
  // }


  fc_always_inline void update(KvWebSocket * ws, njson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionUpdate;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    
    SessionToken token;
    if (getSessionToken(ws, queryRspName, cmd, token))
    {
      if (cmd.size() != 2U)
        ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).dump(), WsSendOpCode);
      else if (!cmd.contains("key"))
        ws->send(createErrorResponse(queryRspName, RequestStatus::ParamMissing).dump(), WsSendOpCode);
      else if (!cmd.at("key").is_string())
        ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "key").dump(), WsSendOpCode);
      else
        sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd));
    }
  }


private:
  std::vector<KvPoolWorker *> m_pools;
  std::function<void(const SessionToken&, SessionPoolId&)> m_createSessionPoolId;
  ks::Sets * m_ks;
};

}
}
}

#endif
