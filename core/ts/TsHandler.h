#ifndef NDB_CORE_TSHANDLER_H
#define NDB_CORE_TSHANDLER_H

#include <core/NemesisCommon.h>
#include <core/ts/Series.h>


namespace nemesis { namespace core { namespace ts {


class TsHandler
{
public:
  TsHandler(const njson& config) : m_config(config)
  {

  }


  TsRequestStatus handle (TsWebSocket * ws, const std::string_view& command, njson&& json)
  {
    static const std::array<std::function<void(TsWebSocket *, njson&&)>, static_cast<std::size_t>(TsCommand::Max)> Handlers =
    {
      std::bind(&TsHandler::createTs, std::ref(*this), std::placeholders::_1, std::placeholders::_2),
      std::bind(&TsHandler::add,      std::ref(*this), std::placeholders::_1, std::placeholders::_2),
      std::bind(&TsHandler::get,      std::ref(*this), std::placeholders::_1, std::placeholders::_2),
      std::bind(&TsHandler::getMultipleRanges,  std::ref(*this), std::placeholders::_1, std::placeholders::_2),
      std::bind(&TsHandler::createIndex,        std::ref(*this), std::placeholders::_1, std::placeholders::_2)
    };

    TsRequestStatus status = TsRequestStatus::Ok;

    if (auto it = ts::QueryNameToType.find(command); it == ts::QueryNameToType.cend())
      return TsRequestStatus::CommandNotExist;
    else
    {
      try
      {
        Handlers[static_cast<std::size_t>(it->second)](ws, std::move(json));
      }
      catch (const std::exception& ex)
      {
        PLOGE << ex.what();
        status = TsRequestStatus::UnknownError;
      }
    }

    return status;
  }

private:

  
  struct Param
  {
  private:
    Param(const JsonType type, const bool required) : type(type), isRequired(required)
    {

    }

  public:
    static std::pair<std::string_view, Param> required (const std::string_view name, const JsonType type)
    {
      return {name, Param {type, true}};
    }

    static std::pair<std::string_view, Param> optional (const std::string_view name, const JsonType type)
    {
      return {name, Param {type, false}};
    }

    bool isRequired;
    JsonType type;
  };


  // bool isValid (const njson& msg, const std::map<const std::string_view, const JsonType>& required)
  // {
  //   for (const auto& [member, type] : required)
  //   {
  //     if (!(msg.contains(member) && msg.at(member).type() == type))
  //       return false;
  //   }
  //   return true;
  // }


  bool isValid (const njson& msg, const std::map<const std::string_view, const Param>& params)
  {
    for (const auto& [member, param] : params)
    {
      if (param.isRequired && !msg.contains(member))
        return false;
      else if (msg.contains(member) && msg.at(member).type() != param.type) // not required but present OR required and present: check type
        return false;
    }
    return true;
  }


  void createTs (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_CREATE";
    static const auto cmdRspName  = "TS_CREATE_RSP";

    const auto& cmd = msg[cmdName];

    //if (!isValid(cmd, {{"name", JsonString}, {"type", JsonString}}))
    if (!isValid(cmd, {Param::required("name", JsonString), Param::required("type", JsonString)}))
      PLOGD << "Invalid";
    else if (!(cmd.at("type") == "Ordered" && !cmd.at("name").empty()))
      PLOGD << "Invalid series type";
    else
      PLOGD << m_series.create(cmd.at("name").as_string(), cmdRspName).rsp;
  }


  void createIndex (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_CREATE_INDEX";
    static const auto cmdRspName  = "TS_CREATE_INDEX_RSP";

    const auto& cmd = msg[cmdName];

    if (!isValid(cmd, {Param::required("ts", JsonString), Param::required("key", JsonString)}))
      PLOGD << "Invalid";
    else
      PLOGD << m_series.createIndex(cmd.at("ts").as_string(), cmd.at("key").as_string(), cmdRspName).rsp;
  }


  void add (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_ADD";
    static const auto cmdRspName  = "TS_ADD_RSP";

    auto& cmd = msg.at(cmdName);

    //if (!isValid(cmd, {{"ts", JsonString}, {"t", JsonArray}, {"v", JsonArray}}))
    if (!isValid(cmd, {Param::required("ts", JsonString), Param::required("t", JsonArray), Param::required("v", JsonArray)}))
      PLOGD << "Invalid";
    else
      PLOGD << m_series.add(std::move(cmd), cmdRspName).rsp;
  }


  void get (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_GET";
    static const auto cmdRspName  = "TS_GET_RSP";

    auto& cmd = msg.at(cmdName);

    //if (!isValid(cmd, {{"ts", JsonArray}, {"rng", JsonArray}}))
    if (!isValid(cmd, {Param::required("ts", JsonArray), Param::required("rng", JsonArray), Param::optional("where", JsonObject)}))
      PLOGD << "Invalid";
    else
    {
      bool valid = true;

      if (cmd.at("rng").size() > 2)
      {
        PLOGD << "'rng' cannot have more than 2 values";
        valid = false;
      }
      else if (cmd.contains("where"))
      {
        // only objects permitted in "where"
        for (const auto& member : cmd.at("where").object_range())
        {
          if (!member.value().is_object())
          {
            valid = false;
            break;
          }
        }
      }

      if (valid)
        PLOGD << m_series.get(cmd, cmdRspName).rsp;
    }
  }


  void getMultipleRanges (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_GET_MULTI";
    static const auto cmdRspName  = "TS_GET_MULTI_RSP";

    auto& cmd = msg.at(cmdName);
    
    for (const auto& member : cmd.object_range())
    {
      //if (!isValid(member.value(), {{"rng", JsonArray}}))
      if (!isValid(member.value(), {Param::required("rng", JsonArray)}))
      {
        PLOGD << "Invalid";
        return;
      }
    }

    PLOGD << m_series.getMultipleRanges(cmd, cmdRspName).rsp;
  }


private:
  njson m_config;
  Series m_series;
};



}
}
}

#endif
