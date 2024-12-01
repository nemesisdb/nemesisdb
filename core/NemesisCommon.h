#ifndef NDB_CORE_NEMESISCOMMON_H
#define NDB_CORE_NEMESISCOMMON_H

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


namespace nemesis { 

namespace fs = std::filesystem;
namespace chrono = std::chrono;
namespace jsonpath = jsoncons::jsonpath;

static const char * NEMESIS_VERSION = "0.7.1";
static const std::size_t NEMESIS_CONFIG_VERSION = 5U;

static const std::size_t NEMESIS_KV_MINPAYLOAD = 64U;
static const std::size_t NEMESIS_KV_MAXPAYLOAD = 8U * 1024U;


enum class ServerMode
{
  None,
  KV,
  KvSessions
};


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
  KeyNotExist = 22,
  ParamMissing = 26,
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
  Unknown = 1000
};



// general
const uWS::OpCode WsSendOpCode = uWS::OpCode::TEXT;

using njson = jsoncons::json;
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
using cachedvalue = njson;
using KvSaveClock = chrono::system_clock;
using KvSaveMetaDataUnit = chrono::milliseconds;



template <typename E>
constexpr typename std::underlying_type<E>::type toUnderlying(const E e) noexcept
{
  // replace with std::to_underlying() in C++23
  return static_cast<typename std::underlying_type_t<E>>(e);
}


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

  JsonType type;
  bool isRequired;
};


// Checks the params (if present and correct type). If that passes and onPostValidate is set, onPostValidate() is called for custom checks.
template <class JSON, typename StatusT, StatusT Ok, StatusT ParamMissing, StatusT ParamType>
std::tuple<StatusT, const std::string_view> isCmdValid (const JSON& msg,
                                                        const std::map<const std::string_view, const Param>& params,
                                                        std::function<std::tuple<StatusT, const std::string_view>(const JSON&)> onPostValidate)
  //requires(std::is_enum_v<StatusT>)
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


template<typename T, std::size_t Size>
struct PmrResource
{
  PmrResource() : mbr(std::data(buffer), std::size(buffer)), alloc(&mbr)
  {

  }

  std::pmr::polymorphic_allocator<T>& getAlloc ()
  {
    return alloc;
  }

  private:
    std::array<std::byte, Size> buffer;
    std::pmr::monotonic_buffer_resource mbr;
    std::pmr::polymorphic_allocator<T> alloc;
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


struct Response
{
  njson rsp;
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
  rsp[commandRsp]["st"] = toUnderlying(status);
  rsp[commandRsp]["tkn"] = njson::null();
  rsp[commandRsp]["m"] = msg;
  return rsp;
}


static inline njson createErrorResponseNoTkn (const std::string_view commandRsp, const RequestStatus status, const std::string_view msg = "")
{
  njson rsp;
  rsp[commandRsp]["st"] = toUnderlying(status);
  rsp[commandRsp]["m"] = msg;
  return rsp;
}


// Response is the original command is unknown (i.e. JSON parse error).
static inline njson createErrorResponse (const RequestStatus status, const std::string_view msg = "")
{
  njson rsp;
  rsp["ERR"]["st"] = toUnderlying(status);
  rsp["ERR"]["m"] = msg;
  return rsp;
}


std::tuple<bool, njson> doIsValid (const std::string_view queryRspName, 
                const njson& cmd, const std::map<const std::string_view, const Param>& params,
                std::function<std::tuple<RequestStatus, const std::string_view>(const njson&)> onPostValidate = nullptr)
{
  const auto [stat, msg] = isCmdValid<njson,  RequestStatus,
                                              RequestStatus::Ok,
                                              RequestStatus::ParamMissing,
                                              RequestStatus::ValueTypeInvalid>(cmd, params, onPostValidate);
  
  if (stat != RequestStatus::Ok) [[unlikely]]
  {
    PLOGD << msg;
    return {false, createErrorResponse(queryRspName, stat, msg)};
  }
  else
    return {true, njson{}};
}


std::tuple<bool, njson> isValid ( const std::string_view queryRspName, 
                                  const njson& req,
                                  const std::map<const std::string_view, const Param>& params,
                                  std::function<std::tuple<RequestStatus, const std::string_view>(const njson&)> onPostValidate = nullptr)
{
  return doIsValid(queryRspName, req, params, onPostValidate);
}


} // namespace nemesis


#endif
