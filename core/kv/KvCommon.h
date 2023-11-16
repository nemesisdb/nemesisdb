#ifndef NDB_CORE_KVCOMMON_H
#define NDB_CORE_KVCOMMON_H

#include <map>
#include <array>
#include <any>
#include <uuid_v4/uuid_v4.h>
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
  KvSet,
  KvSetQ,
  KvGet,
  KvAdd,
  KvAddQ,
  KvRemove,
  KvClear,
  KvCount,
  KvContains,
  KvFind,
  KvUpdate,
  KvKeys,  
  KvSave,
  MAX,
  InternalSessionMonitor,
  InternalLoad,
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
  {"KV_SET",          {KvQueryType::KvSet}},
  {"KV_SETQ",         {KvQueryType::KvSetQ}},
  {"KV_GET",          {KvQueryType::KvGet}},
  {"KV_ADD",          {KvQueryType::KvAdd}},
  {"KV_ADDQ",         {KvQueryType::KvAddQ}},
  {"KV_RMV",          {KvQueryType::KvRemove}},
  {"KV_CLEAR",        {KvQueryType::KvClear}},
  {"KV_COUNT",        {KvQueryType::KvCount}},
  {"KV_CONTAINS",     {KvQueryType::KvContains}},
  {"KV_FIND",         {KvQueryType::KvFind}},
  {"KV_UPDATE",       {KvQueryType::KvUpdate}},
  {"KV_KEYS",         {KvQueryType::KvKeys}},
  {"KV_SAVE",         {KvQueryType::KvSave}}
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
  {KvQueryType::KvSet,        "KV_SET"},
  {KvQueryType::KvSetQ,       "KV_SETQ"},
  {KvQueryType::KvGet,        "KV_GET"},
  {KvQueryType::KvAdd,        "KV_ADD"},
  {KvQueryType::KvAddQ,       "KV_ADDQ"},
  {KvQueryType::KvRemove,     "KV_RMV"},
  {KvQueryType::KvClear,      "KV_CLEAR"},
  {KvQueryType::KvCount,      "KV_COUNT"},
  {KvQueryType::KvContains,   "KV_CONTAINS"},
  {KvQueryType::KvFind,       "KV_FIND"},
  {KvQueryType::KvUpdate,     "KV_UPDATE"},
  {KvQueryType::KvKeys,       "KV_KEYS"},
  {KvQueryType::KvSave,       "KV_SAVE"}
};


struct PoolRequestResponse
{ 
  using enum RequestStatus;

  static int toInt (RequestStatus v)
  {
    return static_cast<int>(v);
  }


  // SESSION
  static njson sessionNew (const RequestStatus status, const SessionToken& token, const SessionName name)
  {
    njson rsp;
    rsp["SH_NEW_RSP"]["st"] = toInt(status);
    rsp["SH_NEW_RSP"]["name"] = name;
    rsp["SH_NEW_RSP"]["tkn"] = token;
    return rsp;
  }
  
  static njson sessionEnd (const RequestStatus status, const SessionToken& token)
  {
    njson rsp;
    rsp["SH_END_RSP"]["st"] = toInt(status);
    rsp["SH_END_RSP"]["tkn"] = token;
    return rsp;
  }

  static njson sessionInfo (const RequestStatus status, const SessionToken& token)
  {
    njson rsp;
    rsp["SH_INFO_RSP"]["st"] = toInt(status);
    rsp["SH_INFO_RSP"]["tkn"] = token;
    rsp["SH_INFO_RSP"]["shared"] = njson::null();
    rsp["SH_INFO_RSP"]["keyCnt"] = njson::null();
    
    return rsp;
  }

  static njson sessionInfo (const RequestStatus status, const SessionToken& token, const bool shared, const bool expires, const bool deleteOnExpire, const SessionDuration duration, const SessionDuration remaining, const std::size_t keyCount)
  {
    njson rsp = sessionInfo(status, token);
    rsp["SH_INFO_RSP"]["shared"] = shared;
    rsp["SH_INFO_RSP"]["keyCnt"] = keyCount;
    rsp["SH_INFO_RSP"]["expiry"]["expires"] = expires;
    rsp["SH_INFO_RSP"]["expiry"]["duration"] = duration.count();
    rsp["SH_INFO_RSP"]["expiry"]["remaining"] = remaining.count();
    rsp["SH_INFO_RSP"]["expiry"]["deleteSession"] = deleteOnExpire;
    return rsp;
  }
    
  static njson sessionRemove (const SessionToken& tkn, const bool removed, const std::string&& k)
  {
    njson rsp;
    rsp["KV_RMV_RSP"]["st"] = removed ? toInt(KeyRemoved) : toInt(KeyNotExist);
    rsp["KV_RMV_RSP"]["k"] = k;
    rsp["KV_RMV_RSP"]["tkn"] = tkn;
    return rsp;
  }

  static njson sessionClear (const SessionToken& tkn, const bool cleared, const std::size_t count)
  {
    njson rsp;
    rsp["KV_CLEAR_RSP"]["st"] = cleared ? toInt(Ok) : toInt(Unknown);
    rsp["KV_CLEAR_RSP"]["cnt"] = count;
    rsp["KV_CLEAR_RSP"]["tkn"] = tkn;
    return rsp;
  }

  static njson sessionCount (const SessionToken& tkn, const std::size_t count)
  {
    njson rsp;
    rsp["KV_COUNT_RSP"]["st"] = toInt(Ok);
    rsp["KV_COUNT_RSP"]["cnt"] = count;
    rsp["KV_COUNT_RSP"]["tkn"] = tkn;
    return rsp;
  }

  static njson sessionKeys(const SessionToken& tkn, njson&& keys)
  {
    njson rsp;
    rsp["KV_KEYS_RSP"]["tkn"] = tkn;
    rsp["KV_KEYS_RSP"]["st"] = toInt(Ok);
    rsp["KV_KEYS_RSP"]["keys"] = std::move(keys);
    return rsp;
  }
};


struct KvCommand
{
  uWS::WebSocket<false, true, WsSession> * ws;  // to access the websocket and userdata
  uWS::Loop * loop; // TODO can this be moved to WsSession, only set once in .open handler? the uWS event loop, so we can defer() websocket calls on an event loop thread
  njson contents;
  KvQueryType type; 
  std::function<void(std::any)> syncResponseHandler; 
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


fc_always_inline uuid createUuid ()
{
  static UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator; 
  
  uuid s;
  uuidGenerator.getUUID().str(s);
  return s;
}


// TODO this isn't used, but really should be
// fc_always_inline bool valueTypeValid (const njson& value)
// {
//   static const std::set<jsoncons::json_type> DisallowedTypes = 
//   {
//     jsoncons::json_type::byte_string_value
//   };

//   if (value.is_array())
//   {
//     static const std::set<jsoncons::json_type> DisallowedTypes = 
//     {
//       jsoncons::json_type::byte_string_value
//     };

//     for(const auto& item : value.array_range())
//     {
//       if (item.type() == jsoncons::json_type::byte_string_value)
//         return false;
//     }

//     return true;
//   }
//   else
//     return value.type() != jsoncons::json_type::byte_string_value;
// }



}
}
}

#endif
