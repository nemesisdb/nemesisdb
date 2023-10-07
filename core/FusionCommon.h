#ifndef FC_CORE_FUSIONCOMMON_H
#define FC_CORE_FUSIONCOMMON_H

#include <string_view>
#include <mutex>
#include <ostream>
#include <thread>


namespace fusion { namespace core {

#define fc_always_inline inline __attribute__((always_inline))

static const char * FUSION_VERSION = "0.2.5";
static const std::size_t FUSION_CONFIG_VERSION = 1U;

static const std::size_t FUSION_KV_MINPAYLOAD = 64U;
static const std::size_t FUSION_KV_MAXPAYLOAD = 2U * 1024U * 1024U;

static const std::size_t FUSION_MAX_CORES = 64U;


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
