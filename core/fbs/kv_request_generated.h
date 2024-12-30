// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_KVREQUEST_NDB_REQUEST_H_
#define FLATBUFFERS_GENERATED_KVREQUEST_NDB_REQUEST_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

namespace ndb {
namespace request {

struct UInt64;
struct UInt64Builder;

struct Int64;
struct Int64Builder;

struct Double;
struct DoubleBuilder;

struct Bool;
struct BoolBuilder;

struct String;
struct StringBuilder;

struct KV;
struct KVBuilder;

struct Request;
struct RequestBuilder;

enum ValueType : uint8_t {
  ValueType_NONE = 0,
  ValueType_UInt64 = 1,
  ValueType_Int64 = 2,
  ValueType_Double = 3,
  ValueType_Bool = 4,
  ValueType_String = 5,
  ValueType_MIN = ValueType_NONE,
  ValueType_MAX = ValueType_String
};

inline const ValueType (&EnumValuesValueType())[6] {
  static const ValueType values[] = {
    ValueType_NONE,
    ValueType_UInt64,
    ValueType_Int64,
    ValueType_Double,
    ValueType_Bool,
    ValueType_String
  };
  return values;
}

inline const char * const *EnumNamesValueType() {
  static const char * const names[7] = {
    "NONE",
    "UInt64",
    "Int64",
    "Double",
    "Bool",
    "String",
    nullptr
  };
  return names;
}

inline const char *EnumNameValueType(ValueType e) {
  if (::flatbuffers::IsOutRange(e, ValueType_NONE, ValueType_String)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesValueType()[index];
}

template<typename T> struct ValueTypeTraits {
  static const ValueType enum_value = ValueType_NONE;
};

template<> struct ValueTypeTraits<ndb::request::UInt64> {
  static const ValueType enum_value = ValueType_UInt64;
};

template<> struct ValueTypeTraits<ndb::request::Int64> {
  static const ValueType enum_value = ValueType_Int64;
};

template<> struct ValueTypeTraits<ndb::request::Double> {
  static const ValueType enum_value = ValueType_Double;
};

template<> struct ValueTypeTraits<ndb::request::Bool> {
  static const ValueType enum_value = ValueType_Bool;
};

template<> struct ValueTypeTraits<ndb::request::String> {
  static const ValueType enum_value = ValueType_String;
};

bool VerifyValueType(::flatbuffers::Verifier &verifier, const void *obj, ValueType type);
bool VerifyValueTypeVector(::flatbuffers::Verifier &verifier, const ::flatbuffers::Vector<::flatbuffers::Offset<void>> *values, const ::flatbuffers::Vector<uint8_t> *types);

struct UInt64 FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef UInt64Builder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VAL = 4
  };
  uint64_t val() const {
    return GetField<uint64_t>(VT_VAL, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint64_t>(verifier, VT_VAL, 8) &&
           verifier.EndTable();
  }
};

struct UInt64Builder {
  typedef UInt64 Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_val(uint64_t val) {
    fbb_.AddElement<uint64_t>(UInt64::VT_VAL, val, 0);
  }
  explicit UInt64Builder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<UInt64> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<UInt64>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<UInt64> CreateUInt64(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint64_t val = 0) {
  UInt64Builder builder_(_fbb);
  builder_.add_val(val);
  return builder_.Finish();
}

struct Int64 FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef Int64Builder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VAL = 4
  };
  int64_t val() const {
    return GetField<int64_t>(VT_VAL, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_VAL, 8) &&
           verifier.EndTable();
  }
};

struct Int64Builder {
  typedef Int64 Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_val(int64_t val) {
    fbb_.AddElement<int64_t>(Int64::VT_VAL, val, 0);
  }
  explicit Int64Builder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Int64> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Int64>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Int64> CreateInt64(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    int64_t val = 0) {
  Int64Builder builder_(_fbb);
  builder_.add_val(val);
  return builder_.Finish();
}

struct Double FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef DoubleBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VAL = 4
  };
  double val() const {
    return GetField<double>(VT_VAL, 0.0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<double>(verifier, VT_VAL, 8) &&
           verifier.EndTable();
  }
};

struct DoubleBuilder {
  typedef Double Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_val(double val) {
    fbb_.AddElement<double>(Double::VT_VAL, val, 0.0);
  }
  explicit DoubleBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Double> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Double>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Double> CreateDouble(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    double val = 0.0) {
  DoubleBuilder builder_(_fbb);
  builder_.add_val(val);
  return builder_.Finish();
}

struct Bool FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef BoolBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VAL = 4
  };
  bool val() const {
    return GetField<uint8_t>(VT_VAL, 0) != 0;
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_VAL, 1) &&
           verifier.EndTable();
  }
};

