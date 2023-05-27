#include "tappcontrolservice.h"
#include "tappcontrol.h"
#include <meteo/commons/global/global.h>
#include <qprocess.h>
#include <qmap.h>


namespace meteo {
namespace app {

using namespace google::protobuf;

static const QString HANDLER_NOD_FOUND = QObject::tr("Не удается получить handler для контроллера");

Service::Service() :
    method_(new TapControlServiceMethodBag())
{
}

Service::~Service()
{
  delete method_;
  method_ = nullptr;
}

void Service::GetProcList( google::protobuf::RpcController* c,
                      const meteo::app::Dummy* req,
                      meteo::app::AppState* res,
                      google::protobuf::Closure* d )
{
  Q_UNUSED(c);
  Q_UNUSED(req);
  const QMap<QProcess*, Application>& apps = Control::instance()->applications();
  QMap<QProcess*, Application>::const_iterator it = apps.constBegin();
  QMap<QProcess*, Application>::const_iterator end = apps.constEnd();
  const QMap< QProcess*, QDateTime>& startdates = Control::instance()->startdates();
  const QMap< QProcess*, QDateTime>& stopdates = Control::instance()->stopdates();
  for ( ; it != end; ++it ) {
    QProcess* p = it.key();
    const Application& app = it.value();
    AppState::Proc* proc = res->add_procs();
    OperationStatus status;
    Control::instance()->getStatus( p, &status );
    proc->mutable_status()->CopyFrom(status);
    if ( QProcess::Running != p->state() ) {
      proc->set_state( AppState::PROC_STOPPED );
    }
    else {
      proc->set_state( AppState::PROC_RUNNING );
    }
    if ( kManagerId.toStdString() == app.id() ) {
      proc->set_state( AppState::PROC_RUNNING );
    }
    if ( true == startdates.contains(p) ) {
      proc->set_startdt( startdates[p].toString( Qt::SystemLocaleShortDate ).toStdString() );
    }
    if ( true == stopdates.contains(p) ) {
      proc->set_stopdt( stopdates[p].toString( Qt::SystemLocaleShortDate ).toStdString() );
    }
    proc->set_id( reinterpret_cast<int64_t>(p) );
    proc->mutable_app()->CopyFrom(app);
  }
  d->Run();
}

void Service::StartProc( google::protobuf::RpcController* c,
                  const meteo::app::AppState::Proc* req,
                  meteo::app::Dummy* res,
                  google::protobuf::Closure* d )
{
  Q_UNUSED(c);
  Q_UNUSED(res);
  Control::instance()->startProc( req->id() );
  d->Run();
}

void Service::StopProc( google::protobuf::RpcController* c,
                  const meteo::app::AppState::Proc* req,
                  meteo::app::Dummy* res,
                  google::protobuf::Closure* d )
{
  Q_UNUSED(c);
  Q_UNUSED(res);
  Control::instance()->stopProc( req->id() );
  d->Run();
}

void Service::ProcChanged( google::protobuf::RpcController* c,
                      const meteo::app::Dummy* req,
                      meteo::app::AppState::Proc* res,
                      google::protobuf::Closure* d )
{
  Q_UNUSED(c);
  Q_UNUSED(req);
  Control::instance()->changesSubscribe( res, d );
}

void Service::GetAppOut( google::protobuf::RpcController* c,
                    const meteo::app::AppOutRequest* req,
                    meteo::app::AppOutReply* res,
                    google::protobuf::Closure* d )
{
  Q_UNUSED(c);
  res->set_id( req->id() );
  Control::instance()->appOutSubscribe( res, d );
}

void Service::SetOperaionStatus(google::protobuf::RpcController* c,
                                const meteo::app::OperationStatus* req,
                                meteo::app::Dummy* res,
                                google::protobuf::Closure* d)
{
 Q_UNUSED(c);
 Q_UNUSED(res);
 Control::instance()->changeProc(req);
 d->Run();
}

void Service::AuthUser(google::protobuf::RpcController* ctrl, const UserAuthRequest* req,
  UserAuthResponse* resp, google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(ctrl);
  if (handler == nullptr) {
    resp->set_result(false);
    resp->set_comment(HANDLER_NOD_FOUND.toStdString());
    error_log << HANDLER_NOD_FOUND;
    done->Run();
    return ;
  }

  handler->runMethod(ctrl, method_, &TapControlServiceMethodBag::AuthUser, req, resp);
}

void Service::NewUser(google::protobuf::RpcController* ctrl, const UserRequest* req,
  UserResponse* resp, google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(ctrl);
  if (handler == nullptr) {
    resp->set_result(false);
    resp->set_comment(HANDLER_NOD_FOUND.toStdString());
    error_log << HANDLER_NOD_FOUND;
    done->Run();
    return ;
  }

  handler->runMethod(ctrl, method_, &TapControlServiceMethodBag::NewUser, req, resp);
}

void Service::AddUser(google::protobuf::RpcController* ctrl, const UserRequest* req,
  UserResponse* resp, google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(ctrl);
  if (handler == nullptr) {
    resp->set_result(false);
    resp->set_comment(HANDLER_NOD_FOUND.toStdString());
    error_log << HANDLER_NOD_FOUND;
    done->Run();
    return ;
  }

  handler->runMethod(ctrl, method_, &TapControlServiceMethodBag::AddUser, req, resp);
}

void Service::RegisterUser(google::protobuf::RpcController* ctrl, const UserRequest* req,
  UserResponse* resp, google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(ctrl);
  if (handler == nullptr) {
    resp->set_result(false);
    resp->set_comment(HANDLER_NOD_FOUND.toStdString());
    error_log << HANDLER_NOD_FOUND;
    done->Run();
    return ;
  }

  handler->runMethod(ctrl, method_, &TapControlServiceMethodBag::RegisterUser, req, resp);
}

void Service::UpdateUser(google::protobuf::RpcController* ctrl, const UserRequest* req,
  UserResponse* resp, google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(ctrl);
  if (handler == nullptr) {
    resp->set_result(false);
    resp->set_comment(HANDLER_NOD_FOUND.toStdString());
    error_log << HANDLER_NOD_FOUND;
    done->Run();
    return ;
  }

  handler->runMethod(ctrl, method_, &TapControlServiceMethodBag::UpdateUser, req, resp);
}

void Service::BlockUser(google::protobuf::RpcController* ctrl, const UserRequest* req,
  UserResponse* resp, google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(ctrl);
  if (handler == nullptr) {
    resp->set_result(false);
    resp->set_comment(HANDLER_NOD_FOUND.toStdString());
    error_log << HANDLER_NOD_FOUND;
    done->Run();
    return ;
  }

  handler->runMethod(ctrl, method_, &TapControlServiceMethodBag::BlockUser, req, resp);
}

void Service::UnlockUser(google::protobuf::RpcController* ctrl, const UserRequest* req,
  UserResponse* resp, google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(ctrl);
  if (handler == nullptr) {
    resp->set_result(false);
    resp->set_comment(HANDLER_NOD_FOUND.toStdString());
    error_log << HANDLER_NOD_FOUND;
    done->Run();
    return ;
  }

  handler->runMethod(ctrl, method_, &TapControlServiceMethodBag::UnlockUser, req, resp);
}

void Service::HomeDirUser(google::protobuf::RpcController* ctrl, const UserRequest* req,
  UserResponse* resp, google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(ctrl);
  if (handler == nullptr) {
    resp->set_result(false);
    resp->set_comment(HANDLER_NOD_FOUND.toStdString());
    error_log << HANDLER_NOD_FOUND;
    done->Run();
    return ;
  }

  handler->runMethod(ctrl, method_, &TapControlServiceMethodBag::HomeDirUser, req, resp);
}

void Service::CheckUser(google::protobuf::RpcController* ctrl, const UserRequest* req,
  UserResponse* resp, google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(ctrl);
  if (handler == nullptr) {
    resp->set_result(false);
    resp->set_comment(HANDLER_NOD_FOUND.toStdString());
    error_log << HANDLER_NOD_FOUND;
    done->Run();
    return ;
  }

  handler->runMethod(ctrl, method_, &TapControlServiceMethodBag::CheckUser, req, resp);
}

void Service::UserList(google::protobuf::RpcController* ctrl, const Dummy* req,
  UserResponse* resp, google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(ctrl);
  if (handler == nullptr) {
    resp->set_result(false);
    resp->set_comment(HANDLER_NOD_FOUND.toStdString());
    error_log << HANDLER_NOD_FOUND;
    done->Run();
    return ;
  }

  handler->runMethod(ctrl, method_, &TapControlServiceMethodBag::UserList, req, resp);
}

void Service::RoleList(google::protobuf::RpcController* ctrl, const Dummy* req,
  RoleResponse* resp, google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(ctrl);
  if (handler == nullptr) {
    resp->set_result(false);
    resp->set_comment(HANDLER_NOD_FOUND.toStdString());
    error_log << HANDLER_NOD_FOUND;
    done->Run();
    return ;
  }

  handler->runMethod(ctrl, method_, &TapControlServiceMethodBag::RoleList, req, resp);
}

void Service::RankList(google::protobuf::RpcController* ctrl, const Dummy* req,
  RankResponse* resp, google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(ctrl);
  if (handler == nullptr) {
    resp->set_result(false);
    resp->set_comment(HANDLER_NOD_FOUND.toStdString());
    error_log << HANDLER_NOD_FOUND;
    done->Run();
    return ;
  }

  handler->runMethod(ctrl, method_, &TapControlServiceMethodBag::RankList, req, resp);
}

void Service::PutRank(google::protobuf::RpcController* ctrl, const Rank* req,
  DefaultResponce* resp, google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(ctrl);
  if (handler == nullptr) {
    resp->set_result(false);
    resp->set_comment(HANDLER_NOD_FOUND.toStdString());
    error_log << HANDLER_NOD_FOUND;
    done->Run();
    return ;
  }

  handler->runMethod(ctrl, method_, &TapControlServiceMethodBag::PutRank, req, resp);
}

void Service::DepartmentList(google::protobuf::RpcController* ctrl, const Dummy* req,
  DepartmentResponse* resp, google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(ctrl);
  if (handler == nullptr) {
    resp->set_result(false);
    resp->set_comment(HANDLER_NOD_FOUND.toStdString());
    error_log << HANDLER_NOD_FOUND;
    done->Run();
    return ;
  }

  handler->runMethod(ctrl, method_, &TapControlServiceMethodBag::DepartmentList, req, resp);
}

void Service::PutDepartment(google::protobuf::RpcController* ctrl, const Department* req,
  DefaultResponce* resp, google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(ctrl);
  if (handler == nullptr) {
    resp->set_result(false);
    resp->set_comment(HANDLER_NOD_FOUND.toStdString());
    error_log << HANDLER_NOD_FOUND;
    done->Run();
    return ;
  }

  handler->runMethod(ctrl, method_, &TapControlServiceMethodBag::PutDepartment, req, resp);
}

void Service::UserAppointmentAdd(::google::protobuf::RpcController* c,
                          const ::meteo::app::UserAppointment* request,
                          ::meteo::app::DefaultResponce* response,
                          ::google::protobuf::Closure* done)
{  
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }

