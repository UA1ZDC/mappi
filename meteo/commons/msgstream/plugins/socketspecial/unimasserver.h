#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UNIMASSERVER_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UNIMASSERVER_H

#include <qtcpserver.h>

#include <commons/container/ts_list.h>

#include <meteo/commons/proto/msgstream.pb.h>
#include <meteo/commons/msgstream/streammodule.h>

#include "unimas.h"
#include "unimassender.h"


namespace meteo {

class UnimasServer : public QTcpServer
{
  Q_OBJECT

public:
  explicit UnimasServer(AppStatusThread *status,
                        StreamModule* p = nullptr);
  virtual ~UnimasServer() override;
  void init(const msgstream::Options& options,
            TSList<tlg::MessageNew>* outgoing,
            TSList<tlg::MessageNew>* incoming);


private slots:
  void slotStreamFinihsed();
  void slotMsgReceived(qint64 r)  {emit signalMessagesRecv(r);}
  void slotSizeReceived(qint64 r) {emit signalBytesRecv(r);}
  void slotMsgSended(qint64 r)    {emit signalMessagesSended(r);}
  void slotSizeSended( qint64 r)  {emit signalBytesSended(r);}

private:
  void incomingConnection(qintptr socketDescriptor) override;

  UnimasSender* sender_ = nullptr;
  msgstream::Options opt_;
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

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UNIMASSERVER_H
