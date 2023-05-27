#include "binproto.h"

#include <arpa/inet.h>
#include <google/protobuf/message.h>

namespace meteo {
namespace binproto {

bool parsePartial(const QByteArray& raw, google::protobuf::Message* msg, int* error)
{
  if ( nullptr == msg ) {
    if ( nullptr != error ) { *error = kNullPointer; }
    return false;
  }

  if ( !raw.startsWith(kProtoMagic) ) {
    if ( nullptr != error ) { *error = kMagicNotFound; }
    return false;
  }

  const quint32* szptr = reinterpret_cast<const quint32*>(raw.data() + kMagicSize);
  quint32 sz = ntohl(*szptr);

  if ( quint32(raw.size()) < sz + kHeaderSize ) {
    if ( nullptr != error ) { *error = kIncorrectSize; }
    return false;
  }

  if ( !msg->ParsePartialFromArray(raw.constData() + kHeaderSize, raw.size() - kHeaderSize) ) {
    if ( nullptr != error ) { *error = kParseError; }
    return false;
  }

  if ( nullptr != error ) { *error = kNoError; }
  return true;
}

bool serializePartial(const google::protobuf::Message& msg, QByteArray* raw, int* error)
{
  if ( nullptr == raw ) {
    if ( nullptr != error ) { *error = kNullPointer; }
    return false;
  }

  std::string str;
  if ( !msg.SerializePartialToString(&str) ) {
    if ( nullptr != error ) { *error = kSerializeError; }
    return false;
  }

  raw->resize(str.size() + kHeaderSize);
  memcpy(raw->data(), kProtoMagic.constData(), kMagicSize);

  quint32* pSize = reinterpret_cast<quint32*>(raw->data() + kMagicSize);
  *pSize = htonl(str.size());

  memcpy(raw->data() + kHeaderSize, str.data(), str.size());

  if ( nullptr != error ) { *error = kNoError; }
  return true;
}

} // binproto
} // meteo
