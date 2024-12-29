// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: set.proto

#include "set.pb.h"

#include <algorithm>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>

PROTOBUF_PRAGMA_INIT_SEG

namespace _pb = ::PROTOBUF_NAMESPACE_ID;
namespace _pbi = _pb::internal;

namespace ndb {
namespace request {
PROTOBUF_CONSTEXPR Set::Set(
    ::_pbi::ConstantInitialized): _impl_{
    /*decltype(_impl_.kv_)*/nullptr
  , /*decltype(_impl_._cached_size_)*/{}} {}
struct SetDefaultTypeInternal {
  PROTOBUF_CONSTEXPR SetDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~SetDefaultTypeInternal() {}
  union {
    Set _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 SetDefaultTypeInternal _Set_default_instance_;
PROTOBUF_CONSTEXPR Get::Get(
    ::_pbi::ConstantInitialized): _impl_{
    /*decltype(_impl_.keys_)*/{}
  , /*decltype(_impl_._cached_size_)*/{}} {}
struct GetDefaultTypeInternal {
  PROTOBUF_CONSTEXPR GetDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~GetDefaultTypeInternal() {}
  union {
    Get _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 GetDefaultTypeInternal _Get_default_instance_;
PROTOBUF_CONSTEXPR Request::Request(
    ::_pbi::ConstantInitialized): _impl_{
    /*decltype(_impl_.kv_)*/{}
  , /*decltype(_impl_._cached_size_)*/{}
  , /*decltype(_impl_._oneof_case_)*/{}} {}
struct RequestDefaultTypeInternal {
  PROTOBUF_CONSTEXPR RequestDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~RequestDefaultTypeInternal() {}
  union {
    Request _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 RequestDefaultTypeInternal _Request_default_instance_;
}  // namespace request
}  // namespace ndb
static ::_pb::Metadata file_level_metadata_set_2eproto[3];
static constexpr ::_pb::EnumDescriptor const** file_level_enum_descriptors_set_2eproto = nullptr;
static constexpr ::_pb::ServiceDescriptor const** file_level_service_descriptors_set_2eproto = nullptr;

const uint32_t TableStruct_set_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::ndb::request::Set, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::ndb::request::Set, _impl_.kv_),
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::ndb::request::Get, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::ndb::request::Get, _impl_.keys_),
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::ndb::request::Request, _internal_metadata_),
  ~0u,  // no _extensions_
  PROTOBUF_FIELD_OFFSET(::ndb::request::Request, _impl_._oneof_case_[0]),
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  ::_pbi::kInvalidFieldOffsetTag,
  ::_pbi::kInvalidFieldOffsetTag,
  PROTOBUF_FIELD_OFFSET(::ndb::request::Request, _impl_.kv_),
};
static const ::_pbi::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, -1, sizeof(::ndb::request::Set)},
  { 7, -1, -1, sizeof(::ndb::request::Get)},
  { 14, -1, -1, sizeof(::ndb::request::Request)},
};

static const ::_pb::Message* const file_default_instances[] = {
  &::ndb::request::_Set_default_instance_._instance,
  &::ndb::request::_Get_default_instance_._instance,
  &::ndb::request::_Request_default_instance_._instance,
};

const char descriptor_table_protodef_set_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\tset.proto\022\013ndb.request\032\034google/protobu"
  "f/struct.proto\"*\n\003Set\022#\n\002kv\030\001 \001(\0132\027.goog"
  "le.protobuf.Struct\"\023\n\003Get\022\014\n\004keys\030\002 \003(\t\""
  "Q\n\007Request\022\037\n\003get\030\001 \001(\0132\020.ndb.request.Ge"
  "tH\000\022\037\n\003set\030\002 \001(\0132\020.ndb.request.SetH\000B\004\n\002"
  "kvB\002H\001b\006proto3"
  ;
