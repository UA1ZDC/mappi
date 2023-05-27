#include "test.h"

#include <sql/nosql/nosqlquery.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/appconf.pb.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/global/dbnames.h>

using namespace meteo;

static auto kQueryNameDropCollection = QObject::tr("drop_collection");
static auto kQueryNameCountCollection = QObject::tr("count_collection");
static auto kQueryNameFindCollection = QObject::tr("find_collection");


bool Test::dropCollection(const ConnectProp& conf, const QString& kDbName, const QString& collectionName)
{
  if ( 0 == countCollection(conf, kDbName, collectionName) ){
    return true;
  }
  nosql::NosqlQuery query(meteo::global::kMongoQueriesNew[kQueryNameDropCollection]);
  query.arg(collectionName);

  nosql::NoSql db(conf.host(), conf.port().toUShort());
  if ( false == db.connect() )
  {
    return false;
  }
  if (false== db.execQuery(kDbName, query.query())){
    return false;
  }
  if ( 0 !=  countCollection(conf, kDbName, collectionName) ){
    return false;
  }
  return true;
}

int Test::countCollection(const ConnectProp& conf, const QString& kDbName, const QString& collection)
{
  nosql::NosqlQuery query(global::kMongoQueriesNew[kQueryNameCountCollection]);
  query.arg(collection);
  nosql::NoSql db(conf.host(), conf.port().toUShort());
  if (false == db.connect()){
    return -1;
  }
  if (false == db.execQuery(kDbName, query.query())){
    return -1;
  }
  auto result = db.result();
  bool res = false;
  auto ok = qRound(result.valueDouble("ok", &res));
  if ( false == res ){
    return -1;
  }
  auto n = result.valueInt32("n", &res);
  if ( false == res){
    return -1;
  }
  if ( 1 != ok ){
    return -1;
  }
  return n;
}

bool Test::dropDepartments()
{
  return dropCollection(global::mongodbConfUsers(),
                        db::kDbUsers,
                        db::usersdb::kDepartments);
}

bool Test::dropUsers()
{
  return dropCollection(global::mongodbConfMeteo(),
                        db::kDbUsers,
                        db::usersdb::kUsers);
}

int Test::countDepartments()
{
  return countCollection(global::mongodbConfUsers(),
                         db::kDbUsers,
                         db::usersdb::kDepartments);
}

int Test::countUsers()
{
  return countCollection(global::mongodbConfUsers(),
                         db::kDbUsers,
                         db::usersdb::kUsers);
}



void Test::initTestCase()
{
  TAPPLICATION_NAME("meteo");
  ::setlocale(LC_NUMERIC, "C"); // локаль для записи координат в mongo (libbson)

  gSettings(global::Settings::instance());
  if ( false == global::Settings::instance()->load() ){
    error_log << msglog::kSettingsLoadFailed;
    return;
  }
}

void Test::testNosqlQuery()
{
  QStringList queryList = {
    kQueryNameDropCollection,
    kQueryNameCountCollection,
    kQueryNameFindCollection,
  };
  for (auto query: queryList ){
    assert( true == global::kMongoQueriesNew.contains(query));
  }
}

app::Department departmentProto(int i)
{
  app::Department dep;
  dep.set_name(QObject::tr("Подразделение %1").arg(i).toStdString());
  return dep;
}

void Test::testSaveLoadDepartments()
{
  QVERIFY(true == this->dropDepartments() );
  QStringList departments({"dep1", "dep2", "dep3"});
  //Создаем новые подразделения
  for ( auto dep : departments)
  {
    auto channel = std::unique_ptr<rpc::Channel>(meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic));
    QVERIFY(nullptr != channel);
    app::Department depProto;
    depProto.set_name(dep.toStdString());
    auto resp = std::unique_ptr<app::DefaultResponce>(channel->remoteCall(&app::ControlService::SaveDepartment, depProto, 10000));
    QVERIFY(nullptr != resp);
    QVERIFY( true == resp->result() );
    QVERIFY( 0 == resp->comment().size() );
  }
  //Проверка записи подразделений в БД
  {
    nosql::NosqlQuery query(global::kMongoQueriesNew[kQueryNameFindCollection]);
    query.arg(db::usersdb::kDepartments);
    auto conf = global::mongodbConfInter();
    nosql::NoSql db(conf.host(), conf.port().toUShort());
    QVERIFY(db.connect());
    QVERIFY(db.execQuery(db::kDbUsers, query.query()));
    QVERIFY(db.toCursor());
    QStringList nameValues;
    while ( true == db.next() ){
      auto result = db.document();
      auto name = result.valueString("name");
      QVERIFY(true == departments.contains(name));
      QVERIFY(false == nameValues.contains(name));
      nameValues << name;
    }
    QVERIFY(nameValues.size() == departments.size());
  }
  //Проверка получения подразделений из БД
  {
    auto channel = std::unique_ptr<rpc::Channel>(meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic));
    QVERIFY(nullptr != channel);
    app::Dummy dummy;
    auto resp = std::unique_ptr<app::DepartmentList>(channel->remoteCall(&app::ControlService::LoadDepartmentList, dummy, 10000));
    QVERIFY(resp->list_size() == departments.size());
    for (auto departmentProto: resp->list()){
      auto name = QString::fromStdString(departmentProto.name());
      QVERIFY(departments.contains(name));
      departments.removeAll(name);
    }
  }
}


