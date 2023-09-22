#ifndef FC_CORE_FUSIONCOMMON_H
#define FC_CORE_FUSIONCOMMON_H

#include <string_view>
#include <mutex>
#include <ostream>


namespace fusion { namespace core {

#define fc_always_inline inline __attribute__((always_inline))

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

#endif
