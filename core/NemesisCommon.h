#ifndef NDB_CORE_FUSIONCOMMON_H
#define NDB_CORE_FUSIONCOMMON_H

#ifdef NDB_NOLOG
  #define PLOG_DISABLE_LOGGING
#endif

#include <string_view>
#include <mutex>
#include <ostream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <uwebsockets/App.h>
#include <uuid_v4/uuid_v4.h>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>
#include <plog/Init.h>
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>

#define ndb_always_inline inline __attribute__((always_inline))

#ifndef NDEBUG
  #define NDB_DEBUG
#else
  #define NDB_RELEASE
#endif

namespace nemesis { namespace core {

namespace fs = std::filesystem;
namespace chrono = std::chrono;
namespace jsonpath = jsoncons::jsonpath;

static const char * NEMESIS_VERSION = "0.4";
static const std::size_t NEMESIS_CONFIG_VERSION = 2U;
static const std::size_t NEMESIS_MAX_CORES = 4U;

static const std::size_t NEMESIS_KV_MINPAYLOAD = 64U;
static const std::size_t NEMESIS_KV_MAXPAYLOAD = 2U * 1024U * 1024U;

static const std::size_t NEMESIS_TS_MINPAYLOAD = 64U;
static const std::size_t NEMESIS_TS_MAXPAYLOAD = 2U * 1024U * 1024U;


enum class ServerMode { None, KV, TS };


// general
const uWS::OpCode WsSendOpCode = uWS::OpCode::TEXT;
using njson = jsoncons::ojson;
using uuid = std::string;
using NemesisClock = chrono::steady_clock;
using NemesisTimePoint = NemesisClock::time_point;

using JsonType = jsoncons::json_type;

const JsonType JsonString = JsonType::string_value;
const JsonType JsonBool = JsonType::bool_value;
const JsonType JsonInt = JsonType::int64_value;
const JsonType JsonUInt = JsonType::uint64_value;
const JsonType JsonObject = JsonType::object_value;
const JsonType JsonArray = JsonType::array_value;


// kv
using cachedkey = std::string;
using cachedvalue2 = njson;
using cachedpair2 = njson;
using KvSaveClock = chrono::system_clock;
using KvSaveMetaDataUnit = chrono::milliseconds;

// session
using PoolId = std::size_t;
using SessionToken = std::uint64_t;
using SessionName = std::string;
using SessionClock = chrono::steady_clock;
using SessionExpireTime = SessionClock::time_point;
using SessionDuration = chrono::seconds;
using SessionExpireTimePoint = chrono::time_point<SessionClock, std::chrono::seconds>;



struct Param
{
private:
  Param () = delete;

  Param(const JsonType type, const bool required) : type(type), isRequired(required)
  {
  }


public:
  static std::pair<std::string_view, Param> required (const std::string_view name, const JsonType type)
  {
    return {name, Param {type, true}};
  }

  static std::pair<std::string_view, Param> optional (const std::string_view name, const JsonType type)
  {
    return {name, Param {type, false}};
  }

  bool isRequired;
  JsonType type;
};


// Checks the params (if present and correct type). If that passes and onPostValidate is set, onPostValidate() is called for custom checks.
template <typename StatusT, StatusT Ok, StatusT ParamMissing, StatusT ParamType>
std::tuple<StatusT, const std::string_view> isCmdValid (const njson& msg,
                                                        const std::map<const std::string_view, const Param>& params,
                                                        std::function<std::tuple<StatusT, const std::string_view>(const njson&)> onPostValidate = nullptr)
{
  for (const auto& [member, param] : params)
  {
    if (param.isRequired && !msg.contains(member))
      return {ParamMissing, "Missing parameter"};
    else if (msg.contains(member) && msg.at(member).type() != param.type) // not required but present OR required and present: check type
      return {ParamType, "Param type incorrect"};
  }

  return onPostValidate ? onPostValidate(msg) : std::make_tuple(Ok, "");
}


struct ServerStats
{
  std::atomic_size_t queryCount{0U};

};


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


enum class RequestStatus
{
  Ok = 1,
  OpCodeInvalid,
  JsonInvalid,
  PathInvalid,
  NoPath,
  CommandNotExist = 10,
  CommandMultiple,
  CommandType,
  CommandSyntax,
  CommandDisabled,
  KeySet = 20,
  KeyUpdated,
  KeyNotExist,
  KeyExists,
  KeyRemoved,
  Reserved2,
  ParamMissing,
  KeyTypeInvalid,
  ValueMissing = 40,
  ValueTypeInvalid,
  ValueSize,
  SessionNotExist = 100,
  SessionTokenInvalid,
  SessionOpenFail,
  SessionNewFail,  
  SaveStart = 120,
  SaveComplete,
  SaveDirWriteFail,
  SaveError,
  Loading = 140,
  LoadComplete,
  LoadError,
  LoadDuplicate,
  Unknown = 1000
};


enum class KvSaveStatus
{
  Pending = 0,
  Complete,
  Error
};


template<class Formatter>
static inline void initLogger (plog::ConsoleAppender<Formatter>& appender)
{
  static bool init = false;

  if (!init)
  {
    #ifdef NDB_DEBUG
      plog::init(plog::verbose, &appender);
    #else
      plog::init(plog::info, &appender);    
    #endif
  }
  
  init = true;
}


static inline bool setThreadAffinity(const std::thread::native_handle_type handle, const size_t core)
{
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core, &cpuset);
  return pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset) == 0;
}


static inline njson createErrorResponse (const std::string_view commandRsp, const RequestStatus status, const std::string_view msg = "")
{
  njson rsp;
  rsp[commandRsp]["st"] = static_cast<int>(status);
  rsp[commandRsp]["tkn"] = njson::null();
  rsp[commandRsp]["m"] = msg;
  return rsp;
}


static inline njson createErrorResponse (const std::string_view commandRsp, const RequestStatus status, const SessionToken tkn)
{
  njson rsp;
  rsp[commandRsp]["st"] = static_cast<int>(status);
  rsp[commandRsp]["tkn"] = tkn;
  rsp[commandRsp]["m"] = "";
  return rsp;
}


static inline njson createErrorResponseNoTkn (const std::string_view commandRsp, const RequestStatus status, const std::string_view msg = "")
{
  njson rsp;
  rsp[commandRsp]["st"] = static_cast<int>(status);
  rsp[commandRsp]["m"] = msg;
  return rsp;
}


// Response is the original command is unknown (i.e. JSON parse error).
static inline njson createErrorResponse (const RequestStatus status, const std::string_view msg = "")
{
  njson rsp;
  rsp["ERR"]["st"] = static_cast<int>(status);
  rsp["ERR"]["m"] = msg;
  return rsp;
}


template <typename E>
constexpr typename std::underlying_type<E>::type toUnderlying(const E e) noexcept
{
  // replace with std::to_underlying() in C++23
  return static_cast<typename std::underlying_type_t<E>>(e);
}


std::size_t countFiles (const fs::path& path)
{
  return (std::size_t)std::distance(fs::directory_iterator{path}, fs::directory_iterator{});
};


} // namespace core
} // namespace nemesis


#endif
