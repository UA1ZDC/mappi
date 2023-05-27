#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UnimasReceiver_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UnimasReceiver_H

#include "unimasbase.h"

#include <qobject.h>
#include <QtNetwork/qtcpsocket.h>

#include <commons/container/ts_list.h>
#include <meteo/commons/global/appstatusthread.h>
#include <meteo/commons/proto/msgstream.pb.h>

class QTimer;
class QTcpSocket;

namespace meteo {

//class DataStorage;

//! Обеспечивает информационный обмен с MTS-,Unimas-сервером.
class UnimasReceiver : public UnimasBase
{
  Q_OBJECT

public:
public:
  UnimasReceiver(const msgstream::Options& opt,
                TSList<tlg::MessageNew> *outgoing, TSList<tlg::MessageNew> *incoming,
                AppStatusThread* status,
                QObject* p = nullptr);
  virtual ~UnimasReceiver();

  virtual void stop();

public slots:
  virtual void slotStart();
private slots:
  void slotConnected();
  virtual void slotReceiveTimeout();
  virtual void slotReconnectTimeout();


};

} // meteo


#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UnimasReceiver_H
