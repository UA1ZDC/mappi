#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_socksp2g_SERVER_TRANSMETSTREAM_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_socksp2g_SERVER_TRANSMETSTREAM_H

#include <commons/container/ts_list.h>

#include <meteo/commons/msgstream/plugins/socketspec2g/socksp2g.h>
#include <meteo/commons/msgstream/plugins/socketspec2g/socksp2gsender.h>
#include <meteo/commons/msgstream/streammodule.h>

namespace meteo {

class Socksp2gServer;
//!
class Socksp2gStreamServer : public StreamModule
{
  Q_OBJECT
public:
  static QString moduleId() { return QString("socksp2g"); }

public:
  explicit Socksp2gStreamServer(QObject *parent = 0);
  virtual ~Socksp2gStreamServer();

  virtual void setOptions(const msgstream::Options &options);
public slots:
  virtual bool slotInit();

private slots:
  virtual void slotNewIncoming();

private:

  msgstream::Options opt_;
  Socksp2gServer* server_ = nullptr;

};

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_socksp2g_TRANSMETSTREAM_H
