#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_SRIV512_TRANSMETSERVER_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_SRIV512_TRANSMETSERVER_H

#include <QtNetwork/qtcpserver.h>

#include "sriv512base.h"

namespace meteo {

class Sriv512Sender : public Sriv512Base
{
  Q_OBJECT
public:
  explicit Sriv512Sender(qintptr socket,
                         const msgstream::Options &opt,
                         TSList<tlg::MessageNew> *outgoing,
                         TSList<tlg::MessageNew> *incoming,
                         AppStatusThread *status,
                         QObject *parent = nullptr);
  virtual ~Sriv512Sender();
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
