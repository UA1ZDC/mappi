#include "tappcontrolservicemethodbag.h"
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <sql/nosql/nosqlquery.h>
#include <sql/dbi/gridfs.h>
#include <meteo/commons/global/dbnames.h>

#include <meteo/commons/appmanager/usersettings.h>

#include "tappcontrol.h"

namespace meteo {
namespace app {

static const QString kUsersDb = QObject::tr("usersdb");
//static const QString kUsersInitFailed = QObject::tr("Ошибка инициализации подключения к системе пользователей");

TapControlServiceMethodBag::TapControlServiceMethodBag() :
    auth_(new AuthHandler())
{
}

TapControlServiceMethodBag::~TapControlServiceMethodBag()
{
  delete auth_;
  auth_ = nullptr;
}

void TapControlServiceMethodBag::AuthUser(const UserAuthRequest* req, UserAuthResponse* resp)
{
  auth_->AuthUser(req, resp);
}

void TapControlServiceMethodBag::NewUser(const UserRequest* req, UserResponse* resp)
{
  auth_->NewUser(req, resp);
}

void TapControlServiceMethodBag::AddUser(const UserRequest* req, UserResponse* resp)
{
  auth_->AddUser(req, resp);
}

void TapControlServiceMethodBag::RegisterUser(const UserRequest* req, UserResponse* resp)
{
  auth_->RegisterUser(req, resp);
}

void TapControlServiceMethodBag::UpdateUser(const UserRequest* req, UserResponse* resp)
{
  auth_->UpdateUser(req, resp);
}

void TapControlServiceMethodBag::BlockUser(const UserRequest* req, UserResponse* resp)
{
  auth_->BlockUser(req, resp);
}

