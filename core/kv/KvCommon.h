#ifndef FC_CORE_KVCOMMON_H
#define FC_CORE_KVCOMMON_H

#include <map>
#include <array>
#include <any>
#include <nlohmann/json.hpp>
#include <uwebsockets/App.h>
#include <core/FusionCommon.h>


namespace fusion { namespace core { namespace kv {


using kvhash_t = std::uint32_t;
using PoolId = std::size_t;

const uWS::OpCode WsSendOpCode = uWS::OpCode::TEXT;
static const SessionToken defaultSessionToken = "-";


static kvhash_t MaxPools = 1U;

enum class KvQueryType : std::uint8_t
{  
  Set,
  SetQ,
  Get,
  Add,
  AddQ,
  Remove,
  Clear,
  ServerInfo,
  Count,
  Append,
  Contains,
  ArrayMove,
  Find,
  SessionNew,
  SessionEnd,
  SessionSet,
  SessionSetQ,
  SessionGet,
  SessionAdd,
  SessionAddQ,
  SessionRemove,
  SessionClear,
  SessionCount,
  SessionAppend,
  SessionContains,
  SessionArrayMove,
  SessionFind,
  SessionUpdate,
  Max,
  Unknown,
};


const std::map<const std::string_view, std::tuple<const KvQueryType, const fcjson::value_t>> QueryNameToType = 
{
  {"KV_SET",          {KvQueryType::Set,          fcjson::value_t::object}},
  {"KV_SETQ",         {KvQueryType::SetQ,         fcjson::value_t::object}},
  {"KV_GET",          {KvQueryType::Get,          fcjson::value_t::array}},
  {"KV_ADD",          {KvQueryType::Add,          fcjson::value_t::object}},
  {"KV_ADDQ",         {KvQueryType::AddQ,         fcjson::value_t::object}},
  {"KV_RMV",          {KvQueryType::Remove,       fcjson::value_t::array}},
  {"KV_CLEAR",        {KvQueryType::Clear,        fcjson::value_t::object}},
  {"KV_COUNT",        {KvQueryType::Count,        fcjson::value_t::object}},
  {"KV_SERVER_INFO",  {KvQueryType::ServerInfo,   fcjson::value_t::object}},
  {"KV_APPEND",       {KvQueryType::Append,       fcjson::value_t::object}},
  {"KV_CONTAINS",     {KvQueryType::Contains,     fcjson::value_t::array}},
  {"KV_ARRAY_MOVE",   {KvQueryType::ArrayMove,    fcjson::value_t::object}},
  {"KV_FIND",         {KvQueryType::Find,         fcjson::value_t::object}},
  // session
  {"SH_NEW",          {KvQueryType::SessionNew,       fcjson::value_t::object}},
  {"SH_END",          {KvQueryType::SessionEnd,       fcjson::value_t::object}},
  {"SH_SET",          {KvQueryType::SessionSet,       fcjson::value_t::object}},
  {"SH_SETQ",         {KvQueryType::SessionSetQ,      fcjson::value_t::object}},
  {"SH_GET",          {KvQueryType::SessionGet,       fcjson::value_t::object}},
  {"SH_ADD",          {KvQueryType::SessionAdd,       fcjson::value_t::object}},
  {"SH_ADDQ",         {KvQueryType::SessionAddQ,      fcjson::value_t::object}},
  {"SH_RMV",          {KvQueryType::SessionRemove,    fcjson::value_t::object}},
  {"SH_CLEAR",        {KvQueryType::SessionClear,     fcjson::value_t::object}},
  {"SH_COUNT",        {KvQueryType::SessionCount,     fcjson::value_t::object}},
  {"SH_APPEND",       {KvQueryType::SessionAppend,    fcjson::value_t::object}},
  {"SH_CONTAINS",     {KvQueryType::SessionContains,  fcjson::value_t::object}},
  {"SH_ARRAY_MOVE",   {KvQueryType::SessionArrayMove, fcjson::value_t::object}},
  {"SH_FIND",         {KvQueryType::SessionFind,      fcjson::value_t::object}},
  {"SH_UPDATE",       {KvQueryType::SessionUpdate,    fcjson::value_t::object}}
};


const std::map<const KvQueryType, const std::string> QueryTypeToName = 
{
  {KvQueryType::Set,            "KV_SET"},
  {KvQueryType::SetQ,           "KV_SETQ"},
  {KvQueryType::Get,            "KV_GET"},
  {KvQueryType::Add,            "KV_ADD"},
  {KvQueryType::AddQ,           "KV_ADDQ"},
  {KvQueryType::Remove,         "KV_RMV"},
  {KvQueryType::Clear,          "KV_CLEAR"},
  {KvQueryType::Count,          "KV_COUNT"},
  {KvQueryType::ServerInfo,     "KV_SERVER_INFO"},
  {KvQueryType::Append,         "KV_APPEND"},
  {KvQueryType::Contains,       "KV_CONTAINS"},
  {KvQueryType::ArrayMove,      "KV_ARRAY_MOVE"},
  {KvQueryType::Find,           "KV_FIND"},
  // Session
  {KvQueryType::SessionNew,       "SH_NEW"},
  {KvQueryType::SessionEnd,       "SH_END"},
  {KvQueryType::SessionSet,       "SH_SET"},
  {KvQueryType::SessionSetQ,      "SH_SETQ"},
  {KvQueryType::SessionGet,       "SH_GET"},
  {KvQueryType::SessionAdd,       "SH_ADD"},
  {KvQueryType::SessionAddQ,      "SH_ADDQ"},
  {KvQueryType::SessionRemove,    "SH_RMV"},
  {KvQueryType::SessionClear,     "SH_CLEAR"},
  {KvQueryType::SessionCount,     "SH_COUNT"},
  {KvQueryType::SessionAppend,    "SH_APPEND"},
  {KvQueryType::SessionContains,  "SH_CONTAINS"},
  {KvQueryType::SessionArrayMove, "SH_ARRAY_MOVE"},
  {KvQueryType::SessionFind,      "SH_FIND"},
  {KvQueryType::SessionUpdate,    "SH_UPDATE"}
};


struct PoolRequestResponse
{ 
  using enum RequestStatus;

