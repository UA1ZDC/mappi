#include "authhandler.h"
#include <meteo/commons/global/global.h>
#include <sql/nosql/nosqlquery.h>


namespace meteo {

namespace app {

using namespace auth;

static const QString kUsersInitFailed = QObject::tr("Ошибка инициализации подключения к системе пользователей");

AuthHandler::AuthHandler() :
    auth_(new auth::Auth())
{
  isInit_ = auth_->init(Auth::kCachedDirect);
}

AuthHandler::~AuthHandler()
{
  delete auth_;
  auth_ = nullptr;
}

QString AuthHandler::getUserId(const QString& username)
{
  std::unique_ptr<Dbi> db(global::dbUsers());
  if (db.get() == nullptr) {
    error_log << msglog::kDbConnectHostPortFailed;
    return QString();
  }

  QString queryName = "auth_user_by_name";
  auto query = db->queryptrByName(queryName);
  if (query == nullptr) {
    error_log << msglog::kDbRequestNotFound.arg(queryName);
    return QString();
  }

  query->arg("username", username);

  QString err;
  if (!query->execInit(&err)) {
    error_log << err;
    return QString();
  }

  if (query->next()) {
    const DbiEntry& doc = query->entry();
    return doc.valueString("id");
  }

  return QString();
}

QString AuthHandler::getUserLogin(const QString& id)
{
  bool isOk = true;
  int v = id.toInt(&isOk);
  if (isOk)
    return getUserLogin(v);

  return QString();
}

QString AuthHandler::getUserLogin(const int32_t& id)
{
  std::unique_ptr<Dbi> db(global::dbUsers());
  if (db.get() == nullptr) {
    error_log << msglog::kDbConnectHostPortFailed;
    return QString();
  }

  QString queryName = "auth_user_by_id";
  auto query = db->queryptrByName(queryName);
  if (query == nullptr) {
    error_log << msglog::kDbRequestNotFound.arg(queryName);
    return QString();
  }

  query->arg("id", id);

  QString err;
  if (!query->execInit(&err)) {
    error_log << err;
    return QString();
  }

  if (query->next()) {
    const DbiEntry& doc = query->entry();
    return doc.valueString("username");
  }

  return QString();
}

void AuthHandler::AuthUser(const UserAuthRequest* req, UserAuthResponse* resp)
{
  if (!isInit_) {
    resp->set_comment(kUsersInitFailed.toStdString());
    return ;
  }

  resp->set_result(true);
  resp->set_status(auth_->tryAuth(req->username().c_str(), req->password().c_str()));
  // resp->set_status(true);
}

void AuthHandler::NewUser(const UserRequest* req, UserResponse* resp)
{
  RegisterUser(req, resp);
  if (resp->result() == false)
    return;

  resp->set_result(false);
  AddUser(req, resp);
}

void AuthHandler::AddUser(const UserRequest* req, UserResponse* resp)
{
  if (!isInit_) {
    resp->set_comment(kUsersInitFailed.toStdString());
    return ;
  }

  app::User user = req->user();
  
  // debug_log << req->DebugString();

  if (user.has_surname() || user.has_name() || user.has_patron()) {
    Gecos gecos(user.surname().c_str(), user.name().c_str(), user.patron().c_str());
    if (auth_->setGecos(user.username().c_str(), gecos) == false) {
      resp->set_comment(QObject::tr("Ошибка установки gecos").toStdString());
      return ;
    }
  }

  std::unique_ptr<Dbi> db(global::dbUsers());
  if (db.get() == nullptr) {
    resp->set_comment(msglog::kDbConnectHostPortFailed.toStdString());
    return ;
  }

  QString queryName = "auth_user_put";
  auto query = db->queryptrByName(queryName);
  if (query == nullptr) {
    QString msg = msglog::kDbRequestNotFound.arg(queryName);
    resp->set_comment(msg.toStdString());
    return ;
  }

  query->arg("id", user.id());
  query->arg("username", user.username());
  if (user.has_role())
    query->arg("role", user.role().id());

  if (user.has_rank())
    query->arg("rank", user.rank().id());

  if (user.has_gender())
    query->arg("gender", user.gender());

  if (user.has_department())
    query->arg("department", user.department().id());

  if (user.permissions().has_inter_contact_mode())
     query->arg("permissions",user.permissions().inter_contact_mode());

  if (user.has_birthdate())
    query->arg("birthdate", user.birthdate());

  if (user.has_block())
    query->arg("block", user.block());
  else
    query->arg("block", false);

  if (user.has_avatar())
    query->arg("avatar", user.avatar());

  // debug_log<<query->query();

  if (!query->exec()) {
    resp->set_comment(msglog::kDbRequestFailed.toStdString());
    return ;
  }

  resp->set_result(true);
}

void AuthHandler::RegisterUser(const UserRequest* req, UserResponse* resp)
{
  if (!isInit_) {
    resp->set_comment(kUsersInitFailed.toStdString());
    return ;
  }

  QString msg;
  app::User user = req->user();
  if (auth_->registry(user.username().c_str(), user.password().c_str(), &msg) == false) {
    resp->set_comment(msg.toStdString());
    return ;
  }

  resp->set_result(true);
}

void AuthHandler::UpdateUser(const UserRequest* req, UserResponse* resp)
{
  AddUser(req, resp);
}

void AuthHandler::BlockUser(const UserRequest* req, UserResponse* resp)
{
  blockUser(req, resp, true);
}

void AuthHandler::UnlockUser(const UserRequest* req, UserResponse* resp)
{
  blockUser(req, resp, false);
}

void AuthHandler::HomeDirUser(const UserRequest* req, UserResponse* resp)
{
  if (!isInit_) {
    resp->set_comment(kUsersInitFailed.toStdString());
    return ;
  }

  QString dir = auth_->homeDir(req->user().username().c_str());
  if (dir.isEmpty()) {
    resp->set_comment(QObject::tr("Ошибка получения домашней директории пользователя").toStdString());
    return ;
  }

  resp->set_result(true);
  app::User* user = resp->add_list();
  user->set_homedir(dir.toStdString());
}

void AuthHandler::CheckUser(const UserRequest* req, UserResponse* resp)
{
  std::unique_ptr<Dbi> db(global::dbUsers());
  if (db.get() == nullptr) {
    resp->set_comment(msglog::kDbConnectHostPortFailed.toStdString());
    return ;
  }

  QString queryName = "auth_user_by_name";
  auto query = db->queryptrByName(queryName);
  if (query == nullptr) {
    QString msg = msglog::kDbRequestNotFound.arg(queryName);
    resp->set_comment(msg.toStdString());
    return ;
  }

  app::User user = req->user();
  query->arg("username", user.username());

  QString err;
  if (!query->execInit(&err)) {
    resp->set_comment(err.toStdString());
    return ;
  }

  resp->set_result(query->next());
}

void AuthHandler::UserList(const Dummy* req, UserResponse* resp)
{
  Q_UNUSED(req)
  if (!isInit_) {
    resp->set_comment(kUsersInitFailed.toStdString());
    return ;
  }

  std::unique_ptr<Dbi> db(global::dbUsers());
  if (db.get() == nullptr) {
    resp->set_comment(msglog::kDbConnectHostPortFailed.toStdString());
    return ;
  }

  QString queryName = "auth_user_list";
  auto query = db->queryptrByName(queryName);
  if (query == nullptr) {
    QString msg = msglog::kDbRequestNotFound.arg(queryName);
    resp->set_comment(msg.toStdString());
    return ;
  }

  QString err;
  if (!query->execInit(&err)) {
    resp->set_comment(err.toStdString());
    return ;
  }

  for (int i = 0; query->next(); ++i) {
    const DbiEntry& doc = query->entry();

    QString username = doc.valueString("username");
    app::User* user = resp->add_list();
    user->set_id(doc.valueInt32("id"));
    user->set_username(username.toUtf8());

    // default, Male
    int gender = doc.valueInt32("gender");
    if (gender == User_GenderType::User_GenderType_Female)
      user->set_gender(User_GenderType::User_GenderType_Female);
    else
      user->set_gender(User_GenderType::User_GenderType_Male);

    // WARNING не хранится в бд
    // user->set_passwrod();

    if (doc.hasField("birthdate"))
      user->set_birthdate(doc.valueString("birthdate").toStdString());

    if (doc.hasField("avatar"))
      user->set_avatar(doc.valueString("avatar").toStdString());

    if (doc.hasField("block"))
      user->set_block(doc.valueBool("block"));

    if (doc.hasField("role"))
      user->mutable_role()->set_id(doc.valueInt32("role"));

    if (doc.hasField("rank"))
      user->mutable_rank()->set_id(doc.valueInt32("rank"));

    if (doc.hasField("department"))
      user->mutable_department()->set_id(doc.valueInt32("department"));

    if (doc.hasField("permissions")) {
      auto permission = doc.valueDocument("permissions");
      if (permission.hasField("contact_visibility")) {
        user->mutable_permissions()->set_inter_contact_mode(
          static_cast<UserPermissions_InterContactMode>(permission.valueInt32("contact_visibility"))
        );
      }
    } else {
      user->mutable_permissions()->set_inter_contact_mode(
        UserPermissions_InterContactMode::UserPermissions_InterContactMode_kHierarchical
      );
    }

    Gecos gecos;
    if (!auth_->getGecos(username, gecos)) {
      resp->set_comment(QObject::tr("Ошибка получения gecos").toStdString());
      return ;
    }

    user->set_surname(gecos.surname().toStdString());
    user->set_name(gecos.name().toStdString());
    user->set_patron(gecos.patron().toStdString());
  }

  resp->set_result(true);
}

void AuthHandler::RoleList(const Dummy* req, RoleResponse* resp)
{
  Q_UNUSED(req)
  std::unique_ptr<Dbi> db(global::dbUsers());
  if (db.get() == nullptr) {
    resp->set_comment(msglog::kDbConnectHostPortFailed.toStdString());
    return ;
  }

  QString queryName = "auth_role_list";
  auto query = db->queryptrByName(queryName);
  if (query == nullptr) {
    QString msg = msglog::kDbRequestNotFound.arg(queryName);
    resp->set_comment(msg.toStdString());
    return ;
  }

  QString err;
  if (!query->execInit(&err)) {
    resp->set_comment(err.toStdString());
    return ;
  }

  resp->set_result(true);
  while (query->next()) {
    const DbiEntry& doc = query->entry();
    app::Role* item = resp->add_list();
    item->set_id(doc.valueInt32("id"));
    item->set_name(doc.valueString("name").toStdString());
  }
}

void AuthHandler::RankList(const Dummy* req, RankResponse* resp)
{
  Q_UNUSED(req);
  std::unique_ptr<Dbi> db(global::dbUsers());
  if (db.get() == nullptr) {
    resp->set_comment(msglog::kDbConnectHostPortFailed.toStdString());
    return ;
  }

  QString queryName = "auth_rank_list";
  auto query = db->queryptrByName(queryName);
  if (query == nullptr) {
    QString msg = msglog::kDbRequestNotFound.arg(queryName);
    resp->set_comment(msg.toStdString());
    return ;
  }

  QString err;
  if (!query->execInit(&err)) {
    resp->set_comment(err.toStdString());
    return ;
  }

  resp->set_result(true);
  while (query->next()) {
    const DbiEntry& doc = query->entry();
    app::Rank* item = resp->add_list();
    item->set_id(doc.valueInt32("id"));
    item->set_name(doc.valueString("name").toStdString());
  }
}

void AuthHandler::PutRank(const Rank* req, DefaultResponce* resp)
{
  resp->set_result(false);

  std::unique_ptr<Dbi> db(global::dbUsers());
  if (db.get() == nullptr) {
    resp->set_comment(msglog::kDbConnectHostPortFailed.toStdString());
    return ;
  }

  QString queryName = "auth_rank_put";
  auto query = db->queryptrByName(queryName);
  if (query == nullptr) {
    QString msg = msglog::kDbRequestNotFound.arg(queryName);
    resp->set_comment(msg.toStdString());
    return ;
  }

  query->arg("id", req->id());
  query->arg("name", req->name());
  if (!query->exec()) {
    resp->set_comment(msglog::kDbRequestFailed.toStdString());
    return ;
  }

  resp->set_result(true);
}

void AuthHandler::DepartmentList(const Dummy* req, DepartmentResponse* resp)
{
  Q_UNUSED(req);
  std::unique_ptr<Dbi> db(global::dbUsers());
  if (db.get() == nullptr) {
    resp->set_comment(msglog::kDbConnectHostPortFailed.toStdString());
    return ;
  }

  QString queryName = "auth_department_list";
  std::unique_ptr<DbiQuery> query = db->queryptrByName(queryName);
  if (query == nullptr) {
    QString msg = msglog::kDbRequestNotFound.arg(queryName);
    resp->set_comment(msg.toStdString());
    return ;
  }

  QString err;
  if (!query->execInit(&err)) {
    resp->set_comment(err.toStdString());
    return ;
  }

  resp->set_result(true);
  while (query->next()) {
    const DbiEntry& doc = query->entry();
    app::Department* item = resp->add_list();
    item->set_id(doc.valueInt32("id"));
    item->set_name(doc.valueString("name").toStdString());
  }
}

void AuthHandler::PutDepartment(const Department* req, DefaultResponce* resp)
{
  resp->set_result(false);

  std::unique_ptr<Dbi> db(global::dbUsers());
  if (db.get() == nullptr) {
    resp->set_comment(msglog::kDbConnectHostPortFailed.toStdString());
    return ;
  }

  QString queryName = "auth_department_put";
  auto query = db->queryptrByName(queryName);
  if (query == nullptr) {
    QString msg = msglog::kDbRequestNotFound.arg(queryName);
    resp->set_comment(msg.toStdString());
    return ;
  }

  query->arg("id", req->id());
  query->arg("name", req->name());
  if (!query->exec()) {
    resp->set_comment(msglog::kDbRequestFailed.toStdString());
    return ;
  }

  resp->set_result(true);
}

void AuthHandler::blockUser(const UserRequest* req, UserResponse* resp, bool isBlock)
{
  if (!isInit_) {
    resp->set_comment(kUsersInitFailed.toStdString());
    return ;
  }

  std::unique_ptr<Dbi> db(global::dbUsers());
  if (db.get() == nullptr) {
    resp->set_comment(msglog::kDbConnectHostPortFailed.toStdString());
    return ;
  }

  QString queryName = "auth_user_block";
  auto query = db->queryptrByName(queryName);
  if (query == nullptr) {
    QString msg = msglog::kDbRequestNotFound.arg(queryName);
    resp->set_comment(msg.toStdString());
    return ;
  }

  app::User user = req->user();
  query->arg("username", user.username());
  query->arg("block", isBlock);
  if (!query->exec()) {
    resp->set_comment(msglog::kDbRequestFailed.toStdString());
    return ;
  }

  resp->set_result(true);
}

}

}
