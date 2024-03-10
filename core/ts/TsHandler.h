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
      std::bind_front(&TsHandler::createTimeSeries,   std::ref(*this)),
      std::bind_front(&TsHandler::deleteTimeSeries,   std::ref(*this)),
      std::bind_front(&TsHandler::addEvt,             std::ref(*this)),
      std::bind_front(&TsHandler::get,                std::ref(*this)),
      std::bind_front(&TsHandler::getMultipleRanges,  std::ref(*this)),
      std::bind_front(&TsHandler::createIndex,        std::ref(*this))
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


  ndb_always_inline void send (TsWebSocket * ws, const njson& msg)
  {
    send(ws, msg.to_string());
  }

  
  bool isValid (TsWebSocket * ws, const std::string& cmdRspName, const njson& cmd, 
                const std::map<const std::string_view, const Param>& params,
                std::function<std::tuple<TsRequestStatus, const std::string_view>(const njson&)> onPostValidate = nullptr)
  {
    const auto [stat, msg] = isCmdValid<TsRequestStatus, TsRequestStatus::Ok, TsRequestStatus::ParamMissing, TsRequestStatus::ParamType>(cmd, params, onPostValidate);
    
    if (stat != TsRequestStatus::Ok)
    {
      static njson ErrRsp (jsoncons::json_object_arg, {});

      njson rsp {ErrRsp};
      rsp[cmdRspName]["st"] = toUnderlying(stat);
      rsp[cmdRspName]["msg"] = msg;

      send(ws, rsp);
      return false;
    }
    else
      return true;
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


  void deleteTimeSeries (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_DELETE"s;
    static const auto cmdRspName  = "TS_DELETE_RSP"s;


    auto validate = [](const njson& cmd) -> std::tuple<TsRequestStatus, const std::string_view>
    {
      if (cmd.at("name").empty())
        return {TsRequestStatus::ParamValue, "Series name empty"};
      
      return {TsRequestStatus::Ok, ""};
    };

    const auto& cmd = msg[cmdName];

    if (isValid(ws, cmdRspName, cmd, {Param::required("name", JsonString)}, validate))
      send(ws, m_series.deleteSeries(cmd.at("name").as_string(), cmdRspName).rsp);
  }


  void createIndex (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_CREATE_INDEX"s;
    static const auto cmdRspName  = "TS_CREATE_INDEX_RSP"s;

    const auto& cmd = msg[cmdName];

    if (isValid(ws, cmdRspName, cmd, {Param::required("ts", JsonString), Param::required("key", JsonString)}))
      send(ws, m_series.createIndex(cmd.at("ts").as_string(), cmd.at("key").as_string(), cmdRspName).rsp);
  }


  void addEvt (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_ADD_EVT"s;
    static const auto cmdRspName  = "TS_ADD_EVT_RSP"s;

    auto validate = [](const njson& cmd) -> std::tuple<TsRequestStatus, const std::string_view>
    {
      const auto tSize = cmd.at("t").size();
      const auto evtSize = cmd.at("evt").size();

      if (tSize == 0 || evtSize == 0)
        return {TsRequestStatus::ParamValue, "'evt' and/or 't' are empty"};
      else if (tSize != evtSize)
        return {TsRequestStatus::ParamValue, "'evt' and 't' not same length"};

      for (const auto& item : cmd.at("t").array_range())
      {
        if (!item.is<SeriesTime>())
          return {TsRequestStatus::ParamType, "'t' must be an array of signed integers"};
      }  

      for (const auto& item : cmd.at("evt").array_range())
      {
        if (!item.is_object())
          return {TsRequestStatus::ParamType, "'evt' must be an array of objects"};
      }        
      
      return {TsRequestStatus::Ok, ""};
    };


    auto& cmd = msg.at(cmdName);

    if (isValid(ws, cmdRspName, cmd, {Param::required("ts", JsonString), Param::required("t", JsonArray), Param::required("evt", JsonArray)}, validate))
      send(ws, m_series.add(std::move(cmd), cmdRspName).rsp);
  }


  void get (TsWebSocket * ws, njson&& msg)
  {
    static const auto cmdName     = "TS_GET"s;
    static const auto cmdRspName  = "TS_GET_RSP"s;
    
    auto& cmd = msg.at(cmdName);

    if (isValid(ws, cmdRspName, cmd, {Param::required("ts", JsonString),Param::required("rng", JsonArray), Param::optional("where", JsonObject)}, std::bind_front(&TsHandler::validateGet, this)))
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
    const auto& rng = cmd.at("rng");
    const auto rngSize = rng.size();

    if (!(rngSize == 2 || rngSize == 0))
      return {TsRequestStatus::RngSize, "'rng' must have zero or two values"};
    else if (rngSize == 2 && rng[0].as<SeriesTime>() > rng[1].as<SeriesTime>())
      return {TsRequestStatus::RngValues,"'rng' invalid, first value must not be greater than second"};
    else if (cmd.contains("where"))
    {
      if (cmd.at("where").empty())
        return {TsRequestStatus::ParamValue, "'where' has no terms"};

      // only objects permitted in "where" and must not be empty
      for (const auto& member : cmd.at("where").object_range())
      {
        const auto& term = member.value();

        if (!term.is_object())
          return {TsRequestStatus::ParamType, "Term in 'where' is not an object"};
        else if (term.size() != 1)
          return {TsRequestStatus::ParamValue, "Term in 'where' must have one member"};

        // confirm operators have valid value types
        const auto& op = term.object_range().cbegin()->key();
        const auto& operand = term.object_range().cbegin()->value();
        // disallow these operators with object and bool (">=":{"a":1} or ">":true  don't mean anything)
        if ((op == ">" || op == ">="|| op == "<" || op == "<=") && (operand.is_object() || operand.is_bool()))
          return {TsRequestStatus::ParamType, "Operator in 'where' has an invalid operand type"};
        else if (op == "[]")
        {
          if (!(operand.is_array() && operand.size() == 2))
            return {TsRequestStatus::ParamType, "Operator [] in 'where' value must be an array of two items"};
          else if (operand[0] > operand[1])
            return {TsRequestStatus::ParamType, "Range operator in 'where' has min is greater than max"};
        }
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