  handler->runMethod( c, method_, &TapControlServiceMethodBag::UserAppointmentAdd, request, response );
}

void Service::UserAppointmentSetCurrentUser(::google::protobuf::RpcController* c,
                          const ::meteo::app::UserAppointment* request,
                          ::meteo::app::DefaultResponce* response,
                          ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }

  handler->runMethod( c, method_, &TapControlServiceMethodBag::UserAppointmentSetCurrentUser, request, response );
}

void Service::UserAppointmentUnsetCurrentUser(::google::protobuf::RpcController* c,
                          const ::meteo::app::UserAppointment* request,
                          ::meteo::app::DefaultResponce* response,
                          ::google::protobuf::Closure* done)
{  
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }

  handler->runMethod( c, method_, &TapControlServiceMethodBag::UserAppointmentUnsetCurrentUser, request, response );
}

void Service::UserAppointmentsLoad(::google::protobuf::RpcController* c,
                                    const ::meteo::app::Dummy* request,
                                    ::meteo::app::UserAppointmentsResponce* responce,
                                    ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }

  handler->runMethod( c, method_, &TapControlServiceMethodBag::UserAppointmentLoad, request, responce );
}

void Service::UserAppointmentSetUserList (  ::google::protobuf::RpcController* c,
                                          const ::meteo::app::UserAppointment* request,
                                          ::meteo::app::DefaultResponce* responce,
                                          ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }

  handler->runMethod( c, method_, &TapControlServiceMethodBag::UserAppointmentSetUserList, request, responce );
}

