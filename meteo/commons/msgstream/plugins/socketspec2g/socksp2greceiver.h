#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_socksp2gsendrecv_TRANSMETCLIENT_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_socksp2gsendrecv_TRANSMETCLIENT_H

#include <QtNetwork/qtcpsocket.h>

#include <meteo/commons/msgstream/plugins/socketspec2g/socksp2gbase.h>

namespace meteo {

//!
class Socksp2gReceiver : public Socksp2gBase
{
  Q_OBJECT
public:
  //!
  explicit Socksp2gReceiver(const msgstream::Options& opt,
                            TSList<tlg::MessageNew>* outgoing,
                            TSList<tlg::MessageNew>* incoming,
                            AppStatusThread* status,
                           QObject* p = nullptr);
  virtual ~Socksp2gReceiver();
  virtual void stop();

public slots:
  virtual void slotStart();

private slots:
  void stopv();
  void slotConnected();
  virtual void slotReceiveTimeout();
  virtual void slotReconnectTimeout();


private:

};

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_SRIV512_TRANSMETCLIENT_H
