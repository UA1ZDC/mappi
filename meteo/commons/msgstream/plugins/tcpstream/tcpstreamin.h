#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_TCPSTREAM_TCPSTREAMIN_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_TCPSTREAM_TCPSTREAMIN_H

#include <qstringlist.h>

#include <commons/container/ts_list.h>
#include <meteo/commons/proto/msgstream.pb.h>
#include <meteo/commons/msgstream/streammodule.h>

#include "tcpstream.pb.h"

class QTcpSocket;

namespace meteo {
class RawDataDriver;
} // meteo

namespace meteo {

class TcpStreamIn : public StreamModule
{
  Q_OBJECT

  enum StatusParam {
    kRecvBytes = 1024,
    kRecvSpeed,
    kMkMsg,
    kState,
    kCrcError,
    kStateParam = 2048,
  };

public:
  TcpStreamIn(QObject* p = 0);
  virtual ~TcpStreamIn();

  virtual void setOptions(const msgstream::Options &options);

public slots:
  virtual bool slotInit();

private:
  bool run();
  bool reconnect();

private:
  // параметры
  msgstream::Options opt_;
  msgstream::TcpInOptions params_;

  QString id_;
  QString host_;
  int port_;

  QStringList prefixes_;

  // данные
  quint64 stRecvBytes_;
  quint32 stMsgCount_;

  // служебные
  QTcpSocket* sock_;
  RawDataDriver* driver_;
};


} // meteo


#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_TCPSTREAM_TCPSTREAMIN_H
