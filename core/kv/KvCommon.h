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

  using KeyVector = flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>;
  using FlexBuilder = flexbuffers::Builder;
  using FlatBuilder = flatbuffers::FlatBufferBuilder;
  using ValueVariant = std::variant<std::string, int64_t, uint64_t, double, bool>;
  using ValueExtractF = void (*)(const char *, const ValueVariant&, FlexBuilder&);

  // This struct is 48 bytes which is larger than I want. The 
  // std::string in ValueVariant is 32 bytes, but it will do
  // for now until memory slabs are implemented
  struct CachedValue
  {
    static const std::size_t GET_STR = 0;
    static const std::size_t GET_INT = 1;
    static const std::size_t GET_UINT = 2;
    static const std::size_t GET_DBL = 3;
    static const std::size_t GET_BOOL = 4;

    ValueVariant value;
    ValueExtractF extract;
  };


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
