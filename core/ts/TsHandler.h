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
      std::bind(&TsHandler::createTimeSeries,   std::ref(*this), std::placeholders::_1, std::placeholders::_2),
      std::bind(&TsHandler::add,                std::ref(*this), std::placeholders::_1, std::placeholders::_2),
      std::bind(&TsHandler::get,                std::ref(*this), std::placeholders::_1, std::placeholders::_2),
      std::bind(&TsHandler::getMultipleRanges,  std::ref(*this), std::placeholders::_1, std::placeholders::_2),
      std::bind(&TsHandler::createIndex,        std::ref(*this), std::placeholders::_1, std::placeholders::_2)
    };

    TsRequestStatus status = TsRequestStatus::Ok;

    if (const auto it = ts::QueryNameToType.find(command); it == ts::QueryNameToType.cend())
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
  
  // Checks the params (if present and correct type). If that passes, onPostValidate() is called (if set) for custom checks
  bool isValid (const njson& cmd,
                const std::map<const std::string_view, const Param>& params,
                std::function<std::tuple<TsRequestStatus, const std::string_view>(const njson&)> onPostValidate = nullptr)
  {
    const auto& [stat, msg] = isCmdValid<TsRequestStatus, TsRequestStatus::Ok, TsRequestStatus::ParamMissing, TsRequestStatus::ParamType>(cmd, params, onPostValidate);
    
    if (stat != TsRequestStatus::Ok)
      PLOGD << msg;
      
    return stat == TsRequestStatus::Ok;
  }


  void createTimeSeries (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_CREATE";
    static const auto cmdRspName  = "TS_CREATE_RSP";


    auto validate = [](const njson& cmd) -> std::tuple<TsRequestStatus, const std::string_view>
    {
      if (!(cmd.at("type") == "Ordered" && !cmd.at("name").empty()))
        return {TsRequestStatus::SeriesType, "Series type invalid"};
      
      return {TsRequestStatus::Ok, ""};
    };

    const auto& cmd = msg[cmdName];

    if (isValid(cmd, {Param::required("name", JsonString), Param::required("type", JsonString)}, validate))
      PLOGD << m_series.create(cmd.at("name").as_string(), cmdRspName).rsp;
  }


  void createIndex (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_CREATE_INDEX";
    static const auto cmdRspName  = "TS_CREATE_INDEX_RSP";

    const auto& cmd = msg[cmdName];

    if (isValid(cmd, {Param::required("ts", JsonString), Param::required("key", JsonString)}))
      PLOGD << m_series.createIndex(cmd.at("ts").as_string(), cmd.at("key").as_string(), cmdRspName).rsp;
  }


  void add (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_ADD";
    static const auto cmdRspName  = "TS_ADD_RSP";

    auto& cmd = msg.at(cmdName);

    if (isValid(cmd, {Param::required("ts", JsonString), Param::required("t", JsonArray), Param::required("v", JsonArray)}))
      PLOGD << m_series.add(std::move(cmd), cmdRspName).rsp;
  }


  void get (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_GET";
    static const auto cmdRspName  = "TS_GET_RSP";
    

    auto validate = [](const njson& cmd) -> std::tuple<TsRequestStatus, const std::string_view>
    {
      if (const auto& rng = cmd.at("rng") ; rng.size() > 2)
        return {TsRequestStatus::RngSize, "'rng' cannot have more than 2 values"};
      else if (rng[0].as<SeriesTime>() > rng[1].as<SeriesTime>())
        return {TsRequestStatus::RngValues,"'rng' invalid, first value must not be greater than second"};
      else if (cmd.contains("where"))
      {
        // only objects permitted in "where"
        for (const auto& member : cmd.at("where").object_range())
        {
          if (!member.value().is_object())
            return {TsRequestStatus::ParamType, "Member in 'where' is not an object"};
        }
      }
      return {TsRequestStatus::Ok,""};
    };


    auto& cmd = msg.at(cmdName);

    if (isValid(cmd, {Param::required("ts", JsonArray),Param::required("rng", JsonArray), Param::optional("where", JsonObject)}, validate))
      PLOGD << m_series.get(cmd, cmdRspName).rsp;
  }


  void getMultipleRanges (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_GET_MULTI";
    static const auto cmdRspName  = "TS_GET_MULTI_RSP";

    auto& cmd = msg.at(cmdName);
    
    for (const auto& member : cmd.object_range())
    {
      if (!isValid(member.value(), {Param::required("rng", JsonArray), Param::optional("where", JsonObject)}))
        return;
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
