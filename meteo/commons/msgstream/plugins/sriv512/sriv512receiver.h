#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_SRIV512_TRANSMETCLIENT_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_SRIV512_TRANSMETCLIENT_H

#include <QtNetwork/qtcpsocket.h>

#include <meteo/commons/msgstream/plugins/sriv512/sriv512base.h>

namespace meteo {

//!
class Sriv512Receiver : public Sriv512Base
{
  Q_OBJECT
public:
  //!
  explicit Sriv512Receiver(const msgstream::Options& opt,
                            TSList<tlg::MessageNew>* outgoing,
                            TSList<tlg::MessageNew>* incoming,
                            AppStatusThread* status,
                           QObject* p = nullptr);
  virtual ~Sriv512Receiver();
  virtual void stop();

public slots:
  virtual void slotStart();

private slots:
  void slotConnected();
  virtual void slotReceiveTimeout();
  virtual void slotReconnectTimeout();


private:
  void stopv();

};

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_SRIV512_TRANSMETCLIENT_H
