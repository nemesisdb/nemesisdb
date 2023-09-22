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
  Ok = 0,
  KeySet,
  KeyUpdated,
  KeyNotExist,
  KeyExists,
  KeyRemoved,
  KeyLengthInvalid,
  TypeInvalid,
  MissingKeyOrValue,
  ValueTypeInvalid,
  JsonParseError,  
  ExcessKeys,
  QueryRootInvalidType,
  Unknown
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

  static kvjson keySet (const bool isAdded, const std::string_view k/*, const KvMetrics m*/)
  {
    kvjson rsp;
    rsp["SET_RSP"]["st"] = isAdded ? KeySet : KeyUpdated;
    rsp["SET_RSP"]["k"] = std::move(k);
    return rsp;
  }

  static kvjson keySet (const bool isAdded, std::string&& k/*, const KvMetrics m*/)
  {
    kvjson rsp;
    rsp["SET_RSP"]["st"] = isAdded ? KeySet : KeyUpdated;
    rsp["SET_RSP"]["k"] = std::move(k);
    return rsp;
  }
  
  static PoolRequestResponse keyAdd (const bool isAdded, const std::string_view k/*, const KvMetrics m*/)
  {
    return PoolRequestResponse{.status = isAdded ? KeySet : KeyExists, .contents = kvjson{{"k", k}}/*, .metrics = m*/};
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
  uWS::WebSocket<false, true, KvRequest> * ws;
  uWS::Loop * loop;
  kvjson contents;
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