const QString Test::getUserDepartment(const QString& login)
{
  nosql::NosqlQuery query(global::kMongoQueriesNew[kQueryNameFindCollection]);
  query.arg(db::usersdb::kUsers);
  auto conf = meteo::global::mongodbConfMeteo();//TODO - пользователи должны храниться в другой коллекции
  nosql::NoSql db(conf.host(), conf.port().toUShort());
  if ( false == db.connect() ) {
    error_log << meteo::msglog::kDbConnectFailed;
    return QString();
  }
  if ( false == db.execQuery(db::kDbUsers, query.query()) ){
    error_log << meteo::msglog::kDbRequestFailedArg.arg(query.query());
    return QString();
  }
  if( false == db.toCursor()) {
    error_log << meteo::msglog::kDbCursorCreationFailed;
    return QString();
  }
  while (true == db.next()){
    auto doc = db.document();
    auto docUserName = doc.valueString("login");
    if ( 0 == login.compare(docUserName) ){
      return doc.valueOid("department");
    }
  }
  return  QString();
}

const QString Test::getDepartmentName(const QString& departmentId) {
  nosql::NosqlQuery query(meteo::global::kMongoQueriesNew[kQueryNameFindCollection]);
  query.arg(db::usersdb::kDepartments);
  auto conf = global::mongodbConfUsers();
  nosql::NoSql dbUsers(conf.host(), conf.port().toUShort());
  if ( false == dbUsers.connect() ){
    error_log << meteo::msglog::kDbConnectFailed;
    return QString();
  }
  if ( false == dbUsers.execQuery(db::kDbUsers, query.query()))  {
    error_log << msglog::kDbRequestFailedArg.arg(query.query());
    return QString();
  }
  if ( false == dbUsers.toCursor() ){
    error_log << msglog::kDbCursorCreationFailed;
    return QString();
  }
  while( true == dbUsers.next())
  {
    auto doc = dbUsers.document();
    auto id = doc.valueOid("_id");
    if ( 0 ==  id.compare(departmentId) ){
      auto name = doc.valueString("name");
      return name;
    }
  }
  return QString();
}

