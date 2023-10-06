#ifndef FC_CORE_KVHANDLERS_H
#define FC_CORE_KVHANDLERS_H


#include <functional>
#include <vector>
#include <tuple>
#include <latch>
#include <core/kv/KvCommon.h>
#include <core/kv/KvPoolWorker.h>


namespace fusion { namespace core { namespace kv {


class KvHandler
{
public:
  KvHandler(const std::size_t nPools, const std::size_t coreOffset) : m_createPoolId(nPools == 1U ? PoolIndexers[1U] : PoolIndexers[0U])
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
  const std::function<void(KvWebSocket *, kvjson&&)> Handlers[static_cast<std::size_t>(KvQueryType::Max)] =
  {
    std::bind(&KvHandler::set, std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::setQ, std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::get, std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::add, std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::addQ, std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::rmv, std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::clear, std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::serverInfo, std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::count, std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::append, std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::contains, std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::arrayMove, std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KvHandler::find, std::ref(*this), std::placeholders::_1, std::placeholders::_2)
  };

public:

  std::tuple<KvRequestStatus,std::string> handle(KvWebSocket * ws, kvjson&& json)
  {
    KvRequestStatus status = KvRequestStatus::Ok;

    if (json.size() != 1U) [[unlikely]]
      return std::make_tuple(KvRequestStatus::CommandMultiple, "");
    else  [[likely]]
    {
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
            status = KvRequestStatus::Unknown;
          }
        }
        else
          status = KvRequestStatus::CommandType;
      }
      else
        status = KvRequestStatus::CommandNotExist;

      return std::make_tuple(status, queryName);
    }
  }


private:

