#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UNIMASSENDER_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UNIMASSENDER_H

#include "unimasbase.h"

#include <commons/container/ts_list.h>

#include <meteo/commons/proto/msgparser.pb.h>
#include <meteo/commons/global/appstatusthread.h>
#include <meteo/commons/proto/msgstream.pb.h>

#include <qlist.h>
#include <qpair.h>
#include <qtimer.h>
#include <qdatetime.h>

class QTimer;
class QTcpSocket;

namespace meteo {
class AppStatus;
} // meteo

namespace meteo {

class UnimasSender : public UnimasBase
{
  Q_OBJECT

public:
  UnimasSender(int socket, const msgstream::Options &opt,
               TSList<tlg::MessageNew> *outgoing,
               TSList<tlg::MessageNew> *incoming,
               AppStatusThread *status,
               QObject *p = nullptr);
  ~UnimasSender()override;
  //void setSocket(QTcpSocket* s);

  bool processOutput();

  void stop();

public slots:
  virtual void slotStart();

private slots:
  virtual void slotResendTimeout();
  virtual void slotReceiveTimeout();
  virtual void slotReconnectTimeout();

signals:
  void finished();

private:
  int socket_descriptor_=-1;


};

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UNIMASSENDER_H
