#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_Socksp2g_TRANSMETSERVER_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_Socksp2g_TRANSMETSERVER_H

#include <QtNetwork/qtcpserver.h>

#include "socksp2gbase.h"

namespace meteo {

class Socksp2gSender : public Socksp2gBase
{
  Q_OBJECT
public:
  explicit Socksp2gSender(qintptr socket,
                         const msgstream::Options &opt,
                         TSList<tlg::MessageNew> *outgoing,
                         TSList<tlg::MessageNew> *incoming,
                         AppStatusThread *status,
                         QObject *parent = nullptr);
  virtual ~Socksp2gSender();
  void stop();
 public slots:
  virtual void slotStart();
  virtual void slotReceiveTimeout();
  virtual void slotReconnectTimeout();
signals:
  void finished();
 private:
  void stopv();
  qintptr socket_descriptor_=-1;
};

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_SRIV512_TRANSMETSERVER_H
