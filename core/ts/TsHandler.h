#ifndef NDB_CORE_TSHANDLER_H
#define NDB_CORE_TSHANDLER_H

#include <core/NemesisCommon.h>
#include <core/ts/Series.h>
#include <string_view>


namespace nemesis { namespace core { namespace ts {

using namespace std::string_literals; 


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

  // Send response to client: must only be called from the originating I/O thread
  ndb_always_inline void send (TsWebSocket * ws, const std::string& msg)
  {
    #ifdef NDB_DEBUG
    PLOGD << msg;
    #endif

    ws->send(msg, WsSendOpCode);
  }


  // Send response to client: must only be called from the originating I/O thread
  ndb_always_inline void send (TsWebSocket * ws, const std::string_view msg)
  {
    #ifdef NDB_DEBUG
    PLOGD << msg;
    #endif

    ws->send(msg, WsSendOpCode);
  }


  ndb_always_inline void send (TsWebSocket * ws, njson&& msg)
  {
    send(ws, msg.to_string());
  }


  ndb_always_inline void send (TsWebSocket * ws, const njson& msg)
  {
    send(ws, msg.to_string());
  }

  
  bool isValid (TsWebSocket * ws, const std::string& cmdRspName, const njson& cmd, 
                const std::map<const std::string_view, const Param>& params,
                std::function<std::tuple<TsRequestStatus, const std::string_view>(const njson&)> onPostValidate = nullptr)
  {
    const auto& [stat, msg] = isCmdValid<TsRequestStatus, TsRequestStatus::Ok, TsRequestStatus::ParamMissing, TsRequestStatus::ParamType>(cmd, params, onPostValidate);
    
    if (stat != TsRequestStatus::Ok)
    {
      static njson ErrRsp (jsoncons::json_object_arg, {});

      njson rsp {ErrRsp};
      rsp[cmdRspName]["st"] = toUnderlying(stat);
      rsp[cmdRspName]["msg"] = msg;

      send(ws, rsp);
    }      
      
    return stat == TsRequestStatus::Ok;
  }


  void createTimeSeries (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_CREATE"s;
    static const auto cmdRspName  = "TS_CREATE_RSP"s;


    auto validate = [](const njson& cmd) -> std::tuple<TsRequestStatus, const std::string_view>
    {
      if (!(cmd.at("type") == "Ordered" && !cmd.at("name").empty()))
        return {TsRequestStatus::SeriesType, "Series type invalid"};
      
      return {TsRequestStatus::Ok, ""};
    };

    const auto& cmd = msg[cmdName];

    if (isValid(ws, cmdRspName, cmd, {Param::required("name", JsonString), Param::required("type", JsonString)}, validate))
      send(ws, m_series.create(cmd.at("name").as_string(), cmdRspName).rsp);
  }


  void createIndex (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_CREATE_INDEX"s;
    static const auto cmdRspName  = "TS_CREATE_INDEX_RSP"s;

    const auto& cmd = msg[cmdName];

    if (isValid(ws, cmdRspName, cmd, {Param::required("ts", JsonString), Param::required("key", JsonString)}))
      send(ws, m_series.createIndex(cmd.at("ts").as_string(), cmd.at("key").as_string(), cmdRspName).rsp);
  }


  void add (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_ADD_EVT"s;
    static const auto cmdRspName  = "TS_ADD_EVT_RSP"s;

    auto validate = [](const njson& cmd) -> std::tuple<TsRequestStatus, const std::string_view>
    {
      if (cmd.at("t").size() != cmd.at("v").size())
        return {TsRequestStatus::ValueSize, "'v' and 't' not same length"};

      for (const auto& item : cmd.at("v").array_range())
      {
        if (!item.is_object())
          return {TsRequestStatus::ParamType, "'v' must be an array of objects"};
      }        
      
      return {TsRequestStatus::Ok, ""};
    };


    auto& cmd = msg.at(cmdName);

    if (isValid(ws, cmdRspName, cmd, {Param::required("ts", JsonString), Param::required("t", JsonArray), Param::required("v", JsonArray)}, validate))
      send(ws, m_series.add(std::move(cmd), cmdRspName).rsp);
  }


  void get (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_GET"s;
    static const auto cmdRspName  = "TS_GET_RSP"s;
    
    auto& cmd = msg.at(cmdName);

    if (isValid(ws, cmdRspName, cmd, {Param::required("ts", JsonArray),Param::required("rng", JsonArray), Param::optional("where", JsonObject)}, std::bind_front(&TsHandler::validateGet, this)))
      send(ws, m_series.get(cmd, cmdRspName).rsp);
  }


  void getMultipleRanges (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_GET_MULTI"s;
    static const auto cmdRspName  = "TS_GET_MULTI_RSP"s;

    auto& cmd = msg.at(cmdName);
    
    for (const auto& member : cmd.object_range())
    {
      if (!isValid(ws, cmdRspName, member.value(), {Param::required("rng", JsonArray), Param::optional("where", JsonObject)}, std::bind_front(&TsHandler::validateGet, this)))
        return;
    }

    send(ws, m_series.getMultipleRanges(cmd, cmdRspName).rsp);
  }


  std::tuple<TsRequestStatus, const std::string_view> validateGet(const njson& cmd)
  {
    if (const auto& rng = cmd.at("rng") ; rng.size() > 2)
      return {TsRequestStatus::RngSize, "'rng' cannot have more than 2 values"};
    else if (rng[0].as<SeriesTime>() > rng[1].as<SeriesTime>())
      return {TsRequestStatus::RngValues,"'rng' invalid, first value must not be greater than second"};
    else if (cmd.contains("where"))
    {
      if (cmd.at("where").empty())
        return {TsRequestStatus::ValueSize, "'where' has no terms"};

      // only objects permitted in "where" and must not be empty
      for (const auto& member : cmd.at("where").object_range())
      {
        const auto& term = member.value();

        if (!term.is_object())
          return {TsRequestStatus::ParamType, "Term in 'where' is not an object"};
        else if (term.size() != 1)
          return {TsRequestStatus::ValueSize, "Term in 'where' must have one member"};
      }
    }
    return {TsRequestStatus::Ok,""};
  }

private:
  njson m_config;
  Series m_series;
};



}
}
}

#endif