struct BoolBuilder {
  typedef Bool Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_val(bool val) {
    fbb_.AddElement<uint8_t>(Bool::VT_VAL, static_cast<uint8_t>(val), 0);
  }
  explicit BoolBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Bool> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Bool>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Bool> CreateBool(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    bool val = false) {
  BoolBuilder builder_(_fbb);
  builder_.add_val(val);
  return builder_.Finish();
}

struct String FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef StringBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VAL = 4
  };
  const ::flatbuffers::String *val() const {
    return GetPointer<const ::flatbuffers::String *>(VT_VAL);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_VAL) &&
           verifier.VerifyString(val()) &&
           verifier.EndTable();
  }
};

struct StringBuilder {
  typedef String Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_val(::flatbuffers::Offset<::flatbuffers::String> val) {
    fbb_.AddOffset(String::VT_VAL, val);
  }
  explicit StringBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<String> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<String>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<String> CreateString(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> val = 0) {
  StringBuilder builder_(_fbb);
  builder_.add_val(val);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<String> CreateStringDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *val = nullptr) {
  auto val__ = val ? _fbb.CreateString(val) : 0;
  return ndb::request::CreateString(
      _fbb,
      val__);
}

struct KV FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef KVBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_KEY = 4,
    VT_VAL_TYPE = 6,
    VT_VAL = 8
  };
  const ::flatbuffers::String *key() const {
    return GetPointer<const ::flatbuffers::String *>(VT_KEY);
  }
  ndb::request::ValueType val_type() const {
    return static_cast<ndb::request::ValueType>(GetField<uint8_t>(VT_VAL_TYPE, 0));
  }
  const void *val() const {
    return GetPointer<const void *>(VT_VAL);
  }
  template<typename T> const T *val_as() const;
  const ndb::request::UInt64 *val_as_UInt64() const {
    return val_type() == ndb::request::ValueType_UInt64 ? static_cast<const ndb::request::UInt64 *>(val()) : nullptr;
  }
  const ndb::request::Int64 *val_as_Int64() const {
    return val_type() == ndb::request::ValueType_Int64 ? static_cast<const ndb::request::Int64 *>(val()) : nullptr;
  }
  const ndb::request::Double *val_as_Double() const {
    return val_type() == ndb::request::ValueType_Double ? static_cast<const ndb::request::Double *>(val()) : nullptr;
  }
  const ndb::request::Bool *val_as_Bool() const {
    return val_type() == ndb::request::ValueType_Bool ? static_cast<const ndb::request::Bool *>(val()) : nullptr;
  }
  const ndb::request::String *val_as_String() const {
    return val_type() == ndb::request::ValueType_String ? static_cast<const ndb::request::String *>(val()) : nullptr;
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_KEY) &&
           verifier.VerifyString(key()) &&
           VerifyField<uint8_t>(verifier, VT_VAL_TYPE, 1) &&
           VerifyOffset(verifier, VT_VAL) &&
           VerifyValueType(verifier, val(), val_type()) &&
           verifier.EndTable();
  }
};

template<> inline const ndb::request::UInt64 *KV::val_as<ndb::request::UInt64>() const {
  return val_as_UInt64();
}

template<> inline const ndb::request::Int64 *KV::val_as<ndb::request::Int64>() const {
  return val_as_Int64();
}

template<> inline const ndb::request::Double *KV::val_as<ndb::request::Double>() const {
  return val_as_Double();
}

template<> inline const ndb::request::Bool *KV::val_as<ndb::request::Bool>() const {
  return val_as_Bool();
}

template<> inline const ndb::request::String *KV::val_as<ndb::request::String>() const {
  return val_as_String();
}

