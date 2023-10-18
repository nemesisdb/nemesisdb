#ifndef FC_CORE_KVHANDLERS_H
#define FC_CORE_KVHANDLERS_H


#include <functional>
#include <vector>
#include <tuple>
#include <latch>
#include <ranges>
#include <core/FusionCommon.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvPoolWorker.h>
#include <core/ks/KsSets.h>

namespace fusion { namespace core { namespace kv {


class KvHandler
{
public:
  KvHandler(const std::size_t nPools, const std::size_t coreOffset, ks::Sets * ks) :
    m_createPoolId(nPools == 1U ? PoolIndexers[1U] : PoolIndexers[0U]),
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
  const std::array<std::function<void(KvWebSocket *, fcjson&&)>, static_cast<std::size_t>(KvQueryType::Max)> Handlers = 
  {
    std::bind(&KvHandler::sessionNew,       std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionEnd,       std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionOpen,      std::ref(*this), std::placeholders::_1, std::placeholders::_2),
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
    std::bind(&KvHandler::arrayMove,        std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::find,             std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::update,           std::ref(*this), std::placeholders::_1, std::placeholders::_2)
  };


public:

  std::tuple<RequestStatus,std::string> handle(KvWebSocket * ws, fcjson&& json)
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

    return std::make_tuple(status, queryName);
  }


private:
  

  fc_always_inline PoolId getPoolId (const SessionToken& shtk)
  {
    PoolId id;
    m_createSessionPoolId(shtk, id);
    return id;
  }


  bool getSessionToken(KvWebSocket * ws, const std::string_view queryRspName, fcjson& cmd, SessionToken& t)
  {
    if ((!cmd.contains ("tkn")) || cmd.at("tkn").get_ref<const std::string&>().empty())
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


  fc_always_inline void sessionSubmit(KvWebSocket * ws, const SessionToken& token, const KvQueryType queryType, const std::string_view command, const std::string_view rspName, fcjson&& cmd = "")
  {
    const auto poolId = getPoolId(token);
    m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                        .loop = uWS::Loop::get(),
                                        .contents = std::move(cmd),
                                        .type = queryType,                                        
                                        .shtk = token});
  } 


  fc_always_inline std::any sessionSubmitSync(KvWebSocket * ws, const SessionToken& token, const KvQueryType queryType, const std::string_view command, const std::string_view rspName, fcjson&& cmd = "")
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


  fc_always_inline void sessionSubmitPairs(KvWebSocket * ws, fcjson&& cmd, const SessionToken& token, const KvQueryType queryType, const std::string_view command, const std::string_view rspName)
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


  fc_always_inline void sessionSubmitKeys(KvWebSocket * ws, fcjson&& cmd, const SessionToken& token, const KvQueryType queryType, const std::string_view command, const std::string_view rspName)
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
  
