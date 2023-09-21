
#include <string_view>
#include <mutex>
#include <ostream>
#include <uwebsockets/App.h>

namespace fusion { namespace core {

struct QueryData
{
  uWS::WebSocket<false, true, QueryData> * ws;
};


using WebSocket = uWS::WebSocket<false, true, QueryData>;


static const char * FUSION_VERSION = "0.2.0";


static inline bool setThreadAffinity(const std::thread::native_handle_type handle, const size_t core)
{
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core, &cpuset);
  return pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset) == 0;
}


} // namespace core
} // namespace fusion