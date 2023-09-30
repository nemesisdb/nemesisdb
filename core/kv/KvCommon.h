#ifndef FC_CORE_KVCOMMON_H
#define FC_CORE_KVCOMMON_H

#include <map>
#include <array>
#include <any>
#include <nlohmann/json.hpp>
#include <uwebsockets/App.h>
#include <core/FusionCommon.h>


namespace fusion { namespace core { namespace kv {

using kvjson = nlohmann::ordered_json;
using kvhash_t = std::uint32_t;
using PoolId = std::size_t;
using cachedkey = std::string;
using cachedvalue = nlohmann::ordered_json;
using cachedpair = nlohmann::ordered_json;

const uWS::OpCode WsSendOpCode = uWS::OpCode::TEXT;

static kvhash_t MaxPools = 1U;
static const std::size_t MinKeySize = 6U; // characters, not bytes. See createPoolIndex() if changing this


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
  //RenameKey,
  Max,
  Unknown,
};


const std::map<const std::string_view, std::tuple<const KvQueryType, const kvjson::value_t>> QueryNameToType = 
{
  {"KV_SET",          {KvQueryType::Set, kvjson::value_t::object}},
  {"KV_SETQ",         {KvQueryType::SetQ, kvjson::value_t::object}},
  {"KV_GET",          {KvQueryType::Get, kvjson::value_t::array}},
  {"KV_ADD",          {KvQueryType::Add, kvjson::value_t::object}},
  {"KV_ADDQ",         {KvQueryType::AddQ, kvjson::value_t::object}},
  {"KV_RMV",          {KvQueryType::Remove, kvjson::value_t::array}},
  {"KV_CLEAR",        {KvQueryType::Clear, kvjson::value_t::object}},
  {"KV_COUNT",        {KvQueryType::Count, kvjson::value_t::object}},
  {"KV_SERVER_INFO",  {KvQueryType::ServerInfo, kvjson::value_t::object}},
  {"KV_APPEND",       {KvQueryType::Append, kvjson::value_t::object}}
  //{"KV_RNM",       KvQueryType::RenameKey}
};


enum class KvRequestStatus
{
  Ok = 1,
  OpCodeInvalid,
  JsonInvalid,
  CommandNotExist = 10,
  CommandMultiple,
  CommandType, 
  KeySet = 20,
  KeyUpdated,
  KeyNotExist,
  KeyExists,
  KeyRemoved,
  KeyLengthInvalid,
  KeyMissing,
  KeyTypeInvalid,
  ValueMissing = 40,
  ValueTypeInvalid,
  Unknown = 100
};


enum class KvErrorCode
{
  None = 0,
  KeyInvalidLength,
  TypeInvalid,
  QueryUnknown,
  ExcessiveValues,
  JsonParseError,
  Unknown  
};


struct PoolRequestResponse
{ 
  using enum KvRequestStatus;

  static kvjson getFound (cachedpair pair)
  {
    kvjson rsp;    
    rsp["KV_GET_RSP"] = std::move(pair);
    rsp["KV_GET_RSP"]["st"] = Ok;
    return rsp;
  }

  static kvjson getNotFound (cachedpair key)
  {
    kvjson rsp;
    rsp["KV_GET_RSP"]["st"] = KeyNotExist;
    rsp["KV_GET_RSP"]["k"] = std::move(key);
    return rsp;
  }

  static kvjson keySet (const bool isSet, const std::string_view k)
  {
    kvjson rsp;
    rsp["KV_SET_RSP"]["st"] = isSet ? KeySet : KeyUpdated;
    rsp["KV_SET_RSP"]["k"] = k;
    return rsp;
  }
  
  static kvjson keyAdd (const bool isAdded, std::string&& k)
  {
    kvjson rsp;
    rsp["KV_ADDQ_RSP"]["st"] = isAdded ? KeySet : KeyExists;
    rsp["KV_ADDQ_RSP"]["k"] = std::move(k);
    return rsp;
  }

