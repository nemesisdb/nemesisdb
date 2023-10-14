#ifndef FC_CORE_KVHANDLERS_H
#define FC_CORE_KVHANDLERS_H


#include <functional>
#include <vector>
#include <tuple>
#include <latch>
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
    std::bind(&KvHandler::set,          std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::setQ,         std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::get,          std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::add,          std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::addQ,         std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::rmv,          std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::clear,        std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::serverInfo,   std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::count,        std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::append,       std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::contains,     std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::arrayMove,    std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::find,         std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionNew,   std::ref(*this), std::placeholders::_1, std::placeholders::_2)
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

  SessionToken getSessionToken(fcjson& cmd)
  {
    if (cmd.is_object() && cmd.contains ("shtk"))
    {
      auto token = std::move(cmd.at("shtk"));
      cmd.erase("shtk");
      return token;
    } 
    else
      return defaultSessionToken;
  }


  fc_always_inline bool getPoolId (const SessionToken& shtk, const cachedkey& key, PoolId id)
  {
    if (shtk == defaultSessionToken)
      return m_createPoolId(key, id);
    else
    {
      m_createSessionPoolId(shtk, id);
      return true;
    }
  }


  fc_always_inline bool getPoolId (const SessionToken& shtk, PoolId id)
  {
    m_createSessionPoolId(shtk, id);
    return true;
  }


  fc_always_inline void set(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Set;
    static const std::string_view queryName = "KV_SET";
    static const std::string_view queryRspName = "KV_SET_RSP";

    auto& cmd = json.at(queryName);
    const SessionToken token = getSessionToken(cmd);

    for (auto& pair : cmd.items())
    {
      auto& key = pair.key(); 
      auto& value = pair.value(); 

      if (valueTypeValid(value))    [[likely]]
      {
        PoolId poolId;        
        if (getPoolId(token, key, poolId))   [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                              .loop = uWS::Loop::get(),
                                              .contents = std::move(pair),
                                              .type = queryType,
                                              .shtk = token});
        }
        else
          ws->send(createErrorResponse(queryRspName, RequestStatus::KeyLengthInvalid, key).dump(), WsSendOpCode);
      }
      else
        ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, key).dump(), WsSendOpCode);
    }
  }


  fc_always_inline void setQ(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SetQ;
    static const std::string_view queryName = "KV_SETQ";
    static const std::string_view queryRspName = "KV_SETQ_RSP";

    auto& cmd = json.at(queryName);
    const SessionToken token = getSessionToken(cmd);

    for (auto& pair : cmd.items())
    {
      auto& key = pair.key(); 
      auto& value = pair.value(); 

      if (valueTypeValid(value))    [[likely]]
      {
        PoolId poolId;
        if (getPoolId(token, key, poolId))    [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                              .loop = uWS::Loop::get(),
                                              .contents = std::move(pair),
                                              .type = queryType, 
                                              .shtk = token});
        }
        else
          ws->send(createErrorResponse(queryRspName, RequestStatus::KeyLengthInvalid, key).dump(), WsSendOpCode);
      }
      else
        ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, key).dump(), WsSendOpCode);
    }
  }


  fc_always_inline void get(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Get;
    static const std::string_view queryName = "KV_GET";
    static const std::string_view queryRspName = "KV_GET_RSP";

    auto& cmd = json.at(queryName);
    const SessionToken token = getSessionToken(cmd);

    for (auto& jsonKey : cmd)
    {
      bool valid = true;

      if (!jsonKey.is_string())
        ws->send(createErrorResponse(queryRspName, RequestStatus::KeyTypeInvalid).dump(), WsSendOpCode);
      else
      {
        PoolId poolId;
        auto key = jsonKey.get<std::string_view>();

        if (getPoolId(token, jsonKey, poolId))  [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                              .loop = uWS::Loop::get(),
                                              .contents = std::move(key),
                                              .type = queryType, 
                                              .shtk = token});
        }
        else
          ws->send(createErrorResponse(queryRspName, RequestStatus::KeyLengthInvalid, key).dump(), WsSendOpCode);
      }
    }
  }


  fc_always_inline void doAdd(fcjson&& json, KvWebSocket * ws, const KvQueryType queryType, const std::string_view queryName, const std::string_view queryRspName)
  {
    auto& cmd = json.at(queryName);
    const SessionToken token = getSessionToken(cmd);

    for (auto& pair : cmd.items())
    {
      auto& key = pair.key(); 
      auto& value = pair.value(); 

      if (valueTypeValid(value))    [[likely]]
      {
        PoolId poolId;
        if (getPoolId(token, key, poolId))    [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                              .loop = uWS::Loop::get(),
                                              .contents = std::move(pair),
                                              .type = queryType, 
                                              .shtk = token});
        }
        else
          ws->send(createErrorResponse(queryRspName, RequestStatus::KeyLengthInvalid, key).dump(), WsSendOpCode);
      }
      else
        ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, key).dump(), WsSendOpCode);
    }
  }


  fc_always_inline void add(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Add;
    static const std::string_view queryName = "KV_ADD";
    static const std::string_view queryRspName = "KV_ADD_RSP";

    doAdd(std::move(json), ws, queryType, queryName, queryRspName);
  }

  fc_always_inline void addQ(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::AddQ;
    static const std::string_view queryName = "KV_ADDQ";
    static const std::string_view queryRspName = "KV_ADDQ_RSP";

    doAdd(std::move(json), ws, queryType, queryName, queryRspName);
  }

  fc_always_inline void rmv(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Remove;
    static const std::string_view queryName     = "KV_RMV";
    static const std::string_view queryRspName  = "KV_RMV_RSP";

    auto& cmd = json.at(queryName);
    const SessionToken token = getSessionToken(cmd);

    for (auto& key : cmd)
    {
      if (key.is_string())  [[likely]]
      {
        auto k = key.get<std::string_view>();

        PoolId poolId;
        if (getPoolId(token, key, poolId))  [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                            .loop = uWS::Loop::get(),
                                            .contents = std::move(k),
                                            .type = queryType, 
                                            .shtk = token});
        }
        else
          ws->send(createErrorResponse(queryRspName, RequestStatus::KeyLengthInvalid, k).dump(), WsSendOpCode);
      }
      else
        ws->send(createErrorResponse(queryRspName, RequestStatus::KeyTypeInvalid).dump(), WsSendOpCode);
    }
  }

  fc_always_inline void clear(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Clear;
    static const std::string_view queryName = "KV_CLEAR";
    

    const SessionToken token = getSessionToken(json.at(queryName));

    std::size_t count{0};
    bool cleared = true;

    std::atomic_ref<std::size_t> countRef{count}; // TODO why are these not atomic_size_t and atomic_bool?
    std::atomic_ref<bool> clearedRef{cleared};

    std::latch done{static_cast<std::ptrdiff_t>(token == defaultSessionToken ? 1U : m_pools.size())};

    auto onPoolResponse = [this, &done, &countRef, &clearedRef](std::any poolResult)
    {
      auto [success, count] = std::any_cast<std::tuple<bool, std::size_t>>(poolResult);
      
      if (success)
        countRef += count;
      else
        clearedRef = false;

      done.count_down();
    };
    
    for (auto& pool : m_pools)
    {
      pool->execute(KvCommand{ .ws = ws,
                              .type = queryType,
                              .cordinatedResponseHandler = onPoolResponse,
                              .shtk = token});
    }
    
    done.wait();

    fcjson rsp;
    rsp["KV_CLEAR_RSP"]["st"] = cleared ? RequestStatus::Ok : RequestStatus::Unknown;
    rsp["KV_CLEAR_RSP"]["cnt"] = count;

    ws->send(rsp.dump(), WsSendOpCode);
  }

  fc_always_inline void serverInfo(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::ServerInfo;
    static const std::string_view queryName = "KV_SERVER_INFO";

    static fcjson rsp {{"KV_SERVER_INFO_RSP", {{"st", RequestStatus::Ok}, {"version", FUSION_VERSION}}}};
    rsp["KV_SERVER_INFO_RSP"]["qryCnt"] = serverStats->queryCount.load();

    ws->send(rsp.dump(), WsSendOpCode);
  }

  fc_always_inline void count(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Count;
    static const std::string_view queryName = "KV_COUNT";

    std::size_t count{0};
    bool cleared = true;

    std::atomic_ref<std::size_t> countRef{count};

    std::latch done{static_cast<std::ptrdiff_t>(m_pools.size())};


    auto onPoolResponse = [this, &done, &countRef](std::any poolResult)
    {
      countRef += std::any_cast<std::size_t>(poolResult);
      done.count_down();
    };

    
    const SessionToken token = getSessionToken(json.at(queryName));

    for (auto& pool : m_pools)
    {
      pool->execute(KvCommand {.ws = ws,
                               .type = queryType,
                               .cordinatedResponseHandler = onPoolResponse,
                               .shtk = token});
    }
    
    done.wait();

    fcjson rsp;
    rsp["KV_COUNT_RSP"]["st"] = RequestStatus::Ok;
    rsp["KV_COUNT_RSP"]["cnt"] = count;

    ws->send(rsp.dump(), WsSendOpCode);
  }

  fc_always_inline void append(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Append;
    static const std::string_view queryName     = "KV_APPEND";
    static const std::string_view queryRspName  = "KV_APPEND_RSP";

    auto& cmd = json.at(queryName);
    const SessionToken token = getSessionToken(cmd);

    // don't check value type here (i.e. string, object or array) because pool worker has to check anyway
    for (auto& kv : cmd.items())
    {
      PoolId poolId;
      if (getPoolId(token, kv.key(), poolId))
      {
        m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                            .loop = uWS::Loop::get(),
                                            .contents = std::move(kv),
                                            .type = queryType,
                                            .shtk = token});
      }
      else
        ws->send(createErrorResponse(queryRspName, RequestStatus::KeyLengthInvalid, kv.key()).dump(), WsSendOpCode);
    }
  }

  fc_always_inline void contains(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Contains;
    static const std::string_view queryName = "KV_CONTAINS";
    static const std::string_view queryRspName = "KV_CONTAINS_RSP";

    auto& cmd = json.at(queryName);
    const SessionToken token = getSessionToken(cmd);

    for (auto& jsonKey : cmd)
    {
      if (!jsonKey.is_string())
        ws->send(createErrorResponse(queryRspName, RequestStatus::KeyTypeInvalid).dump(), WsSendOpCode);
      else
      {
        PoolId poolId;
        auto key = jsonKey.get<std::string_view>();

        if (getPoolId(token, jsonKey, poolId))  [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                              .loop = uWS::Loop::get(),
                                              .contents = key,
                                              .type = queryType,
                                              .shtk = token});
        }
        else
          ws->send(createErrorResponse(queryRspName, RequestStatus::KeyLengthInvalid, key).dump(), WsSendOpCode);
      }
    } 
  }


  fc_always_inline void arrayMove(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::ArrayMove;
    static const std::string_view queryName     = "KV_ARRAY_MOVE";
    static const std::string_view queryRspName  = "KV_ARRAY_MOVE_RSP";

    auto& cmd = json.at(queryName);
    const SessionToken token = getSessionToken(cmd);

    for (auto& kv : cmd.items())
    {
      if (!kv.value().is_array())
        ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid).dump(), WsSendOpCode);
      else
      {
        PoolId poolId;
        if (getPoolId(token, kv.key(), poolId))  [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                              .loop = uWS::Loop::get(),
                                              .contents = std::move(kv),
                                              .type = queryType,
                                              .shtk = token});
        }
        else
          ws->send(createErrorResponse(queryRspName, RequestStatus::KeyLengthInvalid, kv.key()).dump(), WsSendOpCode);
      }
    } 
  }


  fc_always_inline void find(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Find;
    static const std::string_view queryName = "KV_FIND";
    static const std::string_view queryRspName = "KV_FIND_RSP";
    
    auto& cmd = json.at(queryName);
    const SessionToken token = getSessionToken(cmd);

    const auto cmdSize = cmd.size();

    bool havePath = false, haveRegex = false, haveOp = false, unknownKey = false;
    fcjson::const_iterator itOp, itPath, itRegEx;

    for (auto it = cmd.cbegin() ; it != cmd.cend() ; ++it)
    {
      if (it.key() == "path")
      {
        havePath = true;
        itPath = it;
      }
      else if (it.key() == "keyrgx")
      {
        haveRegex = true;
        itRegEx = it;
      }
      else if (findConditions.isValidOperator(it.key()))
      {
        itOp = it;
        haveOp = true;
      }
      else
        unknownKey = true;
    }

    // rules:
    //  - op always required
    //  - path and regex are optional
    //  - path cannot be empty
    //  - regex cannot be empty TODO what about disallowing "*" ?
    if (!haveOp && !haveRegex && !havePath)
      ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).dump(), WsSendOpCode);
    else if (!haveOp) 
      ws->send(createErrorResponse(queryRspName, RequestStatus::FindNoOperator).dump(), WsSendOpCode);
    else if (unknownKey || (cmdSize < 1U || cmdSize > 3U))
      ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).dump(), WsSendOpCode);
    else if (havePath && (!cmd.at("path").is_string() || cmd.at("path").get_ref<const std::string&>().empty()))
      ws->send(createErrorResponse(queryRspName, RequestStatus::FindPathInvalid).dump(), WsSendOpCode);
    else if (haveRegex && (!cmd.at("keyrgx").is_string() || cmd.at("keyrgx").get_ref<const std::string&>().empty()))
      ws->send(createErrorResponse(queryRspName, RequestStatus::FindRegExInvalid).dump(), WsSendOpCode);
    else
    {
      std::mutex resultMux;
      std::vector<std::vector<cachedkey>> results;

      std::latch rspLatch{static_cast<std::ptrdiff_t>(m_pools.size())};

      auto onResponse = [&rspLatch, &resultMux, &results](std::any result)
      {
        {
          std::scoped_lock lck{resultMux};
          results.emplace_back(std::any_cast<std::vector<cachedkey>>(result));
        }
        
        rspLatch.count_down();
      };
      
      const fcjson s {{"path", havePath ? itPath.value() : ""}, {"keyrgx", haveRegex ? itRegEx.value() : ""}, {itOp.key(), itOp.value()}};
      const KvFind find {.condition = findConditions.OpStringToOp.at(itOp.key())};

      for (auto& worker : m_pools)
      {
        worker->execute(KvCommand{.ws = ws,
                                  .loop = uWS::Loop::get(),
                                  .contents = s,
                                  .type = queryType,
                                  .cordinatedResponseHandler = onResponse,
                                  .find = find,
                                  .shtk = token});
      }
    
      rspLatch.wait();
    
      fcjson rsp;
      rsp[queryRspName]["st"] = RequestStatus::Ok;
      rsp[queryRspName]["k"] = fcjson::array();

      auto& resultArray = rsp[queryRspName]["k"];

      for(auto& result : results)
      {
        for (auto&& key : result)
          resultArray.insert(resultArray.cend(), std::move(key));
      }

      ws->send(rsp.dump(), WsSendOpCode);
    }
  }


  // SESSION
  fc_always_inline void sessionNew(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionNew;
    static const std::string_view queryName     = "KV_SESSION_NEW";
    static const std::string_view queryRspName  = "KV_SESSION_NEW_RSP";

    auto& cmd = json.at(queryName);

    if (cmd.size() != 1U)
      ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).dump(), WsSendOpCode);
    else if (!cmd.contains("name"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing, "name").dump(), WsSendOpCode);
    else if (!cmd.at("name").is_string())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "name").dump(), WsSendOpCode);
    else
    {
      auto token = createSessionToken(cmd.at("name"));
      
      PoolId poolId;
      getPoolId(token, poolId);

      m_pools[poolId]->execute(KvCommand{.ws = ws,
                                      .loop = uWS::Loop::get(),
                                      .contents = {{"name", cmd.at("name")}},
                                      .type = queryType,
                                      .shtk = token});
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
