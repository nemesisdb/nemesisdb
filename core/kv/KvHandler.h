#ifndef FC_CORE_KVHANDLERS_H
#define FC_CORE_KVHANDLERS_H


#include <functional>
#include <vector>
#include <tuple>
#include <latch>
#include <core/kv/KvCommon.h>
#include <core/kv/KvPool.h>


namespace fusion { namespace core { namespace kv {


class KvHandler
{
public:
  KvHandler(const std::size_t nPools, const std::size_t coreOffset) : m_poolIndex(0U) // TODO
  {
    for (std::size_t pool = 0, core = coreOffset ; pool < nPools ; ++pool, ++core)
      m_pools.emplace_back(new KvPool{core, pool});
  }

private:
  
  // CAREFUL: these have to be in the order of KvQueryType enum
  const std::function<void(KvRequest *)> Handlers[static_cast<std::size_t>(7U/*KvQueryType::Max*/)] = 
  {
    std::bind(&KvHandler::set, std::ref(*this), std::placeholders::_1),
    std::bind(&KvHandler::setQ, std::ref(*this), std::placeholders::_1),
    std::bind(&KvHandler::get, std::ref(*this), std::placeholders::_1),
    std::bind(&KvHandler::add, std::ref(*this), std::placeholders::_1),
    std::bind(&KvHandler::addQ, std::ref(*this), std::placeholders::_1),
    std::bind(&KvHandler::rmv, std::ref(*this), std::placeholders::_1),
    std::bind(&KvHandler::clear, std::ref(*this), std::placeholders::_1)
    // std::bind(&KvHandler::executeServerInfo, std::ref(*this), std::placeholders::_1),
    // std::bind(&KvHandler::executeRenameKey, std::ref(*this), std::placeholders::_1)
  };

public:

  std::tuple<KvRequestStatus,std::string> handle(KvRequest * query)
  {
    std::tuple<KvRequestStatus,std::string> status = std::make_tuple(KvRequestStatus::Ok, "");

    auto& request = query->json;

    if (request.size() != 1U)
      std::get<KvRequestStatus>(status) = KvRequestStatus::CommandInvalid;
    else  [[likely]]
    {
      const std::string& queryName = request.cbegin().key();
    
      if (auto itType = QueryNameToType.find(queryName) ; itType != QueryNameToType.cend())
      {
        auto handler = Handlers[static_cast<std::size_t>(itType->second)];
        
        try
        {
          handler(query);
        }
        catch (const std::exception& kex)
        {
          std::get<KvRequestStatus>(status) = KvRequestStatus::Unknown;
        }
      }
      else
        status = std::make_tuple(KvRequestStatus::CommandUnknown, queryName);
    }

    return status;
  }


private:

  void set(KvRequest * query)
  {
    static const KvQueryType queryType = KvQueryType::Set;
    static const std::string_view queryName = "SET";

    for (auto& pair : query->json[queryName].items())
    {
      auto& key = pair.key(); 
      auto& value = pair.value(); 

      if (valueTypeValid(value))    [[likely]]
      {
        PoolId poolId;
        if (PoolIndexers[m_poolIndex](key, poolId))    [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = query->ws,
                                              .loop = uWS::Loop::get(),
                                              .contents = std::move(pair),
                                              .type = queryType});
        }
        else
        {
          // auto rsp = createErrorResponse(QueryStatus::KeyLengthInvalid, queryType, key);
          // m_ws->write(asio::buffer(rsp.dump()));
        }
      }
      else
      {
        // auto rsp = createErrorResponse(QueryStatus::ValueTypeInvalid, queryType, key);
        // m_ws->write(asio::buffer(rsp.dump()));
      }
    }
  }


