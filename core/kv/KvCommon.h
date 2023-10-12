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
using cachedvalue = nlohmann::ordered_json;
using cachedpair = nlohmann::ordered_json;

const uWS::OpCode WsSendOpCode = uWS::OpCode::TEXT;

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
  Max,
  Unknown,
};


const std::map<const std::string_view, std::tuple<const KvQueryType, const fcjson::value_t>> QueryNameToType = 
{
  {"KV_SET",          {KvQueryType::Set,        fcjson::value_t::object}},
  {"KV_SETQ",         {KvQueryType::SetQ,       fcjson::value_t::object}},
  {"KV_GET",          {KvQueryType::Get,        fcjson::value_t::array}},
  {"KV_ADD",          {KvQueryType::Add,        fcjson::value_t::object}},
  {"KV_ADDQ",         {KvQueryType::AddQ,       fcjson::value_t::object}},
  {"KV_RMV",          {KvQueryType::Remove,     fcjson::value_t::array}},
  {"KV_CLEAR",        {KvQueryType::Clear,      fcjson::value_t::object}},
  {"KV_COUNT",        {KvQueryType::Count,      fcjson::value_t::object}},
  {"KV_SERVER_INFO",  {KvQueryType::ServerInfo, fcjson::value_t::object}},
  {"KV_APPEND",       {KvQueryType::Append,     fcjson::value_t::object}},
  {"KV_CONTAINS",     {KvQueryType::Contains,   fcjson::value_t::array}},
  {"KV_ARRAY_MOVE",   {KvQueryType::ArrayMove,  fcjson::value_t::object}},
  {"KV_FIND",         {KvQueryType::Find,       fcjson::value_t::object}}
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

  static fcjson keyAddQ (const bool isAdded, std::string&& k)
  {
    fcjson rsp;
    rsp["KV_ADDQ_RSP"]["st"] = isAdded ? KeySet : KeyExists;
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

  

  // static fcjson renameKey (fcjson pair)
  // {
  //   fcjson rsp;
  //   rsp["KV_RNM_RSP"] = std::move(pair);
  //   rsp["KV_RNM_RSP"]["st"] = KeySet;    
  //   return rsp;
  // }

  // static fcjson renameKeyFail (const RequestStatus status, fcjson pair)
  // {
  //   fcjson rsp;
  //   rsp["KV_RNM_RSP"] = std::move(pair);
  //   rsp["KV_RNM_RSP"]["st"] = status;    
  //   return rsp;
  // }

  static PoolRequestResponse unknownError ()
  {
    return PoolRequestResponse{.status = RequestStatus::Unknown};
  }

  RequestStatus status;
  fcjson contents;
  std::size_t affectedCount{0};
};


struct FindConditions
{
  enum class Condition { Equals, GT, GTE, LT, LTE  };

  using ConditionOperator = std::function<bool(const fcjson&, const fcjson&)>;


  const std::map<Condition, std::tuple<const std::string, ConditionOperator>> ConditionToOp = 
  {
    {Condition::Equals,   {"==",   [](const fcjson& a, const fcjson& b){ return a == b; }} },
    {Condition::GT,       {">",   [](const fcjson& a, const fcjson& b){ return a > b; }} },
    {Condition::GTE,      {">=",  [](const fcjson& a, const fcjson& b){ return a >= b; }} },
    {Condition::LT,       {"<",   [](const fcjson& a, const fcjson& b){ return a < b; }} },
    {Condition::LTE,      {"<=",  [](const fcjson& a, const fcjson& b){ return a <= b; }} }
  };

  
  const std::map<const std::string, Condition> OpStringToOp = 
  {
    {"==",  Condition::Equals},
    {">",   Condition::GT},
    {">=",  Condition::GTE},
    {"<",   Condition::LT},
    {"<=",  Condition::LTE}
  };


  bool isValidOperator(const std::string& opString)
  {
    return OpStringToOp.contains(opString);
  }


  const std::tuple<const std::string, ConditionOperator>& getOperation (const Condition cond)
  {
    return ConditionToOp.at(cond);
  }

} findConditions ;



struct KvCommand
{
  uWS::WebSocket<false, true, WsSession> * ws;  // to access the websocket and userdata
  uWS::Loop * loop; // TODO can this be moved to WsSession, only set once in .open handler? the uWS event loop, so we can defer() websocket calls on an event loop thread
  fcjson contents;  // json taken from the request, contents depends on the query
  KvQueryType type; 
  std::function<void(std::any)> cordinatedResponseHandler; 

  struct Find
  {
    FindConditions::Condition condition;

  } find;
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

// std::string kvStatusToString (const RequestStatus st)
// {
//   return std::to_string(static_cast<std::uint8_t>(st));
// }

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
