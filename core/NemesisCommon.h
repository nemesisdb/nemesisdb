#ifndef NDB_CORE_FUSIONCOMMON_H
#define NDB_CORE_FUSIONCOMMON_H

#include <string_view>
#include <mutex>
#include <ostream>
#include <thread>
#include <chrono>
#include <uwebsockets/App.h>
#include <nlohmann/json.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>


namespace nemesis { namespace core {

#define fc_always_inline inline __attribute__((always_inline))

static const char * NEMESIS_VERSION = "0.3.1";
static const std::size_t NEMESIS_CONFIG_VERSION = 1U;
static const std::size_t NEMESIS_MAX_CORES = 4U;

static const std::size_t NEMESIS_KV_MINPAYLOAD = 64U;
static const std::size_t NEMESIS_KV_MAXPAYLOAD = 2U * 1024U * 1024U;


// general
using njson = nlohmann::ordered_json;
using njson2 = jsoncons::ojson;
using jcjson = jsoncons::json;
using NemesisClock = std::chrono::steady_clock;
using NemesisTimePoint = NemesisClock::time_point;

// kv
using cachedkey = std::string;
using cachedvalue = nlohmann::ordered_json;
using cachedpair = nlohmann::ordered_json;
using cachedvalue2 = njson2;
using cachedpair2 = njson2;

// session
using SessionPoolId = std::size_t;
using SessionToken = std::string;
using SessionName = std::string;
using SessionClock = std::chrono::steady_clock;
using SessionExpireTime = SessionClock::time_point;
using SessionDuration = std::chrono::seconds;
using SessionExpireTimePoint = std::chrono::time_point<SessionClock, std::chrono::seconds>;


struct WsSession
{
  WsSession () : connected(new std::atomic_bool{true})
  {
  }

  // need this because uWebSockets moves the userdata after upgrade to websocket
  WsSession (WsSession&& other) : connected(other.connected)
  {
    other.connected = nullptr;
  }
  
  ~WsSession()
  {
    if (connected)
      delete connected;

    connected = nullptr;
  }

  std::atomic_bool * connected;
};


using KvWebSocket = uWS::WebSocket<false, true, WsSession>;



struct FindConditions
{
  enum class Condition { Equals, GT, GTE, LT, LTE  };

  using ConditionOperator = std::function<bool(const njson&, const njson&)>;


  const std::map<Condition, std::tuple<const std::string, ConditionOperator>> ConditionToOp = 
  {
    {Condition::Equals,   {"==",  [](const njson& a, const njson& b){ return a == b; }} },
    {Condition::GT,       {">",   [](const njson& a, const njson& b){ return a > b; }} },
    {Condition::GTE,      {">=",  [](const njson& a, const njson& b){ return a >= b; }} },
    {Condition::LT,       {"<",   [](const njson& a, const njson& b){ return a < b; }} },
    {Condition::LTE,      {"<=",  [](const njson& a, const njson& b){ return a <= b; }} }
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


  Condition getOperator(const std::string& opString)
  {
    return OpStringToOp.at(opString);
  }


  const std::tuple<const std::string, ConditionOperator>& getOperation (const Condition cond)
  {
    return ConditionToOp.at(cond);
  }

} findConditions ;


struct KvFind
{
  FindConditions::Condition condition;

};


enum class RequestStatus
{
  Ok = 1,
  OpCodeInvalid,
  JsonInvalid,
  PathInvalid,
  PathNotExist,
  CommandNotExist = 10,
  CommandMultiple,
  CommandType,
  CommandSyntax,
  KeySet = 20,
  KeyUpdated,
  KeyNotExist,
  KeyExists,
  KeyRemoved,
  Key_Reserved1,
  ParamMissing,
  KeyTypeInvalid,
  ValueMissing = 40,
  ValueTypeInvalid,
  ValueSize,
  OutOfBounds,
  FindNoPath = 60,
  FindNoOperator,
  FindInvalidOperator,
  FindRegExInvalid,
  SessionNotExist = 100,
  SessionTokenInvalid,
  SessionOpenFail,
  SessionNewFail,
  Unknown = 1000
};

static inline bool setThreadAffinity(const std::thread::native_handle_type handle, const size_t core)
{
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core, &cpuset);
  return pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset) == 0;
}


// Response when command known but response
static njson createErrorResponse (const std::string_view commandRsp, const RequestStatus status, const SessionToken& tkn, const std::string_view msg)
{
  njson rsp;
  rsp[commandRsp]["st"] = status;
  
  if (tkn.empty())
    rsp[commandRsp]["tkn"] = njson{};
  else
    rsp[commandRsp]["tkn"] = tkn;

  rsp[commandRsp]["m"] = msg;
  return rsp;
}

static njson2 createErrorResponse (const std::string_view commandRsp, const RequestStatus status, const std::string_view msg = "")
{
  njson2 rsp;
  rsp[commandRsp]["st"] = static_cast<int>(status);
  rsp[commandRsp]["tkn"] = njson2::null();
  rsp[commandRsp]["m"] = msg;
  return rsp;
}


// Response is the original command is unknown.
static njson2 createErrorResponse (const RequestStatus status, const std::string_view msg = "")
{
  njson2 rsp;
  rsp["ERR"]["st"] = static_cast<int>(status);
  rsp["ERR"]["m"] = msg;
  return rsp;
}


} // namespace core
} // namespace fusion


#endif