void Test::testCreateUserWithDepartment()
{
  QVERIFY( true == this->dropUsers() );
  QVERIFY( true == this->dropDepartments() );
  const QString login = QObject::tr("testusername");
  const QString departmentName = QObject::tr("testdepartment");
  const QString departmentNameAlt = QObject::tr("testdepname2");

  //Создаем пользователя для тестов
  {
    auto channel = std::unique_ptr<rpc::Channel>(meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic));
    QVERIFY(nullptr != channel);
    app::UserRequest userProto;
    userProto.mutable_user()->set_login(login.toStdString());
    userProto.mutable_user()->set_department(departmentName.toStdString());

    auto resp = std::unique_ptr<app::UserResponse>(channel->remoteCall(&app::ControlService::AddUser, userProto, 10000));
    QVERIFY(nullptr != resp);
    QVERIFY( true == resp->result() );
    QVERIFY( 1 == this->countUsers() );
    QVERIFY( 1 == this->countDepartments() );

    auto userdep = this->getUserDepartment(login);
    auto userDepName = this->getDepartmentName(userdep);
    QVERIFY( 0 == userDepName.compare(departmentName) );
  }

  //Проверяем, что RPC вернет верные данные о пользователе
  {
    auto channel = std::unique_ptr<rpc::Channel>(meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic));
    QVERIFY(nullptr != channel);
    app::UserRequest userProto;
    auto users = std::unique_ptr<app::UserResponse>(channel->remoteCall(&app::ControlService::LoadUsers, userProto, 10000));
    QVERIFY(true == users->result());
    QVERIFY( 1 == users->users_size() );
    auto user = users->users(0);
    QCOMPARE(QString::fromStdString(user.login()), login );
    auto userDepartmentId = QString::fromStdString(user.department());
    QVERIFY( true == nosql::NosqlQuery::isObjectId(userDepartmentId) );

    app::Dummy dummy;
    auto deps = std::unique_ptr<app::DepartmentList>(channel->remoteCall(&app::ControlService::LoadDepartmentList, dummy, 10000));
    QCOMPARE(deps->list_size(), 1);
    for ( auto dep: deps->list() ){
      auto id = QString::fromStdString(dep.id());
      if ( 0 == id.compare(userDepartmentId) ){
        auto name = QString::fromStdString(dep.name());
        QCOMPARE(name, departmentName);
      }
    }
  }

  //Меняем у пользователя подразделение. Должен создать новое подразделение
  {
    auto channel = std::unique_ptr<rpc::Channel>(meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic));
    QVERIFY(nullptr != channel);
    app::UserRequest userProto;
    userProto.mutable_user()->set_login(login.toStdString());
    userProto.mutable_user()->set_department(departmentNameAlt.toStdString());

    auto resp = std::unique_ptr<app::UserResponse>(channel->remoteCall(&app::ControlService::UpdateUser, userProto, 10000));
    QVERIFY(nullptr != resp);
    QVERIFY( true == resp->result() );
    QVERIFY( 1 == this->countUsers() );
    QVERIFY( 2 == this->countDepartments() );
    auto userdep = this->getUserDepartment(login);
    auto userDepName = this->getDepartmentName(userdep);
    QVERIFY( 0 == userDepName.compare(departmentNameAlt) );
  }

  //Меняем подразделение пользователя назад. Новое подразделение не создается!
  {
    auto channel = std::unique_ptr<rpc::Channel>(meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic));
    QVERIFY(nullptr != channel);
    app::UserRequest userProto;
    userProto.mutable_user()->set_login(login.toStdString());
    userProto.mutable_user()->set_department(departmentName.toStdString());

    auto resp = std::unique_ptr<app::UserResponse>(channel->remoteCall(&app::ControlService::UpdateUser, userProto, 10000));
    QVERIFY(nullptr != resp);
    QVERIFY( true == resp->result() );
    QVERIFY( 1 == this->countUsers() );
    QVERIFY( 2 == this->countDepartments() );
    auto userdep = this->getUserDepartment(login);
    auto userDepName = this->getDepartmentName(userdep);
    QVERIFY( 0 == userDepName.compare(departmentName) );
  }

  //Удаляем пользователя
  {
    auto channel = std::unique_ptr<rpc::Channel>(meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic));
    QVERIFY(nullptr != channel);
    app::UserRequest userProto;
    userProto.mutable_user()->set_login(login.toStdString());

    auto resp = std::unique_ptr<app::UserResponse>(channel->remoteCall(&app::ControlService::RemoveUser, userProto, 10000));
    QVERIFY(nullptr != resp);
    QVERIFY( true == resp->result() );
  }

  QVERIFY( 0 == this->countUsers() );
  QVERIFY( 2 == this->countDepartments() );
}

void Test::testRanksLoading()
{
  QHash<int, QString> ranksHash;
  {
    auto channel = std::unique_ptr<rpc::Channel>(meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic));
    QVERIFY(nullptr != channel);
    app::UserRequest userProto;
    auto resp = std::unique_ptr<app::UserResponse>(channel->remoteCall(&app::ControlService::LoadRanks, userProto, 10000));
    QVERIFY(nullptr != resp);
    QVERIFY( true == resp->result() );

    for ( auto rank: resp->ranks() ){
      QVERIFY(false == ranksHash.contains(rank.id()));
      ranksHash[rank.id()] = QString::fromStdString(rank.name());
    }
  }

  {
    nosql::NosqlQuery query(global::kMongoQueriesNew[kQueryNameFindCollection]);
    query.arg(db::usersdb::kRanks);

    auto conf = meteo::global::mongodbConfUsers();
    nosql::NoSql db(conf.host(), conf.port().toUShort());
    QVERIFY( true == db.connect() );
    QVERIFY( true == db.execQuery(db::kDbUsers, query.query()) );
    QVERIFY( true == db.toCursor() );
    int count = 0;
    while( true == db.next() ){
      ++count;
      auto doc = db.document();
      auto id = qRound(doc.valueDouble("_id"));
      auto name = doc.valueString("name");
      QVERIFY(true == ranksHash.contains(id));
      QCOMPARE(name, ranksHash[id] );
    }
    QCOMPARE(count, ranksHash.size());
  }
}

