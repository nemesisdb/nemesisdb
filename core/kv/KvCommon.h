#ifndef NDB_CORE_KVCOMMON_H
#define NDB_CORE_KVCOMMON_H

#include <map>
#include <array>
#include <any>
#include <ankerl/unordered_dense.h>
#include <uuid_v4/uuid_v4.h>
#include <uwebsockets/App.h>
#include <core/NemesisCommon.h>


namespace nemesis {  namespace kv {


  enum class KvQueryType : std::uint8_t
  { 
    KvSet,
    KvSetQ,
    KvGet,
    KvAdd,
    KvAddQ,
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
    MAX,
    InternalSessionMonitor,
    InternalLoad,
    Unknown,
  };

}
}

#endif