struct KVBuilder {
  typedef KV Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_key(::flatbuffers::Offset<::flatbuffers::String> key) {
    fbb_.AddOffset(KV::VT_KEY, key);
  }
  void add_val_type(ndb::request::ValueType val_type) {
    fbb_.AddElement<uint8_t>(KV::VT_VAL_TYPE, static_cast<uint8_t>(val_type), 0);
  }
  void add_val(::flatbuffers::Offset<void> val) {
    fbb_.AddOffset(KV::VT_VAL, val);
  }
  explicit KVBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<KV> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<KV>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<KV> CreateKV(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> key = 0,
    ndb::request::ValueType val_type = ndb::request::ValueType_NONE,
    ::flatbuffers::Offset<void> val = 0) {
  KVBuilder builder_(_fbb);
  builder_.add_val(val);
  builder_.add_key(key);
  builder_.add_val_type(val_type);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<KV> CreateKVDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *key = nullptr,
    ndb::request::ValueType val_type = ndb::request::ValueType_NONE,
    ::flatbuffers::Offset<void> val = 0) {
  auto key__ = key ? _fbb.CreateString(key) : 0;
  return ndb::request::CreateKV(
      _fbb,
      key__,
      val_type,
      val);
}

struct Request FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef RequestBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_KV = 4
  };
  const ::flatbuffers::Vector<::flatbuffers::Offset<ndb::request::KV>> *kv() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<ndb::request::KV>> *>(VT_KV);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_KV) &&
           verifier.VerifyVector(kv()) &&
           verifier.VerifyVectorOfTables(kv()) &&
           verifier.EndTable();
  }
};

struct RequestBuilder {
  typedef Request Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_kv(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<ndb::request::KV>>> kv) {
    fbb_.AddOffset(Request::VT_KV, kv);
  }
  explicit RequestBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Request> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Request>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Request> CreateRequest(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<ndb::request::KV>>> kv = 0) {
  RequestBuilder builder_(_fbb);
  builder_.add_kv(kv);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<Request> CreateRequestDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<::flatbuffers::Offset<ndb::request::KV>> *kv = nullptr) {
  auto kv__ = kv ? _fbb.CreateVector<::flatbuffers::Offset<ndb::request::KV>>(*kv) : 0;
  return ndb::request::CreateRequest(
      _fbb,
      kv__);
}

inline bool VerifyValueType(::flatbuffers::Verifier &verifier, const void *obj, ValueType type) {
  switch (type) {
    case ValueType_NONE: {
      return true;
    }
    case ValueType_UInt64: {
      auto ptr = reinterpret_cast<const ndb::request::UInt64 *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case ValueType_Int64: {
      auto ptr = reinterpret_cast<const ndb::request::Int64 *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case ValueType_Double: {
      auto ptr = reinterpret_cast<const ndb::request::Double *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case ValueType_Bool: {
      auto ptr = reinterpret_cast<const ndb::request::Bool *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case ValueType_String: {
      auto ptr = reinterpret_cast<const ndb::request::String *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return true;
  }
}

inline bool VerifyValueTypeVector(::flatbuffers::Verifier &verifier, const ::flatbuffers::Vector<::flatbuffers::Offset<void>> *values, const ::flatbuffers::Vector<uint8_t> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (::flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyValueType(
        verifier,  values->Get(i), types->GetEnum<ValueType>(i))) {
      return false;
    }
  }
  return true;
}

inline const ndb::request::Request *GetRequest(const void *buf) {
  return ::flatbuffers::GetRoot<ndb::request::Request>(buf);
}

inline const ndb::request::Request *GetSizePrefixedRequest(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<ndb::request::Request>(buf);
}

inline const char *RequestIdentifier() {
  return "KV  ";
}

inline bool RequestBufferHasIdentifier(const void *buf) {
  return ::flatbuffers::BufferHasIdentifier(
      buf, RequestIdentifier());
}

inline bool SizePrefixedRequestBufferHasIdentifier(const void *buf) {
  return ::flatbuffers::BufferHasIdentifier(
      buf, RequestIdentifier(), true);
}

inline bool VerifyRequestBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<ndb::request::Request>(RequestIdentifier());
}

inline bool VerifySizePrefixedRequestBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<ndb::request::Request>(RequestIdentifier());
}

inline void FinishRequestBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<ndb::request::Request> root) {
  fbb.Finish(root, RequestIdentifier());
}

inline void FinishSizePrefixedRequestBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<ndb::request::Request> root) {
  fbb.FinishSizePrefixed(root, RequestIdentifier());
}

}  // namespace request
}  // namespace ndb

#endif  // FLATBUFFERS_GENERATED_KVREQUEST_NDB_REQUEST_H_