void Test::testRolesLoading()
{
  QHash<int, QString> rolesHash;
  {
    auto channel = std::unique_ptr<rpc::Channel>(meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic));
    QVERIFY(nullptr != channel);
    app::UserRequest userProto;
    auto resp = std::unique_ptr<app::UserResponse>(channel->remoteCall(&app::ControlService::LoadRoles, userProto, 10000));
    QVERIFY(nullptr != resp);
    QVERIFY( true == resp->result() );

    for ( auto rank: resp->roles() ){
      QVERIFY(false == rolesHash.contains(rank.id()));
      rolesHash[rank.id()] = QString::fromStdString(rank.name());
    }
  }

  {
    nosql::NosqlQuery query(global::kMongoQueriesNew[kQueryNameFindCollection]);
    query.arg(db::usersdb::kRoles);

    auto conf = meteo::global::mongodbConfUsers();
    nosql::NoSql db(conf.host(), conf.port().toUShort());
    QVERIFY( true == db.connect() );
    QVERIFY( true == db.execQuery(db::kDbUsers, query.query()) );
    QVERIFY( true == db.toCursor() );
    int count = 0;
    while( true == db.next() ){
      ++count;
      auto doc = db.document();
      auto id = qRound(doc.valueDouble("_id"));
      auto name = doc.valueString("name");
      QVERIFY(true == rolesHash.contains(id));
      QCOMPARE(name, rolesHash[id] );
    }
    QCOMPARE(count, rolesHash.size());
  }
}

