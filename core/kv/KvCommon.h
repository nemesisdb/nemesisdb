#ifndef NDB_CORE_KVCOMMON_H
#define NDB_CORE_KVCOMMON_H

#include <map>
#include <array>
#include <vector>
#include <variant>
#include <ankerl/unordered_dense.h>
#include <uuid_v4/uuid_v4.h>
#include <uwebsockets/App.h>
#include <core/NemesisCommon.h>
#include <core/fbs/kv_request_generated.h>
#include <core/fbs/kv_response_generated.h>
#include <core/fbs/common_generated.h>
#include <flatbuffers/flexbuffers.h>

namespace nemesis {  namespace kv {

  struct CachedValue
  {
    static const std::size_t GET_STR = 0;
    static const std::size_t GET_INT = 1;
    static const std::size_t GET_UINT = 2;
    static const std::size_t GET_DBL = 3;
    static const std::size_t GET_BOOL = 4;

    flexbuffers::Type type;
    std::variant<std::string, int64_t, uint64_t, double, bool> value;    
  };
  
  using cachedvalue2 = CachedValue;

  
  // template<typename T>
  // inline void scalarToVector(std::vector<char>& vec, T val)
  // {
  //   for (unsigned long i = 0; i < sizeof(T); ++i)
  //   {
  //     vec[i] = val & 0xFF;
  //     val >>= 8;
  //   }
  // }
  

  // template<typename T>
  // inline T vectorToScalar (const std::vector<char>& vec)
  // {
  //   T v{};

  //   for (auto i = sizeof(T)-1; i < sizeof(T); --i)
  //   {
  //     v = vec[i];
  //     v <<= 8;
  //   }

  //   return v;
  // }

  
  enum class KvQueryType : std::uint8_t
  { 
    KvSet,
    KvGet,
    KvAdd,
    KvRemove,
    KvClear,
    KvCount,
    KvContains,
    KvFind,
    KvUpdate,
    KvKeys,
    KvClearSet,
    KvSave,
    KvLoad,
    KvArrayAppend,
    InternalSessionMonitor,
    InternalLoad,
    Unknown,
  };

}
}

#endif
