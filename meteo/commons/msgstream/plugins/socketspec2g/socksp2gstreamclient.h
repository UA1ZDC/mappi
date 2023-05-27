#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_Socksp2gStreamClientTRANSMETSTREAM_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_Socksp2gStreamClientTRANSMETSTREAM_H

#include <commons/container/ts_list.h>

#include <meteo/commons/msgstream/plugins/socketspec2g/socksp2g.h>
#include <meteo/commons/msgstream/streammodule.h>

namespace meteo {


  class Socksp2gReceiver;
  //class SendBox;

//!
class Socksp2gStreamClient : public StreamModule
{
  Q_OBJECT

public:
  static QString moduleId() { return QString("socksp2g"); }

  explicit Socksp2gStreamClient(QObject *parent = 0);
  virtual ~Socksp2gStreamClient();
  void setOptions(const msgstream::Options& options);
public slots:
  virtual bool slotInit();
private slots:
  virtual void slotNewIncoming();
  void slotStreamFinihsed();
private:
  TSList<QByteArray> dataStorage_;
  Socksp2gReceiver* client_ = nullptr;
  QThread* networkThread_  = nullptr;
  msgstream::Options opt_;
};

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_SRIV512_TRANSMETSTREAM_H
