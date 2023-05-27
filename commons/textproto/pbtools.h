#ifndef COMMONS_TEXTPROTO_PBTOOLS_H
#define COMMONS_TEXTPROTO_PBTOOLS_H

#include <qstringlist.h>
#include <qdatetime.h>
#include <qcryptographichash.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/repeated_field.h>

namespace google {
namespace protobuf {
class Message;
} // protobuf
} // google

namespace pbtools {

inline QDateTime toQDateTime(const std::string& str){
  return QDateTime::fromString(QString::fromStdString(str), Qt::ISODate);
}

inline std::string toString(const QDateTime& dt)
{
  return dt.toString(Qt::ISODate).toStdString();
}


//! Возвращает utf8-строку.
inline std::string toString(const QString& str)
{ QByteArray ba = str.toUtf8(); return std::string(ba.constData(), ba.length()); }

//!
inline QString toQString(const std::string& str)
{ return QString::fromUtf8(str.c_str(), str.length()); }

inline QStringList toQStringList( const google::protobuf::RepeatedField<std::string>& list )
{
  QStringList result;
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    result << toQString( list.Get(i) );
  }
  return result;
}

//!
inline std::string toBytes(const QByteArray& ba)
{ return std::string(ba.constData(), ba.size()); }

//!
inline QByteArray fromBytes(const std::string& str)
{ return QByteArray(str.data(), str.size()); }

//!
inline QString md5hash(const google::protobuf::Message& msg)
{
  std::string str;
  msg.SerializePartialToString(&str);

  QByteArray ba = QByteArray::fromRawData(str.data(), str.size());

  return QCryptographicHash::hash(ba, QCryptographicHash::Md5).toHex();
}

} // pbtools


//! Класс PbHelper предоставляет интерфейс для установки и получения полей протоструктур.
class PbHelper
{
public:
  //!
  PbHelper(google::protobuf::Message* msg) : msg_(msg) {}

  //! При установке значения values будет преобразовано к типу поля.
  //! Возвращает false если:
  //!  - поле не найдено
  //!  - поле не repeated
  //!  - value невозможно привести к типу поля
  bool setValue(const QStringList& fieldPath, const QStringList& values);
  //! При установке значения value будет преобразовано к типу поля.

  //! Возвращает false если поле не найдено или value невозможно привести к типу поля.
  bool setValue(const QStringList& fieldPath, const QString& value);
  //! Возвращает false если поле не найдено или value невозможно привести к типу поля.
  bool setValue(const QStringList& fieldPath, const QByteArray& value);

  //! Возвращает false если поле не найдено, поле не repeated, value невозможно привести к типу поля.
  bool addValue(const QStringList& fieldPath, const QString& value);
  //! Возвращает false если поле не найдено, поле не repeated, value невозможно привести к типу поля.
  bool addValue(const QStringList& fieldPath, const QByteArray& value);

  //! Возвращает true, если в протосообщении есть поле расположенное по пути fieldPath.
  bool hasField(const QStringList& fieldPath) const;
  //! Возвращает c++-тип поля расположенного по пути fieldPath.
  google::protobuf::FieldDescriptor::CppType cppType(const QStringList& fieldPath, bool *ok = 0) const;

private:
  bool findField(google::protobuf::Message** resMsg,
                 const google::protobuf::FieldDescriptor** resFd,
                 const QStringList& path) const;

  bool setValue(google::protobuf::Message* msg,
                const google::protobuf::FieldDescriptor* fd,
                const QStringList& values) const;

  bool setValue(google::protobuf::Message* msg,
                const google::protobuf::FieldDescriptor* fd,
                const QList<QByteArray>& values) const;

  bool setValue(google::protobuf::Message* msg,
                const google::protobuf::FieldDescriptor* fd,
                const QString& value) const;

  bool setValue(google::protobuf::Message* msg,
                const google::protobuf::FieldDescriptor* fd,
                const QByteArray& value) const;

  bool addValue(google::protobuf::Message* msg,
                const google::protobuf::FieldDescriptor* fd,
                const QString& value) const;

  bool addValue(google::protobuf::Message* msg,
                const google::protobuf::FieldDescriptor* fd,
                const QByteArray& value) const;

private:
  google::protobuf::Message* msg_;
};


template<class T> bool pbSetValue(google::protobuf::Message* msg, const QStringList& path, const T& value)
{
  PbHelper help(msg);
  return help.setValue(path, value);
}

template<class T> bool pbAddValue(google::protobuf::Message* msg, const QStringList& path, const T& value)
{
  PbHelper help(msg);
  return help.addValue(path, value);
}

#endif // METEO_COMMONS_TEXTPROTO_PBTOOLS_H