void Service::UserAppointmentDataUpdate (::google::protobuf::RpcController* c,
                                         const ::meteo::app::UserAppointment* request,
                                         ::meteo::app::DefaultResponce* responce,
                                         ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }

  handler->runMethod( c, method_, &TapControlServiceMethodBag::UserAppointmentDataUpdate, request, responce );
}

void Service::UserAppointmentsGetList (::google::protobuf::RpcController* c,
                                       const ::meteo::app::UserAppointment* request,
                                       ::meteo::app::UserAppointmentsResponce* responce,
                                       ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }
  handler->runMethod( c, method_, &TapControlServiceMethodBag::UserAppointmentsGetList, request, responce );
}


void Service::UserAppointmentsGetCurrent (::google::protobuf::RpcController* c,
                                       const ::meteo::app::UserAppointment* request,
                                       ::meteo::app::UserAppointmentsResponce* responce,
                                       ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }
  handler->runMethod( c, method_, &TapControlServiceMethodBag::UserAppointmentsGetCurrent, request, responce );
}


//!< Получаем настройки погоды для пользователя
void Service::UserWeatherSetting (::google::protobuf::RpcController* c,
                                       const ::meteo::app::WeatherSetting* request,
                                       ::meteo::app::WeatherSettingResponce* responce,
                                       ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }
  handler->runMethod( c, method_, &TapControlServiceMethodBag::UserWeatherSetting, request, responce );
}

