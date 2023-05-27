#include "thread.h"

#include <cross-commons/debug/tlog.h>
#include <mappi/global/global.h>

Thread::Thread( const std::string& path )
  : QThread(),
  path_(path)
{
}

Thread::~Thread()
{
  delete ch_; ch_ = 0;
}

void Thread::slotResponse( mappi::proto::ExportResponse* response )
{
  arr_.append( response->data().data(), response->data().size() );
  if ( true == response->finish() ) {
    result_ = true;
    QThread::quit();
  }
}

void Thread::run()
{


  mappi::proto::ExportRequest request;
  request.set_path(path_);
  ch_ = meteo::global::serviceChannel( meteo::settings::proto::kFile );
  if ( 0 == ch_ ) {
    error_log << QObject::tr("Не удалось установить соединение с сервисов передачи файлов");
    return;
  }
  bool res = ch_->subscribe( &mappi::proto::FileService::ExportFile, request, this, &Thread::slotResponse );
  if ( false == res ) {
    error_log << QObject::tr("Не удалось выполнить запрос");
  }
  QThread::exec();
}