static const ::_pbi::DescriptorTable* const descriptor_table_set_2eproto_deps[1] = {
  &::descriptor_table_google_2fprotobuf_2fstruct_2eproto,
};
static ::_pbi::once_flag descriptor_table_set_2eproto_once;
const ::_pbi::DescriptorTable descriptor_table_set_2eproto = {
    false, false, 214, descriptor_table_protodef_set_2eproto,
    "set.proto",
    &descriptor_table_set_2eproto_once, descriptor_table_set_2eproto_deps, 1, 3,
    schemas, file_default_instances, TableStruct_set_2eproto::offsets,
    file_level_metadata_set_2eproto, file_level_enum_descriptors_set_2eproto,
    file_level_service_descriptors_set_2eproto,
};
PROTOBUF_ATTRIBUTE_WEAK const ::_pbi::DescriptorTable* descriptor_table_set_2eproto_getter() {
  return &descriptor_table_set_2eproto;
}

// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 static ::_pbi::AddDescriptorsRunner dynamic_init_dummy_set_2eproto(&descriptor_table_set_2eproto);
namespace ndb {
namespace request {

// ===================================================================

class Set::_Internal {
 public:
  static const ::PROTOBUF_NAMESPACE_ID::Struct& kv(const Set* msg);
};

const ::PROTOBUF_NAMESPACE_ID::Struct&
Set::_Internal::kv(const Set* msg) {
  return *msg->_impl_.kv_;
}
void Set::clear_kv() {
  if (GetArenaForAllocation() == nullptr && _impl_.kv_ != nullptr) {
    delete _impl_.kv_;
  }
  _impl_.kv_ = nullptr;
}
Set::Set(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor(arena, is_message_owned);
  // @@protoc_insertion_point(arena_constructor:ndb.request.Set)
}
Set::Set(const Set& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  Set* const _this = this; (void)_this;
  new (&_impl_) Impl_{
      decltype(_impl_.kv_){nullptr}
    , /*decltype(_impl_._cached_size_)*/{}};

  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  if (from._internal_has_kv()) {
    _this->_impl_.kv_ = new ::PROTOBUF_NAMESPACE_ID::Struct(*from._impl_.kv_);
  }
  // @@protoc_insertion_point(copy_constructor:ndb.request.Set)
}

inline void Set::SharedCtor(
    ::_pb::Arena* arena, bool is_message_owned) {
  (void)arena;
  (void)is_message_owned;
  new (&_impl_) Impl_{
      decltype(_impl_.kv_){nullptr}
    , /*decltype(_impl_._cached_size_)*/{}
  };
}

Set::~Set() {
  // @@protoc_insertion_point(destructor:ndb.request.Set)
  if (auto *arena = _internal_metadata_.DeleteReturnArena<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>()) {
  (void)arena;
    return;
  }
  SharedDtor();
}

inline void Set::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  if (this != internal_default_instance()) delete _impl_.kv_;
}

void Set::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

void Set::Clear() {
// @@protoc_insertion_point(message_clear_start:ndb.request.Set)
  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  if (GetArenaForAllocation() == nullptr && _impl_.kv_ != nullptr) {
    delete _impl_.kv_;
  }
  _impl_.kv_ = nullptr;
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* Set::_InternalParse(const char* ptr, ::_pbi::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ::_pbi::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // .google.protobuf.Struct kv = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 10)) {
          ptr = ctx->ParseMessage(_internal_mutable_kv(), ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

uint8_t* Set::_InternalSerialize(
    uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:ndb.request.Set)
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  // .google.protobuf.Struct kv = 1;
  if (this->_internal_has_kv()) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::
      InternalWriteMessage(1, _Internal::kv(this),
        _Internal::kv(this).GetCachedSize(), target, stream);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:ndb.request.Set)
  return target;
}

size_t Set::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:ndb.request.Set)
  size_t total_size = 0;

  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // .google.protobuf.Struct kv = 1;
  if (this->_internal_has_kv()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::MessageSize(
        *_impl_.kv_);
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData Set::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSourceCheck,
    Set::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*Set::GetClassData() const { return &_class_data_; }


void Set::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg) {
  auto* const _this = static_cast<Set*>(&to_msg);
  auto& from = static_cast<const Set&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:ndb.request.Set)
  GOOGLE_DCHECK_NE(&from, _this);
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  if (from._internal_has_kv()) {
    _this->_internal_mutable_kv()->::PROTOBUF_NAMESPACE_ID::Struct::MergeFrom(
        from._internal_kv());
  }
  _this->_internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void Set::CopyFrom(const Set& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:ndb.request.Set)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Set::IsInitialized() const {
  return true;
}

void Set::InternalSwap(Set* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  swap(_impl_.kv_, other->_impl_.kv_);
}

::PROTOBUF_NAMESPACE_ID::Metadata Set::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_set_2eproto_getter, &descriptor_table_set_2eproto_once,
      file_level_metadata_set_2eproto[0]);
}

