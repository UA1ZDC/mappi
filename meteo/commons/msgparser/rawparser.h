#ifndef METEO_COMMONS_MSGPARSER_RAWPARSER_H
#define METEO_COMMONS_MSGPARSER_RAWPARSER_H

#include <qbytearray.h>
#include <qstringlist.h>

#include <meteo/commons/msgparser/rawparser.pb.h>

namespace meteo {

class RawParser
{
public:
  enum ByteOrder { kHostOrder, kNetOrder };

  RawParser();

  void setByteOrder(ByteOrder order)                    { byteOrder_ = order; }
  void setFormat(const msgparser::RawBinFormat& format) { format_ = format; }

  void setData(const QByteArray& rawdata)               { buffer_ = rawdata;  }
  void appendData(const QByteArray& rawdata)            { buffer_ += rawdata; }
  void prependData(const QByteArray& rawdata)           { buffer_.prepend(rawdata); }

  //! Возвращает true, если буфере был найден и успешно разобран блок данных соответствующий установленному формату.
  //! Если блок данных был успешно найден, то в *rawData будет записан оригинальный блок данных.
  bool parse(::google::protobuf::Message* msg, QByteArray* rawData = 0);

private:
  bool setValue(::google::protobuf::Message* msg, const QStringList& path, const QByteArray& raw) const;

private:
  msgparser::RawBinFormat format_;
  ByteOrder byteOrder_;
  QByteArray buffer_;
};

} // meteo

#endif // METEO_COMMONS_MSGPARSER_RAWPARSER_H
