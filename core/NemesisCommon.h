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


namespace nemesis { namespace core {

namespace fs = std::filesystem;
namespace chrono = std::chrono;
namespace jsonpath = jsoncons::jsonpath;

static const char * NEMESIS_VERSION = "0.6.6";
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


enum class KvSaveStatus
{
  Pending = 0,
  Complete,
  Error
};


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
using cachedvalue = njson;
using KvSaveClock = chrono::system_clock;
using KvSaveMetaDataUnit = chrono::milliseconds;

// session
using SessionToken = std::uint64_t;
using SessionName = std::string;
using SessionClock = chrono::steady_clock;
using SessionExpireTime = SessionClock::time_point;
using SessionDuration = chrono::seconds;
using SessionExpireTimePoint = chrono::time_point<SessionClock, std::chrono::seconds>;




inline const std::int16_t METADATA_VERSION = 2;


enum class SaveDataType
{
  RawKv,
  SessionKv
};


struct DataLoadPaths
{
  fs::path root;
  fs::path md;
  fs::path data;
  bool valid{false};
};


struct LoadResult
{  
  RequestStatus status;
  std::size_t nSessions{0};
  std::size_t nKeys{0};
  NemesisClock::duration duration{0};
};



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


ndb_always_inline void send (KvWebSocket * ws, const njson& msg)
{
  ws->send(msg.to_string(), WsSendOpCode);
}



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


template<typename Json>
bool doIsValid (const std::string_view queryRspName, KvWebSocket * ws, 
                const Json& cmd, const std::map<const std::string_view, const Param>& params,
                std::function<std::tuple<RequestStatus, const std::string_view>(const Json&)> onPostValidate = nullptr)
{
  const auto [stat, msg] = isCmdValid<Json, RequestStatus,
                                            RequestStatus::Ok,
                                            RequestStatus::ParamMissing,
                                            RequestStatus::ValueTypeInvalid>(cmd, params, onPostValidate);
  
  if (stat != RequestStatus::Ok)
  {
    PLOGD << msg;
    send(ws, createErrorResponse(queryRspName, stat, msg));
  }
    
  return stat == RequestStatus::Ok;
}


bool isValid (const std::string_view queryRspName, KvWebSocket * ws, 
              const njson& cmd, const std::map<const std::string_view, const Param>& params,
              typename std::function<std::tuple<RequestStatus, const std::string_view>(const njson&)> onPostValidate = nullptr)
{
  const auto& cmdRoot = cmd.object_range().cbegin()->value();
  return doIsValid(queryRspName, ws, cmdRoot, params, onPostValidate);
}


bool isValid (const std::string_view queryRspName, const std::string_view child, KvWebSocket * ws, 
              const njson& cmd, const std::map<const std::string_view, const Param>& params,
              std::function<std::tuple<RequestStatus, const std::string_view>(const njson&)> onPostValidate = nullptr)
{
  const auto& childObject = cmd.object_range().cbegin()->value()[child];
  return doIsValid(queryRspName, ws, childObject, params, onPostValidate);
}



DataLoadPaths getLoadPaths(const std::filesystem::path& loadRoot)
{
  auto countFiles = [](const fs::path& path) -> std::size_t
  {
    return (std::size_t)std::distance(fs::directory_iterator{path}, fs::directory_iterator{});
  };

  if (countFiles(loadRoot) == 0)
    return {.valid = false};
  else
  {
    std::size_t max = 0;

    // loadRoot may contain several saves (i.e. SH_SAVE/KV_SAVE used multiple times with the same 'name'),
    // they are named with the timestamp, so find the highest (most recent)
    for (const auto& dir : fs::directory_iterator(loadRoot))
    {
      if (dir.is_directory())
        max = std::max<std::size_t>(std::stoul(dir.path().filename()), max);
    }

    const fs::path root {loadRoot / std::to_string(max)};
    return DataLoadPaths {.root = root, .md = root / "md" / "md.json", .data = root / "data", .valid = true};
  }
}



njson createInitialSaveMetaData(const std::string_view name, const bool sessionsEnabled)
{
  njson metadata;
  metadata["name"] = name;
  metadata["version"] = METADATA_VERSION;
  metadata["status"] = toUnderlying(KvSaveStatus::Pending);        
  metadata["start"] = std::chrono::time_point_cast<KvSaveMetaDataUnit>(KvSaveClock::now()).time_since_epoch().count();
  metadata["complete"] = 0; // time completed
  metadata["saveDataType"] = sessionsEnabled ? toUnderlying(SaveDataType::SessionKv) : toUnderlying(SaveDataType::RawKv);
    
  return metadata;
}


void completeSaveMetaData(njson& metadata, const KvSaveStatus status)
{
  metadata["status"] = toUnderlying(status);
  metadata["complete"] = chrono::time_point_cast<KvSaveMetaDataUnit>(KvSaveClock::now()).time_since_epoch().count();
}


std::tuple<bool, const std::string_view> validatePreLoad (const std::string& loadName, const fs::path& persistPath, const bool sessionsEnabled)
{
  if (!fs::exists(persistPath / loadName))
    return {false, "Load name does not exist"};
  else if (const auto [root, mdFile, data, valid] = getLoadPaths(persistPath / loadName); !valid)
    return {false, "Failed to get load paths"};
  else if (!fs::exists(root))
    return {false, "Load root does not exist"};
  else if (!fs::exists(mdFile))
      return {false, "Metadata does not exist"};
  else if (!(fs::exists(data) && fs::is_directory(data)))
    return {false, "Data directory does not exist or is not a directory"}; 
  else
  {
    PLOGI << "Reading metadata in " << mdFile;

    std::ifstream mdStream {mdFile};
    const auto mdJson = njson::parse(mdStream);
    const auto saveType = static_cast<SaveDataType>(mdJson.at("saveDataType").as<unsigned int>()) ;

    if (!mdJson.contains("status") || !mdJson.at("status").is_uint64() || !mdJson.contains("saveDataType"))
      return {false, "Metadata file invalid"};
    else if (mdJson["status"] != toUnderlying(KvSaveStatus::Complete))
      return {false, "Cannot load: save was incomplete"};
    else if (sessionsEnabled && saveType != SaveDataType::SessionKv)
      return {false, "Cannot load: server has sessions enabled but data being loaded is not session data"};
    else if (!sessionsEnabled && saveType != SaveDataType::RawKv)
      return {false, "Cannot load: server has sessions disabled but data being loaded is session data"};
    else
      return {true, ""};
  }
}



} // namespace core
} // namespace nemesis


#endif