// ===================================================================

class Get::_Internal {
 public:
};

Get::Get(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor(arena, is_message_owned);
  // @@protoc_insertion_point(arena_constructor:ndb.request.Get)
}
Get::Get(const Get& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  Get* const _this = this; (void)_this;
  new (&_impl_) Impl_{
      decltype(_impl_.keys_){from._impl_.keys_}
    , /*decltype(_impl_._cached_size_)*/{}};

  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  // @@protoc_insertion_point(copy_constructor:ndb.request.Get)
}

inline void Get::SharedCtor(
    ::_pb::Arena* arena, bool is_message_owned) {
  (void)arena;
  (void)is_message_owned;
  new (&_impl_) Impl_{
      decltype(_impl_.keys_){arena}
    , /*decltype(_impl_._cached_size_)*/{}
  };
}

Get::~Get() {
  // @@protoc_insertion_point(destructor:ndb.request.Get)
  if (auto *arena = _internal_metadata_.DeleteReturnArena<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>()) {
  (void)arena;
    return;
  }
  SharedDtor();
}

inline void Get::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  _impl_.keys_.~RepeatedPtrField();
}

void Get::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

void Get::Clear() {
// @@protoc_insertion_point(message_clear_start:ndb.request.Get)
  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.keys_.Clear();
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* Get::_InternalParse(const char* ptr, ::_pbi::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ::_pbi::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // repeated string keys = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 18)) {
          ptr -= 1;
          do {
            ptr += 1;
            auto str = _internal_add_keys();
            ptr = ::_pbi::InlineGreedyStringParser(str, ptr, ctx);
            CHK_(ptr);
            CHK_(::_pbi::VerifyUTF8(str, "ndb.request.Get.keys"));
            if (!ctx->DataAvailable(ptr)) break;
          } while (::PROTOBUF_NAMESPACE_ID::internal::ExpectTag<18>(ptr));
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

uint8_t* Get::_InternalSerialize(
    uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:ndb.request.Get)
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  // repeated string keys = 2;
  for (int i = 0, n = this->_internal_keys_size(); i < n; i++) {
    const auto& s = this->_internal_keys(i);
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      s.data(), static_cast<int>(s.length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "ndb.request.Get.keys");
    target = stream->WriteString(2, s, target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:ndb.request.Get)
  return target;
}

size_t Get::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:ndb.request.Get)
  size_t total_size = 0;

  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // repeated string keys = 2;
  total_size += 1 *
      ::PROTOBUF_NAMESPACE_ID::internal::FromIntSize(_impl_.keys_.size());
  for (int i = 0, n = _impl_.keys_.size(); i < n; i++) {
    total_size += ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
      _impl_.keys_.Get(i));
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData Get::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSourceCheck,
    Get::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*Get::GetClassData() const { return &_class_data_; }


void Get::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg) {
  auto* const _this = static_cast<Get*>(&to_msg);
  auto& from = static_cast<const Get&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:ndb.request.Get)
  GOOGLE_DCHECK_NE(&from, _this);
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  _this->_impl_.keys_.MergeFrom(from._impl_.keys_);
  _this->_internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void Get::CopyFrom(const Get& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:ndb.request.Get)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Get::IsInitialized() const {
  return true;
}

