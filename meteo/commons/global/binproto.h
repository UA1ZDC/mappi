#ifndef METEO_COMMONS_GLOBAL_WEBSOCKET_H
#define METEO_COMMONS_GLOBAL_WEBSOCKET_H

#include <qobject.h>
#include <qbytearray.h>
#include <cross-commons/debug/tmap.h>

namespace google {
namespace protobuf {

class Message;

} // protobuf
} // google

namespace meteo {
namespace binproto {

const QByteArray kProtoMagic  = "proto";
const int kMagicSize          = kProtoMagic.size();
const int kHeaderSize         = kMagicSize + sizeof(quint32);

enum Error {
  kNoError,         //!< нет ошибок
  kNullPointer,     //!< нулевой указатель возвращаемого значения

  kMagicNotFound,   //!< отсутствует идентификатор начала сообщения
  kIncorrectSize,   //!< размер исходных данных меньше ожидаемого размера сообщения
  kParseError,      //!< не удалось распарсить сообщение

  kSerializeError,  //!< не удалось сериализовать сообщение
};

const TMap<int,QString> kErrorString = TMap<int,QString>()
<< qMakePair(kNoError, QString(""))
<< qMakePair(kNullPointer, QObject::tr("Нулевой указатель возвращаемого значения"))
<< qMakePair(kMagicNotFound, QObject::tr("Отсутствует идентификатор начала сообщения"))
<< qMakePair(kIncorrectSize, QObject::tr("Размер исходных данных меньше ожидаемого размера сообщения"))
<< qMakePair(kParseError, QObject::tr("Ошибка при разборе сообщения"))
<< qMakePair(kSerializeError, QObject::tr("Ошибка при сериализации сообщения"))
   ;

bool parsePartial(const QByteArray& raw, ::google::protobuf::Message* msg, int* error = nullptr);
bool serializePartial(const ::google::protobuf::Message& msg, QByteArray* raw, int* error = nullptr);

} // binproto
} // meteo

#endif // METEO_COMMONS_GLOBAL_WEBSOCKET_H
