#include "fileservice.h"

namespace mappi {

FileService::FileService():
  QObject(nullptr),
  proto::FileService()
{

}

FileService::~FileService()
{

}

void FileService::ExportFile(google::protobuf::RpcController *ctrl, const proto::ExportRequest *req, proto::ExportResponse *resp, google::protobuf::Closure *done)
{
  Q_UNUSED( ctrl );
    
  meteo::rpc::Controller* tctrl = static_cast<meteo::rpc::Controller*>(ctrl);
  if ( 0 == tctrl->channel() ) {
    QString err = QObject::tr("Не найден канал связи для выполнения удаленного вызова.");
    error_log << err;
    resp->set_result(false);
    resp->set_comment( err.toUtf8().data() );
    done->Run();
    return;
  }

  QFile file( QString::fromStdString(req->path()));
  if ( file.exists() ) {
    /*if ( -1 == QString::fromStdString(req->path()).indexOf("var/mappi/")) {
      resp->set_result( false );
      resp->set_comment( "error" );
      resp->set_finish( true );
      error_log << QObject::tr("Задан некорректный файл");
      done->Run();
      return;
    }*/
    resp->set_comment("found");
    resp->set_filename(req->path());
    resp->set_file_size(file.size());
    resp->set_start(true);
    done->Run();
    resp->set_start(false);

    bool res = sendToServer(QString::fromStdString(req->path()), resp, done, tctrl->channel());

    if ( true == res ) {
      resp->set_result( true );
      resp->set_comment( "ok" );
      resp->set_finish( true );
      resp->clear_complete();
    }
    else {
      resp->set_result( false );
      resp->set_comment( "error" );
      resp->set_finish( true );
      error_log << QObject::tr("Ошибка при передаче файла!");
    }
    done->Run();
  }
  else {
    error_log << QObject::tr("Файл не найден!");
    resp->set_result(false);
    resp->set_comment("not found");
    resp->set_filename( req->path() );
    done->Run();
  }
}

  void FileService::slotOnConnected(meteo::rpc::Channel* channel)
{
  if ( 0 != channel ) {
    clients_.append(channel);
  }
}

void FileService::slotOnDisconnected(meteo::rpc::Channel *channel)
{
  if ( clients_.contains(channel) ) {
    clients_.removeOne(channel);
  }
}

bool FileService::sendToServer(QString path, proto::ExportResponse* response, google::protobuf::Closure* done, meteo::rpc::Channel* channel)
{
  QFile file(path);
  if ( false == file.open(QIODevice::ReadOnly) ){
    error_log << QObject::tr("Ошибка. Файл %1: %2").arg(file.fileName()).arg(file.errorString());
//    emit done();
    return false;
  }

  response->set_comment("data");

  const int64_t fileSize = file.size();
  int64_t chunkSize = 1048576;

  int64_t totalReaded = 0;
  int64_t chunkNum = 0;
  response->set_chunk_size(chunkSize);
  QByteArray ba(chunkSize, '\0');

  int64_t readed = file.read(ba.data(), chunkSize);
  while (readed > 0) {
    totalReaded += readed;
    int64_t percent = 100 * totalReaded / fileSize;

    if ( clients_.contains(channel) ) {
      response->set_chunk_num(chunkNum++);
      response->set_complete(percent);
      response->set_data(ba.left(readed).data(), readed);
      response->set_data_size(readed);
      done->Run();
    }
    else {
      return false;
    }

    ba.fill('\0');
    readed = file.read(ba.data(), chunkSize);
  }

//  emit done();
  return true;
}

} //mappi
