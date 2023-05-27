#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UNIMASSTREAM_server_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UNIMASSTREAM_server_H

#include <qlist.h>
#include <qmutex.h>
#include <qobject.h>
#include <qbytearray.h>

#include <commons/container/ts_list.h>

#include <meteo/commons/proto/msgstream.pb.h>
#include <meteo/commons/msgstream/streammodule.h>

class QThread;

namespace meteo {
class TlgParser;
  class UnimasServer;
} // meteo

namespace meteo {

//!
class UnimasStreamServer : public StreamModule
{
  Q_OBJECT



public:
  static QString moduleId() { return QString("socketspecial"); }

public:
  UnimasStreamServer(QObject* p = nullptr);
  virtual ~UnimasStreamServer();

  virtual void setOptions(const msgstream::Options &options);

public slots:
  bool slotInit();

private slots:
  virtual void slotNewIncoming();

protected:
  //virtual void timerEvent(QTimerEvent* event);

private:
  bool init();

  msgstream::Options opt_;
  UnimasServer* server_ = nullptr;

};


} // meteo


#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UNIMASSTREAM_H
