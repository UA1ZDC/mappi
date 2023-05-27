#ifndef METEO_COMMONS_MSGPARSER_COMMON_H
#define METEO_COMMONS_MSGPARSER_COMMON_H

#include <qbytearray.h>
#include <qstringlist.h>

#include <meteo/commons/proto/msgparser.pb.h>

namespace meteo {
namespace tlg {

//!
Priority priority( const MessageNew& msg );
//!
MsgType msgtype( const MessageNew& msg );
//! Возвращает список адресов назначения; строка "*" соответствует циркулярному сообщению.
QStringList targets( const MessageNew& msg );
//! Возвращает идентификатор ЦКС, который сформировал собщение.
QString creator(const MessageNew& msg);
//! Расчитывает md5-хэш для телеграммы (заголовок + сообщение).
QByteArray md5hash(const MessageNew& msg);
//! Возвращает сокращённый заголовок:
//! для ГМС:
//!   циркулярный  - "TTAAii CCCC YYGGgg( BBB)"
//!   адресованные - "TTDDDD CCCC YYGGgg"
//! для ВМО:
//!   циркулярный  - "TTAAii CCCC YYGGgg( BBB)"
//!   адресованные - "TTAAii DDDD YYGGgg СССС"
//! note: Остсутствующие значения заголовка заполняются пробелами.
QString ahd(const MessageNew& msg);

//!
QByteArray raw2tlg( const MessageNew& msg );
//!
QByteArray proto2tlg( const MessageNew& msg );
//!
MessageNew tlg2proto( const QByteArray& arr );

QByteArray proto2image( const MessageNew& msg );
QByteArray tlg2image( const QByteArray& msg );

}
}

#endif