void Get::InternalSwap(Get* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  _impl_.keys_.InternalSwap(&other->_impl_.keys_);
}

::PROTOBUF_NAMESPACE_ID::Metadata Get::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_set_2eproto_getter, &descriptor_table_set_2eproto_once,
      file_level_metadata_set_2eproto[1]);
}

// ===================================================================

class Request::_Internal {
 public:
  static const ::ndb::request::Get& get(const Request* msg);
  static const ::ndb::request::Set& set(const Request* msg);
};

const ::ndb::request::Get&
Request::_Internal::get(const Request* msg) {
  return *msg->_impl_.kv_.get_;
}
const ::ndb::request::Set&
Request::_Internal::set(const Request* msg) {
  return *msg->_impl_.kv_.set_;
}
void Request::set_allocated_get(::ndb::request::Get* get) {
  ::PROTOBUF_NAMESPACE_ID::Arena* message_arena = GetArenaForAllocation();
  clear_kv();
  if (get) {
    ::PROTOBUF_NAMESPACE_ID::Arena* submessage_arena =
      ::PROTOBUF_NAMESPACE_ID::Arena::InternalGetOwningArena(get);
    if (message_arena != submessage_arena) {
      get = ::PROTOBUF_NAMESPACE_ID::internal::GetOwnedMessage(
          message_arena, get, submessage_arena);
    }
    set_has_get();
    _impl_.kv_.get_ = get;
  }
  // @@protoc_insertion_point(field_set_allocated:ndb.request.Request.get)
}
void Request::set_allocated_set(::ndb::request::Set* set) {
  ::PROTOBUF_NAMESPACE_ID::Arena* message_arena = GetArenaForAllocation();
  clear_kv();
  if (set) {
    ::PROTOBUF_NAMESPACE_ID::Arena* submessage_arena =
      ::PROTOBUF_NAMESPACE_ID::Arena::InternalGetOwningArena(set);
    if (message_arena != submessage_arena) {
      set = ::PROTOBUF_NAMESPACE_ID::internal::GetOwnedMessage(
          message_arena, set, submessage_arena);
    }
    set_has_set();
    _impl_.kv_.set_ = set;
  }
  // @@protoc_insertion_point(field_set_allocated:ndb.request.Request.set)
}
Request::Request(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor(arena, is_message_owned);
  // @@protoc_insertion_point(arena_constructor:ndb.request.Request)
}
Request::Request(const Request& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  Request* const _this = this; (void)_this;
  new (&_impl_) Impl_{
      decltype(_impl_.kv_){}
    , /*decltype(_impl_._cached_size_)*/{}
    , /*decltype(_impl_._oneof_case_)*/{}};

  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  clear_has_kv();
  switch (from.kv_case()) {
    case kGet: {
      _this->_internal_mutable_get()->::ndb::request::Get::MergeFrom(
          from._internal_get());
      break;
    }
    case kSet: {
      _this->_internal_mutable_set()->::ndb::request::Set::MergeFrom(
          from._internal_set());
      break;
    }
    case KV_NOT_SET: {
      break;
    }
  }
  // @@protoc_insertion_point(copy_constructor:ndb.request.Request)
}

inline void Request::SharedCtor(
    ::_pb::Arena* arena, bool is_message_owned) {
  (void)arena;
  (void)is_message_owned;
  new (&_impl_) Impl_{
      decltype(_impl_.kv_){}
    , /*decltype(_impl_._cached_size_)*/{}
    , /*decltype(_impl_._oneof_case_)*/{}
  };
  clear_has_kv();
}

Request::~Request() {
  // @@protoc_insertion_point(destructor:ndb.request.Request)
  if (auto *arena = _internal_metadata_.DeleteReturnArena<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>()) {
  (void)arena;
    return;
  }
  SharedDtor();
}

inline void Request::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  if (has_kv()) {
    clear_kv();
  }
}

void Request::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

