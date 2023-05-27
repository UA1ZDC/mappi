#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UNIMASSTREAM_CLIENT_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UNIMASSTREAM_CLIENT_H

#include <qlist.h>
#include <qmutex.h>
#include <qobject.h>
#include <qbytearray.h>

#include <commons/container/ts_list.h>

#include <meteo/commons/msgstream/streammodule.h>

class QThread;

namespace meteo {
class UnimasReceiver;

class UnimasStreamClient : public StreamModule
{
  Q_OBJECT
public:
  static QString moduleId() { return QString("socketspecial"); }

public:
  UnimasStreamClient(QObject* p = nullptr);
  virtual ~UnimasStreamClient();

  virtual void setOptions(const msgstream::Options &options);
public slots:
  bool slotInit();
private slots:
  void slotStreamFinihsed();
  virtual void slotNewIncoming();
private:
  bool initClient();
private:
  QThread* networkThread_  = nullptr;
  TSList<QByteArray> dataStorage_;
  msgstream::Options opt_;
  UnimasReceiver* client_ = nullptr;
};

} // meteo


#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UNIMASSTREAM_H
