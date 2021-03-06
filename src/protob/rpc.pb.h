// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: rpc.proto

#ifndef PROTOBUF_rpc_2eproto__INCLUDED
#define PROTOBUF_rpc_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2006000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2006000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace carrot {
namespace rpc {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_rpc_2eproto();
void protobuf_AssignDesc_rpc_2eproto();
void protobuf_ShutdownFile_rpc_2eproto();

class Request;
class Response;

enum Status {
  SUCCESS = 1,
  ERROR = 2,
  NOT_FOUND = 3,
  FAILED = 4
};
bool Status_IsValid(int value);
const Status Status_MIN = SUCCESS;
const Status Status_MAX = FAILED;
const int Status_ARRAYSIZE = Status_MAX + 1;

const ::google::protobuf::EnumDescriptor* Status_descriptor();
inline const ::std::string& Status_Name(Status value) {
  return ::google::protobuf::internal::NameOfEnum(
    Status_descriptor(), value);
}
inline bool Status_Parse(
    const ::std::string& name, Status* value) {
  return ::google::protobuf::internal::ParseNamedEnum<Status>(
    Status_descriptor(), name, value);
}
// ===================================================================

class Request : public ::google::protobuf::Message {
 public:
  Request();
  virtual ~Request();

  Request(const Request& from);

  inline Request& operator=(const Request& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Request& default_instance();

  void Swap(Request* other);

  // implements Message ----------------------------------------------

  Request* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Request& from);
  void MergeFrom(const Request& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required uint64 id = 1;
  inline bool has_id() const;
  inline void clear_id();
  static const int kIdFieldNumber = 1;
  inline ::google::protobuf::uint64 id() const;
  inline void set_id(::google::protobuf::uint64 value);

  // required string method_identity = 2;
  inline bool has_method_identity() const;
  inline void clear_method_identity();
  static const int kMethodIdentityFieldNumber = 2;
  inline const ::std::string& method_identity() const;
  inline void set_method_identity(const ::std::string& value);
  inline void set_method_identity(const char* value);
  inline void set_method_identity(const char* value, size_t size);
  inline ::std::string* mutable_method_identity();
  inline ::std::string* release_method_identity();
  inline void set_allocated_method_identity(::std::string* method_identity);

  // @@protoc_insertion_point(class_scope:carrot.rpc.Request)
 private:
  inline void set_has_id();
  inline void clear_has_id();
  inline void set_has_method_identity();
  inline void clear_has_method_identity();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::google::protobuf::uint64 id_;
  ::std::string* method_identity_;
  friend void  protobuf_AddDesc_rpc_2eproto();
  friend void protobuf_AssignDesc_rpc_2eproto();
  friend void protobuf_ShutdownFile_rpc_2eproto();

  void InitAsDefaultInstance();
  static Request* default_instance_;
};
// -------------------------------------------------------------------

class Response : public ::google::protobuf::Message {
 public:
  Response();
  virtual ~Response();

  Response(const Response& from);

