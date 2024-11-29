#ifndef NDB_CORE_SHCOMMON_H
#define NDB_CORE_SHCOMMON_H

#include <map>
#include <array>
#include <any>
#include <fstream>
#include <ankerl/unordered_dense.h>
#include <uuid_v4/uuid_v4.h>
#include <uwebsockets/App.h>
#include <core/NemesisCommon.h>


namespace nemesis { namespace core { namespace sh {


  enum class ShQueryType : std::uint8_t
  { 
    ShNew,
    ShEnd,
    ShOpen,
    ShInfo,
    ShInfoAll,
    ShSave,
    ShLoad,
    ShEndAll,
    ShExists,
    //
    Set,
    Get,
    Count,
    Add,
    Clear,
    Contains,
    Keys,
    ClearSet,
    Rmv,
    MAX,
    InternalSessionMonitor,
    InternalLoad,
    Unknown,
  };


  ndb_always_inline SessionToken createSessionToken(const SessionName& name)
  {
    static const std::size_t seed = 99194853094755497U;
    const auto hash = std::hash<SessionName>{}(name);
    return (hash | seed);
  }


  ndb_always_inline SessionToken createSessionToken()
  {
    static UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator; 
    const auto uuid = uuidGenerator.getUUID();
    return uuid.hash();
  }  
}
}
}

#endif
