#ifndef FC_CORE_KSHANDLERS_H
#define FC_CORE_KSHANDLERS_H


#include <functional>
#include <vector>
#include <tuple>
#include <latch>
#include <array>
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
  
  // CAREFUL: these have to be in the order of KsQueryType enum
  const std::array<std::function<void(KvWebSocket *, fcjson&&)>, static_cast<std::size_t>(KsQueryType::Max)> Handlers = 
  {
    std::bind(&KsHandler::create,         std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KsHandler::addKey,         std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KsHandler::get,            std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KsHandler::rmvKey,         std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KsHandler::rmvAll,         std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KsHandler::deleteSet,      std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KsHandler::deleteAllSets,  std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KsHandler::setExists,      std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KsHandler::keyExists,      std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KsHandler::moveKey,        std::ref(*this), std::placeholders::_1, std::placeholders::_2)
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
    static const std::string_view queryName     = "KS_CREATE";
    static const std::string_view queryRspName  = "KS_CREATE_RSP";

    auto& cmd = json.at(queryName);

    if (!cmd.contains("name"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing, "name").dump(), WsSendOpCode);
    else if (!cmd.at("name").is_string())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "name").dump(), WsSendOpCode);
    else
    {
      const auto status = m_ks->create(cmd.at("name")) ? RequestStatus::KeySetCreated : RequestStatus::KeySetExists;
      ws->send(fcjson {{queryRspName, {{"st", status}}}}.dump(), WsSendOpCode);
    }
  }


  fc_always_inline void addKey(KvWebSocket * ws, fcjson&& json)
  {
    static const KsQueryType queryType = KsQueryType::AddKey;
    static const std::string_view queryName     = "KS_ADD_KEY";
    static const std::string_view queryRspName  = "KS_ADD_KEY_RSP";

    auto& cmd = json.at(queryName);

    if (cmd.size() != 2U)
      ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).dump(), WsSendOpCode);
    else if (!cmd.contains("k") || !cmd.contains("ks"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing).dump(), WsSendOpCode);
    else if (!cmd.at("k").is_array())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid, "k").dump(), WsSendOpCode);
    else
    {
      auto status = m_ks->addKeys(cmd.at("ks"), cmd.at("k"));
      ws->send(fcjson {{queryRspName, {"st", status}}}.dump(), WsSendOpCode);
    }
  }


  fc_always_inline void get(KvWebSocket * ws, fcjson&& json)
  {
    static const KsQueryType queryType = KsQueryType::Get;
    static const std::string_view queryName     = "KS_GET";
    static const std::string_view queryRspName  = "KS_GET_RSP";

    auto& cmd = json.at(queryName);

    fcjson rsp;
    std::size_t setsFetched = 0U ;

    for (auto& item : cmd.items())
    {
      if (!item.value().is_string())
        ws->send(createErrorResponse(queryRspName, RequestStatus::KeyTypeInvalid).dump(), WsSendOpCode);
      else
      {
        // this is an array, so cmd.items().key() is the index, and value() is the ... value
        auto& ksName = item.value();

        fcjson set{{ksName, fcjson::array()}} ;
        setsFetched += m_ks->getSet(ksName, set.at(ksName));  // intentional implicit bool to uint
        rsp[queryRspName] = std::move(set);
      }
    }

    rsp[queryRspName]["st"] = setsFetched == cmd.size() ? RequestStatus::Ok : RequestStatus::KeySetNotExist;

    ws->send(rsp.dump(), WsSendOpCode);
  }


  fc_always_inline void rmvKey(KvWebSocket * ws, fcjson&& json)
  {
    static const KsQueryType queryType = KsQueryType::RemoveKey;
    static const std::string_view queryName     = "KS_RMV_KEY";
    static const std::string_view queryRspName  = "KS_RMV_KEY_RSP";

    auto& cmd = json.at(queryName);

    if (!cmd.contains("ks") || !cmd.contains("k"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing).dump(), WsSendOpCode);
    else if (!cmd.at("ks").is_string() || !cmd.at("k").is_array())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid).dump(), WsSendOpCode);
    else
    {
      const auto status = m_ks->removeKeys(cmd.at("ks"), cmd.at("k"));
      ws->send(fcjson {{queryRspName, {{"st", status}}}}.dump(), WsSendOpCode);
    }    
  }


  fc_always_inline void rmvAll(KvWebSocket * ws, fcjson&& json)
  {
    static const KsQueryType queryType = KsQueryType::RemoveAll;
    static const std::string_view queryName     = "KS_RMV_ALL";
    static const std::string_view queryRspName  = "KS_RMV_ALL_RSP";

    auto& cmd = json.at(queryName);

    if (!cmd.contains("ks"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing).dump(), WsSendOpCode);
    else if (!cmd.at("ks").is_string())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid).dump(), WsSendOpCode);
    else
    {
      const auto status = m_ks->clearSet(cmd.at("ks"));
      ws->send(fcjson {{queryRspName, {{"st", status}}}}.dump(), WsSendOpCode);
    }
  }
  

  fc_always_inline void deleteSet(KvWebSocket * ws, fcjson&& json)
  {
    static const KsQueryType queryType = KsQueryType::DeleteSet;
    static const std::string_view queryName     = "KS_DELETE_SET";
    static const std::string_view queryRspName  = "KS_DELETE_SET_RSP";

    auto& cmd = json.at(queryName);

    if (!cmd.contains("ks"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing).dump(), WsSendOpCode);
    else if (!cmd.at("ks").is_string())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid).dump(), WsSendOpCode);
    else
    {
      const auto status = m_ks->deleteSet(cmd.at("ks"));
      ws->send(fcjson {{queryRspName, {{"st", status}}}}.dump(), WsSendOpCode);
    }
  }


  fc_always_inline void deleteAllSets(KvWebSocket * ws, fcjson&& json)
  {
    static const KsQueryType queryType = KsQueryType::DeleteSet;
    static const std::string_view queryName     = "KS_DELETE_ALL";
    static const std::string_view queryRspName  = "KS_DELETE_ALL_RSP";

    auto& cmd = json.at(queryName);

    if (!cmd.empty())
      ws->send(createErrorResponse(queryRspName, RequestStatus::CommandSyntax).dump(), WsSendOpCode);
    else
    {
      const auto status = m_ks->clear();
      ws->send(fcjson {{queryRspName, {{"st", status}}}}.dump(), WsSendOpCode);
    }
  }


  fc_always_inline void setExists(KvWebSocket * ws, fcjson&& json)
  {
    static const KsQueryType queryType = KsQueryType::SetExists;
    static const std::string_view queryName     = "KS_SET_EXISTS";
    static const std::string_view queryRspName  = "KS_SET_EXISTS_RSP";

    auto& cmd = json.at(queryName);

    if (!cmd.contains("ks") || !cmd.contains("k"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing).dump(), WsSendOpCode);
    else if (!cmd.at("k").is_string() || !cmd.at("ks").is_array())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid).dump(), WsSendOpCode);
    else
    {
      bool allExist = true;

      for(auto& ks : cmd.at("ks").items())
        allExist &= m_ks->contains(ks.value()) ;
      
      const RequestStatus status = allExist ? RequestStatus::KeySetExists : RequestStatus::KeySetNotExist;
      ws->send(fcjson {{queryRspName, {{"st", status}}}}.dump(), WsSendOpCode);
    } 
  }

  
  fc_always_inline void keyExists(KvWebSocket * ws, fcjson&& json)
  {
    static const KsQueryType queryType = KsQueryType::KeyExists;
    static const std::string_view queryName     = "KS_KEY_EXISTS";
    static const std::string_view queryRspName  = "KS_KEY_EXISTS_RSP";

    auto& cmd = json.at(queryName);

    if (!cmd.contains("ks") || !cmd.contains("k"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing).dump(), WsSendOpCode);
    else if (!cmd.at("k").is_array() || !cmd.at("ks").is_string())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid).dump(), WsSendOpCode);
    else
    {
      const auto& ks = cmd.at("ks");

      bool allExist = true;
      for(auto& k : cmd.at("k").items())
      {
        if (allExist &= m_ks->contains(ks, k.value()) ; !allExist)
          break;
      }
      
      const RequestStatus status = allExist ? RequestStatus::KeyExists : RequestStatus::KeyNotExist;
      ws->send(fcjson {{queryRspName, {{"st", status}}}}.dump(), WsSendOpCode);
    } 
  }

  
  fc_always_inline void moveKey(KvWebSocket * ws, fcjson&& json)
  {
    static const KsQueryType queryType = KsQueryType::MoveKey;
    static const std::string_view queryName     = "KS_MOVE_KEY";
    static const std::string_view queryRspName  = "KS_MOVE_KEY_RSP";

    auto& cmd = json.at(queryName);

    if (!cmd.contains("source_ks") || !cmd.contains("target_ks") || !cmd.contains("k"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing).dump(), WsSendOpCode);
    else if (!cmd.at("k").is_string() || !cmd.at("source_ks").is_string() || !cmd.at("target_ks").is_string())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid).dump(), WsSendOpCode);
    else
    {
      const auto& source = cmd.at("source_ks");
      const auto& target = cmd.at("target_ks");

      RequestStatus status =m_ks->move(source, target, cmd.at("k"));
      ws->send(fcjson {{queryRspName, {{"st", status}}}}.dump(), WsSendOpCode);
    }
  }

private:
  ks::Sets * m_ks;
};

}
}
}

#endif
