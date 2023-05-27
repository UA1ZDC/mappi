#ifndef FILESERVICE_H
#define FILESERVICE_H

#include <qobject.h>

#include <meteo/commons/global/global.h>
#include <mappi/global/global.h>

#include <mappi/proto/fileservice.pb.h>

#include <mappi/settings/mappisettings.h>

namespace mappi {

class FileService: public QObject, public proto::FileService
{
  Q_OBJECT
public:
  FileService();
  ~FileService();

  void ExportFile(::google::protobuf::RpcController* ctrl,
                  const proto::ExportRequest* req,
                  proto::ExportResponse* resp,
                  ::google::protobuf::Closure* done);
public slots:
  void slotOnConnected(meteo::rpc::Channel* channel);
  void slotOnDisconnected(meteo::rpc::Channel* channel);

private:
  bool sendToServer(QString path, proto::ExportResponse* response, google::protobuf::Closure* done, meteo::rpc::Channel* channel);

  QList<meteo::rpc::Channel*> clients_;

};

} //mappi

#endif // FILESERVICE_H
