#ifndef NDB_CORE_KVCOMMON_H
#define NDB_CORE_KVCOMMON_H

#include <map>
#include <array>
#include <any>
#include <uuid_v4/uuid_v4.h>
#include <nlohmann/json.hpp>
#include <uwebsockets/App.h>
#include <core/NemesisCommon.h>


namespace nemesis { namespace core { namespace kv {


using kvhaKV_t = std::uint32_t;
using PoolId = std::size_t;

const uWS::OpCode WsSendOpCode = uWS::OpCode::TEXT;

static PoolId MaxPools = 1U;

enum class KvQueryType : std::uint8_t
{ 
  SessionNew,
  SessionEnd,
  SessionOpen,
  SessionInfo,
  SessionInfoAll,
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
  SessionFind,
  SessionUpdate,
  Max,
  InternalSessionMonitor,
  Unknown,
};


const std::map<const std::string_view, std::tuple<const KvQueryType>> QueryNameToType = 
{  
  // session
  {"SH_NEW",          {KvQueryType::SessionNew}},
  {"SH_END",          {KvQueryType::SessionEnd}},
  {"SH_OPEN",         {KvQueryType::SessionOpen}},
  {"SH_INFO",         {KvQueryType::SessionInfo}},
  {"SH_INFO_ALL",     {KvQueryType::SessionInfoAll}},
  // 
  {"KV_SET",          {KvQueryType::SessionSet}},
  {"KV_SETQ",         {KvQueryType::SessionSetQ}},
  {"KV_GET",          {KvQueryType::SessionGet}},
  {"KV_ADD",          {KvQueryType::SessionAdd}},
  {"KV_ADDQ",         {KvQueryType::SessionAddQ}},
  {"KV_RMV",          {KvQueryType::SessionRemove}},
  {"KV_CLEAR",        {KvQueryType::SessionClear}},
  {"KV_COUNT",        {KvQueryType::SessionCount}},
  {"KV_APPEND",       {KvQueryType::SessionAppend}},
  {"KV_CONTAINS",     {KvQueryType::SessionContains}},
  {"KV_FIND",         {KvQueryType::SessionFind}},
  {"KV_UPDATE",       {KvQueryType::SessionUpdate}}
};


const std::map<const KvQueryType, const std::string> QueryTypeToName = 
{
  // Session
  {KvQueryType::SessionNew,       "SH_NEW"},
  {KvQueryType::SessionEnd,       "SH_END"},
  {KvQueryType::SessionOpen,      "SH_OPEN"},
  {KvQueryType::SessionInfo,      "SH_INFO"},
  {KvQueryType::SessionInfoAll,   "SH_INFO_ALL"},
  //
  {KvQueryType::SessionSet,       "KV_SET"},
  {KvQueryType::SessionSetQ,      "KV_SETQ"},
  {KvQueryType::SessionGet,       "KV_GET"},
  {KvQueryType::SessionAdd,       "KV_ADD"},
  {KvQueryType::SessionAddQ,      "KV_ADDQ"},
  {KvQueryType::SessionRemove,    "KV_RMV"},
  {KvQueryType::SessionClear,     "KV_CLEAR"},
  {KvQueryType::SessionCount,     "KV_COUNT"},
  {KvQueryType::SessionAppend,    "KV_APPEND"},
  {KvQueryType::SessionContains,  "KV_CONTAINS"},
  {KvQueryType::SessionFind,      "KV_FIND"},
  {KvQueryType::SessionUpdate,    "KV_UPDATE"}
};


struct PoolRequestResponse
{ 
  using enum RequestStatus;

  static int toInt (RequestStatus v)
  {
    return static_cast<int>(v);
  }


  // SESSION
  static njson2 sessionNew (const RequestStatus status, const SessionToken& token, const SessionName name)
  {
    njson2 rsp;
    rsp["SH_NEW_RSP"]["st"] = toInt(status);
    rsp["SH_NEW_RSP"]["name"] = name;
    rsp["SH_NEW_RSP"]["tkn"] = token;
    return rsp;
  }
  
  static njson2 sessionEnd (const RequestStatus status, const SessionToken& token)
  {
    njson2 rsp;
    rsp["SH_END_RSP"]["st"] = toInt(status);
    rsp["SH_END_RSP"]["tkn"] = token;
    return rsp;
  }

