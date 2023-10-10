#ifndef FC_CORE_KSHANDLERS_H
#define FC_CORE_KSHANDLERS_H


#include <functional>
#include <vector>
#include <tuple>
#include <latch>
#include <core/FusionCommon.h>
#include <core/ks/KsSets.h>


namespace fusion { namespace core { namespace ks {


class KsHandler
{
public:
  KsHandler(ks::Sets * ks) : m_ks(ks)
  {
    
  }

  ~KsHandler()
  {
    
  }

private:
  
  // CAREFUL: these have to be in the order of KvQueryType enum
  const std::function<void(KvWebSocket *, fcjson&&)> Handlers[static_cast<std::size_t>(KvQueryType::Max)] =
  {
    std::bind(&KsHandler::create, std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KsHandler::add, std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KsHandler::get, std::ref(*this), std::placeholders::_1, std::placeholders::_2)
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

  fc_always_inline void create(KvWebSocket * ws, fcjson&& json)
  {
    static const KsQueryType queryType = KsQueryType::Create;
    static const std::string_view queryName = "KS_CREATE";
    static const std::string_view queryRspName = "KS_CREATE_RSP";

    auto& cmd = json.at(queryName);

    if (!cmd.contains("name"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing, "name").dump(), WsSendOpCode);
    else if (!cmd.at("name").is_string())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "name").dump(), WsSendOpCode);
    else
    {
      const auto status = m_ks->create(cmd.at("name")) ? RequestStatus::KeySetCreated : RequestStatus::KeySetExists;
      ws->send(fcjson {{queryRspName, {"st", status}}}.dump(), WsSendOpCode);
    }
  }


  fc_always_inline void add(KvWebSocket * ws, fcjson&& json)
  {
    static const KsQueryType queryType = KsQueryType::Add;
    static const std::string_view queryName = "KS_ADD";
    static const std::string_view queryRspName = "KS_ADD_RSP";

    auto& cmd = json.at(queryName);

    if (cmd.size() != 2U)
      ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).dump(), WsSendOpCode);
    else if (!cmd.contains("k") || !cmd.contains("ks"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing).dump(), WsSendOpCode);
    else
    {
      if (isKeyValid(cmd.at("k")))
      {
        const auto status = m_ks->addKey(cmd.at("ks"), cmd.at("k"));
        ws->send(createErrorResponse(queryRspName, status).dump(), WsSendOpCode);
      }
      else
        ws->send(createErrorResponse(queryRspName, RequestStatus::KeyLengthInvalid).dump(), WsSendOpCode);
    }
  }


  fc_always_inline void get(KvWebSocket * ws, fcjson&& json)
  {
    static const KsQueryType queryType = KsQueryType::Get;
    static const std::string_view queryName = "KS_GET";
    static const std::string_view queryRspName = "KS_GET_RSP";

    auto& cmd = json.at(queryName);

    fcjson rsp;
    std::size_t setsFetched = 0U ;

    for (auto& item : cmd.items())
    {
      if (!item.value().is_string())
        ws->send(createErrorResponse(queryRspName, RequestStatus::KeyTypeInvalid).dump(), WsSendOpCode);
      else
      {
        auto& ksName = item.value();
        // this is an array, so cmd.items().key() is the index, and value() is the ... value
        fcjson set{{ksName, fcjson::array()}} ;
        std::cout << set << '\n';
        
        setsFetched += m_ks->getSet(ksName, set.at(ksName));
        rsp[queryRspName] = std::move(set);
      }
    }

    rsp[queryRspName]["st"] = setsFetched == cmd.size() ? RequestStatus::Ok : RequestStatus::KeySetNotExist;

    ws->send(rsp.dump(), WsSendOpCode);
  }

private:
  ks::Sets * m_ks;
};

}
}
}

#endif
