#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_SRIV512_Sriv512StreamClientTRANSMETSTREAM_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_SRIV512_Sriv512StreamClientTRANSMETSTREAM_H

#include <commons/container/ts_list.h>

#include <meteo/commons/msgstream/plugins/sriv512/sriv512.h>
#include <meteo/commons/msgstream/streammodule.h>

namespace meteo {


  class Sriv512Receiver;
  //class SendBox;

//!
class Sriv512StreamClient : public StreamModule
{
  Q_OBJECT

public:
  static QString moduleId() { return QString("sriv512"); }

  explicit Sriv512StreamClient(QObject *parent = 0);
  virtual ~Sriv512StreamClient();
  void setOptions(const msgstream::Options& options);
public slots:
  virtual bool slotInit();
private slots:
  virtual void slotNewIncoming();
  void slotStreamFinihsed();
private:
  TSList<QByteArray> dataStorage_;
  Sriv512Receiver* client_ = nullptr;
  QThread* networkThread_  = nullptr;
  msgstream::Options opt_;
};

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_SRIV512_TRANSMETSTREAM_H
