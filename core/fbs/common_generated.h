// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_COMMON_NDB_COMMON_H_
#define FLATBUFFERS_GENERATED_COMMON_NDB_COMMON_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

namespace ndb {
namespace common {

enum Ident : int8_t {
  Ident_KV = 0,
  Ident_MIN = Ident_KV,
  Ident_MAX = Ident_KV
};

inline const Ident (&EnumValuesIdent())[1] {
  static const Ident values[] = {
    Ident_KV
  };
  return values;
}

inline const char * const *EnumNamesIdent() {
  static const char * const names[2] = {
    "KV",
    nullptr
  };
  return names;
}

inline const char *EnumNameIdent(Ident e) {
  if (::flatbuffers::IsOutRange(e, Ident_KV, Ident_KV)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesIdent()[index];
}

}  // namespace common
}  // namespace ndb

#endif  // FLATBUFFERS_GENERATED_COMMON_NDB_COMMON_H_