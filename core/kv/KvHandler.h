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
  KvHandler(const std::size_t nPools, const std::size_t coreOffset) : m_poolIndex(nPools == 1U ? 1U : 0U)
  {
    for (std::size_t pool = 0, core = coreOffset ; pool < nPools ; ++pool, ++core)
      m_pools.emplace_back(new KvPoolWorker{core, pool});
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
    std::bind(&KvHandler::count, std::ref(*this), std::placeholders::_1, std::placeholders::_2)/*,
    std::bind(&KvHandler::renameKey, std::ref(*this), std::placeholders::_1)*/
  };

public:

  std::tuple<KvRequestStatus,std::string> handle(KvWebSocket * ws, kvjson&& json)
  {
    std::tuple<KvRequestStatus,std::string> status = std::make_tuple(KvRequestStatus::Ok, "");

    if (json.size() != 1U)
      std::get<KvRequestStatus>(status) = KvRequestStatus::CommandMultiple;
    else  [[likely]]
    {
      const std::string& queryName = json.cbegin().key();
    
      if (auto itType = QueryNameToType.find(queryName) ; itType != QueryNameToType.cend())
      {
        auto handler = Handlers[static_cast<std::size_t>(itType->second)];

        // TODO check command root is correct type (most are object, GET is array)
        
        try
        {
          handler(ws, std::move(json));
        }
        catch (const std::exception& kex)
        {
          std::get<KvRequestStatus>(status) = KvRequestStatus::Unknown;
        }
      }
      else
        status = std::make_tuple(KvRequestStatus::CommandNotExist, queryName);
    }

    return status;
  }


private:

  fc_always_inline void set(KvWebSocket * ws, kvjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Set;
    static const std::string_view queryName = "SET";
    static const std::string_view queryRspName = "SET_RSP";

    for (auto& pair : json[queryName].items())
    {
      auto& key = pair.key(); 
      auto& value = pair.value(); 

      if (valueTypeValid(value))    [[likely]]
      {
        PoolId poolId;
        if (PoolIndexers[m_poolIndex](key, poolId))    [[likely]]
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
    static const std::string_view queryName = "SETQ";
    static const std::string_view queryRspName = "SETQ_RSP";


    for (auto& pair : json[queryName].items())
    {
      auto& key = pair.key(); 
      auto& value = pair.value(); 

      if (valueTypeValid(value))    [[likely]]
      {
        PoolId poolId;
        if (PoolIndexers[m_poolIndex](key, poolId))    [[likely]]
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
    static const std::string_view queryName = "GET";
    static const std::string_view queryRspName = "GET_RSP";

    for (auto& jsonKey : json[queryName])
    {
      bool valid = true;

      if (!jsonKey.is_string())
        ws->send(createErrorResponse(queryRspName, KvRequestStatus::KeyTypeInvalid).dump(), WsSendOpCode);
      else
      {
        PoolId poolId;
        auto key = jsonKey.get<std::string_view>();

        if (PoolIndexers[m_poolIndex](key, poolId))  [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = ws,
                                              .loop = uWS::Loop::get(),
                                              .contents = std::move(key),
                                              .type = queryType});
        }
        else
          ws->send(createErrorResponse(queryRspName, KvRequestStatus::KeyLengthInvalid).dump(), WsSendOpCode);
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
        if (PoolIndexers[m_poolIndex](key, poolId))    [[likely]]
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
    static const std::string_view queryName = "ADD";
    static const std::string_view queryRspName = "ADD_RSP";

    doAdd(std::move(json), ws, queryType, queryName, queryRspName);
  }

  fc_always_inline void addQ(KvWebSocket * ws, kvjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::AddQ;
    static const std::string_view queryName = "ADDQ";
    static const std::string_view queryRspName = "ADDQ_RSP";

    doAdd(std::move(json), ws, queryType, queryName, queryRspName);
  }

  fc_always_inline void rmv(KvWebSocket * ws, kvjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Remove;
    static const std::string_view queryName = "RMV";
    static const std::string_view queryRspName = "RMV_RSP";

    for (auto& key : json[queryName])
    {
      if (key.is_string())  [[likely]]
      {
        auto k = key.get<std::string_view>();

        PoolId poolId;

        if (PoolIndexers[m_poolIndex](k, poolId))  [[likely]]
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
    static const std::string_view queryName = "CLEAR";
    
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
    rsp["CLEAR_RSP"]["st"] = cleared ? KvRequestStatus::Ok : KvRequestStatus::Unknown;
    rsp["CLEAR_RSP"]["cnt"] = count;

    ws->send(rsp.dump(), WsSendOpCode);
  }

  fc_always_inline void serverInfo(KvWebSocket * ws, kvjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::ServerInfo;
    static const std::string_view queryName = "SERVER_INFO";

    static kvjson rsp {{"SERVER_INFO_RSP", {{"st", KvRequestStatus::Ok}, {"version", FUSION_VERSION}}}};
    rsp["SERVER_INFO_RSP"]["qryCnt"] = serverStats->queryCount.load();

    ws->send(rsp.dump(), WsSendOpCode);
  }

  fc_always_inline void count(KvWebSocket * ws, kvjson&& json)
  {
    static const KvQueryType queryType = KvQueryType::Count;
    static const std::string_view queryName = "COUNT";

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
    rsp["COUNT_RSP"]["st"] = KvRequestStatus::Ok;
    rsp["COUNT_RSP"]["cnt"] = count;

    ws->send(rsp.dump(), WsSendOpCode);
  }

  // fc_always_inline void renameKey(KvWebSocket * ws, kvjson&& json)
  // {
  //   static const KvQueryType queryType = KvQueryType::RenameKey;
  //   static const std::string_view queryName = "RNM_KEY";

  //   // find the pool for the existing key and new key
  //   // if they are the same pool, can rename key in that pool
  //   // else extract value from existing pool and insert into new pool
  // }


private:
  std::size_t m_poolIndex;
  std::vector<KvPoolWorker *> m_pools;
};

}
}
}

#endif