  void setQ(KvRequest * query)
  {
    static const KvQueryType queryType = KvQueryType::SetQ;
    static const std::string_view queryName = "SETQ";

    for (auto& pair : query->json[queryName].items())
    {
      auto& key = pair.key(); 
      auto& value = pair.value(); 

      if (valueTypeValid(value))    [[likely]]
      {
        PoolId poolId;
        if (PoolIndexers[m_poolIndex](key, poolId))    [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = query->ws,
                                              .loop = uWS::Loop::get(),
                                              .contents = std::move(pair),
                                              .type = queryType});
        }
        else
        {
          // auto rsp = createErrorResponse(QueryStatus::KeyLengthInvalid, queryType, key);
          // m_ws->write(asio::buffer(rsp.dump()));
        }
      }
      else
      {
        // auto rsp = createErrorResponse(QueryStatus::ValueTypeInvalid, queryType, key);
        // m_ws->write(asio::buffer(rsp.dump()));
      }
    }
  }


  void get(KvRequest * query)
  {
    static const KvQueryType queryType = KvQueryType::Get;
    static const std::string_view queryName = "GET";

    for (auto& jsonKey : query->json[queryName])
    {
      bool valid = true;

      if (!jsonKey.is_string())
      {
        //auto rsp = createErrorResponse(QueryStatus::TypeInvalid, queryType, "key is not a string");
        //m_ws->write(asio::buffer(rsp.dump()));
      }
      else
      {
        PoolId poolId;
        auto key = jsonKey.get<std::string_view>();

        if (PoolIndexers[m_poolIndex](key, poolId))  [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = query->ws,
                                              .loop = uWS::Loop::get(),
                                              .contents = std::move(key),
                                              .type = queryType});
        }
        else
        {
          // auto rsp = createErrorResponse(QueryStatus::KeyLengthInvalid, queryType, key);
          // m_ws->write(asio::buffer(rsp.dump()));
        }
      }
    }
  }


  void doAdd(KvRequest * query, const KvQueryType queryType, const std::string_view queryName)
  {
    for (auto& pair : query->json[queryName].items())
    {
      auto& key = pair.key(); 
      auto& value = pair.value(); 

      if (valueTypeValid(value))    [[likely]]
      {
        PoolId poolId;
        if (PoolIndexers[m_poolIndex](key, poolId))    [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = query->ws,
                                              .loop = uWS::Loop::get(),
                                              .contents = std::move(pair),
                                              .type = queryType});
        }
        else
        {
          // auto rsp = createErrorResponse(QueryStatus::KeyLengthInvalid, queryType, key);
          // m_ws->write(asio::buffer(rsp.dump()));
        }
      }
      else
      {
        // auto rsp = createErrorResponse(QueryStatus::ValueTypeInvalid, queryType, key);
        // m_ws->write(asio::buffer(rsp.dump()));
      }
    }
  }


  void add(KvRequest * query)
  {
    static const KvQueryType queryType = KvQueryType::Add;
    static const std::string_view queryName = "ADD";

    doAdd(query, queryType, queryName);
  }

  void addQ(KvRequest * query)
  {
    static const KvQueryType queryType = KvQueryType::AddQ;
    static const std::string_view queryName = "ADDQ";

    doAdd(query, queryType, queryName);
  }

  void rmv(KvRequest * query)
  {
    static const KvQueryType queryType = KvQueryType::Remove;
    static const std::string_view queryName = "RMV";

    for (auto& key : query->json[queryName])
    {
      if (key.is_string())  [[likely]]
      {
        auto k = key.get<std::string_view>();

        PoolId poolId;

        if (PoolIndexers[m_poolIndex](k, poolId))  [[likely]]
        {
          m_pools[poolId]->execute(KvCommand{ .ws = query->ws,
                                            .loop = uWS::Loop::get(),
                                            .contents = std::move(k),
                                            .type = queryType});
        }
        else
        {
          // auto rsp = createErrorResponse(QueryStatus::KeyLengthInvalid, queryType, k);
          // m_ws->write(asio::buffer(rsp.dump()));
        }
      }
      else
      {
        // auto rsp = createErrorResponse(QueryStatus::TypeInvalid, queryType, "key is not a string");
        // m_ws->write(asio::buffer(rsp.dump()));
      }
    }
  }

  void clear(KvRequest * query)
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
        countRef += std::any_cast<std::size_t>(count);
      else
        clearedRef = false;

      done.count_down();
    };
                
    for (auto& pool : m_pools)
    {
      pool->execute(KvCommand{  .ws = query->ws,
                                .loop = uWS::Loop::get(),
                                .type = queryType,
                                .cordinatedResponseHandler = onPoolResponse});
    }
    
    done.wait();

    kvjson rsp;
    rsp["CLEAR_RSP"]["st"] = cleared ? KvRequestStatus::Ok : KvRequestStatus::Unknown;
    rsp["CLEAR_RSP"]["cnt"] = count;

    query->ws->send(rsp.dump(), WsSendOpCode);
  }

private:
  std::size_t m_poolIndex;
  std::vector<KvPool *> m_pools;
};

}
}
}

#endif
