#ifndef NDB_CORE_FUSIONCOMMON_H
#define NDB_CORE_FUSIONCOMMON_H

#include <string_view>
#include <mutex>
#include <ostream>
#include <thread>
#include <chrono>
#include <uwebsockets/App.h>
#include <uuid_v4/uuid_v4.h>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>


namespace nemesis { namespace core {

#define fc_always_inline inline __attribute__((always_inline))


namespace fs = std::filesystem;

static const char * NEMESIS_VERSION = "0.3.6";
static const std::size_t NEMESIS_CONFIG_VERSION = 1U;
static const std::size_t NEMESIS_MAX_CORES = 4U;

static const std::size_t NEMESIS_KV_MINPAYLOAD = 64U;
static const std::size_t NEMESIS_KV_MAXPAYLOAD = 2U * 1024U * 1024U;


// general
using njson = jsoncons::ojson;
using uuid = std::string;
using NemesisClock = std::chrono::steady_clock;
using NemesisTimePoint = NemesisClock::time_point;

// kv
using cachedkey = std::string;
using cachedvalue2 = njson;
using cachedpair2 = njson;
using KvSaveClock = std::chrono::system_clock;
using KvSaveMetaDataUnit = std::chrono::milliseconds;

// session
using PoolId = std::size_t;
using SessionToken = std::uint64_t;
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


static inline bool setThreadAffinity(const std::thread::native_handle_type handle, const size_t core)
{
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core, &cpuset);
  return pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset) == 0;
}


static njson createErrorResponse (const std::string_view commandRsp, const RequestStatus status, const std::string_view msg = "")
{
  njson rsp;
  rsp[commandRsp]["st"] = static_cast<int>(status);
  rsp[commandRsp]["tkn"] = njson::null();
  rsp[commandRsp]["m"] = msg;
  return rsp;
}


static njson createErrorResponse (const std::string_view commandRsp, const RequestStatus status, const SessionToken tkn)
{
  njson rsp;
  rsp[commandRsp]["st"] = static_cast<int>(status);
  rsp[commandRsp]["tkn"] = tkn;
  rsp[commandRsp]["m"] = "";
  return rsp;
}


static njson createErrorResponseNoTkn (const std::string_view commandRsp, const RequestStatus status, const std::string_view msg = "")
{
  njson rsp;
  rsp[commandRsp]["st"] = static_cast<int>(status);
  rsp[commandRsp]["m"] = msg;
  return rsp;
}


// Response is the original command is unknown (i.e. JSON parse error).
static njson createErrorResponse (const RequestStatus status, const std::string_view msg = "")
{
  njson rsp;
  rsp["ERR"]["st"] = static_cast<int>(status);
  rsp["ERR"]["m"] = msg;
  return rsp;
}


template <typename E>
constexpr typename std::underlying_type<E>::type toUnderlying(const E e) noexcept
{
  return static_cast<typename std::underlying_type_t<E>>(e);
}


std::size_t countFiles (const std::filesystem::path& path)
{
  return (std::size_t)std::distance(std::filesystem::directory_iterator{path}, std::filesystem::directory_iterator{});
};


} // namespace core
} // namespace nemesis


#endif