  static fcjson getFound (cachedpair pair)
  {
    fcjson rsp;    
    rsp["KV_GET_RSP"] = std::move(pair);
    rsp["KV_GET_RSP"]["st"] = Ok;
    return rsp;
  }

  static fcjson getNotFound (cachedpair key)
  {
    fcjson rsp;
    rsp["KV_GET_RSP"]["st"] = KeyNotExist;
    rsp["KV_GET_RSP"]["k"] = std::move(key);
    return rsp;
  }

  static fcjson keySet (const bool isSet, const std::string_view k)
  {
    fcjson rsp;
    rsp["KV_SET_RSP"]["st"] = isSet ? KeySet : KeyUpdated;
    rsp["KV_SET_RSP"]["k"] = k;
    return rsp;
  }
  
  static fcjson keyAdd (const bool isAdded, std::string&& k)
  {
    fcjson rsp;
    rsp["KV_ADD_RSP"]["st"] = isAdded ? KeySet : KeyExists;
    rsp["KV_ADD_RSP"]["k"] = std::move(k);
    return rsp;
  }

  static fcjson keyAddQ (std::string&& k)
  {
    fcjson rsp;
    rsp["KV_ADDQ_RSP"]["st"] = KeyExists;
    rsp["KV_ADDQ_RSP"]["k"] = std::move(k);
    return rsp;
  }

  static fcjson keyRemoved (const bool removed, const std::string_view k)
  {
    fcjson rsp;
    rsp["KV_RMV_RSP"]["st"] = removed ? KeyRemoved : KeyNotExist;
    rsp["KV_RMV_RSP"]["k"] = k;
    return rsp;
  }

  static fcjson append (const RequestStatus status, const std::string_view k)
  {
    fcjson rsp;
    rsp["KV_APPEND_RSP"]["st"] = status;
    rsp["KV_APPEND_RSP"]["k"] = k;
    return rsp;
  }

  static fcjson contains (const RequestStatus status, const std::string_view k)
  {
    fcjson rsp;
    rsp["KV_CONTAINS_RSP"]["st"] = status;
    rsp["KV_CONTAINS_RSP"]["k"] = k;
    return rsp;
  }

  static fcjson arrayMove (const RequestStatus status, const std::string_view k)
  {
    fcjson rsp;
    rsp["KV_ARRAY_MOVE_RSP"]["st"] = status;
    rsp["KV_ARRAY_MOVE_RSP"]["k"] = k;
    return rsp;
  }


  // SESSION
  static fcjson sessionNew (const RequestStatus status, const SessionToken& token, const SessionName name)
  {
    fcjson rsp;
    rsp["SH_NEW_RSP"]["st"] = status;
    rsp["SH_NEW_RSP"]["name"] = name;
    rsp["SH_NEW_RSP"]["tkn"] = token;    
    return rsp;
  }
  
