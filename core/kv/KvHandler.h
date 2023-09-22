#ifndef FC_CORE_KVHANDLERS_H
#define FC_CORE_KVHANDLERS_H


#include <functional>
#include <vector>
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
  
  const std::function<void(KvRequest *)> Handlers[static_cast<std::size_t>(3U/*KvQueryType::Max*/)] = 
  {
    std::bind(&KvHandler::set, std::ref(*this), std::placeholders::_1),
    std::bind(&KvHandler::setQ, std::ref(*this), std::placeholders::_1),
    std::bind(&KvHandler::get, std::ref(*this), std::placeholders::_1)
    // std::bind(&KvHandler::executeAdd, std::ref(*this), std::placeholders::_1),
    // std::bind(&KvHandler::executeAddQ, std::ref(*this), std::placeholders::_1),
    // std::bind(&KvHandler::executeRemove, std::ref(*this), std::placeholders::_1),
    // std::bind(&KvHandler::executeClear, std::ref(*this), std::placeholders::_1),
    // std::bind(&KvHandler::executeServerInfo, std::ref(*this), std::placeholders::_1),
    // std::bind(&KvHandler::executeRenameKey, std::ref(*this), std::placeholders::_1)
  };

public:

  bool handle(KvRequest * query)
  {
    bool handled = true;

    auto& request = query->json;

    if (request.size() != 1U)
    {
      //auto rsp = createServerErrorResponse (KvErrorCode::ExcessiveValues, "Only one command permitted"sv);
      //m_ws->write(asio::buffer(rsp.dump()));
      std::cout << "ExcessiveValues\n";
      handled = false;
    }
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
          // auto rsp = createServerErrorResponse (kex.ec(), kex.what());
          // ws->write(asio::buffer(rsp.dump()));
          std::cout << "Exception " << kex.what() << "\n";
          handled = false;
        }
      }
      else
      {
        // auto rsp = createServerErrorResponse (KvErrorCode::QueryUnknown, queryName);
        // m_ws->write(asio::buffer(rsp.dump()));
        std::cout << "QueryUnknown\n";
        handled = false;
      }
    }

    return handled;
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


private:
  std::size_t m_poolIndex;
  std::vector<KvPool *> m_pools;
};

}
}
}

#endif
