#ifndef NDB_CORE_LSTCOMMON_H
#define NDB_CORE_LSTCOMMON_H

#include <core/NemesisCommon.h>


namespace nemesis {  namespace lst {

  enum class LstQueryType : std::uint8_t
  { 
    Create,
    Delete,
    DeleteAll,
    Exist,
    Add,
    SetRng,
    Get,
    GetRng,
    Remove,
    Copy, // TODO
    Move, // TODO
    Len,
    Swap,  // TODO
    MAX
  };
}
}

#endif
