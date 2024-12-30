#ifndef NDB_CORE_KVCOMMON_H
#define NDB_CORE_KVCOMMON_H

#include <map>
#include <array>
#include <any>
#include <ankerl/unordered_dense.h>
#include <uuid_v4/uuid_v4.h>
#include <uwebsockets/App.h>
#include <core/NemesisCommon.h>
#include <core/fbs/kv_request_generated.h>
#include <core/fbs/kv_response_generated.h>
#include <core/fbs/common_generated.h>


namespace nemesis {  namespace kv {

  struct CachedValue
  {
    ndb::common::ValueType type;
    std::string_view value;
  };
  
  using cachedvalue2 = CachedValue;

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
