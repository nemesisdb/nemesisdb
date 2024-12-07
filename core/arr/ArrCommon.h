#ifndef NDB_CORE_ARRCOMMON_H
#define NDB_CORE_ARRCOMMON_H

#include <map>
#include <array>
#include <any>
#include <ankerl/unordered_dense.h>
#include <uuid_v4/uuid_v4.h>
#include <uwebsockets/App.h>
#include <core/NemesisCommon.h>


namespace nemesis {  namespace arr {


  enum class ArrQueryType : std::uint8_t
  { 
    Create,
    Delete,
    DeleteAll,
    Set,
    SetRng,
    Get,
    GetRng,
    Len,
    Swap
  };

}
}

#endif
