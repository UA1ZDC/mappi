#include "usersettings.h"

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/global/dbnames.h>
#include <sql/nosql/nosqlquery.h>
#include <google/protobuf/util/json_util.h>
#include <meteo/commons/proto/appconf.pb.h>

namespace meteo {

bool UserSettings::saveSettings(const google::protobuf::Message& data, const QString& username )
{
  return saveSettings(data, username.toStdString());
}

bool UserSettings::saveSettings(const google::protobuf::Message& data,
                                const std::string& username)
{
  static const auto queryName = QObject::tr("user_app_settings_save");

  std::string json;
  auto status = google::protobuf::util::MessageToJsonString(data, &json, google::protobuf::util::JsonOptions());
  if ( false == status.ok() ){
    error_log << QObject::tr("Ошибка при преобразовании proto-сообщения в JSON.");
    return false;
  }  
  if ( true == username.empty() ){
    error_log << QObject::tr("Ошибка: попытка загрузки настроек пользователя без указания логина пользователя");
    return false;
  }

  std::unique_ptr<Dbi> db( global::dbUsers() );
  std::unique_ptr<DbiQuery> query(db->queryByName(queryName));
  if(nullptr == query) {
    error_log << msglog::kDbRequestNotFound.arg(queryName);
    return false;
  }

  query->arg("login", username);
  query->arg("proto",data.GetTypeName());
  query->arg("param", QString::fromStdString(json));
  
  if ( false == query->exec() ) {
    error_log << msglog::kDbRequestFailedArg.arg(query->result().jsonExtendedString());
    return false;
  }

  return true;
}

bool UserSettings::loadSettings(google::protobuf::Message* data, const std::string& username)
{
  return loadSettings(data, { QString::fromStdString(username) } )  ;
}


bool UserSettings::loadSettings(google::protobuf::Message* data, const QStringList& username)
{
  static const auto queryName = QObject::tr("user_app_settings_load");

  if ( true == username.empty() ){
    error_log << QObject::tr("Ошибка: попытка загрузки настроек пользователя без указания логина пользователя");
    return false;
  }
  std::unique_ptr<Dbi> db( global::dbUsers() );
  std::unique_ptr<DbiQuery> query( db->queryByName(queryName) );
  if ( nullptr == query ) {
    error_log << msglog::kDbRequestNotFound.arg(queryName);
    return false;
  }
  query->arg( "login", username);
  query->arg( "proto", data->GetTypeName());
  if ( false == query->exec() ) {
    error_log << msglog::kDbRequestFailedArg.arg(query->result().jsonExtendedString());
    return false;
  }

  if ( false == query->initIterator() ){
    error_log << msglog::kDbCursorCreationFailed;
    return false;
  }

  int count;
  for ( count = 0; true == query->next(); ++count) {
    const DbiEntry& doc  = query->entry();
    auto json = doc.valueString(QObject::tr("param"));
    auto status = google::protobuf::util::JsonStringToMessage(json.toStdString(), data, google::protobuf::util::JsonParseOptions());
    if ( false == status.ok() ) {
      error_log << QObject::tr("Ошибка при разборе полученных данных. Не удалось преобразовать запись в БД в proto-сообщение.");
      return false;
    }
  }
  return 1 == count || 0 == count;
}

bool UserSettings::loadSettingsMany(QHash<QString, google::protobuf::Message*> users)
{
  static const auto queryName = QObject::tr("user_app_settings_load_many");

  if ( 0 == users.size() ){
    error_log << QObject::tr("Попытка загрузить настройки для множества пользователей без указания пользователей.");
    return false;
  }
  auto typeName = users.values().first()->GetTypeName();
  for ( auto msg: users){
    if ( 0 != typeName.compare(msg->GetTypeName()) ){
         error_log << QObject::tr("Ошибка: в списке типов данных присутствуют как минимум 2 различных типа %1, %2. Прерывание.")
                      .arg(QString::fromStdString(typeName))
                      .arg(QString::fromStdString(msg->GetTypeName()));
         return false;
    }
  }

  std::unique_ptr<Dbi> db( global::dbUsers() );
  std::unique_ptr<DbiQuery> query(db->queryByName(queryName));
  if(nullptr == query) {
    error_log << msglog::kDbRequestNotFound.arg(queryName);
    return false;
  }

  QStringList userList =  users.keys();
  query->arg( "login", userList)
      .arg( "proto", typeName);

  QString err;
  if ( false == query->execInit(&err) ){
    error_log << msglog::kDbRequestFailedArg.arg(query->result().jsonExtendedString())
              << err;
    return false;
  }

  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    auto user = doc.valueString("login");
    auto json = doc.valueDocument(QObject::tr("param")).jsonString();
    auto data = users[user];
    auto status = google::protobuf::util::JsonStringToMessage(json.toStdString(), data, google::protobuf::util::JsonParseOptions());

    if ( false == status.ok() ) {
      error_log << QObject::tr("Ошибка при разборе полученных данных. Не удалось преобразовать запись в БД в proto-сообщение.");
      return false;
    }
  }
  return true;
}

}
