#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_TCPSTREAM_RAWDATADRIVER_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_TCPSTREAM_RAWDATADRIVER_H

#include <qvector.h>

#include <meteo/commons/msgparser/rawparser.h>

class QByteArray;
class QDateTime;

namespace meteo {
namespace tlg {

class MessageNew;

} // tlg
} // meteo

namespace meteo {

class AppStatusThread;

//! Базовый класс для преобразования исходного потока данных с устройства в метеосообщения.
class RawDataDriver
{
public:
  explicit RawDataDriver(AppStatusThread* status)
  {
    status_ = status;
    nCrcError_ = 0;
  }
  virtual ~RawDataDriver(){}

  void setPrefix(const QString& prefix) { prefix_ = prefix; }

  virtual void setData(const QByteArray& data) = 0;
  virtual void appendData(const QByteArray& data) = 0;

  virtual bool parse() = 0;
  virtual bool getMessage(tlg::MessageNew* msg) = 0;

  int crcErrorCount() const { return nCrcError_; }

protected:
  bool mkMsg(tlg::MessageNew* msg, const QDateTime& dt, const QByteArray& data) const;

  void addCrcError(int n) { nCrcError_ += n; }

protected:
  AppStatusThread* status_;
  QString prefix_;
  RawParser rawParser_;

private:
  int nCrcError_;
};

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_TCPSTREAM_RAWDATADRIVER_H
