#include "mapservice.h"

#include <qbuffer.h>
#include <qpainter.h>
#include <qdatetime.h>
#include <qtconcurrentrun.h>
#include <qmessagebox.h>

#include <sql/nosql/nosqlquery.h>
#include <commons/obanal/tfield.h>
#include <commons/textproto/pbtools.h>

#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/map/commongrid.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/ui/map/loader.h>
#include <meteo/commons/ui/map/layeriso.h>

#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/ui/map/weather.h>
#include <meteo/commons/ui/map/aerodocument.h>
#include <meteo/commons/ui/map/vprofiledoc.h>
#include <meteo/commons/msgparser/tlgparser.h>

#include <meteo/commons/rpc/channel.h>

#include "methodbag.h"
#include "appstatus.h"
#include "tfaxdb.h"

namespace meteo {
namespace map {

Service::Service()
  : proto::DocumentService(),
  methodbag_( new MethodBag(this) )
{
  Loader::instance()->registerCitiesHandler(meteo::global::kCitiesLoaderCommon, &meteo::map::loadCitiesLayer);
}

Service::~Service()
{
  delete methodbag_; methodbag_ = nullptr;
}

void Service::CreateDocument( google::protobuf::RpcController* c,
                              const proto::Document* req,
                              proto::Response* resp,
                              google::protobuf::Closure* d )
{
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, methodbag_, &MethodBag::CreateDocument, qMakePair( handler, req ), resp );
}

void Service::GetDocument( google::protobuf::RpcController* c,
                           const proto::ExportRequest* req,
                           proto::ExportResponse* resp,
                           google::protobuf::Closure* d )
{
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, methodbag_, &MethodBag::GetDocument, qMakePair( handler, req ), resp );
}

void Service::RemoveDocument( google::protobuf::RpcController* c,
                              const proto::Document* req,
                              proto::Response* resp,
                              google::protobuf::Closure* d )
{
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, methodbag_, &MethodBag::RemoveDocument, qMakePair( handler, req ), resp );
}

void Service::SaveDocument( google::protobuf::RpcController* c,
                            const proto::ExportRequest* req,
                            proto::Response* resp,
                            google::protobuf::Closure* d )
{
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, methodbag_, &MethodBag::SaveDocument, qMakePair( handler, req ), resp );
}

void Service::ImportDocument(google::protobuf::RpcController *c, const proto::ImportRequest *req, proto::Response *resp, google::protobuf::Closure *d)
{
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, methodbag_, &MethodBag::ImportDocument, qMakePair( handler, req ), resp );
}

void Service::CreateMap( google::protobuf::RpcController* c,
                         const proto::Map* req,
                         proto::Response* resp,
                         google::protobuf::Closure* d )
{
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, methodbag_, &MethodBag::CreateMap, qMakePair( handler, req ), resp );
}

void Service::ProcessJob(::google::protobuf::RpcController* c,
                       const ::meteo::map::proto::Job* req,
                       ::meteo::map::proto::ExportResponse* resp,
                       ::google::protobuf::Closure* d)
{
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
    return;
  }
  methodbag_->ProcessJob( req, resp );
  d->Run();
}

void Service::GetAvailableDocuments( google::protobuf::RpcController* c,
                                  const proto::MapRequest* req,
                                  proto::MapList* resp,
                                  google::protobuf::Closure* d )
{
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, methodbag_, &MethodBag::GetAvailableDocuments, req, resp );
}

void Service::GetLastJobs(google::protobuf::RpcController *c, const proto::Dummy *req, proto::JobList *resp, google::protobuf::Closure *d)
{
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, methodbag_, &MethodBag::GetLastJobs, req, resp );
}

void Service::GetAvailableClimat(google::protobuf::RpcController *c, const proto::ClimatRequest *req, proto::ClimatResponse *resp, google::protobuf::Closure *d)
{
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, methodbag_, &MethodBag::GetAvailableClimat, req, resp );
}

void Service::GetWeatherJobs( google::protobuf::RpcController* c,
                              const proto::JobList* req,
                              proto::JobList* resp,
                              google::protobuf::Closure* d )
{
  auto handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ){
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
  }
  else {
    handler->runMethod( c, methodbag_, &MethodBag::GetWeatherJobs, req, resp );
  }
}

void Service::SetWeatherJobs(google::protobuf::RpcController *c, const proto::JobList *req, proto::Response *resp, google::protobuf::Closure *d)
{
  auto handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ){
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
  }
  else {
    handler->runMethod( c, methodbag_, &MethodBag::SetWeatherJobs, req, resp );
  }
}

void Service::GetFaxes( google::protobuf::RpcController* c,
                        const proto::FaxRequest* req,
                        proto::FaxReply* resp,
                        google::protobuf::Closure* d )
{
  Q_UNUSED(c);
  QString error;

  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
    return;
  }

  handler->runMethod( c, methodbag_, &MethodBag::GetFaxes, qMakePair( handler, req ), resp );
}


void Service::GetSateliteImageList( google::protobuf::RpcController* c,
                           const proto::GetSateliteImageListRequest* req,
                           proto::GetSateliteImageListReply* resp,
                           google::protobuf::Closure* d )
{
  auto handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ){
    auto emessage = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << emessage;
    resp->set_comment(emessage.toStdString());
    resp->set_result(false);
    d->Run();
  }
  else {
    handler->runMethod( c, methodbag_, &MethodBag::GetSateliteImageList, req, resp );
  }

}


void Service::GetSateliteImageFile( google::protobuf::RpcController* c,
                           const proto::GetSateliteImageRequest* req,
                           proto::GetSateliteImageReply* resp,
                           google::protobuf::Closure* d )
{
  auto handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ){
    auto emessage = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << emessage;
    resp->set_comment(emessage.toStdString());
    resp->set_result(false);
    d->Run();
  }
  else {
    handler->runMethod( c, methodbag_, &MethodBag::GetSateliteImageFile, req, resp );
  }
}

void Service::GetTile( ::google::protobuf::RpcController* c,
                       const ::meteo::map::proto::TileRequest* req,
                       ::meteo::map::proto::TileResponse* resp,
                       ::google::protobuf::Closure* d)
{
  auto handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
  }
  else {
    handler->runMethod( c, methodbag_, &MethodBag::GetTile, req, resp );
  }
}

void Service::GetPunches( ::google::protobuf::RpcController* c,
                       const ::meteo::map::proto::PunchRequest* req,
                       ::meteo::map::proto::PunchResponse* resp,
                       ::google::protobuf::Closure* d )
{
  auto handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ){
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
  }
  else {
    handler->runMethod( c, methodbag_, &MethodBag::GetPunches, req, resp );
  }
}

void Service::GetPunchMaps(::google::protobuf::RpcController* c,
                    const ::meteo::map::proto::Map* req,
                    ::meteo::map::proto::MapList* resp,
                    ::google::protobuf::Closure* d)
{
  auto handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ){
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
  }
  else {
    handler->runMethod( c, methodbag_, &MethodBag::GetPunchMaps, req, resp );
  }
}
void Service::GetOrnament(::google::protobuf::RpcController* c,
                   const ::meteo::map::proto::Dummy* req,
                   ::meteo::map::proto::Ornaments* resp,
                   ::google::protobuf::Closure* d)
{
  auto handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ){
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
  }
  else {
    handler->runMethod( c, methodbag_, &MethodBag::GetOrnament, req, resp );
  }
}

}
}