void Test::testUserAuth()
{
  QVERIFY(true == this->dropUsers());



  const QString login = QObject::tr("testusername");
  const QString password = QObject::tr("pass1");

  const QString loginAlt = QObject::tr("testusername2");
  const QString passwordAlt = QObject::tr("pass2");

  //Создаем пользователя для тестов
  for ( auto user: { qMakePair(login, password),
                     qMakePair(loginAlt, passwordAlt) })
  {
    auto channel = std::unique_ptr<rpc::Channel>(meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic));
    QVERIFY(nullptr != channel);
    app::UserRequest userProto;
    userProto.mutable_user()->set_login(user.first.toStdString());
    userProto.mutable_user()->set_password(user.second.toStdString());

    auto resp = std::unique_ptr<app::UserResponse>(channel->remoteCall(&app::ControlService::AddUser, userProto, 10000));
    QVERIFY(nullptr != resp);
    QVERIFY( true == resp->result() );
    resp = std::unique_ptr<app::UserResponse>(channel->remoteCall(&app::ControlService::RegisterUser, userProto, 10000));
    QVERIFY(nullptr != resp);
    QVERIFY( true == resp->result() );
  }

  QCOMPARE( 2, this->countUsers() );

  //Проверка авторизации
  {
    QHash<QPair<QString, QString>, bool> testCases;
    testCases[qMakePair(login,password)] = true;
    testCases[qMakePair(login,passwordAlt)] = false;
    testCases[qMakePair(loginAlt,password)] = false;
    testCases[qMakePair(loginAlt,passwordAlt)] = true;

    for ( auto testCase : testCases.keys() )
    {
      auto login = testCase.first;
      auto password = testCase.second;
      auto expected = testCases[testCase];

      auto channel = std::unique_ptr<rpc::Channel>(meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic));
      QVERIFY(nullptr != channel);
      app::UserAuthenticateRequest userProto;
      userProto.set_login(login.toStdString());
      userProto.set_password(password.toStdString());

      auto resp = std::unique_ptr<app::UserAuthenticateReply>(channel->remoteCall(&app::ControlService::UserAuthenticate, userProto, 10000));
      QVERIFY(nullptr != resp);
      QCOMPARE( expected, resp->authenticated() );
    }
  }

  //Смена пароля
  for ( auto user: { qMakePair(login, passwordAlt),
                     qMakePair(loginAlt, password ) })
  {
    auto channel = std::unique_ptr<rpc::Channel>(meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic));
    QVERIFY(nullptr != channel);
    app::UserRequest userProto;

    userProto.mutable_user()->set_login(user.first.toStdString());
    userProto.mutable_user()->set_password(user.second.toStdString());

    auto resp = std::unique_ptr<app::UserResponse>(channel->remoteCall(&app::ControlService::PasswordChange, userProto, 10000));
    QVERIFY(nullptr != resp);
    QVERIFY( true == resp->result() );
    QVERIFY( 2 == this->countUsers() );
  }


  //Проверка авторизации #2
  {
    QHash<QPair<QString, QString>, bool> testCases;
    testCases[qMakePair(login,password)] = false;
    testCases[qMakePair(login,passwordAlt)] = true;
    testCases[qMakePair(loginAlt,password)] = true;
    testCases[qMakePair(loginAlt,passwordAlt)] = false;

    for ( auto testCase : testCases.keys() )
    {
      auto login = testCase.first;
      auto password = testCase.second;
      auto expected = testCases[testCase];

      auto channel = std::unique_ptr<rpc::Channel>(meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic));
      QVERIFY(nullptr != channel);
      app::UserAuthenticateRequest userProto;
      userProto.set_login(login.toStdString());
      userProto.set_password(password.toStdString());

      auto resp = std::unique_ptr<app::UserAuthenticateReply>(channel->remoteCall(&app::ControlService::UserAuthenticate, userProto, 10000));
      QVERIFY(nullptr != resp);
      QCOMPARE( expected, resp->authenticated() );
    }
  }

  //Блокировка пользователя
  {
    auto channel = std::unique_ptr<rpc::Channel>(meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic));
    QVERIFY(nullptr != channel);
    app::UserRequest userProto;
    userProto.mutable_user()->set_login(login.toStdString());

    auto resp = std::unique_ptr<app::UserResponse>(channel->remoteCall(&app::ControlService::BlockUser, userProto, 10000));
    QVERIFY(nullptr != resp);
    QVERIFY( true == resp->result() );
  }

  //Проверка авторизации #3
  {
    QHash<QPair<QString, QString>, bool> testCases;
    testCases[qMakePair(login,password)] = false;
    testCases[qMakePair(login,passwordAlt)] = false;
    testCases[qMakePair(loginAlt,password)] = true;
    testCases[qMakePair(loginAlt,passwordAlt)] = false;

    for ( auto testCase : testCases.keys() )
    {
      auto login = testCase.first;
      auto password = testCase.second;
      auto expected = testCases[testCase];

      auto channel = std::unique_ptr<rpc::Channel>(meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic));
      QVERIFY(nullptr != channel);
      app::UserAuthenticateRequest userProto;
      userProto.set_login(login.toStdString());
      userProto.set_password(password.toStdString());

      auto resp = std::unique_ptr<app::UserAuthenticateReply>(channel->remoteCall(&app::ControlService::UserAuthenticate, userProto, 10000));
      QVERIFY(nullptr != resp);
      QCOMPARE( expected, resp->authenticated() );
    }
  }

  //Разблокировка пользователя
  {
    auto channel = std::unique_ptr<rpc::Channel>(meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic));
    QVERIFY(nullptr != channel);
    app::UserRequest userProto;
    userProto.mutable_user()->set_login(login.toStdString());

    auto resp = std::unique_ptr<app::UserResponse>(channel->remoteCall(&app::ControlService::UnBlockUser, userProto, 10000));
    QVERIFY(nullptr != resp);
    QVERIFY( true == resp->result() );
  }

  //Проверка авторизации #4
  {
    QHash<QPair<QString, QString>, bool> testCases;
    testCases[qMakePair(login,password)] = false;
    testCases[qMakePair(login,passwordAlt)] = true;
    testCases[qMakePair(loginAlt,password)] = true;
    testCases[qMakePair(loginAlt,passwordAlt)] = false;

    for ( auto testCase : testCases.keys() )
    {
      auto login = testCase.first;
      auto password = testCase.second;
      auto expected = testCases[testCase];

      auto channel = std::unique_ptr<rpc::Channel>(meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic));
      QVERIFY(nullptr != channel);
      app::UserAuthenticateRequest userProto;
      userProto.set_login(login.toStdString());
      userProto.set_password(password.toStdString());

      auto resp = std::unique_ptr<app::UserAuthenticateReply>(channel->remoteCall(&app::ControlService::UserAuthenticate, userProto, 10000));
      QVERIFY(nullptr != resp);
      QCOMPARE( expected, resp->authenticated() );
    }
  }

  //Чистим за собой
  for ( auto userLogin: { login, loginAlt })
  {
    auto channel = std::unique_ptr<rpc::Channel>(meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic));
    QVERIFY(nullptr != channel);
    app::UserRequest userProto;
    userProto.mutable_user()->set_login(userLogin.toStdString());

    auto resp = std::unique_ptr<app::UserResponse>(channel->remoteCall(&app::ControlService::RemoveUser, userProto, 10000));
    QVERIFY(nullptr != resp);
    QVERIFY( true == resp->result() );
  }

  QCOMPARE(0, this->countUsers() );
}


void Test::cleanupTestCase()
{
  QVERIFY(true == this->dropUsers());
  QVERIFY(true == this->dropDepartments());
}

QTEST_MAIN(Test)