  static kvjson keyRemoved (const bool removed, const std::string_view k)
  {
    kvjson rsp;
    rsp["KV_RMV_RSP"]["st"] = removed ? KeyRemoved : KeyNotExist;
    rsp["KV_RMV_RSP"]["k"] = k;
    return rsp;
  }

  static kvjson append (const KvRequestStatus status, const std::string_view k)
  {
    kvjson rsp;
    rsp["KV_APPEND_RSP"]["st"] = status;
    rsp["KV_APPEND_RSP"]["k"] = k;
    return rsp;
  }

  // static kvjson renameKey (kvjson pair)
  // {
  //   kvjson rsp;
  //   rsp["KV_RNM_RSP"] = std::move(pair);
  //   rsp["KV_RNM_RSP"]["st"] = KeySet;    
  //   return rsp;
  // }

  // static kvjson renameKeyFail (const KvRequestStatus status, kvjson pair)
  // {
  //   kvjson rsp;
  //   rsp["KV_RNM_RSP"] = std::move(pair);
  //   rsp["KV_RNM_RSP"]["st"] = status;    
  //   return rsp;
  // }

  static PoolRequestResponse unknownError ()
  {
    return PoolRequestResponse{.status = KvRequestStatus::Unknown};
  }

  KvRequestStatus status;
  kvjson contents;
  std::size_t affectedCount{0};
};



struct KvSession
{
  KvSession () : connected(new std::atomic_bool{true})
  {
  }

  // need this because uWebSockets moves the userdata after upgrade to websocket
  KvSession (KvSession&& other) : connected(other.connected)
  {
    other.connected = nullptr;
  }
  
  ~KvSession()
  {
    if (connected)
      delete connected;

    connected = nullptr;
  }

  std::atomic_bool * connected;
};



struct KvCommand
{
  uWS::WebSocket<false, true, KvSession> * ws;  // to access the websocket and userdata
  uWS::Loop * loop; // TODO can this be moved to KvSession, only set once in .open handler? the uWS event loop, so we can defer() websocket calls on an event loop thread
  kvjson contents;  // json taken from the request, contents depends on the query
  KvQueryType type; 
  std::function<void(std::any)> cordinatedResponseHandler; 
};


using KvWebSocket = uWS::WebSocket<false, true, KvSession>;


struct ServerStats
{
  std::atomic_size_t queryCount{0U};

} * serverStats;


static const std::array<std::function<bool(const std::string_view&, PoolId&)>, 2U> PoolIndexers =
{
  [](const std::string_view& k, PoolId& id) -> bool
  {
    if (k.size() < MinKeySize)
      return false;

    id = ((k[0U] + k[1U] + k[2U] + k[3U] + k[4U] + k[5U]) & 0xFFFFFFFF) % MaxPools;
    return true;
  },

  [](const std::string_view& k, PoolId& id) -> bool
  {
    if (k.size() < MinKeySize)
      return false;

    id = 0U;
    return true;
  }
};

// Response if the original command is known.
static kvjson createErrorResponse (const std::string_view commandRsp, const KvRequestStatus status, const std::string_view key = "")
{
  kvjson rsp;
  rsp[commandRsp]["st"] = status;
  rsp[commandRsp]["k"] = key;
  return rsp;
}

// Response is the original command not unknown.
static kvjson createErrorResponse (const KvRequestStatus status, const std::string_view msg = "")
{
  kvjson rsp;
  rsp["KV_ERR"]["st"] = status;
  rsp["KV_ERR"]["m"] = msg;
  return rsp;
}

// std::string kvStatusToString (const KvRequestStatus st)
// {
//   return std::to_string(static_cast<std::uint8_t>(st));
// }

fc_always_inline bool valueTypeValid (const kvjson& value)
{
  static const std::set<kvjson::value_t> DisallowedTypes = 
  {
    kvjson::value_t::binary,
    kvjson::value_t::discarded  // technically not possible, but for sanity
  };

  if (value.is_array())
  {
    static const std::set<kvjson::value_t> DisallowedArrayTypes = 
    {
      kvjson::value_t::binary,
      kvjson::value_t::discarded
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
