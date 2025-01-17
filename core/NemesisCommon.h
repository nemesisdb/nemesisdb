#ifndef NDB_CORE_NEMESISCOMMON_H
#define NDB_CORE_NEMESISCOMMON_H

#ifdef NDB_NOLOG
  #define PLOG_DISABLE_LOGGING
#endif

// disable old logging defines
#define PLOG_OMIT_LOG_DEFINES

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
#include <fixed_string.hpp>


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

  static const char * NEMESIS_VERSION = "0.9";
  static const std::size_t NEMESIS_CONFIG_VERSION = 6U;

  static const std::size_t NEMESIS_MINPAYLOAD = 64U;
  // max: this is quite arbitrary but needed for config sanity checks
  static const std::size_t NEMESIS_MAXPAYLOAD = 32U * 1024U; 


  enum class RequestStatus
  {
    Ok                    = 1,
    OpCodeInvalid,
    JsonInvalid,
    PathInvalid,
    NoPath,
    CommandNotExist       = 10,
    CommandMultiple,
    CommandType,
    CommandSyntax,
    CommandDisabled,
    NotExist              = 22,
    ParamMissing          = 26,
    ValueMissing          = 40,
    ValueTypeInvalid,
    ValueSize,
    SaveComplete          = 120,
    SaveDirWriteFail,
    SaveError,
    Loading               = 140,
    LoadComplete,
    LoadError,
    Duplicate             = 160,
    Bounds                = 161,
    Unknown               = 1000
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


  using namespace fixstr;

  template<std::size_t N>
  using FixedString = fixstr::fixed_string<N>;

  
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


  struct Response
  {
    njson rsp;
  };


  struct Handler
  {
    using Handle = std::function<Response(njson&)>;

    Handler(Handle&& h) : handler(std::move(h))
    {

    }

    Handler(const Handler&) = default;
    Handler(Handler&&) = default;
    Handler& operator= (Handler&&) = default;

    Response operator()(njson& request) const
    {
      return handler(request);
    }

    Handle handler;
  };


  struct Param
  {
  private:
    Param () = delete;

    Param(const JsonType type, const bool required, const bool variableType = false) : type(type), isRequired(required), variableType(variableType)
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

    // if type is variable (i.e. in IARR, SARR, OARR, see isArrayCmdValid())
    static std::pair<std::string_view, Param> variable (const std::string_view name)
    {
      return {name, Param {JsonType::null_value, true, true}};
    }

    JsonType type;
    bool isRequired;
    bool variableType{false};
  };



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


  // TODO use in ArrExecutor
  struct Rng
  {    
    std::size_t start;
    std::size_t stop{};
    bool hasStop{false};  // if false, ignore stop
    bool hasRng{false};   // if false, ignore everything else
  };

  static inline Rng rangeFromRequest(const njson& body, const std::string_view name)
  {
    if (!body.contains(name))
      return {};
    else
    {
      Rng result{.hasRng = true};

      const auto& element = body.at(name);
      const auto& rng = element.array_range();
      
      result.start = rng.cbegin()->as<std::size_t>();
      
      if (element.size() == 2)
      {
        result.stop = rng.crbegin()->as<std::size_t>();
        result.hasStop = true;
      }

      return result;
    }      
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


  // command is not known at time of err, send a general "ERR" response
  static inline njson createErrorResponse (const RequestStatus status)
  {
    try
    {
      njson rsp;
      rsp["ERR"]["st"] = toUnderlying(status);
      return rsp;
    }
    catch(const std::exception& e)
    {
      return njson{};
    }
  }


  static inline njson createErrorResponse (const std::string_view rspName, const RequestStatus status) noexcept
  {
    try
    {
      njson rsp;
      rsp[rspName]["st"] = toUnderlying(status);
      return rsp;
    }
    catch(const std::exception& e)
    {
      return njson{};
    }
  }


  using ValidateParams = std::map<const std::string_view, const Param>;

  // Checks the params (if present and correct type). If that passes and onPostValidate is set, onPostValidate() is called for custom checks.
  RequestStatus isCmdValid (const njson& msg,
                                                                const ValidateParams& params,
                                                                std::function<RequestStatus(const njson&)> onPostValidate)
  {
    for (const auto& [member, param] : params)
    {
      if (param.isRequired && !msg.contains(member))
        return RequestStatus::ParamMissing;
      else if (msg.contains(member) && msg.at(member).type() != param.type) // not required but present OR required and present: check type
        return RequestStatus::ValueTypeInvalid;
    }

    return onPostValidate ? onPostValidate(msg) : RequestStatus::Ok;
  }


  RequestStatus isValid ( const std::string_view queryRspName, 
                          const njson& req,
                          const ValidateParams& params,
                          std::function<RequestStatus(const njson&)> onPostValidate = nullptr)
  {
    
    #ifdef NDB_DEBUG
      const auto status = isCmdValid(req, params, onPostValidate);
      PLOGD_IF(status != RequestStatus::Ok) << "Status: " << toUnderlying(status);
      return status;
    #else
      return isCmdValid(req, params, onPostValidate);
    #endif

    // if (const auto status = isCmdValid(req, params, onPostValidate); status == RequestStatus::Ok) [[unlikely]]
    //   return {true, njson{}};
    // else
    // {
    //   PLOGD << "Status: " << toUnderlying(status);
    //   return {false, createErrorResponse(queryRspName, status)};
    // } 
  }
} // namespace nemesis


#endif