  static fcjson sessionEnd (const RequestStatus status, const SessionToken& token)
  {
    fcjson rsp;
    rsp["SH_END_RSP"]["st"] = status;
    rsp["SH_END_RSP"]["tkn"] = token;
    return rsp;
  }
    
  static fcjson sessionRemove (const SessionToken& tkn, const bool removed, const std::string&& k)
  {
    fcjson rsp;
    rsp["SH_RMV_RSP"]["st"] = removed ? KeyRemoved : KeyNotExist;
    rsp["SH_RMV_RSP"]["k"] = k;
    rsp["SH_RMV_RSP"]["tkn"] = tkn;
    return rsp;
  }

  static fcjson sessionClear (const SessionToken& tkn, const bool cleared, const std::size_t count)
  {
    fcjson rsp;
    rsp["SH_CLEAR_RSP"]["st"] = cleared ? Ok : Unknown;
    rsp["SH_CLEAR_RSP"]["cnt"] = count;
    rsp["SH_CLEAR_RSP"]["tkn"] = tkn;
    return rsp;
  }

  static fcjson sessionCount (const SessionToken& tkn, const std::size_t count)
  {
    fcjson rsp;
    rsp["SH_COUNT_RSP"]["st"] = Ok;
    rsp["SH_COUNT_RSP"]["cnt"] = count;
    rsp["SH_COUNT_RSP"]["tkn"] = tkn;
    return rsp;
  }

  static fcjson sessionAppend (const SessionToken& tkn, const RequestStatus status, const std::string_view k)
  {
    fcjson rsp;
    rsp["SH_APPEND_RSP"]["st"] = status;
    rsp["SH_APPEND_RSP"]["k"] = k;
    rsp["SH_APPEND_RSP"]["tkn"] = tkn;
    return rsp;
  }


  static PoolRequestResponse unknownError ()
  {
    return PoolRequestResponse{.status = RequestStatus::Unknown};
  }

  RequestStatus status;
  fcjson contents;
  std::size_t affectedCount{0};
};


struct KvCommand
{
  uWS::WebSocket<false, true, WsSession> * ws;  // to access the websocket and userdata
  uWS::Loop * loop; // TODO can this be moved to WsSession, only set once in .open handler? the uWS event loop, so we can defer() websocket calls on an event loop thread
  fcjson contents;  // json taken from the request, contents depends on the query
  KvQueryType type; 
  std::function<void(std::any)> cordinatedResponseHandler; 
  KvFind find;
  SessionToken shtk;
};


struct ServerStats
{
  std::atomic_size_t queryCount{0U};

} * serverStats;


static const std::array<std::function<bool(const std::string_view&, PoolId&)>, 2U> PoolIndexers =
{
  [](const std::string_view& k, PoolId& id) -> bool
  {
    if (isKeyValid(k))
    {
      id = ((k[0U] + k[1U] + k[2U] + k[3U] + k[4U] + k[5U]) & 0xFFFFFFFF) % MaxPools;
      return true;
    }

    return false;
  },

  [](const std::string_view& k, PoolId& id) -> bool
  {
    if (isKeyValid(k))
    {
      id = 0U;
      return true;
    }

    return false;
  }
};


static const std::array<std::function<void(const SessionToken&, SessionPoolId&)>, 2U> SessionIndexers =
{
  [](const SessionToken& t, SessionPoolId& id) 
  {
    id = ((t[0U] + t[1U] + t[2U] + t[3U] + t[4U] + t[5U]) & 0xFFFFFFFF) % MaxPools;
  },

  [](const SessionToken& t, SessionPoolId& id)
  {
    id = 0U;
  }
};


SessionToken createSessionToken(const SessionName& name)
{
  return std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()); // TODO TODO
}


fc_always_inline bool valueTypeValid (const fcjson& value)
{
  static const std::set<fcjson::value_t> DisallowedTypes = 
  {
    fcjson::value_t::binary,
    fcjson::value_t::discarded  // technically not possible, but for sanity
  };

  if (value.is_array())
  {
    static const std::set<fcjson::value_t> DisallowedArrayTypes = 
    {
      fcjson::value_t::binary,
      fcjson::value_t::discarded
    };

    for(auto& item : value)
    {
      if (DisallowedArrayTypes.contains(item.type()))
        return false;
    }

    return true;
  }
  else
    return !DisallowedTypes.contains(value.type());
}



}
}
}

#endif
