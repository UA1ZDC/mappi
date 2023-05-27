#ifndef socksp2gSERVER_H
#define socksp2gSERVER_H

#include <qtcpserver.h>

#include <commons/container/ts_list.h>

#include <meteo/commons/proto/msgstream.pb.h>
#include <meteo/commons/msgstream/streammodule.h>

#include "socksp2g.h"
#include "socksp2gsender.h"


namespace meteo {

class Socksp2gSender;

class Socksp2gServer : public QTcpServer
{
  Q_OBJECT

public:
  explicit Socksp2gServer(AppStatusThread* , StreamModule *p = nullptr);

  virtual ~Socksp2gServer() override;
  void init(const msgstream::Options& options,
                           TSList<tlg::MessageNew> *outgoing ,
                           TSList<tlg::MessageNew>* incoming);

private slots:
  void slotStreamFinihsed();
  void slotMsgReceived(qint64 r)  {emit signalMessagesRecv(r);}
  void slotSizeReceived(qint64 r) {emit signalBytesRecv(r);}
  void slotMsgSended(qint64 r)    {emit signalMessagesSended(r);}
  void slotSizeSended( qint64 r)  {emit signalBytesSended(r);}

private:
  void incomingConnection(qintptr socketDescriptor) override;

  msgstream::Options opt_;
  Socksp2gSender* sender_ = nullptr;
  TSList<tlg::MessageNew>* incoming_ = nullptr;
  TSList<tlg::MessageNew>* outgoing_ = nullptr;
  StreamModule* stream_module_ = nullptr;
  AppStatusThread* status_ = nullptr;
  QThread* networkThread_  = nullptr;

signals:
  void signalBytesRecv(qint64);
  void signalMessagesRecv(qint64);
  void signalBytesSended(qint64);
  void signalMessagesSended(qint64);
  void signalConfirmSended(qint64);
  void signalConfirmRecv(qint64);
};

}
#endif