  static njson2 sessionInfo (const RequestStatus status, const SessionToken& token)
  {
    njson2 rsp;
    rsp["SH_INFO_RSP"]["st"] = toInt(status);
    rsp["SH_INFO_RSP"]["tkn"] = token;
    rsp["SH_INFO_RSP"]["shared"] = njson2::null();
    rsp["SH_INFO_RSP"]["keyCnt"] = njson2::null();
    
    return rsp;
  }

  static njson2 sessionInfo (const RequestStatus status, const SessionToken& token, const bool shared, const bool expires, const bool deleteOnExpire, const SessionDuration duration, const SessionDuration remaining, const std::size_t keyCount)
  {
    njson2 rsp = sessionInfo(status, token);
    rsp["SH_INFO_RSP"]["shared"] = shared;
    rsp["SH_INFO_RSP"]["keyCnt"] = keyCount;
    rsp["SH_INFO_RSP"]["expiry"]["expires"] = expires;
    rsp["SH_INFO_RSP"]["expiry"]["duration"] = duration.count();
    rsp["SH_INFO_RSP"]["expiry"]["remaining"] = remaining.count();
    rsp["SH_INFO_RSP"]["expiry"]["deleteSession"] = deleteOnExpire;
    return rsp;
  }
    
  static njson2 sessionRemove (const SessionToken& tkn, const bool removed, const std::string&& k)
  {
    njson2 rsp;
    rsp["KV_RMV_RSP"]["st"] = removed ? toInt(KeyRemoved) : toInt(KeyNotExist);
    rsp["KV_RMV_RSP"]["k"] = k;
    rsp["KV_RMV_RSP"]["tkn"] = tkn;
    return rsp;
  }

  static njson2 sessionClear (const SessionToken& tkn, const bool cleared, const std::size_t count)
  {
    njson2 rsp;
    rsp["KV_CLEAR_RSP"]["st"] = cleared ? toInt(Ok) : toInt(Unknown);
    rsp["KV_CLEAR_RSP"]["cnt"] = count;
    rsp["KV_CLEAR_RSP"]["tkn"] = tkn;
    return rsp;
  }

  static njson2 sessionCount (const SessionToken& tkn, const std::size_t count)
  {
    njson2 rsp;
    rsp["KV_COUNT_RSP"]["st"] = toInt(Ok);
    rsp["KV_COUNT_RSP"]["cnt"] = count;
    rsp["KV_COUNT_RSP"]["tkn"] = tkn;
    return rsp;
  }

  static njson2 sessionAppend (const SessionToken& tkn, const RequestStatus status, const std::string_view k)
  {
    njson2 rsp;
    rsp["KV_APPEND_RSP"]["st"] = toInt(status);
    rsp["KV_APPEND_RSP"]["k"] = k;
    rsp["KV_APPEND_RSP"]["tkn"] = tkn;
    return rsp;
  }
};


struct KvCommand
{
  uWS::WebSocket<false, true, WsSession> * ws;  // to access the websocket and userdata
  uWS::Loop * loop; // TODO can this be moved to WsSession, only set once in .open handler? the uWS event loop, so we can defer() websocket calls on an event loop thread
  //njson contents;  // json taken from the request, contents depends on the query
  njson2 contents;
  KvQueryType type; 
  std::function<void(std::any)> syncResponseHandler; 
  KvFind find;
  SessionToken shtk;
};


struct ServerStats
{
  std::atomic_size_t queryCount{0U};

} * serverStats;



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


fc_always_inline SessionToken createSessionToken(const SessionName& name, const bool shared)
{
  if (shared)
  {
    static const std::size_t seed = 99194853094755497U;
    const auto hash = std::hash<SessionName>{}(name);
    return std::to_string((std::size_t)(hash | seed));
  }
  else
  {
    static UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator; 
    const auto uuid = uuidGenerator.getUUID();
    return std::to_string(uuid.hash());
  }
}


fc_always_inline bool valueTypeValid (const njson& value)
{
  static const std::set<njson::value_t> DisallowedTypes = 
  {
    njson::value_t::binary,
    njson::value_t::discarded  // technically not possible, but for sanity
  };

  if (value.is_array())
  {
    static const std::set<njson::value_t> DisallowedArrayTypes = 
    {
      njson::value_t::binary,
      njson::value_t::discarded
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
