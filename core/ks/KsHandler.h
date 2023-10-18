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
    std::bind(&KsHandler::list,           std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KsHandler::addKey,         std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KsHandler::get,            std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KsHandler::rmvKey,         std::ref(*this), std::placeholders::_1, std::placeholders::_2),
    std::bind(&KsHandler::clearSet,       std::ref(*this), std::placeholders::_1, std::placeholders::_2),
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
    else if (cmd.at("name").get_ref<const std::string&>().empty())
      ws->send(createErrorResponse(queryRspName, RequestStatus::KeySetNameInvalid, "name").dump(), WsSendOpCode);
    else
    {
      const auto status = m_ks->create(cmd.at("name")) ? RequestStatus::KeySetCreated : RequestStatus::KeySetExists;
      ws->send(fcjson {{queryRspName, {{"st", status}, {"name", cmd.at("name")}}}}.dump(), WsSendOpCode);
    }
  }


  fc_always_inline void list(KvWebSocket * ws, fcjson&& json)
  {
    static const KsQueryType queryType = KsQueryType::List;
    static const std::string_view queryName     = "KS_LIST";
    static const std::string_view queryRspName  = "KS_LIST_RSP";

    auto& cmd = json.at(queryName);

    auto list = m_ks->list();
    ws->send(fcjson {{queryRspName, {{"st", RequestStatus::Ok}, {"list", std::move(list)}}}}.dump(), WsSendOpCode);
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
      auto status = RequestStatus::Ok;

      if (!cmd.at("k").empty())
        status = m_ks->addKeys(cmd.at("ks"), cmd.at("k"));
      
      ws->send(fcjson {{queryRspName, {{"st", status}, {"ks", cmd.at("ks")}}}}.dump(), WsSendOpCode);
    }
  }


  fc_always_inline void get(KvWebSocket * ws, fcjson&& json)
  {
    static const KsQueryType queryType = KsQueryType::Get;
    static const std::string_view queryName     = "KS_GET";
    static const std::string_view queryRspName  = "KS_GET_RSP";

    auto& cmd = json.at(queryName);

    fcjson rsp;

    if (cmd.empty())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing).dump(), WsSendOpCode);
    else
    {
      for (auto& item : cmd.items())
      {
        if (item.value().is_string())
        {
          // this is an array, so cmd.items().key() is the index, and value() is the ... value
          auto& ksName = item.value();

          fcjson keys = fcjson::array();
          
          m_ks->getSet(ksName, keys);
          rsp[queryRspName][ksName] = std::move(keys);
        }
      }

      rsp[queryRspName]["st"] = RequestStatus::Ok;
      ws->send(rsp.dump(), WsSendOpCode);
    }
  }


  fc_always_inline void rmvKey(KvWebSocket * ws, fcjson&& json)
  {
    static const KsQueryType queryType = KsQueryType::RemoveKey;
    static const std::string_view queryName     = "KS_RMV_KEY";
    static const std::string_view queryRspName  = "KS_RMV_KEY_RSP";

    auto& cmd = json.at(queryName);

    if (!cmd.contains("ks") || !cmd.contains("k"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing).dump(), WsSendOpCode);
    else if (!cmd.at("ks").is_string() || !cmd.at("k").is_string())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid).dump(), WsSendOpCode);
    else
    {
      const auto status = m_ks->removeKey(cmd.at("ks"), cmd.at("k"));
      ws->send(fcjson {{queryRspName, {{"st", status}, {"ks", cmd.at("ks")}, {"k", cmd.at("k")}}}}.dump(), WsSendOpCode);
    }    
  }


  fc_always_inline void clearSet(KvWebSocket * ws, fcjson&& json)
  {
    static const KsQueryType queryType = KsQueryType::ClearSet;
    static const std::string_view queryName     = "KS_CLEAR_SET";
    static const std::string_view queryRspName  = "KS_CLEAR_SET_RSP";

    auto& cmd = json.at(queryName);

    if (!cmd.contains("ks"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing).dump(), WsSendOpCode);
    else if (!cmd.at("ks").is_string())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid).dump(), WsSendOpCode);
    else
    {
      const auto status = m_ks->clearSet(cmd.at("ks"));
      ws->send(fcjson {{queryRspName, {{"st", status}, {"ks", cmd.at("ks")}}}}.dump(), WsSendOpCode);
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
      ws->send(fcjson {{queryRspName, {{"st", status}, {"ks", cmd.at("ks")}}}}.dump(), WsSendOpCode);
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

    if (!cmd.contains("ks"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing).dump(), WsSendOpCode);
    else if (!cmd.at("ks").is_string())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid).dump(), WsSendOpCode);
    else
    {    
      const RequestStatus status = m_ks->contains(cmd.at("ks")) ? RequestStatus::KeySetExists : RequestStatus::KeySetNotExist;
      ws->send(fcjson {{queryRspName, {{"st", status}, {"ks", cmd.at("ks")}}}}.dump(), WsSendOpCode);
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
    else if (!cmd.at("k").is_string() || !cmd.at("ks").is_string())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid).dump(), WsSendOpCode);
    else
    {
      const auto status = m_ks->contains(cmd.at("ks"), cmd.at("k"));
      ws->send(fcjson {{queryRspName, {{"st", status}, {"ks", cmd.at("ks")}, {"k", cmd.at("k")}}}}.dump(), WsSendOpCode);
    } 
  }

  
  fc_always_inline void moveKey(KvWebSocket * ws, fcjson&& json)
  {
    static const KsQueryType queryType = KsQueryType::MoveKey;
    static const std::string_view queryName     = "KS_MOVE_KEY";
    static const std::string_view queryRspName  = "KS_MOVE_KEY_RSP";

    auto& cmd = json.at(queryName);

    if (!cmd.contains("sourceKs") || !cmd.contains("targetKs") || !cmd.contains("k"))
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueMissing).dump(), WsSendOpCode);
    else if (!cmd.at("k").is_string() || !cmd.at("sourceKs").is_string() || !cmd.at("targetKs").is_string())
      ws->send(createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid).dump(), WsSendOpCode);
    else
    {
      const auto& source = cmd.at("sourceKs");
      const auto& target = cmd.at("targetKs");

      const RequestStatus status = m_ks->move(source, target, cmd.at("k"));
      ws->send(fcjson {{queryRspName, {{"st", status}, {"sourceKs", cmd.at("sourceKs")}, {"targetKs", cmd.at("targetKs")}, {"k", cmd.at("k")}}}}.dump(), WsSendOpCode);
    }
  }

private:
  ks::Sets * m_ks;
};

}
}
}

#endif