void Request::clear_kv() {
// @@protoc_insertion_point(one_of_clear_start:ndb.request.Request)
  switch (kv_case()) {
    case kGet: {
      if (GetArenaForAllocation() == nullptr) {
        delete _impl_.kv_.get_;
      }
      break;
    }
    case kSet: {
      if (GetArenaForAllocation() == nullptr) {
        delete _impl_.kv_.set_;
      }
      break;
    }
    case KV_NOT_SET: {
      break;
    }
  }
  _impl_._oneof_case_[0] = KV_NOT_SET;
}


void Request::Clear() {
// @@protoc_insertion_point(message_clear_start:ndb.request.Request)
  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  clear_kv();
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* Request::_InternalParse(const char* ptr, ::_pbi::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ::_pbi::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // .ndb.request.Get get = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 10)) {
          ptr = ctx->ParseMessage(_internal_mutable_get(), ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // .ndb.request.Set set = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 18)) {
          ptr = ctx->ParseMessage(_internal_mutable_set(), ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

uint8_t* Request::_InternalSerialize(
    uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:ndb.request.Request)
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  // .ndb.request.Get get = 1;
  if (_internal_has_get()) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::
      InternalWriteMessage(1, _Internal::get(this),
        _Internal::get(this).GetCachedSize(), target, stream);
  }

  // .ndb.request.Set set = 2;
  if (_internal_has_set()) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::
      InternalWriteMessage(2, _Internal::set(this),
        _Internal::set(this).GetCachedSize(), target, stream);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:ndb.request.Request)
  return target;
}

size_t Request::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:ndb.request.Request)
  size_t total_size = 0;

  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  switch (kv_case()) {
    // .ndb.request.Get get = 1;
    case kGet: {
      total_size += 1 +
        ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::MessageSize(
          *_impl_.kv_.get_);
      break;
    }
    // .ndb.request.Set set = 2;
    case kSet: {
      total_size += 1 +
        ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::MessageSize(
          *_impl_.kv_.set_);
      break;
    }
    case KV_NOT_SET: {
      break;
    }
  }
  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData Request::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSourceCheck,
    Request::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*Request::GetClassData() const { return &_class_data_; }


void Request::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg) {
  auto* const _this = static_cast<Request*>(&to_msg);
  auto& from = static_cast<const Request&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:ndb.request.Request)
  GOOGLE_DCHECK_NE(&from, _this);
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  switch (from.kv_case()) {
    case kGet: {
      _this->_internal_mutable_get()->::ndb::request::Get::MergeFrom(
          from._internal_get());
      break;
    }
    case kSet: {
      _this->_internal_mutable_set()->::ndb::request::Set::MergeFrom(
          from._internal_set());
      break;
    }
    case KV_NOT_SET: {
      break;
    }
  }
  _this->_internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void Request::CopyFrom(const Request& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:ndb.request.Request)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Request::IsInitialized() const {
  return true;
}

void Request::InternalSwap(Request* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  swap(_impl_.kv_, other->_impl_.kv_);
  swap(_impl_._oneof_case_[0], other->_impl_._oneof_case_[0]);
}

::PROTOBUF_NAMESPACE_ID::Metadata Request::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_set_2eproto_getter, &descriptor_table_set_2eproto_once,
      file_level_metadata_set_2eproto[2]);
}

// @@protoc_insertion_point(namespace_scope)
}  // namespace request
}  // namespace ndb
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::ndb::request::Set*
Arena::CreateMaybeMessage< ::ndb::request::Set >(Arena* arena) {
  return Arena::CreateMessageInternal< ::ndb::request::Set >(arena);
}
template<> PROTOBUF_NOINLINE ::ndb::request::Get*
Arena::CreateMaybeMessage< ::ndb::request::Get >(Arena* arena) {
  return Arena::CreateMessageInternal< ::ndb::request::Get >(arena);
}
template<> PROTOBUF_NOINLINE ::ndb::request::Request*
Arena::CreateMaybeMessage< ::ndb::request::Request >(Arena* arena) {
  return Arena::CreateMessageInternal< ::ndb::request::Request >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