  inline Response& operator=(const Response& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Response& default_instance();

  void Swap(Response* other);

  // implements Message ----------------------------------------------

  Response* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Response& from);
  void MergeFrom(const Response& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required uint64 id = 1;
  inline bool has_id() const;
  inline void clear_id();
  static const int kIdFieldNumber = 1;
  inline ::google::protobuf::uint64 id() const;
  inline void set_id(::google::protobuf::uint64 value);

  // required .carrot.rpc.Status status = 2;
  inline bool has_status() const;
  inline void clear_status();
  static const int kStatusFieldNumber = 2;
  inline ::carrot::rpc::Status status() const;
  inline void set_status(::carrot::rpc::Status value);

  // optional string message = 3;
  inline bool has_message() const;
  inline void clear_message();
  static const int kMessageFieldNumber = 3;
  inline const ::std::string& message() const;
  inline void set_message(const ::std::string& value);
  inline void set_message(const char* value);
  inline void set_message(const char* value, size_t size);
  inline ::std::string* mutable_message();
  inline ::std::string* release_message();
  inline void set_allocated_message(::std::string* message);

  // @@protoc_insertion_point(class_scope:carrot.rpc.Response)
 private:
  inline void set_has_id();
  inline void clear_has_id();
  inline void set_has_status();
  inline void clear_has_status();
  inline void set_has_message();
  inline void clear_has_message();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::google::protobuf::uint64 id_;
  ::std::string* message_;
  int status_;
  friend void  protobuf_AddDesc_rpc_2eproto();
  friend void protobuf_AssignDesc_rpc_2eproto();
  friend void protobuf_ShutdownFile_rpc_2eproto();

  void InitAsDefaultInstance();
  static Response* default_instance_;
};
// ===================================================================


// ===================================================================

// Request

// required uint64 id = 1;
inline bool Request::has_id() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void Request::set_has_id() {
  _has_bits_[0] |= 0x00000001u;
}
inline void Request::clear_has_id() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void Request::clear_id() {
  id_ = GOOGLE_ULONGLONG(0);
  clear_has_id();
}
inline ::google::protobuf::uint64 Request::id() const {
  // @@protoc_insertion_point(field_get:carrot.rpc.Request.id)
  return id_;
}
inline void Request::set_id(::google::protobuf::uint64 value) {
  set_has_id();
  id_ = value;
  // @@protoc_insertion_point(field_set:carrot.rpc.Request.id)
}

// required string method_identity = 2;
inline bool Request::has_method_identity() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void Request::set_has_method_identity() {
  _has_bits_[0] |= 0x00000002u;
}
inline void Request::clear_has_method_identity() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void Request::clear_method_identity() {
  if (method_identity_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    method_identity_->clear();
  }
  clear_has_method_identity();
}
inline const ::std::string& Request::method_identity() const {
  // @@protoc_insertion_point(field_get:carrot.rpc.Request.method_identity)
  return *method_identity_;
}
inline void Request::set_method_identity(const ::std::string& value) {
  set_has_method_identity();
  if (method_identity_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    method_identity_ = new ::std::string;
  }
  method_identity_->assign(value);
  // @@protoc_insertion_point(field_set:carrot.rpc.Request.method_identity)
}
inline void Request::set_method_identity(const char* value) {
  set_has_method_identity();
  if (method_identity_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    method_identity_ = new ::std::string;
  }
  method_identity_->assign(value);
  // @@protoc_insertion_point(field_set_char:carrot.rpc.Request.method_identity)
}
inline void Request::set_method_identity(const char* value, size_t size) {
  set_has_method_identity();
  if (method_identity_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    method_identity_ = new ::std::string;
  }
  method_identity_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:carrot.rpc.Request.method_identity)
}
inline ::std::string* Request::mutable_method_identity() {
  set_has_method_identity();
  if (method_identity_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    method_identity_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:carrot.rpc.Request.method_identity)
  return method_identity_;
}
inline ::std::string* Request::release_method_identity() {
  clear_has_method_identity();
  if (method_identity_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = method_identity_;
    method_identity_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void Request::set_allocated_method_identity(::std::string* method_identity) {
  if (method_identity_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete method_identity_;
  }
  if (method_identity) {
    set_has_method_identity();
    method_identity_ = method_identity;
  } else {
    clear_has_method_identity();
    method_identity_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:carrot.rpc.Request.method_identity)
}

// -------------------------------------------------------------------

// Response

// required uint64 id = 1;
inline bool Response::has_id() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void Response::set_has_id() {
  _has_bits_[0] |= 0x00000001u;
}
inline void Response::clear_has_id() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void Response::clear_id() {
  id_ = GOOGLE_ULONGLONG(0);
  clear_has_id();
}
inline ::google::protobuf::uint64 Response::id() const {
  // @@protoc_insertion_point(field_get:carrot.rpc.Response.id)
  return id_;
}
inline void Response::set_id(::google::protobuf::uint64 value) {
  set_has_id();
  id_ = value;
  // @@protoc_insertion_point(field_set:carrot.rpc.Response.id)
}

// required .carrot.rpc.Status status = 2;
inline bool Response::has_status() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void Response::set_has_status() {
  _has_bits_[0] |= 0x00000002u;
}
inline void Response::clear_has_status() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void Response::clear_status() {
  status_ = 1;
  clear_has_status();
}
inline ::carrot::rpc::Status Response::status() const {
  // @@protoc_insertion_point(field_get:carrot.rpc.Response.status)
  return static_cast< ::carrot::rpc::Status >(status_);
}
inline void Response::set_status(::carrot::rpc::Status value) {
  assert(::carrot::rpc::Status_IsValid(value));
  set_has_status();
  status_ = value;
  // @@protoc_insertion_point(field_set:carrot.rpc.Response.status)
}

// optional string message = 3;
inline bool Response::has_message() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void Response::set_has_message() {
  _has_bits_[0] |= 0x00000004u;
}
inline void Response::clear_has_message() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void Response::clear_message() {
  if (message_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    message_->clear();
  }
  clear_has_message();
}
inline const ::std::string& Response::message() const {
  // @@protoc_insertion_point(field_get:carrot.rpc.Response.message)
  return *message_;
}
inline void Response::set_message(const ::std::string& value) {
  set_has_message();
  if (message_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    message_ = new ::std::string;
  }
  message_->assign(value);
  // @@protoc_insertion_point(field_set:carrot.rpc.Response.message)
}
inline void Response::set_message(const char* value) {
  set_has_message();
  if (message_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    message_ = new ::std::string;
  }
  message_->assign(value);
  // @@protoc_insertion_point(field_set_char:carrot.rpc.Response.message)
}
inline void Response::set_message(const char* value, size_t size) {
  set_has_message();
  if (message_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    message_ = new ::std::string;
  }
  message_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:carrot.rpc.Response.message)
}
inline ::std::string* Response::mutable_message() {
  set_has_message();
  if (message_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    message_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:carrot.rpc.Response.message)
  return message_;
}
inline ::std::string* Response::release_message() {
  clear_has_message();
  if (message_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = message_;
    message_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void Response::set_allocated_message(::std::string* message) {
  if (message_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete message_;
  }
  if (message) {
    set_has_message();
    message_ = message;
  } else {
    clear_has_message();
    message_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:carrot.rpc.Response.message)
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace rpc
}  // namespace carrot

#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::carrot::rpc::Status> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::carrot::rpc::Status>() {
  return ::carrot::rpc::Status_descriptor();
}

}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_rpc_2eproto__INCLUDED