// 
// сохраняем настройки погоды пользователя
// 
void Service::UserSaveWeatherSetting (  ::google::protobuf::RpcController* c,
                                         const ::meteo::app::WeatherSetting* request,
                                         ::meteo::app::DefaultResponce* responce,
                                         ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }

  handler->runMethod( c, method_, &TapControlServiceMethodBag::UserSaveWeatherSetting, request, responce );
}

void Service::SavePreset(google::protobuf::RpcController *controller, const FolderEntity *request, DefaultResponce *response, google::protobuf::Closure *done)
{
  auto handler = rpc::Channel::handlerFromController(controller);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }
  handler->runMethod( controller, method_, &TapControlServiceMethodBag::SavePreset, request, response);
}

void Service::GetPreset(google::protobuf::RpcController *controller, const FolderEntity *request, FolderEntity *response, google::protobuf::Closure *done)
{
  auto handler = rpc::Channel::handlerFromController(controller);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }
  handler->runMethod( controller, method_, &TapControlServiceMethodBag::GetPreset, request, response);
}

void Service::SaveIcon(google::protobuf::RpcController *c, const EntityIcon *req, DefaultResponce *resp, google::protobuf::Closure *d)
{
  auto handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ){
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
  }
  handler->runMethod( c, method_, &TapControlServiceMethodBag::SaveIcon, req, resp);
}

void Service::GetIcon(google::protobuf::RpcController *c, const EntityIcon *req, EntityIcon *resp, google::protobuf::Closure *d)
{
  auto handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ){
    QString error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
  }
  handler->runMethod( c, method_, &TapControlServiceMethodBag::GetIcon, req, resp);
}

void Service::UserAppointmentSetList (::google::protobuf::RpcController* controller,
                                      const ::meteo::app::UserAppointmentListRequest* request,
                                      ::meteo::app::DefaultResponce* responce,
                                      ::google::protobuf::Closure* done) {
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }
  handler->runMethod( controller, method_, &TapControlServiceMethodBag::UserAppointmentSetList, request, responce );
}

void Service::PositionCreate (::google::protobuf::RpcController* controller,
                             const ::meteo::app::PositionCreateRequest* request,
                             ::meteo::app::PositionCreateResponce* responce,
                              ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }
  handler->runMethod( controller, method_, &TapControlServiceMethodBag::PositionCreate, request, responce );
}

void Service::PositionsList (::google::protobuf::RpcController* controller,
                             const ::meteo::app::PositionsListRequest* request,
                             ::meteo::app::PositionsListResponce* responce,
                            ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }
  handler->runMethod( controller, method_, &TapControlServiceMethodBag::PositionsList, request, responce );
}

