#ifndef FC_CORE_FUSIONCOMMON_H
#define FC_CORE_FUSIONCOMMON_H

#include <string_view>
#include <mutex>
#include <ostream>
#include <thread>


namespace fusion { namespace core {

#define fc_always_inline inline __attribute__((always_inline))

static const char * FUSION_VERSION = "0.2.6";
static const std::size_t FUSION_CONFIG_VERSION = 1U;

static const std::size_t FUSION_KV_MINPAYLOAD = 64U;
static const std::size_t FUSION_KV_MAXPAYLOAD = 2U * 1024U * 1024U;

static const std::size_t FUSION_MAX_CORES = 8U;

static const std::size_t MinKeySize = 6U; // characters, not bytes. See createPoolIndex() if changing this

using fcjson = nlohmann::ordered_json;
using cachedkey = std::string;


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
  CommandNotExist = 10,
  CommandMultiple,
  CommandType,
  CommandSyntax,
  KeySet = 20,
  KeyUpdated,
  KeyNotExist,
  KeyExists,
  KeyRemoved,
  KeyLengthInvalid,
  KeyMissing,
  KeyTypeInvalid,
  ValueMissing = 40,  // NOTE not actually used
  ValueTypeInvalid,
  ValueSize,
  OutOfBounds,
  FindNoPath = 60,
  FindNoOperator,
  FindPathInvalid,
  FindRegExInvalid,
  KeySetCreated = 80,
  KeySetExists,
  KeySetNotExist,
  KeyAddFailed,
  KeyRemoveFailed,
  KeySetRemoveAllFailed,
  KeySetClearFailed,
  KeyMoveFailed,
  Unknown = 1000
};


static inline bool setThreadAffinity(const std::thread::native_handle_type handle, const size_t core)
{
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core, &cpuset);
  return pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset) == 0;
}


bool isKeyValid(const cachedkey& k)
{
  return k.size() >= MinKeySize;
}


bool isKeyValid(const std::string_view& k)
{
  return k.size() >= MinKeySize;
}


// Response if the original command is known.
static fcjson createErrorResponse (const std::string_view commandRsp, const RequestStatus status, const std::string_view key = "")
{
  fcjson rsp;
  rsp[commandRsp]["st"] = status;
  rsp[commandRsp]["k"] = key;
  return rsp;
}


// Response is the original command is unknown.
static fcjson createErrorResponse (const RequestStatus status, const std::string_view msg = "")
{
  fcjson rsp;
  rsp["KV_ERR"]["st"] = status;
  rsp["KV_ERR"]["m"] = msg;
  return rsp;
}

} // namespace core
} // namespace fusion

#endif
