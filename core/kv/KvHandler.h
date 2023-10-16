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
    std::bind(&KvHandler::sessionNew,   std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionEnd,   std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionSet,   std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionSetQ,  std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionGet,   std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionAdd,   std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionAddQ,  std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionRemove,    std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionClear,     std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionCount,     std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionAppend,    std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionContains,  std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionArrayMove, std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionFind,      std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::sessionUpdate,    std::ref(*this), std::placeholders::_1, std::placeholders::_2)
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

  fc_always_inline void submitPairs(KvWebSocket * ws, fcjson&& cmd, const KvQueryType queryType, const std::string_view command, const std::string_view rspName)
  {
    for (auto& pair : cmd.items())
    {
      cachedkey key = pair.key(); 
      auto& value = pair.value(); 

      if (valueTypeValid(value))    [[likely]]
      {
        PoolId poolId;
        if (getPoolId(key, poolId))   [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                              .loop = uWS::Loop::get(),
                                              .contents = std::move(pair),
                                              .type = queryType,
                                              .shtk = defaultSessionToken});
        }
        else
          ws->send(createErrorResponse(rspName, RequestStatus::KeyLengthInvalid, key).dump(), WsSendOpCode);
      }
      else
        ws->send(createErrorResponse(rspName, RequestStatus::ValueTypeInvalid, key).dump(), WsSendOpCode);
    }
  }


  fc_always_inline void submitKeys(KvWebSocket * ws, fcjson&& cmd, const KvQueryType queryType, const std::string_view command, const std::string_view rspName)
  {
    for (auto& key : cmd)
    {
      if (key.is_string())  [[likely]]
      {
        auto k = key.get<std::string_view>();

        PoolId poolId;
        if (getPoolId(key, poolId))  [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                              .loop = uWS::Loop::get(),
                                              .contents = std::move(k),
                                              .type = queryType, 
                                              .shtk = defaultSessionToken});
        }
        else
          ws->send(createErrorResponse(rspName, RequestStatus::KeyLengthInvalid, k).dump(), WsSendOpCode);
      }
      else
        ws->send(createErrorResponse(rspName, RequestStatus::KeyTypeInvalid).dump(), WsSendOpCode);
    }
  }

  
  // KV
  fc_always_inline bool getPoolId (const cachedkey& key, PoolId& id)
  {
    return m_createPoolId(key, id);
  }


  fc_always_inline void set(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Set;
    static const std::string_view queryName     = "KV_SET";
    static const std::string_view queryRspName  = "KV_SET_RSP";

    auto& cmd = json.at(queryName);
    submitPairs(ws, std::move(cmd), queryType, queryName, queryRspName);
  }
  

  fc_always_inline void setQ(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SetQ;
    static const std::string_view queryName     = "KV_SETQ";
    static const std::string_view queryRspName  = "KV_SETQ_RSP";

    auto& cmd = json.at(queryName);
    submitPairs(ws, std::move(cmd), queryType, queryName, queryRspName);
  }


  fc_always_inline void get(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Get;
    static const std::string_view queryName     = "KV_GET";
    static const std::string_view queryRspName  = "KV_GET_RSP";

    submitKeys(ws, std::move(json.at(queryName)), queryType, queryName, queryRspName);
  }


  fc_always_inline void add(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Add;
    static const std::string_view queryName     = "KV_ADD";
    static const std::string_view queryRspName  = "KV_ADD_RSP";

    submitPairs(ws, std::move(json.at(queryName)), queryType, queryName, queryRspName);
  }


  fc_always_inline void addQ(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::AddQ;
    static const std::string_view queryName     = "KV_ADDQ";
    static const std::string_view queryRspName  = "KV_ADDQ_RSP";

    submitPairs(ws, std::move(json.at(queryName)), queryType, queryName, queryRspName);
  }


  fc_always_inline void rmv(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Remove;
    static const std::string_view queryName     = "KV_RMV";
    static const std::string_view queryRspName  = "KV_RMV_RSP";

    auto& cmd = json.at(queryName);
    submitKeys(ws, std::move(cmd), queryType, queryName, queryRspName);
  }


  fc_always_inline void clear(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Clear;
    static const std::string_view queryName = "KV_CLEAR";
    
    std::size_t count{0};
    bool cleared = true;

    std::atomic_ref<std::size_t> countRef{count}; // TODO why are these not atomic_size_t and atomic_bool?
    std::atomic_ref<bool> clearedRef{cleared};

    std::latch done{static_cast<std::ptrdiff_t>(m_pools.size())};

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
      pool->execute(KvCommand{  .ws = ws,
                                .type = queryType,
                                .cordinatedResponseHandler = onPoolResponse,
                                .shtk = defaultSessionToken});
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


    for (auto& pool : m_pools)
    {
      pool->execute(KvCommand {.ws = ws,
                              .type = queryType,
                              .cordinatedResponseHandler = onPoolResponse,
                              .shtk = defaultSessionToken});
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

    submitPairs(ws, std::move(json.at(queryName)), queryType, queryName, queryRspName);
  }


  fc_always_inline void contains(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Contains;
    static const std::string_view queryName     = "KV_CONTAINS";
    static const std::string_view queryRspName  = "KV_CONTAINS_RSP";

    submitKeys(ws, std::move(json.at(queryName)), queryType, queryName, queryRspName);
  }


  fc_always_inline void arrayMove(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::ArrayMove;
    static const std::string_view queryName     = "KV_ARRAY_MOVE";
    static const std::string_view queryRspName  = "KV_ARRAY_MOVE_RSP";

    submitPairs(ws, std::move(json.at(queryName)), queryType, queryName, queryRspName);
  }


  fc_always_inline void find(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Find;
    static const std::string_view queryName = "KV_FIND";
    static const std::string_view queryRspName = "KV_FIND_RSP";
    
    auto& cmd = json.at(queryName);
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
      ws->send(createErrorResponse(queryRspName, RequestStatus::PathInvalid).dump(), WsSendOpCode);
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
                                  .shtk = defaultSessionToken});
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
  fc_always_inline PoolId getPoolId (const SessionToken& shtk)
  {
    PoolId id;
    m_createSessionPoolId(shtk, id);
    return id;
  }


  bool getSessionToken(KvWebSocket * ws, const std::string_view queryRspName, fcjson& cmd, SessionToken& t)
  {
    if (!cmd.contains ("tkn"))
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
    auto poolId = getPoolId(token);
    m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                        .loop = uWS::Loop::get(),
                                        .contents = std::move(cmd),
                                        .type = queryType,                                        
                                        .shtk = token});
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
        ws->send(createMessageResponse(rspName, RequestStatus::ValueTypeInvalid, pair.key()).dump(), WsSendOpCode);
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
        ws->send(createMessageResponse(rspName, RequestStatus::KeyTypeInvalid).dump(), WsSendOpCode);
    }
  }


  fc_always_inline void sessionNew(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionNew;
    static const std::string_view queryName     = "SH_NEW";
    static const std::string_view queryRspName  = "SH_NEW_RSP";

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


  fc_always_inline void sessionEnd(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionEnd;
    static const std::string_view queryName     = "SH_END";
    static const std::string_view queryRspName  = "SH_END_RSP";

    SessionToken token;
    
    if (getSessionToken(ws, queryName, json.at(queryName), token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName);
  }


  fc_always_inline void sessionSet(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionSet;
    static const std::string_view queryName     = "SH_SET";
    static const std::string_view queryRspName  = "SH_SET_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (getSessionToken(ws, queryRspName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd));
  }


  fc_always_inline void sessionSetQ(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionSetQ;
    static const std::string_view queryName     = "SH_SETQ";
    static const std::string_view queryRspName  = "SH_SETQ_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (getSessionToken(ws, queryRspName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd));
  }


  fc_always_inline void sessionGet(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionGet;
    static const std::string_view queryName     = "SH_GET";
    static const std::string_view queryRspName  = "SH_GET_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createMessageResponse(queryRspName, RequestStatus::KeyMissing).dump(), WsSendOpCode);
    else if (!cmd.at("keys").is_array())
      ws->send(createMessageResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").dump(), WsSendOpCode);
    else if (getSessionToken(ws, queryRspName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }


  fc_always_inline void sessionAdd(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionAdd;
    static const std::string_view queryName     = "SH_ADD";
    static const std::string_view queryRspName  = "SH_ADD_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (getSessionToken(ws, queryRspName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd));
  }


  fc_always_inline void sessionAddQ(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionAddQ;
    static const std::string_view queryName     = "SH_ADDQ";
    static const std::string_view queryRspName  = "SH_ADDQ_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (getSessionToken(ws, queryRspName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd));
  }


  fc_always_inline void sessionRemove(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionRemove;
    static const std::string_view queryName     = "SH_RMV";
    static const std::string_view queryRspName  = "SH_RMV_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createMessageResponse(queryRspName, RequestStatus::KeyMissing).dump(), WsSendOpCode);
    else if (!cmd.at("keys").is_array())
      ws->send(createMessageResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").dump(), WsSendOpCode);
    else if (getSessionToken(ws, queryRspName, cmd, token))
      sessionSubmitKeys(ws, std::move(cmd.at("keys")), token, queryType, queryName, queryRspName);
  }


  fc_always_inline void sessionClear(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionClear;
    static const std::string_view queryName     = "SH_CLEAR";
    static const std::string_view queryRspName  = "SH_CLEAR_RSP";

    SessionToken token;

    if (getSessionToken(ws, queryName, json.at(queryName), token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName);
  }


  fc_always_inline void sessionCount(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionCount;
    static const std::string_view queryName     = "SH_COUNT";
    static const std::string_view queryRspName  = "SH_COUNT_RSP";

    SessionToken token;

    if (getSessionToken(ws, queryName, json.at(queryName), token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName);
  }


  fc_always_inline void sessionAppend(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionAppend;
    static const std::string_view queryName     = "SH_APPEND";
    static const std::string_view queryRspName  = "SH_APPEND_RSP";

    SessionToken token;

    if (getSessionToken(ws, queryName, json.at(queryName), token))
      sessionSubmitPairs(ws, std::move(json.at(queryName)), token, queryType, queryName, queryRspName);
  }


  fc_always_inline void sessionContains(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionContains;
    static const std::string_view queryName     = "SH_CONTAINS";
    static const std::string_view queryRspName  = "SH_CONTAINS_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createMessageResponse(queryRspName, RequestStatus::KeyMissing).dump(), WsSendOpCode);
    else if (!cmd.at("keys").is_array())
      ws->send(createMessageResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").dump(), WsSendOpCode);
    else if (getSessionToken(ws, queryName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }


  fc_always_inline void sessionArrayMove(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionArrayMove;
    static const std::string_view queryName     = "SH_ARRAY_MOVE";
    static const std::string_view queryRspName  = "SH_ARRAY_MOVE_RSP";

    auto& cmd = json.at(queryName);
    SessionToken token;

    if (!cmd.contains("keys"))
      ws->send(createMessageResponse(queryRspName, RequestStatus::KeyMissing).dump(), WsSendOpCode);
    else if (!cmd.at("keys").is_object())
      ws->send(createMessageResponse(queryRspName, RequestStatus::ValueTypeInvalid, "keys").dump(), WsSendOpCode);
    else if (getSessionToken(ws, queryName, cmd, token))
      sessionSubmit(ws, token, queryType, queryName, queryRspName, std::move(cmd.at("keys")));
  }


  fc_always_inline void sessionFind(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionFind;
    static const std::string_view queryName     = "SH_FIND";
    static const std::string_view queryRspName  = "SH_FIND_RSP";

    auto& cmd = json.at(queryName);
    
    SessionToken token;
    if (getSessionToken(ws, queryRspName, cmd, token))
    {
      // getSessionToken() deletes the "tkn" so only path and operator should remain
      if (cmd.size() != 2U)
        ws->send(createMessageResponse(queryRspName, RequestStatus::CommandSyntax).dump(), WsSendOpCode);
      else if (!cmd.contains("path"))
        ws->send(createMessageResponse(queryRspName, RequestStatus::FindNoPath).dump(), WsSendOpCode);
      else if (!cmd.at("path").is_string())
        ws->send(createMessageResponse(queryRspName, RequestStatus::ValueTypeInvalid, "path").dump(), WsSendOpCode);
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


  fc_always_inline void sessionUpdate(KvWebSocket * ws, fcjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SessionUpdate;
    static const std::string_view queryName     = "SH_UPDATE";
    static const std::string_view queryRspName  = "SH_UPDATE_RSP";

    auto& cmd = json.at(queryName);
    
    SessionToken token;
    if (getSessionToken(ws, queryRspName, cmd, token))
    {
      if (cmd.size() != 2U)
        ws->send(createMessageResponse(queryRspName, RequestStatus::CommandSyntax).dump(), WsSendOpCode);
      else if (!cmd.contains("key"))
        ws->send(createMessageResponse(queryRspName, RequestStatus::KeyMissing).dump(), WsSendOpCode);
      else if (!cmd.at("key").is_string())
        ws->send(createMessageResponse(queryRspName, RequestStatus::ValueTypeInvalid, "key").dump(), WsSendOpCode);
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