  fc_always_inline void set(KvWebSocket * ws, kvjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Set;
    static const std::string_view queryName = "KV_SET";
    static const std::string_view queryRspName = "KV_SET_RSP";

    for (auto& pair : json[queryName].items())
    {
      auto& key = pair.key(); 
      auto& value = pair.value(); 

      if (valueTypeValid(value))    [[likely]]
      {
        PoolId poolId;
        if (m_createPoolId(key, poolId))    [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                              .loop = uWS::Loop::get(),
                                              .contents = std::move(pair),
                                              .type = queryType});
        }
        else
          ws->send(createErrorResponse(queryRspName, KvRequestStatus::KeyLengthInvalid, key).dump(), WsSendOpCode);
      }
      else
        ws->send(createErrorResponse(queryRspName, KvRequestStatus::ValueTypeInvalid, key).dump(), WsSendOpCode);
    }
  }


  fc_always_inline void setQ(KvWebSocket * ws, kvjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::SetQ;
    static const std::string_view queryName = "KV_SETQ";
    static const std::string_view queryRspName = "KV_SETQ_RSP";


    for (auto& pair : json[queryName].items())
    {
      auto& key = pair.key(); 
      auto& value = pair.value(); 

      if (valueTypeValid(value))    [[likely]]
      {
        PoolId poolId;
        if (m_createPoolId(key, poolId))    [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                              .loop = uWS::Loop::get(),
                                              .contents = std::move(pair),
                                              .type = queryType});
        }
        else
          ws->send(createErrorResponse(queryRspName, KvRequestStatus::KeyLengthInvalid, key).dump(), WsSendOpCode);
      }
      else
        ws->send(createErrorResponse(queryRspName, KvRequestStatus::ValueTypeInvalid, key).dump(), WsSendOpCode);
    }
  }


  fc_always_inline void get(KvWebSocket * ws, kvjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Get;
    static const std::string_view queryName = "KV_GET";
    static const std::string_view queryRspName = "KV_GET_RSP";

    for (auto& jsonKey : json[queryName])
    {
      bool valid = true;

      if (!jsonKey.is_string())
        ws->send(createErrorResponse(queryRspName, KvRequestStatus::KeyTypeInvalid).dump(), WsSendOpCode);
      else
      {
        PoolId poolId;
        auto key = jsonKey.get<std::string_view>();

        if (m_createPoolId(key, poolId))  [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                              .loop = uWS::Loop::get(),
                                              .contents = std::move(key),
                                              .type = queryType});
        }
        else
          ws->send(createErrorResponse(queryRspName, KvRequestStatus::KeyLengthInvalid, key).dump(), WsSendOpCode);
      }
    }
  }


  fc_always_inline void doAdd(kvjson&& json, KvWebSocket * ws, const KvQueryType queryType, const std::string_view queryName, const std::string_view queryRspName)
  {
    for (auto& pair : json[queryName].items())
    {
      auto& key = pair.key(); 
      auto& value = pair.value(); 

      if (valueTypeValid(value))    [[likely]]
      {
        PoolId poolId;
        if (m_createPoolId(key, poolId))    [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                              .loop = uWS::Loop::get(),
                                              .contents = std::move(pair),
                                              .type = queryType});
        }
        else
          ws->send(createErrorResponse(queryRspName, KvRequestStatus::KeyLengthInvalid, key).dump(), WsSendOpCode);
      }
      else
        ws->send(createErrorResponse(queryRspName, KvRequestStatus::ValueTypeInvalid, key).dump(), WsSendOpCode);
    }
  }


  fc_always_inline void add(KvWebSocket * ws, kvjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Add;
    static const std::string_view queryName = "KV_ADD";
    static const std::string_view queryRspName = "KV_ADD_RSP";

    doAdd(std::move(json), ws, queryType, queryName, queryRspName);
  }

  fc_always_inline void addQ(KvWebSocket * ws, kvjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::AddQ;
    static const std::string_view queryName = "KV_ADDQ";
    static const std::string_view queryRspName = "KV_ADDQ_RSP";

    doAdd(std::move(json), ws, queryType, queryName, queryRspName);
  }

  fc_always_inline void rmv(KvWebSocket * ws, kvjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Remove;
    static const std::string_view queryName = "KV_RMV";
    static const std::string_view queryRspName = "KV_RMV_RSP";

    for (auto& key : json[queryName])
    {
      if (key.is_string())  [[likely]]
      {
        auto k = key.get<std::string_view>();

        PoolId poolId;

        if (m_createPoolId(k, poolId))  [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                            .loop = uWS::Loop::get(),
                                            .contents = std::move(k),
                                            .type = queryType});
        }
        else
          ws->send(createErrorResponse(queryRspName, KvRequestStatus::KeyLengthInvalid, k).dump(), WsSendOpCode);
      }
      else
        ws->send(createErrorResponse(queryRspName, KvRequestStatus::KeyTypeInvalid).dump(), WsSendOpCode);
    }
  }

  fc_always_inline void clear(KvWebSocket * ws, kvjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Clear;
    static const std::string_view queryName = "KV_CLEAR";
    
    std::size_t count{0};
    bool cleared = true;

    std::atomic_ref<std::size_t> countRef{count};
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
      pool->execute(KvCommand{ .ws = ws,
                              .type = queryType,
                              .cordinatedResponseHandler = onPoolResponse});
    }
    
    done.wait();

    kvjson rsp;
    rsp["KV_CLEAR_RSP"]["st"] = cleared ? KvRequestStatus::Ok : KvRequestStatus::Unknown;
    rsp["KV_CLEAR_RSP"]["cnt"] = count;

    ws->send(rsp.dump(), WsSendOpCode);
  }

  fc_always_inline void serverInfo(KvWebSocket * ws, kvjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::ServerInfo;
    static const std::string_view queryName = "KV_SERVER_INFO";

    static kvjson rsp {{"KV_SERVER_INFO_RSP", {{"st", KvRequestStatus::Ok}, {"version", FUSION_VERSION}}}};
    rsp["KV_SERVER_INFO_RSP"]["qryCnt"] = serverStats->queryCount.load();

    ws->send(rsp.dump(), WsSendOpCode);
  }

  fc_always_inline void count(KvWebSocket * ws, kvjson&& json)
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
                               .cordinatedResponseHandler = onPoolResponse});
    }
    
    done.wait();

    kvjson rsp;
    rsp["KV_COUNT_RSP"]["st"] = KvRequestStatus::Ok;
    rsp["KV_COUNT_RSP"]["cnt"] = count;

    ws->send(rsp.dump(), WsSendOpCode);
  }

  fc_always_inline void append(KvWebSocket * ws, kvjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Append;
    static const std::string_view queryName = "KV_APPEND";
    static const std::string_view queryRspName = "KV_APPEND_RSP";

    // don't check value type here (i.e. string, object or array) because pool worker has to check anyway
    for (auto& kv : json.at(queryName).items())
    {
      PoolId poolId;
      if (m_createPoolId(kv.key(), poolId))
      {
        m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                            .loop = uWS::Loop::get(),
                                            .contents = std::move(kv),
                                            .type = queryType});
      }
      else
        ws->send(createErrorResponse(queryRspName, KvRequestStatus::KeyLengthInvalid, kv.key()).dump(), WsSendOpCode);
    }
  }

  fc_always_inline void contains(KvWebSocket * ws, kvjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Contains;
    static const std::string_view queryName = "KV_CONTAINS";
    static const std::string_view queryRspName = "KV_CONTAINS_RSP";

    for (auto& jsonKey : json[queryName])
    {
      if (!jsonKey.is_string())
        ws->send(createErrorResponse(queryRspName, KvRequestStatus::KeyTypeInvalid).dump(), WsSendOpCode);
      else
      {
        PoolId poolId;
        auto key = jsonKey.get<std::string_view>();

        if (m_createPoolId(key, poolId))  [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                              .loop = uWS::Loop::get(),
                                              .contents = key,
                                              .type = queryType});
        }
        else
          ws->send(createErrorResponse(queryRspName, KvRequestStatus::KeyLengthInvalid, key).dump(), WsSendOpCode);
      }
    } 
  }

  fc_always_inline void arrayMove(KvWebSocket * ws, kvjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::ArrayMove;
    static const std::string_view queryName = "KV_ARRAY_MOVE";
    static const std::string_view queryRspName = "KV_ARRAY_MOVE_RSP";

    for (auto& kv : json[queryName].items())
    {
      if (!kv.value().is_array())
        ws->send(createErrorResponse(queryRspName, KvRequestStatus::ValueTypeInvalid).dump(), WsSendOpCode);
      else
      {
        PoolId poolId;
        if (m_createPoolId(kv.key(), poolId))  [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                              .loop = uWS::Loop::get(),
                                              .contents = std::move(kv),
                                              .type = queryType});
        }
        else
          ws->send(createErrorResponse(queryRspName, KvRequestStatus::KeyLengthInvalid, kv.key()).dump(), WsSendOpCode);
      }
    } 
  }

  fc_always_inline void find(KvWebSocket * ws, kvjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Find;
    static const std::string_view queryName = "KV_FIND";
    static const std::string_view queryRspName = "KV_FIND_RSP";

    auto& cmd = json.at(queryName);
        
    if (cmd.size() != 1U && cmd.size() != 2U)
      ws->send(createErrorResponse(queryRspName, KvRequestStatus::CommandSyntax).dump(), WsSendOpCode);
    else if (cmd.size() == 1U && cmd.contains("path"))
      ws->send(createErrorResponse(queryRspName, KvRequestStatus::FindNoOperator).dump(), WsSendOpCode);
    else if (cmd.size() == 2U && !cmd.contains("path"))
      ws->send(createErrorResponse(queryRspName, KvRequestStatus::FindNoPath).dump(), WsSendOpCode);
    else if (cmd.contains("path") && (!cmd.at("path").is_string() || cmd.at("path").get_ref<const std::string&>().empty()))
      ws->send(createErrorResponse(queryRspName, KvRequestStatus::FindPathInvalid).dump(), WsSendOpCode);
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
      
      kvjson::const_iterator itCondition, itPath;
      const auto havePath = cmd.contains("path");

      if (havePath)
      {
        // can only have two members, so begin() is not path then it must be begin()+1
        if (auto it = cmd.begin(); it.key() == "path")
        {
          itPath = it;
          itCondition = std::next (it, 1);
        }        
        else
        {
          itPath = std::next (it, 1);
          itCondition = it;
        }
      }
      else
      {
        itCondition = cmd.begin();
      }
      
      if (!findConditions.isValidOperator(itCondition.key()))
        ws->send(createErrorResponse(queryRspName, KvRequestStatus::FindOperatorInvalid).dump(), WsSendOpCode);
      else
      {
        const kvjson s {{"path", havePath ? itPath.value() : ""}, {itCondition.key(), itCondition.value()}};
        const KvCommand::Find find {.condition = findConditions.OpStringToOp.at(itCondition.key())};

        for (auto& worker : m_pools)
          worker->execute(KvCommand{.ws = ws,
                                    .loop = uWS::Loop::get(),
                                    .contents = s,
                                    .type = queryType,
                                    .cordinatedResponseHandler = onResponse,
                                    .find = find});
      
      
        rspLatch.wait();
      
        kvjson rsp;
        rsp[queryRspName]["st"] = KvRequestStatus::Ok;
        rsp[queryRspName]["k"] = kvjson::array();

        auto& resultArray = rsp[queryRspName]["k"];

        for(auto& result : results)
        {
          for (auto&& key : result)
            resultArray.insert(resultArray.cend(), std::move(key));
        }

        ws->send(rsp.dump(), WsSendOpCode);
      }
    }
  }

  // fc_always_inline void rename(KvWebSocket * ws, kvjson&& json)
  // {
  //   static const KvQueryType queryType = KvQueryType::RenameKey;
  //   static const std::string_view queryName = "KV_RNM";

  //   // find the pool for the existing key and new key
  //   // if they are the same pool, can rename key in that pool
  //   // else extract value from existing pool and insert into new pool
  // }


private:
  std::vector<KvPoolWorker *> m_pools;
  std::function<bool(const std::string_view&, PoolId&)> m_createPoolId;
};

}
}
}

#endif