 void TapControlServiceMethodBag::UnlockUser(const UserRequest* req, UserResponse* resp)
{
  auth_->UnlockUser(req, resp);
}

void TapControlServiceMethodBag::HomeDirUser(const UserRequest* req, UserResponse* resp)
{
  auth_->HomeDirUser(req, resp);
}

void TapControlServiceMethodBag::CheckUser(const UserRequest* req, UserResponse* resp)
{
  auth_->CheckUser(req, resp);
}

void TapControlServiceMethodBag::UserList(const Dummy* req, UserResponse* resp)
{
  auth_->UserList(req, resp);
}

void TapControlServiceMethodBag::RoleList(const Dummy* req, RoleResponse* resp)
{
  auth_->RoleList(req, resp);
}

void TapControlServiceMethodBag::RankList(const Dummy* req, RankResponse* resp)
{
  auth_->RankList(req, resp);
}

void TapControlServiceMethodBag::PutRank(const Rank* req, DefaultResponce* resp)
{
  auth_->PutRank(req, resp);
}

void TapControlServiceMethodBag::DepartmentList(const Dummy* req, DepartmentResponse* resp)
{
  auth_->DepartmentList(req, resp);
}

void TapControlServiceMethodBag::PutDepartment(const Department* req, DefaultResponce* resp)
{
  auth_->PutDepartment(req, resp);
}

void TapControlServiceMethodBag::UserAppointmentAdd( const UserAppointment *request, DefaultResponce* response )
{
 if ( false == request->has_name() ||
      false == request->has_owner() ||
      false == request->has_type() ){
   static const QString& emessage = QObject::tr("Отсутствует один из обязательны аргументов");
   response->set_result(false);
   error_log << emessage;
   response->set_comment(emessage.toStdString());
   return;
  }

 static const QString& addUserPositionQueryName = QObject::tr("insert_user_appointment");

 std::unique_ptr<Dbi> db(meteo::global::dbUsers());
 if ( nullptr == db.get() ) {
   response->set_result(false);
   QString emessage = meteo::msglog::kDbConnectHostPortFailed;
   error_log << emessage;
   response->set_comment(emessage.toStdString());
   return;
  }
 auto query = db->queryptrByName(addUserPositionQueryName);
 if(nullptr == query) {
   static const QString& emessage = meteo::msglog::kDbRequestNotFound.arg(addUserPositionQueryName);
   error_log << emessage;
   response->set_result(false);
   response->set_comment(emessage.toStdString());
   return;
  }
 query->arg("name",request->name());
 query->argOid("owner", auth_->getUserId(QString::fromStdString(request->owner())));
 query->arg("type",request->type());

 if ( false == query->exec() ){
   response->set_result(false);
   auto emessage = meteo::msglog::kDbRequestFailed;
   response->set_comment(emessage.toStdString());
   error_log << emessage;
   return;
  }

 const DbiEntry& result = query->result();

 double ok = result.hasField("ok")? result.valueDouble("ok"): -1;
 double n = result.hasField("n")? result.valueDouble("n"): -1;
 double nModified = result.hasField("nModified")? result.valueDouble("nModified"): -1;
 if ( qRound(ok) != 1 || qRound(n) != 1 || qRound(nModified) != 0 || false == result.hasField("upserted") ) {
   response->set_result(false);
   QString emessage = QObject::tr("Должностное лицо с именем %1 уже существует").arg(QString::fromStdString(request->name()));
   response->set_comment(emessage.toStdString());
   return;
  }
 response->set_result(true);
}

void TapControlServiceMethodBag::UserAppointmentSetCurrentUser( const UserAppointment *request, DefaultResponce* response )
{
 if ( false == request->has_id() ||
      false == request->has_currentuser() ){
   static const QString& emessage = QObject::tr("Отсутствует один из обязательны аргументов");
   response->set_result(false);
   error_log << emessage;
   response->set_comment(emessage.toStdString());
   return;
  }
 static const QString& findUserPositionsQueryName = QObject::tr("find_appointments_by_user");

 std::unique_ptr<Dbi> db(meteo::global::dbUsers());
 if ( nullptr == db.get() ) {
   response->set_result(false);
   QString emessage = meteo::msglog::kDbConnectHostPortFailed;
   error_log << emessage;
   response->set_comment(emessage.toStdString());
   return;
  }
 auto findUserPositions = db->queryptrByName(findUserPositionsQueryName);
 if(nullptr == findUserPositions) {
   static const QString& emessage = meteo::msglog::kDbRequestNotFound.arg(findUserPositionsQueryName);
   error_log << emessage;
   response->set_result(false);
   response->set_comment(emessage.toStdString());
   return;
  }
 QString user = auth_->getUserId(QString::fromStdString(request->currentuser()));
 findUserPositions->argOid("user",user);
 QString error;
 if(false == findUserPositions->exec()){
   response->set_result(false);
   response->set_comment(meteo::msglog::kDbRequestFailed.toStdString());
   //error_log << response->comment();
   return;
  }

 if ( true == findUserPositions->initIterator() ) { //Для MongoDb при ошибки инициализации курсора должен быть return
   while ( true == findUserPositions->next()) {
     const DbiEntry& doc = findUserPositions->entry();
     QString name = doc.valueString("name");
     auto message = QObject::tr("Пользователь уже занимает должность \"%1\".").arg(name);
     response->set_result(false);
     response->set_comment(message.toStdString());
     return;
   }
 }

 static const QString& setUserPositionCurrentUserQueryName = QObject::tr("set_user_appointments_current_user");
 auto setUserPositionQuery = db->queryptrByName(setUserPositionCurrentUserQueryName);
 if(nullptr == setUserPositionQuery) {
   static const QString& emessage = meteo::msglog::kDbRequestNotFound.arg(setUserPositionCurrentUserQueryName);
   error_log << emessage;
   response->set_result(false);
   response->set_comment(emessage.toStdString());
   return;

  }
 setUserPositionQuery->argOid("id",request->id());
 setUserPositionQuery->argOid("user",user);
 setUserPositionQuery->arg("dtStart",QDateTime::currentDateTimeUtc());


 if ( false == setUserPositionQuery->exec()){
   response->set_result(false);
   response->set_comment(meteo::msglog::kDbRequestFailed.toStdString());
   return;
  }

 const DbiEntry& result = setUserPositionQuery->result();

 int ok = qRound(result.hasField("ok")? result.valueDouble("ok"): -1);
 int n = qRound(result.hasField("n")? result.valueDouble("n"): -1);
 int nModified = qRound(result.hasField("nModified")? result.valueDouble("nModified"): -1);

 if ( ok != 1 ){
   response->set_result(false);
   QString emessage = QObject::tr("При выполнении команды произошла неизвестная ошибка в базе данных");
   response->set_comment(emessage.toStdString());
   return;
  }

 if ( n != 1 ){
   response->set_result(false);
   QString emessage = QObject::tr("Ошибка: должностное лицо не найдено");
   response->set_comment(emessage.toStdString());
   return;
  }

 if ( nModified != 1 ){
   response->set_result(false);
   QString emessage = QObject::tr("Ошибка: должностное лицо уже занято");
   response->set_comment(emessage.toStdString());
   return;
  }
 response->set_result(true);
}

void TapControlServiceMethodBag::UserAppointmentUnsetCurrentUser( const UserAppointment *request, DefaultResponce* response )
{
 if ( false == request->has_id() ){
   static const QString& emessage = QObject::tr("Отсутствует один из обязательны аргументов");
   response->set_result(false);
   error_log << emessage;
   response->set_comment(emessage.toStdString());
   return;
  }
 static const QString& unsetUserPositionCurrentUserQueryName = QObject::tr("unset_user_appointments_current_user");

 std::unique_ptr<Dbi> db(meteo::global::dbUsers());
 if ( nullptr == db.get() ) {
   response->set_result(false);
   QString emessage = meteo::msglog::kDbConnectHostPortFailed;
   error_log << emessage;
   response->set_comment(emessage.toStdString());
   return;
  }
 auto query = db->queryptrByName(unsetUserPositionCurrentUserQueryName);
 if(nullptr == query) {
   static const QString& emessage = meteo::msglog::kDbRequestNotFound.arg(unsetUserPositionCurrentUserQueryName);
   error_log << emessage;
   response->set_result(false);
   response->set_comment(emessage.toStdString());
   return;
  }
 query->argOid("id",request->id());

 if ( false ==  query->exec() ){
   response->set_result(false);
   response->set_comment(meteo::msglog::kDbRequestFailed.toStdString());
   return;
  }

 const DbiEntry& result = query->result();
 int ok = qRound(result.hasField("ok")? result.valueDouble("ok"): -1);
 int n = qRound(result.hasField("n")? result.valueDouble("n"): -1);
 //int nModified = qRound(result.hasField("nModified")? result.valueDouble("nModified"): -1);

 if ( ok != 1 ){
   response->set_result(false);
   QString emessage = QObject::tr("При выполнении команды произошла неизвестная ошибка в базе данных");
   response->set_comment(emessage.toStdString());
   return;
  }

 if ( n != 1 ){
   response->set_result(false);
   QString emessage = QObject::tr("Ошибка: должностное лицо не найдено");
   response->set_comment(emessage.toStdString());
   return;
  }

 response->set_result(true);
}

void TapControlServiceMethodBag::UserAppointmentLoad( const ::meteo::app::Dummy* request,
                                                      ::meteo::app::UserAppointmentsResponce* response )
{
 Q_UNUSED(request);
 static const QString collectionFindQueryName = QObject::tr("find_user_appointments");

 std::unique_ptr<Dbi> db(meteo::global::dbUsers());
 if ( nullptr == db.get() ) {
   response->set_result(false);
   QString emessage = meteo::msglog::kDbConnectHostPortFailed;
   error_log << emessage;
   response->set_comment(emessage.toStdString());
   return;
  }
 auto query = db->queryptrByName(collectionFindQueryName);
 if(nullptr == query) {
   static const QString& emessage = meteo::msglog::kDbRequestNotFound.arg(collectionFindQueryName);
   error_log << emessage;
   response->set_result(false);
   response->set_comment(emessage.toStdString());
   return;
  }
 QString err;
 if ( false == query->execInit(&err) ){
   response->set_result(false);
   response->set_comment(err.toStdString());
   return;
  }
 while ( true == query->next() ) {
   const DbiEntry& doc = query->entry();
   if ( false == doc.hasField("_id") ){
     continue;
    }
   auto token = response->add_appointment();
   token->set_id(doc.valueOid("_id").toStdString());

   if ( true == doc.hasField("name")){
     token->set_name(doc.valueString("name").toStdString());
    }
   if ( true == doc.hasField("type") ){
     token->set_type(static_cast<meteo::app::AppointmentType>(doc.valueInt32("type")));
    }
   if ( true == doc.hasField("users") ){
     auto usersArray = doc.valueArray("users");
     while ( true == usersArray.next() ){
       auto uid = usersArray.valueInt32();
       token->add_users(auth_->getUserLogin(uid).toStdString());
      }
    }
   if ( true == doc.hasField("currentUser") ){
     auto currentUserOid = doc.valueOid("currentUser");
     token->set_currentuser(auth_->getUserLogin(currentUserOid).toStdString());
    }
   if ( true == doc.hasField("owner")) {
     auto ownerId = doc.valueOid("owner");
     token->set_owner(auth_->getUserLogin(ownerId).toStdString());
    }
   if ( true == doc.hasField("dtStart") ) {
     token->set_dtbegin(doc.valueDt("dtStart").toString(Qt::ISODate).toStdString());
    }
  }

 response->set_result(true);
}

void TapControlServiceMethodBag::UserAppointmentSetUserList( const ::meteo::app::UserAppointment* request,
                                                             ::meteo::app::DefaultResponce* response )
{
 if ( false == request->has_id() ){
   static const QString& emessage = QObject::tr("Отсутствует один из обязательны аргументов");
   response->set_result(false);
   error_log << emessage;
   response->set_comment(emessage.toStdString());
   return;
  }
 static const QString& addUserPositionUserQueryName = QObject::tr("set_user_appointments_users");

 QStringList users;
 for ( int i = 0; i < request->users_size(); ++i ){
   auto user = QString::fromStdString(request->users(i));
   users << auth_->getUserId(user);
  }

 std::unique_ptr<Dbi> db(meteo::global::dbUsers());
 if ( nullptr == db.get() ) {
   response->set_result(false);
   QString emessage = meteo::msglog::kDbConnectHostPortFailed;
   response->set_comment(emessage.toStdString());
   return;
  }
 auto query = db->queryptrByName(addUserPositionUserQueryName);
 if(nullptr == query) {
   static const QString& emessage = meteo::msglog::kDbRequestNotFound.arg(addUserPositionUserQueryName);
   response->set_result(false);
   response->set_comment(emessage.toStdString());
   return;

  }
 query->argOid("id",request->id());
 query->argOid("users",users);

 if ( false == query->exec() ){
   response->set_result(false);
   response->set_comment(meteo::msglog::kDbRequestFailed.toStdString());
   return;
  }

 const DbiEntry& result = query->result();
 double ok = result.hasField("ok")? result.valueDouble("ok"): -1;
 double n = result.hasField("n")? result.valueDouble("n"): -1;
 //double nModified = result.hasField("nModified")? result.valueDouble("nModified"): -1;

 if ( qRound(ok) != 1 || qRound(n) != 1  ) {
   response->set_result(false);
   QString emessage = QObject::tr("Ошибка: должностное лицо не найдено");
   response->set_comment(emessage.toStdString());
   return;
  }
 response->set_result(true);
}

void TapControlServiceMethodBag::UserAppointmentDataUpdate( const UserAppointment *request, DefaultResponce* response )
{
 if ( false == request->has_id() ||
      false == request->has_name() ||
      false == request->has_type() ){
   static const QString& emessage = QObject::tr("Отсутствует один из обязательны аргументов");
   response->set_result(false);
   error_log << emessage;
   response->set_comment(emessage.toStdString());
   return;
  }

 static const QString& findByNameQueryName = QObject::tr("find_user_appointment_by_name");

 std::unique_ptr<Dbi> db(meteo::global::dbInter());
 if ( nullptr == db.get() ) {
   response->set_result(false);
   QString emessage = meteo::msglog::kDbConnectHostPortFailed;
   response->set_comment(emessage.toStdString());
   return;
  }
 auto findByNameQuery = db->queryptrByName(findByNameQueryName);
 if(nullptr == findByNameQuery) {
   static const QString& emessage = meteo::msglog::kDbRequestNotFound.arg(findByNameQueryName);
   response->set_result(false);
   response->set_comment(emessage.toStdString());
   return;
  }

 QStringList users;
 for ( int i = 0; i < request->users_size(); ++i ){
   auto user = QString::fromStdString(request->users(i));
   users << auth_->getUserId(user);
  }
 findByNameQuery->arg("name",request->name());
 QString err;
 if ( false == findByNameQuery->execInit(&err) ){
   response->set_result(false);
   response->set_comment(err.toStdString());
   return;
  }

 int count = 0;
 while ( findByNameQuery->next()){
   ++count;
  }
 if ( count != 0 ) {
   static const QString& emessage = QObject::tr("Должностное лицо с данным именем уже существует.");
   error_log << emessage;
   response->set_result(false);
   response->set_comment(emessage.toStdString());
   return;
  }

 static const QString& updateQueryName = QObject::tr("update_user_appointment");

 auto updateQuery = db->queryptrByName(updateQueryName);
 if(nullptr == updateQuery) {
   static const QString& emessage = meteo::msglog::kDbRequestNotFound.arg(updateQueryName);
   response->set_result(false);
   response->set_comment(emessage.toStdString());
   return;
  }

 updateQuery->argOid("id",request->id());
 updateQuery->arg("name",request->name());
 updateQuery->argOid("owner",auth_->getUserId(QString::fromStdString(request->owner())));
 updateQuery->arg("type",request->type());

 if ( false == updateQuery->exec() ){
   response->set_result(false);
   response->set_comment(meteo::msglog::kDbRequestFailed.toStdString());
   return;
  }
 const DbiEntry& result = updateQuery->result();
 int ok = result.hasField("ok")? qRound(result.valueDouble("ok")): -1;
 int n = result.hasField("n")? qRound(result.valueDouble("n")): -1;

 if ( 1 != ok && 1 != n ) {
   static const QString& emessage = QObject::tr("Ошибка: данные не обновлены.");
   error_log << emessage;
   response->set_result(false);
   response->set_comment(emessage.toStdString());
   return;
  }
 response->set_result(true);
}

void TapControlServiceMethodBag::UserAppointmentsGetList( const ::meteo::app::UserAppointment* request,
                                                          ::meteo::app::UserAppointmentsResponce* response )
{
 if ( false == request->has_currentuser() ){
   static const QString& emessage = QObject::tr("Ошибка: отсутствует один из обязательных параметров");
   error_log << emessage;
   response->set_result(false);
   response->set_comment(emessage.toStdString());
   return;
  }

 static const QString& userAppointmenstFindQueryName = QObject::tr("find_appointments_user_can_take");

 std::unique_ptr<Dbi> db(meteo::global::dbInter());
 if ( nullptr == db.get() ) {
   static const QString& emessage =  meteo::msglog::kDbConnectHostPortFailed;
   response->set_result(false);
   error_log << emessage;
   response->set_comment(emessage.toStdString());
   return;
  }
 auto query = db->queryptrByName(userAppointmenstFindQueryName);
 if(nullptr == query) {
   static const QString& emessage = meteo::msglog::kDbRequestNotFound.arg(userAppointmenstFindQueryName);
   response->set_result(false);
   error_log << emessage;
   response->set_comment(emessage.toStdString());
   return;
  }
 query->argOid("users",auth_->getUserId(QString::fromStdString(request->currentuser())));

 QString err;
 if ( false == query->execInit(&err) ){
   response->set_result(false);
   auto emessage = err;
   response->set_comment(emessage.toStdString());
   //error_log << emessage;
   return;
  }

 while ( query->next() ){
   const DbiEntry& doc = query->entry();
   auto id = doc.valueOid("_id");
   auto name = doc.valueString("name");
   auto newAppointment = response->add_appointment();
   newAppointment->set_id(id.toStdString());
   newAppointment->set_name(name.toStdString());
  }
 response->set_result(true);
}

void TapControlServiceMethodBag::UserAppointmentsGetCurrent( const ::meteo::app::UserAppointment* request,
                                                             ::meteo::app::UserAppointmentsResponce* response )
{
 if ( false == request->has_currentuser() ){
   static const QString& emessage = QObject::tr("Ошибка: отсутствует один из обязательных параметров");
   error_log << emessage;
   response->set_result(false);
   response->set_comment(emessage.toStdString());
   return;
  }
 static const QString& findAppointmentsByUserQueryName = QObject::tr("find_appointments_by_user");

 std::unique_ptr<Dbi> db(meteo::global::dbUsers());
 if ( nullptr == db.get() ) {
   static const QString& emessage =  meteo::msglog::kDbConnectHostPortFailed;
   response->set_result(false);
   error_log << emessage;
   response->set_comment(emessage.toStdString());
   return;
  }
 auto query = db->queryptrByName(findAppointmentsByUserQueryName);
 if(nullptr == query) {
   static const QString& emessage = meteo::msglog::kDbRequestNotFound.arg(findAppointmentsByUserQueryName);
   response->set_result(false);
   error_log << emessage;
   response->set_comment(emessage.toStdString());
   return;
  }
 query->argOid("user",auth_->getUserId(QString::fromStdString(request->currentuser())));

 QString err;
 if ( false == query->execInit(&err) ){
   response->set_result(false);
   auto emessage = err;
   response->set_comment(emessage.toStdString());
   //error_log << emessage;
   return;
  }

 while ( query->next() ){
   const DbiEntry& doc = query->entry();
   auto id = doc.valueOid("_id");
   auto name = doc.valueString("name");

   auto newAppointment = response->add_appointment();
   newAppointment->set_id(id.toStdString());
   newAppointment->set_name(name.toStdString());
  }
 response->set_result(true);
}

//!< Получаем настройки погоды для пользователя
void TapControlServiceMethodBag::UserWeatherSetting( const ::meteo::app::WeatherSetting* request,
                                                     ::meteo::app::WeatherSettingResponce* response )
{
 // проверяем есть ли айди пользователя
 if ( false == request->has_user_login() ){
   static const QString& emessage = QObject::tr("Ошибка: отсутствует айди пользователя");
   error_log << emessage;
   response->set_result(false);
   response->set_comment(emessage.toStdString());
   return;
  }
 static const QString& findSettingsByUserAndType = QObject::tr("find_user_settings");
 std::unique_ptr<Dbi> db(meteo::global::dbUsers());
 if ( nullptr == db.get() ) {
   static const QString& emessage =  meteo::msglog::kDbConnectHostPortFailed;
   response->set_result(false);
   error_log << emessage;
   response->set_comment(emessage.toStdString());
   return;
  }
 auto query = db->queryptrByName(findSettingsByUserAndType);
 if(nullptr == query) {
   static const QString& emessage = meteo::msglog::kDbRequestNotFound.arg(findSettingsByUserAndType);
   response->set_result(false);
   error_log << emessage;
   response->set_comment(emessage.toStdString());
   return;
  }
 query->arg("user_login", QString::fromStdString(request->user_login()) );
 query->arg("setting_type", QString::fromStdString(request->setting_type()) );

 QString err;
 if ( false == query->execInit(&err) ){
   response->set_result(false);
   auto emessage = err;
   response->set_comment(emessage.toStdString());
   //error_log << emessage;
   return;
  }

 auto wSettings = response->mutable_settings();

 // проходимся по ответу и получаем
 // сохраненные параметры
 while ( query->next() ){
   const DbiEntry& doc = query->entry();
   auto user_login = doc.valueOid("user_login");
   auto type       = doc.valueString("setting_type");


   wSettings->set_user_login(      user_login.toStdString() );
   wSettings->set_setting_type(    type.toStdString()    );

   // получаем поле с настройками
   Array setting = doc.valueArray("settings");
   while ( setting.next() ) {
     Document item;
     // заполняем документ
     if (!setting.valueDocument(&item)) {
       continue;
      }
     // устанавливаем ответ
     auto newSettings = wSettings->add_settings();

     //                             string station
     //                             string station_type
     //                             int32  position
     //                             bool   visible
     //                             int32  ump_height
     //                             int32  ump_visible
     newSettings->set_station(      item.valueString("station").toStdString()        );
     newSettings->set_station_type( item.valueString("station_type").toStdString()   );
     newSettings->set_position(     item.valueInt32("position")       );
     newSettings->set_visible(      item.valueBool("visible")        );
     newSettings->set_ump_height(   item.valueInt32("ump_height")     );
     newSettings->set_ump_visible(  item.valueInt32("ump_visible")    );
    }

  }
 response->set_result(true);
 return;
}


// сохраняем настройки погоды пользователя
void TapControlServiceMethodBag::UserSaveWeatherSetting( const ::meteo::app::WeatherSetting* request,
                                                         ::meteo::app::DefaultResponce* response  ){

 if ( false == request->has_user_login() ||
      false == request->has_setting_type() ){
   static const QString& emessage = QObject::tr("Отсутствует один из обязательны аргументов");
   response->set_result(false);
   error_log << emessage;
   response->set_comment(emessage.toStdString());
   return;
  }

 static const QString& updateQueryName = QObject::tr("update_user_settings");
 std::unique_ptr<Dbi> db(meteo::global::dbUsers());
 if ( nullptr == db.get() ) {
   static const QString& emessage =  meteo::msglog::kDbConnectHostPortFailed;
   response->set_result(false);
   error_log << emessage;
   response->set_comment(emessage.toStdString());
   return;
  }

 auto query = db->queryptrByName(updateQueryName);
 if(nullptr == query) {
   static const QString& emessage = meteo::msglog::kDbRequestNotFound.arg(updateQueryName);
   response->set_result(false);
   error_log << emessage;
   response->set_comment(emessage.toStdString());
   return;
  }
 query->arg("user_login", request->user_login() );
 query->arg("setting_type", request->setting_type() );
 /**
* заполняем запрос
*/

 QJsonArray itemsArray;
 QJsonDocument itemsDoc;
 // заполняем json массив данными
 if ( request->settings().size()>0 ){
   for (int i = 0; i < request->settings().size(); ++i)
    {
     auto item = request->settings(i);
     QJsonObject obj;

     obj["station"]      = QJsonValue( QString::fromStdString(item.station()) );
     obj["station_type"] = QJsonValue( QString::fromStdString(item.station_type()) );
     obj["position"]     = QJsonValue( item.position() );
     obj["visible"]      = QJsonValue( item.visible() );
     obj["ump_height"]   = QJsonValue( item.ump_height() );
     obj["ump_visible"]  = QJsonValue( item.ump_visible() );

     itemsArray.push_back(obj);
    }
  }
 // устанавливаем массив в документ
 itemsDoc.setArray( itemsArray );

 // проверяем есть ли записи
 if ( false == itemsArray.empty() ) {
   // заполняем квери
   query->argJson("settings", QString( itemsDoc.toJson() ) );
  }

 // error_log << updateQuery.query();

 if ( false == query->exec( ) ){
   response->set_result(false);
   auto emessage = meteo::msglog::kDbRequestFailed;
   response->set_comment(emessage.toStdString());
   error_log << emessage;
   return;
  }

 const DbiEntry& result = query->result();
 int ok = result.hasField("ok")? qRound(result.valueDouble("ok")): -1;
 int n = result.hasField("n")? qRound(result.valueDouble("n")): -1;

 if ( 1 != ok && 1 != n ) {
   static const QString& emessage = QObject::tr("Ошибка: данные не обновлены.");
   error_log << emessage;
   response->set_result(false);
   response->set_comment(emessage.toStdString());
   return;
  }
 response->set_result(true);
 return;
}

QString TapControlServiceMethodBag::createMapJson(const FolderEntity &entity)
{
 auto errorFunc = [](QString err)
 {
  error_log << err;
  return QString();
 };

 if ( false == entity.has_map() ) {
   return errorFunc(QObject::tr("Неверная сущность"));
  }

 const QString mongoQuery = "folder_maps";

 if ( false == meteo::global::kMongoQueriesNew.contains(mongoQuery)) {
   return errorFunc(QObject::tr("Не найдена функция для получения данных = '%1'").arg(mongoQuery));
  }

 int position = 0;
 if ( true == entity.has_position() ) {
   position = entity.position();
  }

 QString q = meteo::global::kMongoQueriesNew[mongoQuery];

 NosqlQuery query;

 query.setQuery(q).arg("position",position)
   .arg("map",entity.map());
 if ( true == entity.has_title() ) {
   query.arg("title",entity.title());
  }
 if ( true == entity.has_icon() ) {
   query.arg("icon",entity.icon());
  }
 return query.query();
}

QString TapControlServiceMethodBag::createFolderJson(const FolderEntity &entity)
{
 auto errorFunc = [](QString err)
 {
  error_log << err;
  return QString();
 };

 if ( false == entity.has_folder() && false == entity.server() && false == entity.has_regexp() ) {
   return errorFunc(QObject::tr("Неверная сущность"));
  }

 if ( true == entity.has_regexp() && false == entity.has_date_format() ) {
   return errorFunc(QObject::tr("Для шаблона файла не указан формат даты"));
  }

 if ( false == entity.has_title() ) {
   return errorFunc(QObject::tr("Директория должна иметь название"));
  }

 const QString mongoQuery = "folders";

 if ( false == meteo::global::kMongoQueriesNew.contains(mongoQuery)) {
   return errorFunc(QObject::tr("Не найдена функция для получения данных = '%1'").arg(mongoQuery));
  }

 QString q = meteo::global::kMongoQueriesNew[mongoQuery];

 QString entities;
 for ( int i = 0, sz = entity.folder().entities_size(); i < sz; ++i ) {
   if ( entity.folder().entities(i).has_map() ) {
     entities += createMapJson(entity.folder().entities(i)) + ",";
    }
   else if ( entity.folder().entities(i).has_folder() || entity.folder().entities(i).has_regexp()) {
     entities += createFolderJson(entity.folder().entities(i)) + ",";
    }
   else if ( entity.folder().entities(i).has_job() ) {
     entities += createJobJson(entity.folder().entities(i)) + ",";
    }
  }
 if ( false == entities.isEmpty() ) {
   entities.remove(-1, 1);
  }

 int position = 0;
 if ( true == entity.has_position() ) {
   position = entity.position();
  }

 NosqlQuery query;
 query.setQuery(q)
   .arg("title",entity.title())
   .arg("position",position);
 if ( false == entities.isEmpty() ) {
   query.argJson("folder",entities);
  }
 if ( true == entity.has_icon() ) {
   query.arg("icon",entity.icon());
  }
 query.arg("server",entity.server());
 if ( true == entity.has_regexp() ) {
   query.arg("regexp",entity.regexp());
  }
 if ( true == entity.has_date_format() ) {
   auto date_format = createDateFormatJson(entity.date_format());
   query.argJson("date_format",date_format);
  }
 if ( true == entity.has_length() ) {
   query.arg("length",entity.length());
  }

 return query.query();
}

QString TapControlServiceMethodBag::createJobJson(const FolderEntity &entity)
{
 auto errorFunc = [](QString err)
 {
  error_log << err;
  return QString();
 };

 if ( false == entity.has_job() ) {
   return errorFunc(QObject::tr("Неверная сущность"));
  }

 if ( false == entity.job().has_title() || false == entity.job().has_map_id() || false == entity.job().has_name() ) {
   return errorFunc(QObject::tr("Работа не заполнена"));
  }

 int position = 0;
 if ( true == entity.has_position() ) {
   position = entity.position();
  }

 const QString mongoQuery = "folder_jobs";

 if ( false == meteo::global::kMongoQueriesNew.contains(mongoQuery)) {
   return errorFunc(QObject::tr("Не найдена функция для получения данных = '%1'").arg(mongoQuery));
  }

 QString q = meteo::global::kMongoQueriesNew[mongoQuery];
 NosqlQuery query;

 query.setQuery(q).
   arg("position",position).
   arg("name",entity.job().name()).
   arg("jtitle",entity.job().title()).
   arg("map",entity.job().map_id());
 if ( true == entity.has_title() ) {
   query.arg("title",entity.title());
  }
 if ( true == entity.has_icon() ) {
   query.arg("icon",entity.icon());
  }
 return query.query();
}

QString TapControlServiceMethodBag::createDelimiterJson(const FolderEntity &entity)
{
 auto errorFunc = [](QString err)
 {
  error_log << err;
  return QString();
 };

 if ( false == entity.has_delimiter() ) {
   return errorFunc(QObject::tr("Неверная сущность"));
  }

 int position = 0;
 if ( true == entity.has_position() ) {
   position = entity.position();
  }

 const QString mongoQuery = "folder_delimiters";

 if ( false == meteo::global::kMongoQueriesNew.contains(mongoQuery)) {
   return errorFunc(QObject::tr("Не найдена функция для получения данных = '%1'").arg(mongoQuery));
  }

 QString q = meteo::global::kMongoQueriesNew[mongoQuery];
 NosqlQuery query;
 query.setQuery(q)
   .arg("position",position);
 if ( true == entity.has_title() ) {
   query.arg("title",entity.title());
  }

 return query.query();
}

QString TapControlServiceMethodBag::createDateFormatJson(const DateFormat &format)
{
 auto errorFunc = [](QString err)
 {
  error_log << err;
  return QString();
 };

 const QString mongoQuery = "date_format";

 if ( false == meteo::global::kMongoQueriesNew.contains(mongoQuery)) {
   return errorFunc(QObject::tr("Не найдена функция для получения данных = '%1'").arg(mongoQuery));
  }

 QString q = meteo::global::kMongoQueriesNew[mongoQuery];
 NosqlQuery query;

 query.setQuery(q);
 if ( true == format.has_year() ) {
   auto year = createDateFormatElementJson(format.year());
   query.argJson("year",year);
  }
 if ( true == format.has_month() ) {
   auto month = createDateFormatElementJson(format.month());
   query.argJson("month",month);
  }
 if ( true == format.has_day() ) {
   auto day = createDateFormatElementJson(format.day());
   query.argJson("day",day);
  }
 if ( true == format.has_hour() ) {
   auto hour = createDateFormatElementJson(format.hour());
   query.argJson("hour",hour);
  }
 if ( true == format.has_minute() ) {
   auto minute = createDateFormatElementJson(format.minute());
   query.argJson("minute",minute);
  }

 return query.query();
}

QString TapControlServiceMethodBag::createDateFormatElementJson(const DateFormatElement &element)
{
 auto errorFunc = [](QString err)
 {
  error_log << err;
  return QString();
 };

 if ( false == element.has_index() || false == element.has_regexp() || false == element.has_format_string() ) {
   return errorFunc(QObject::tr("Элемент формата даты не заполнен"));
  }

 const QString mongoQuery = "date_format_element";

 if ( false == meteo::global::kMongoQueriesNew.contains(mongoQuery)) {
   return errorFunc(QObject::tr("Не найдена функция для получения данных = '%1'").arg(mongoQuery));
  }

 QString q = meteo::global::kMongoQueriesNew[mongoQuery];
 NosqlQuery query;

 query.setQuery(q)
   .arg("index",element.index())
   .arg("regexp",element.regexp())
   .arg("format_string",element.format_string());

 return query.query();
}

FolderEntity TapControlServiceMethodBag::entityFromJson(const DbiEntry& doc)
{
 FolderEntity entity;
 if ( true == doc.hasField("user") ) {
   entity.set_title(doc.valueString("user").toStdString());
  }
 if ( true == doc.hasField("title") ) {
   entity.set_title(doc.valueString("title").toStdString());
  }
 if ( true == doc.hasField("position") ) {
   entity.set_position(doc.valueInt32("position"));
  }
 if ( true == doc.hasField("icon") ) {
   entity.set_icon(doc.valueString("icon").toStdString());
  }
 if ( true == doc.hasField("server") ) {
   entity.set_server(doc.valueBool("server"));
  }

 if ( true == doc.hasField("regexp") ) {
   entity.set_regexp(doc.valueString("regexp").toStdString());
  }

 if ( true == doc.hasField("length") ) {
   entity.set_length(doc.valueInt32("length"));
  }

 if ( true == doc.hasField("folder") || true == entity.server()) {
   DocumentsFolder folder;

   auto entities = doc.valueDocument("folder").valueArray("entities");
   while ( true == entities.next() ) {
     auto e = folder.add_entities();
     Document eDoc;
     entities.valueDocument(&eDoc);
     e->CopyFrom(entityFromJson(eDoc));
    }

   entity.mutable_folder()->CopyFrom(folder);
  }
 else if ( true == doc.hasField("map") ) {
   entity.set_map(doc.valueString("map").toStdString());
  }
 else if ( true == doc.hasField("job") ) {
   meteo::map::proto::Job job;
   auto jd = doc.valueDocument("job");
   job.set_name(jd.valueString("name").toStdString());
   job.set_title(jd.valueString("title").toStdString());
   job.set_map_id(jd.valueString("map").toStdString());
   entity.mutable_job()->CopyFrom(job);
  }
 else if ( true == doc.hasField("delimiter") ) {
   entity.set_delimiter(true);
  }

 if ( true == doc.hasField("date_format") ) {
   DateFormat date_format;
   auto df = doc.valueDocument("date_format");
   if ( true == df.hasField("year") ) {
     DateFormatElement year;
     auto e = df.valueDocument("year");
     year.set_index(e.valueInt32("index"));
     year.set_regexp(e.valueString("regexp").toStdString());
     year.set_format_string(e.valueString("format_string").toStdString());
     date_format.mutable_year()->CopyFrom(year);
    }
   if ( true == df.hasField("month") ) {
     DateFormatElement month;
     auto e = df.valueDocument("month");
     month.set_index(e.valueInt32("index"));
     month.set_regexp(e.valueString("regexp").toStdString());
     month.set_format_string(e.valueString("format_string").toStdString());
     date_format.mutable_month()->CopyFrom(month);
    }
   if ( true == df.hasField("day") ) {
     DateFormatElement day;
     auto e = df.valueDocument("day");
     day.set_index(e.valueInt32("index"));
     day.set_regexp(e.valueString("regexp").toStdString());
     day.set_format_string(e.valueString("format_string").toStdString());
     date_format.mutable_day()->CopyFrom(day);
    }
   if ( true == df.hasField("hour") ) {
     DateFormatElement hour;
     auto e = df.valueDocument("hour");
     hour.set_index(e.valueInt32("index"));
     hour.set_regexp(e.valueString("regexp").toStdString());
     hour.set_format_string(e.valueString("format_string").toStdString());
     date_format.mutable_hour()->CopyFrom(hour);
    }
   if ( true == df.hasField("minute") ) {
     DateFormatElement minute;
     auto e = df.valueDocument("minute");
     minute.set_index(e.valueInt32("index"));
     minute.set_regexp(e.valueString("regexp").toStdString());
     minute.set_format_string(e.valueString("format_string").toStdString());
     date_format.mutable_minute()->CopyFrom(minute);
    }
   entity.mutable_date_format()->CopyFrom(date_format);
  }

 return entity;
}

void TapControlServiceMethodBag::UserAppointmentSetList (const ::meteo::app::UserAppointmentListRequest* request,
                                                         ::meteo::app::DefaultResponce* response)
{
 auto errorFunc = [response] (const QString& message) {
   error_log << message;
   response->set_result(false);
   response->set_comment(message.toStdString());
   return;
  };
 static auto setListQueryName = QObject::tr("set_appointments_for_user");
 static auto unsetAllQueryName = QObject::tr("user_appointment_remove_from_all");

 auto queryTemplate = (0 == request->id_size()) ? unsetAllQueryName : setListQueryName;
 std::unique_ptr<Dbi> db(meteo::global::dbUsers());
 if ( nullptr == db.get() ) {
   static const QString& emessage =  meteo::msglog::kDbConnectHostPortFailed;
   return errorFunc(emessage);
  }
 auto query = db->queryptrByName(queryTemplate);
 if(nullptr == query) {
   static const QString& emessage = meteo::msglog::kDbRequestNotFound.arg(queryTemplate);
   return errorFunc(emessage);
  }

 auto login = QString::fromStdString(request->login());
 auto uid = auth_->getUserId(login);

 if ( 0 != request->id_size() ){
   query->argOid("id",request->id());
  }
 query->argOid("users",uid.toStdString());

 if ( false == query->exec() ){
   return errorFunc(msglog::kDbRequestFailedArg.arg(query->query()));
  }
 const DbiEntry& result = query->result();
 auto ok = qRound(result.valueDouble("ok"));
 if ( 1 != ok ){
   return errorFunc(msglog::kDbRequestFailedArg.arg(result.jsonExtendedString()));
  }
 response->set_result(true);
}

void TapControlServiceMethodBag::SavePreset(const FolderEntity* req, DefaultResponce *resp)
{
 const QString& userLogin  = QString::fromStdString(req->title());
 static const QStringList excludeList = { QObject::tr("AnonymousUser") };
 if ( true == excludeList.contains(userLogin) ){
   error_log << QObject::tr("Сохранение настроек пользователя %1 не предусмотрена")
                .arg(userLogin);
   return;
 }

 bool result = UserSettings::saveSettings(*req, userLogin);
 resp->set_result(result);
}

void TapControlServiceMethodBag::GetPreset(const FolderEntity *req, FolderEntity *resp)
{
  bool loaded = UserSettings::loadSettings(resp, req->title());

  if ( false == loaded ){
    static const std::string defaultUserName = "AnonymousUser";
    loaded = UserSettings::loadSettings(resp, defaultUserName);
  }

  if ( false == loaded ){
    resp->set_result(false);
    error_log << QObject::tr("Не удалось загрузить настройки пользователя")
              << req->title();
    return;
  }
  resp->set_result(true);
}

void TapControlServiceMethodBag::SaveIcon(const EntityIcon *req, DefaultResponce *resp)
{
 resp->set_result(false);
 auto errorFunc = [resp](QString err)
 {
  error_log << err;
  resp->set_result(false);
  resp->set_comment(err.toStdString());
  return;
 };

 GridFs gridfs;
 auto dbparams = meteo::global::mongodbConfDocument();
 if ( false == gridfs.connect( dbparams ) ) {
   return errorFunc(QObject::tr("Не удалось подключиться к БД с параметрами подключения %1:%2")
                    .arg( dbparams.host() )
                    .arg( dbparams.port() ));
  }
 gridfs.use( kUsersDb, "icons" );
 QByteArray bar(req->icon().data(), req->icon().size());

 GridFile file;
 bool res = gridfs.put( QString::fromStdString(req->name()), bar, &file );
 if ( false == res ) {
   return errorFunc(QObject::tr("Не удалось сохранить файл в GRIDFS"));
  }
 resp->set_result(true);
 resp->set_comment(file.id().toStdString());
 return;
}

void TapControlServiceMethodBag::GetIcon(const EntityIcon *req, EntityIcon *resp)
{
 resp->set_result(false);
 auto errorFunc = [resp](QString err)
 {
  error_log << err;
  resp->set_result(false);
  resp->set_comment(err.toStdString());
  return;
 };


 GridFs gridfs;
 auto documentDbConf = meteo::global::mongodbConfDocument();
 if ( false == gridfs.connect( documentDbConf ) ) {
   return errorFunc(QObject::tr("Не удалось подключиться к БД документов"));
  }

 gridfs.use( kUsersDb, "icons" );
 GridFile file = ( true == req->has_id() ) ? gridfs.findOneById(req->id())
                                           : gridfs.findOneByName(req->name());

 if ( false == file.isValid() ) {
   resp->set_result(false);
   return;
  }

 bool result;
 QByteArray arr = file.readAll( &result );
 if ( false == result ) {
   return errorFunc(QObject::tr("Не удалось прочитать файл с именем '%1', id = %2")
                    .arg(QString::fromStdString(req->name()))
                    .arg(QString::fromStdString(req->id())));
  }
 resp->set_icon( arr.data(), arr.size() );
 resp->set_result(true);
 return;
}

void TapControlServiceMethodBag::PositionCreate (const ::meteo::app::PositionCreateRequest* request,
                                                 ::meteo::app::PositionCreateResponce* response)
{
 auto errorFunc = [response] ( const QString& emessage ){
   response->set_result(false);
   response->set_comment(emessage.toStdString());
   error_log << emessage;
  };
 static auto queryTemplate = QObject::tr("user_position_insert");

 std::unique_ptr<Dbi> db(meteo::global::dbUsers());
 if ( nullptr == db.get() ) {
   static const QString& emessage =  meteo::msglog::kDbConnectHostPortFailed;
   return errorFunc(emessage);
  }
 auto query = db->queryptrByName(queryTemplate);
 if(nullptr == query) {
   static const QString& emessage = meteo::msglog::kDbRequestNotFound.arg(queryTemplate);
   return errorFunc(emessage);
  }

 if ( true == request->has_name() ) {
   query->arg("name",request->name());
  }
 if ( true == request->has_parent_position() ){
   query->arg("parent",QString::fromStdString(request->parent_position()).toInt());
  }
 if ( true == request->has_current_login() ) {
   query->arg("current",request->current_login());
  }



 if ( false == query->exec() ){
   auto emsg = msglog::kDbRequestFailedArg.arg(query->query());
   return errorFunc(emsg);
  }

 const DbiEntry& result = query->result();
 auto ok = qRound(result.valueDouble("ok"));
 if ( 1 != ok ){
   auto emsg = msglog::kDbRequestFailedArg.arg(result.jsonExtendedString());
   return errorFunc(emsg);
  }

 response->set_result(true);
}

void TapControlServiceMethodBag::PositionsList (const ::meteo::app::PositionsListRequest* request,
                                                ::meteo::app::PositionsListResponce* response)
{
 Q_UNUSED(request);
 static auto queryTemplate = QObject::tr("find_positions");
 auto errorFunc = [response] ( const QString& emessage ){
   response->set_result(false);
   response->set_comment(emessage.toStdString());
   error_log << emessage;
  };
 std::unique_ptr<Dbi> db(meteo::global::dbUsers());
 if ( nullptr == db.get() ) {
   static const QString& emessage =  meteo::msglog::kDbConnectHostPortFailed;
   return errorFunc(emessage);
  }
 auto query = db->queryptrByName(queryTemplate);
 if(nullptr == query) {
   static const QString& emessage = meteo::msglog::kDbRequestNotFound.arg(queryTemplate);
   return errorFunc(emessage);
  }

 QString err;
 if ( false == query->execInit(&err) ){
   return errorFunc(err);
  }

 while ( query->next() ) {
   const DbiEntry& doc = query->entry();
   auto positionProto = response->add_list();
   auto id = doc.valueOid("_id");
   auto name = doc.valueString("name");

   positionProto->set_id(id.toStdString());
   positionProto->set_name(name.toStdString());
   if ( true == doc.hasField("parent") ) {
     auto parent = doc.valueString("parent");
     positionProto->set_parent_position(parent.toStdString());
    }
   if ( true == doc.hasField("current") ){
     auto current = doc.valueString("current");
     positionProto->set_current_login(current.toStdString());
    }
  }
 response->set_result(true);
}

void TapControlServiceMethodBag::PositionEdit(const ::meteo::app::PositionEditRequest* request,
                                              ::meteo::app::PositionEditResponce* response)
{
 static auto queryTemplate = QObject::tr("user_position_update");
 auto errorFunc = [response] ( const QString& emessage ){
   response->set_result(false);
   response->set_comment(emessage.toStdString());
   error_log << emessage;
  };

 std::unique_ptr<Dbi> db(meteo::global::dbUsers());
 if ( nullptr == db.get() ) {
   static const QString& emessage =  meteo::msglog::kDbConnectHostPortFailed;
   return errorFunc(emessage);
  }
 auto query = db->queryptrByName(queryTemplate);
 if(nullptr == query) {
   static const QString& emessage = meteo::msglog::kDbRequestNotFound.arg(queryTemplate);
   return errorFunc(emessage);
  }
 query->argOid("id",request->id());
 query->arg("name",request->name());
 if ( true == request->has_parent_position() ){
   query->arg("parent",QString::fromStdString(request->parent_position()).toInt());
  }
 if ( true == request->has_current_login() ){
   query->arg("current",request->current_login());
  }


 if ( false == query->exec()){
   auto emsg = msglog::kDbRequestFailedArg.arg(query->query());
   return errorFunc(emsg);
  }

 const DbiEntry& result = query->result();
 auto ok = result.valueDouble("ok");
 auto doc = result.valueDocument("lastErrorObject");
 auto updates = doc.valueBool("updatedExisting");
 auto n = doc.valueInt32("n");

 if ( qRound(ok) != 1 || false == updates || 1 != n) {
   auto emsg = msglog::kDbRequestFailedArg.arg(result.jsonExtendedString());
   return errorFunc(emsg);
  }

 response->set_result(true);
}

void TapControlServiceMethodBag::PositionDelete (const ::meteo::app::PositionDeleteRequest* request,
                                                 ::meteo::app::PositionDeleteResponce* response)
{
 auto errorFunc = [response] (const QString& emsg){
   response->set_result(false);
   response->set_comment(emsg.toStdString());
   error_log << emsg;
  };
 static auto queryTemplate = QObject::tr("user_position_remove");

 std::unique_ptr<Dbi> db(meteo::global::dbUsers());
 if ( nullptr == db.get() ) {
   static const QString& emessage =  meteo::msglog::kDbConnectHostPortFailed;
   return errorFunc(emessage);
  }
 auto query = db->queryptrByName(queryTemplate);
 if(nullptr == query) {
   static const QString& emessage = meteo::msglog::kDbRequestNotFound.arg(queryTemplate);
   return errorFunc(emessage);
  }

 query->argOid("id",request->id());


 if ( false == query->exec() ){
   auto emsg = msglog::kDbRequestFailedArg.arg(query->query());
   return errorFunc(emsg);
  }

 const DbiEntry& result = query->result();
 auto ok = qRound(result.valueDouble("ok"));
 if ( ok != 1 ){
   auto emsg = msglog::kDbRequestFailedArg.arg(result.jsonExtendedString());
   return errorFunc(emsg);
  }
 auto n = qRound(result.valueDouble("n"));
 if ( n != 1 ){
   auto emsg = QObject::tr("Должностная обязанность не найдена. Возможно она была удалена ранее.");
   response->set_result(true);
   response->set_comment(emsg.toStdString());
   warning_log << emsg;
   return ;
  }
 response->set_result(true);
}


void TapControlServiceMethodBag::UserPogodaInputStsSave(const ::meteo::app::UserPogodaInputStationsSaveRequest* request,
                                                        ::meteo::app::PogodaInputStationsSaveResponce* response)
{
 auto errorFunc = [response](const QString& emsg){
   error_log << emsg;
   response->set_result(false);
   response->set_comment(emsg.toStdString());
  };

 if ( 0 == request->data_size() ) {
   auto emsg = QObject::tr("Ошибка: попытка загрузки настроек пользователя без указания пользователя");
   return errorFunc(emsg);
  }

 for ( auto data : request->data() ){
   UserSettings::saveSettings(data.param(), data.login());
  }

 response->set_result(true);
}



void TapControlServiceMethodBag::UserPogodaInputStsLoad(const ::meteo::app::UserPogodaInputStationsLoadRequest* request,
                                                        ::meteo::app::PogodaInputStationsLoadResponce* response)
{
 auto errorFunc = [response](const QString& emsg){
   error_log << emsg;
   response->set_result(false);
   response->set_comment(emsg.toStdString());
  };

 if ( 0 == request->login_size() ) {
   auto emsg = QObject::tr("Ошибка: попытка загрузки настроек пользователя без указания пользователя");
   return errorFunc(emsg);
  }

 QHash<QString, google::protobuf::Message*> msgs;
 for ( int i = 0; i < request->login_size(); ++i){
   auto login = QString::fromStdString(request->login(i));
   auto data = response->add_data();
   data->set_login(login.toStdString());
   google::protobuf::Message* proto = data->mutable_param();
   msgs[login] = proto;
  }

 bool result = UserSettings::loadSettingsMany(msgs);
 if ( false == result ){
   auto emsg = QObject::tr("Ошибка при взаимодействии с базой данных");
   return errorFunc(emsg);
  }
 response->set_result(true);
}

void TapControlServiceMethodBag::SaveUserSetStations(const ::meteo::app::UserSetStations *request, ::meteo::app::DefaultResponce *response)
{
  if ( nullptr == request || nullptr == response) {
    error_log << "nullptr here";
    return;
  }
  UserSettings::saveSettings(*request, request->login());
  response->set_result(true);
}

void TapControlServiceMethodBag::GetUserSetStations(const ::meteo::app::UserSetStations *request, ::meteo::app::UserSetStations *response)
{
  bool loaded = UserSettings::loadSettings(response, request->login());
  if ( false == loaded ){
    error_log << QObject::tr("Не удалось загрузить настройки пользователя")
              << request->login();
    return;
  }
  response->set_result(true);
}

void  TapControlServiceMethodBag::msgviewerUserSettingsSave( const ::meteo::app::MsgViewerUserSettingsSaveRequest* request,
                                                             ::meteo::app::Dummy* response)
{
  Q_UNUSED(response);
  QString username = QString::fromStdString(request->username());
  auto conf = request->config();

  bool result = UserSettings::saveSettings(conf, username);
  if ( false == result ){
    error_log << QObject::tr("Не удалось сохранить настройки пользователя");
    return;
  }
}

void  TapControlServiceMethodBag::msgviewerUserSettingsLoad( const ::meteo::app::MsgViewerUserSettingsLoadRequest* request,
                                                             ::meteo::proto::CustomViewerUserConfig* response )
{
  bool ok = UserSettings::loadSettings(response, request->username());
  if ( false == ok ){
    error_log << QObject::tr("Не удалось загрузить настройки пользователя");
  }
}

void TapControlServiceMethodBag::SaveCustomPreset( const ::meteo::app::CustomSettings* req, ::meteo::app::Result* resp )
{
  auto errorFunc = [resp]( const QString& msg ) {
    error_log << msg;
    resp->set_comment( msg.toStdString() );
    resp->set_result(false);
  };
  if ( false == req->has_login() ) {
    return errorFunc( QObject::tr("Не указан логин пользователя. Сохранение настроек невозможно.") );
  }
  if ( false == req->has_params() ) {
    return errorFunc( QObject::tr("Нет параметров. Сохраненять нечего, выход.") );
  }
  GridFs gridfs;
  ConnectProp param = Global::instance()->mongodbConfUser();
  if ( false == gridfs.connect( param ) ) {
    return errorFunc( gridfs.lastError() );
  }
  gridfs.use( param.name(), "fs" );
  GridFile file;
  QString filename = QString::fromStdString( req->login() + "_" + req->name() );
  QByteArray raw( req->params().data(), req->params().size() );
  if ( false == gridfs.put( filename, raw, &file ) && GridFs::kErrorDuplicate != gridfs.error() ) {
    return errorFunc( gridfs.lastError() );
  }

  std::unique_ptr<Dbi> db(meteo::global::dbUsers());
  if ( nullptr == db ) {
    return errorFunc(meteo::msglog::kDbConnectHostPortFailed);
  }
  QString queryname("save_custom_settings");
  auto query = db->queryptrByName(queryname);
  if ( nullptr == query ) {
    return errorFunc( meteo::msglog::kDbRequestNotFound.arg(queryname) );
  }
  query->arg( "login", req->login() );
  query->arg( "name", req->name() );
  query->argOid( "fileid", file.id() );

  if ( false == query->exec() ) {
    return errorFunc( msglog::kDbRequestFailedArg.arg( query->query() ) );
  }

  const DbiEntry& result = query->result();
  auto res = result.valueBool("result");
  if ( false == res ) {
    QString err = QObject::tr("Ошибка сохранения настроек = %1 %2")
      .arg( QString::fromStdString( req->login() ) )
      .arg( QString::fromStdString( req->name() ) );
    return errorFunc(err);
  }
  resp->set_result(true);
}

void TapControlServiceMethodBag::LoadCustomPreset( const ::meteo::app::CustomSettings* req, ::meteo::app::CustomSettings* resp )
{
  auto errorFunc = [resp]( const QString& msg ) {
    error_log << msg;
    resp->set_comment( msg.toStdString() );
    resp->set_result(false);
  };
  std::unique_ptr<Dbi> db(meteo::global::dbUsers());
  if ( nullptr == db ) {
    return errorFunc(meteo::msglog::kDbConnectHostPortFailed);
  }
  QString queryname("load_custom_settings");
  auto query = db->queryptrByName(queryname);
  if ( nullptr == query ) {
    return errorFunc( meteo::msglog::kDbRequestNotFound.arg(queryname) );
  }
  query->arg( "login", req->login() );
  query->arg( "name", req->name() );

  if ( false == query->exec() ) {
    return errorFunc( msglog::kDbRequestFailedArg.arg( query->query() ) );
  }

  const DbiEntry& result = query->result();
  auto fileid = result.valueOid("fileid");
  if ( true == fileid.isEmpty() ) {
    return errorFunc( QObject::tr("Настройки %1 для пользователя %2 не найдены")
        .arg( QString::fromStdString( req->name() ) )
        .arg( QString::fromStdString( req->login() ) )
        );
  }

  GridFs gridfs;
  if ( false == gridfs.connect( db->params() ) ) {
    return errorFunc( gridfs.lastError() );
  }
  gridfs.use( db->params().name(), "fs" );
  GridFile file = gridfs.findOneById(fileid);
  if ( false == file.hasFile() ) {
    return errorFunc( QObject::tr("Файл %1 не найден").arg(fileid) );
  }

  bool ok = false;
  QByteArray raw = file.readAll(&ok);
  if ( false == ok ) {
    return errorFunc( QObject::tr("Не удалось прочитать файл %1").arg(fileid) );
  }

  resp->set_login( req->login() );
  resp->set_name( req->name() );
  resp->set_params( raw.toStdString() );
  resp->set_result(true);
}

void TapControlServiceMethodBag::ProcState(const ::meteo::app::AppState::Proc* request,
                                           ::meteo::app::AppState::Proc* response)
{
  Q_UNUSED(request);
  Q_UNUSED(response);
  //TODO - meteo::app::Control не поддерживает многопоточность
}

}
}