void Service::PositionDelete (::google::protobuf::RpcController* controller,
                             const ::meteo::app::PositionDeleteRequest* request,
                             ::meteo::app::PositionDeleteResponce* responce,
                             ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }
  handler->runMethod( controller, method_, &TapControlServiceMethodBag::PositionDelete, request, responce );
}

void Service::PositionEdit (::google::protobuf::RpcController* controller,
                            const ::meteo::app::PositionEditRequest* request,
                            ::meteo::app::PositionEditResponce* responce,
                            ::google::protobuf::Closure* done) {
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }
  handler->runMethod( controller, method_, &TapControlServiceMethodBag::PositionEdit, request, responce );
}

void Service::UserPogodaInputStsSave(::google::protobuf::RpcController* controller,
                                     const ::meteo::app::UserPogodaInputStationsSaveRequest* request,
                                     ::meteo::app::PogodaInputStationsSaveResponce* response,
                                     ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }
  handler->runMethod( controller, method_, &TapControlServiceMethodBag::UserPogodaInputStsSave, request, response );
}

void Service::UserPogodaInputStsLoad(::google::protobuf::RpcController* controller,
                                     const ::meteo::app::UserPogodaInputStationsLoadRequest* request,
                                     ::meteo::app::PogodaInputStationsLoadResponce* response,
                                     ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }
  handler->runMethod( controller, method_, &TapControlServiceMethodBag::UserPogodaInputStsLoad, request, response );
}

void Service::SaveUserSetStations(::google::protobuf::RpcController *controller,
                                  const ::meteo::app::UserSetStations *request,
                                  ::meteo::app::DefaultResponce *response,
                                  ::google::protobuf::Closure *done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }
  handler->runMethod( controller, method_, &TapControlServiceMethodBag::SaveUserSetStations, request, response );
}

void Service::GetUserSetStations(::google::protobuf::RpcController *controller,
                                 const ::meteo::app::UserSetStations *request,
                                 ::meteo::app::UserSetStations *response,
                                 ::google::protobuf::Closure *done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }
  handler->runMethod( controller, method_, &TapControlServiceMethodBag::GetUserSetStations, request, response );
}

void  Service::msgviewerUserSettingsSave(::google::protobuf::RpcController* controller,
                                        const ::meteo::app::MsgViewerUserSettingsSaveRequest* request,
                                        ::meteo::app::Dummy* response,
                                        ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }
  handler->runMethod( controller, method_, &TapControlServiceMethodBag::msgviewerUserSettingsSave, request, response );
}

void  Service::msgviewerUserSettingsLoad(::google::protobuf::RpcController* controller,
                                        const ::meteo::app::MsgViewerUserSettingsLoadRequest* request,
                                        ::meteo::proto::CustomViewerUserConfig* response,
                                        ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }
  handler->runMethod( controller, method_, &TapControlServiceMethodBag::msgviewerUserSettingsLoad, request, response );
}
void Service::SaveCustomPreset(::google::protobuf::RpcController* controller,
                   const ::meteo::app::CustomSettings* request,
                   ::meteo::app::Result* response,
                   ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }
  handler->runMethod( controller, method_, &TapControlServiceMethodBag::SaveCustomPreset, request, response );
}

void Service::LoadCustomPreset(::google::protobuf::RpcController* controller,
                   const ::meteo::app::CustomSettings* request,
                   ::meteo::app::CustomSettings* response,
                   ::google::protobuf::Closure* done)
{
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(controller);
  if ( nullptr == handler ){
    static const QString emessage = QObject::tr("Не удается получить handler для контроллера");
    error_log << emessage;
    done->Run();
    return;
  }
  handler->runMethod( controller, method_, &TapControlServiceMethodBag::LoadCustomPreset, request, response );
}

void  Service::ProcState(::google::protobuf::RpcController* controller,
                         const ::meteo::app::AppState::Proc* request,
                         ::meteo::app::AppState::Proc* response,
                         ::google::protobuf::Closure* done)
{
  //TODO - meteo::app::Control не поддерживает многопоточность
  Q_UNUSED(controller);

  auto control = meteo::app::Control::instance();
  control->getStatus(request->id(), response->mutable_status());
  // debug_log << response->Utf8DebugString();
  done->Run();
}

}
}