  fc_always_inline void sessionNew(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionNew;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    auto& cmd = json.at(queryName);

    if (cmd.size() != 1U && cmd.size() != 2U)
      ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).dump(), WsSendOpCode);
    else if (!cmd.contains("name"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing, "name").dump(), WsSendOpCode);
    else if (!cmd.at("name").is_string())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "name").dump(), WsSendOpCode);
    else if (cmd.at("name").get_ref<const std::string&>().empty())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "name").dump(), WsSendOpCode);
    else
    {
      const bool shareable = cmd.value("shareable", false);
      const auto token = createSessionToken(cmd.at("name"), shareable);
      
      PoolId poolId = getPoolId(token);
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd));
    } 
  }


  fc_always_inline void sessionOpen(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionOpen;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    auto& cmd = json.at(queryName);

    if (cmd.size() != 1U)
      ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).dump(), WsSendOpCode);
    else if (!cmd.contains("name"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing, "name").dump(), WsSendOpCode);
    else if (!cmd.at("name").is_string())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "name").dump(), WsSendOpCode);
    else if (cmd.at("name").get_ref<const std::string&>().empty())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "name").dump(), WsSendOpCode);
    else
    {
      // ask the pool if the session token exists

      // we don't need to check if the pool is shareable because if it isn't, the token will be completely different,
      // so it'll either go to the wrong pool, and not exist even if it happens to go to the correct pool
      const auto token = createSessionToken(cmd.at("name"), true);
      
      const PoolId pool = getPoolId(token);
      const auto result = sessionSubmitSync(ws, token, queryType, queryName, queryRspName);
      
      fcjson rsp;
      
      if (std::any_cast<bool>(result))
        rsp[queryRspName]["tkn"] = token;
      else
        rsp[queryRspName]["tkn"] = fcjson{}; // null

      ws->send(rsp.dump(), WsSendOpCode);
    }
  }


  fc_always_inline void sessionEnd(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionEnd;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    SessionToken token;
    
    if (getSessionToken(ws, queryName, json.at(queryName), token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName);
  }

  
  // DATA

  fc_always_inline void set(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionSet;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (getSessionToken(ws, queryRspName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd));
  }


  fc_always_inline void setQ(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionSetQ;
    static const std::string_view queryName     = "KV_SETQ";
    static const std::string_view queryRspName  = "KV_SETQ_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (getSessionToken(ws, queryRspName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd));
  }


  fc_always_inline void get(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionGet;
    static const std::string_view queryName     = "KV_GET";
    static const std::string_view queryRspName  = "KV_GET_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::KeyMissing).dump(), WsSendOpCode);
    else if (!cmd.at("keys").is_array())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").dump(), WsSendOpCode);
    else if (getSessionToken(ws, queryRspName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }


  fc_always_inline void add(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionAdd;
    static const std::string_view queryName     = "KV_ADD";
    static const std::string_view queryRspName  = "KV_ADD_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (getSessionToken(ws, queryRspName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd));
  }


  fc_always_inline void addQ(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionAddQ;
    static const std::string_view queryName     = "KV_ADDQ";
    static const std::string_view queryRspName  = "KV_ADDQ_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (getSessionToken(ws, queryRspName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd));
  }


  fc_always_inline void remove(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionRemove;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::KeyMissing).dump(), WsSendOpCode);
    else if (!cmd.at("keys").is_array())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").dump(), WsSendOpCode);
    else if (getSessionToken(ws, queryRspName, cmd, token))
      sessionSubmitKeys(ws, std::move(cmd.at("keys")), token, queryType, queryName, queryRspName);
  }


  fc_always_inline void clear(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionClear;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    SessionToken token;

    if (getSessionToken(ws, queryName, json.at(queryName), token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName);
  }


  fc_always_inline void count(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionCount;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    SessionToken token;

    if (getSessionToken(ws, queryName, json.at(queryName), token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName);
  }


  fc_always_inline void append(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionAppend;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    SessionToken token;

    if (getSessionToken(ws, queryName, json.at(queryName), token))
      sessionSubmitPairs(ws, std::move(json.at(queryName)), token, queryType, queryName, queryRspName);
  }


  fc_always_inline void contains(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionContains;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::KeyMissing).dump(), WsSendOpCode);
    else if (!cmd.at("keys").is_array())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").dump(), WsSendOpCode);
    else if (getSessionToken(ws, queryName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }


  fc_always_inline void arrayMove(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionArrayMove;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::KeyMissing).dump(), WsSendOpCode);
    else if (!cmd.at("keys").is_object())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").dump(), WsSendOpCode);
    else if (getSessionToken(ws, queryName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }


  fc_always_inline void find(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionFind;
    static const std::string queryName     = QueryTypeToName.at(queryType);
    static const std::string queryRspName  = queryName +"_RSP";

    auto& cmd = json.at(queryName);
    
    SessionToken token;
    if (getSessionToken(ws, queryRspName, cmd, token))
    {
      // getSessionToken() deletes the "tkn" so only path and operator should remain
      if (cmd.size() != 2U)
        ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).dump(), WsSendOpCode);
      else if (!cmd.contains("path"))
        ws->send(createErrorResponse(queryRspName, RequestStatus::FindNoPath).dump(), WsSendOpCode);
      else if (!cmd.at("path").is_string())
        ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "path").dump(), WsSendOpCode);
      else
      {
        fcjson::const_iterator  itPath = cmd.cbegin(),
                                itOp = std::next(cmd.cbegin(), 1);

        if (cmd.cbegin().key() != "path")
        {
          itOp = cmd.cbegin(),
          itPath = std::next(cmd.cbegin(), 1);
        }

        if (!findConditions.isValidOperator(itOp.key()))
          ws->send(createErrorResponse(queryRspName, RequestStatus::FindNoOperator).dump(), WsSendOpCode);
        else
          sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd));
      }
    }    
  }


  fc_always_inline void update(KvWebSocket * ws, fcjson&& json)
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
        ws->send(createErrorResponse(queryRspName, RequestStatus::KeyMissing).dump(), WsSendOpCode);
      else if (!cmd.at("key").is_string())
        ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "key").dump(), WsSendOpCode);
      else
        sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd));
    }
  }


private:
  std::vector<KvPoolWorker *> m_pools;
  std::function<bool(const std::string_view&, PoolId&)> m_createPoolId;
  std::function<void(const SessionToken&, SessionPoolId&)> m_createSessionPoolId;
  ks::Sets * m_ks;
};

}
}
}

#endif
