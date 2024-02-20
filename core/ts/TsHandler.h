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
      std::bind(&TsHandler::create, std::ref(*this), std::placeholders::_1, std::placeholders::_2),
      std::bind(&TsHandler::add, std::ref(*this), std::placeholders::_1, std::placeholders::_2),
      std::bind(&TsHandler::get, std::ref(*this), std::placeholders::_1, std::placeholders::_2),
      std::bind(&TsHandler::getMultipleRanges, std::ref(*this), std::placeholders::_1, std::placeholders::_2)
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

  bool isValid (const njson& msg, const std::map<const std::string_view, const JsonType>& required)
  {
    for (const auto& [member, type] : required)
    {
      if (!(msg.contains(member) && msg.at(member).type() == type))
        return false;
    }
    return true;
  }


  void create (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_CREATE";
    static const auto cmdRspName  = "TS_CREATE_RSP";

    const auto& cmd = msg[cmdName];

    if (!isValid(cmd, {{"name", JsonString}, {"type", JsonString}}))
      PLOGD << "Invalid";
    else if (!(cmd.at("type") == "Ordered" && !cmd.at("name").empty()))
      PLOGD << "Invalid series type";
    else
      PLOGD << m_series.create(cmd.at("name").as_string(), cmdRspName).rsp;
  }


  void add (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_ADD";
    static const auto cmdRspName  = "TS_ADD_RSP";

    auto& cmd = msg.at(cmdName);

    if (!isValid(cmd, {{"ts", JsonString}, {"t", JsonArray}, {"v", JsonArray}}))
      PLOGD << "Invalid";
    else
      PLOGD << m_series.add(std::move(cmd), cmdRspName).rsp;
  }


  void get (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_GET";
    static const auto cmdRspName  = "TS_GET_RSP";

    auto& cmd = msg.at(cmdName);

    if (!isValid(cmd, {{"ts", JsonArray}, {"rng", JsonArray}}))
      PLOGD << "Invalid";
    else
      PLOGD << m_series.get(cmd, cmdRspName).rsp;
  }


  void getMultipleRanges (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_GET_MULTI";
    static const auto cmdRspName  = "TS_GET_MULTI_RSP";

    auto& cmd = msg.at(cmdName);
    
    for (const auto& member : cmd.object_range())
    {
      if (!isValid(member.value(), {{"rng", JsonArray}}))
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
