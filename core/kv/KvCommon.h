#ifndef FC_CORE_KVCOMMON_H
#define FC_CORE_KVCOMMON_H

#include <map>
#include <array>
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
  RenameKey,
  Max,
  Unknown,
};


const std::map<std::string_view, KvQueryType> QueryNameToType = 
{
  {"SET",           KvQueryType::Set},
  {"SETQ",          KvQueryType::SetQ},
  {"GET",           KvQueryType::Get},
  {"ADD",           KvQueryType::Add},
  {"ADDQ",          KvQueryType::AddQ},
  {"RMV",           KvQueryType::Remove},
  {"CLEAR",         KvQueryType::Clear},
  {"SERVER_INFO",   KvQueryType::ServerInfo},
  {"RNM_KEY",       KvQueryType::RenameKey}
};


enum class KvRequestStatus
{
  Ok = 1,
  CommandUnknown = 10,
  CommandNotExist,
  CommandInvalid, // root invalid
  KeySet = 20,
  KeyUpdated,
  KeyNotExist,
  KeyExists,
  KeyRemoved,
  KeyLengthInvalid,
  KeyMissing,  
  ValueMissing,
  ValueTypeInvalid,
  JsonInvalid,
  TypeInvalid,
  ExcessKeys,
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

// TODO this won't work, statis members are static for all pools. See about making the response object static per thread, thread_local may be useful or static PoolRequestResponse in the KvPool class
struct PoolRequestResponse
{ 
  using enum KvRequestStatus;

  static kvjson getFound (cachedpair pair/*, const KvMetrics m*/)
  {
    kvjson rsp;    
    rsp["GET_RSP"] = std::move(pair);
    rsp["GET_RSP"]["st"] = Ok;
    return rsp;
  }

  static kvjson getNotFound (cachedpair key/*, const KvMetrics m*/)
  {
    kvjson rsp;
    rsp["GET_RSP"]["st"] = KeyNotExist;
    rsp["GET_RSP"]["k"] = std::move(key);
    return rsp;
  }

  static kvjson keySet (const bool isSet, const std::string_view k/*, const KvMetrics m*/)
  {
    kvjson rsp;
    rsp["SET_RSP"]["st"] = isSet ? KeySet : KeyUpdated;
    rsp["SET_RSP"]["k"] = std::move(k);
    return rsp;
  }

  static kvjson keySet (const bool isSet, std::string&& k/*, const KvMetrics m*/)
  {
    kvjson rsp;
    rsp["SET_RSP"]["st"] = isSet ? KeySet : KeyUpdated;
    rsp["SET_RSP"]["k"] = std::move(k);
    return rsp;
  }
  
  static kvjson keyAdd (const bool isAdded, const std::string_view k/*, const KvMetrics m*/)
  {
    kvjson rsp;
    rsp["ADD_RSP"]["st"] = isAdded ? KeySet : KeyExists;
    rsp["ADD_RSP"]["k"] = std::move(k);
    return rsp;
  }

  static PoolRequestResponse keyRemoved (const bool removed, const std::string_view k/*, const KvMetrics m*/)
  {
    return PoolRequestResponse{.status = removed ? KeyRemoved : KeyNotExist, .contents = kvjson{{"k", k}}/*, .metrics = m*/};
  }

  static PoolRequestResponse clear (const std::size_t nCleared/*, const KvMetrics m*/)
  {
    return PoolRequestResponse{.status = Ok/*, .metrics = m*/, .affectedCount = nCleared};
  }

  static PoolRequestResponse count (const std::size_t count/*, const KvMetrics m*/)
  {
    return PoolRequestResponse{.status = Ok/*, .metrics = m*/, .affectedCount = count};
  }

  static PoolRequestResponse renameKey (kvjson pair/*, const KvMetrics m*/)
  {
    return PoolRequestResponse{.status = KeySet, .contents = std::move(pair)/*, .metrics = m*/};
  }

  static PoolRequestResponse renameKeyFail (const KvRequestStatus status, kvjson pair/*, const KvMetrics m*/)
  {
    return PoolRequestResponse{.status = status, .contents = std::move(pair)/*, .metrics = m*/};
  }

  static PoolRequestResponse unknownError ()
  {
    return PoolRequestResponse{.status = KvRequestStatus::Unknown};
  }

  KvRequestStatus status;
  kvjson contents;
  //KvMetrics metrics;
  std::size_t affectedCount{0};
};


struct KvRequest
{
  uWS::WebSocket<false, true, KvRequest> * ws;
  kvjson json;
};

struct KvCommand
{
  uWS::WebSocket<false, true, KvRequest> * ws;  // to access the websocket and userdata
  uWS::Loop * loop; // the uWS event loop, so we can defer() websocket calls on an event loop thread
  kvjson contents;  // json taken from the request, contents depends on the query
  KvQueryType type; 
};

using KvWebSocket = uWS::WebSocket<false, true, KvRequest>;


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

// Response is the original command not unknown
static kvjson createErrorResponse (const KvRequestStatus status, const std::string_view msg = "")
{
  kvjson rsp;
  rsp["ERR"]["st"] = status;
  rsp["ERR"]["m"] = msg;
  return rsp;
}


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
