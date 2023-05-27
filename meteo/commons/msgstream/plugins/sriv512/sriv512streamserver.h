#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_SRIV512_SERVER_TRANSMETSTREAM_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_SRIV512_SERVER_TRANSMETSTREAM_H

#include <commons/container/ts_list.h>

#include <meteo/commons/msgstream/plugins/sriv512/sriv512.h>
#include <meteo/commons/msgstream/plugins/sriv512/sriv512sender.h>
#include <meteo/commons/msgstream/streammodule.h>

namespace meteo {

class Sriv512Server;
//!
class Sriv512StreamServer : public StreamModule
{
  Q_OBJECT
public:
  static QString moduleId() { return QString("sriv512"); }

public:
  explicit Sriv512StreamServer(QObject *parent = 0);
  virtual ~Sriv512StreamServer();

  virtual void setOptions(const msgstream::Options &options);
public slots:
  virtual bool slotInit();

private slots:
  virtual void slotNewIncoming();

private:

  msgstream::Options opt_;
  Sriv512Server* server_ = nullptr;

};

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_SRIV512_TRANSMETSTREAM_H
