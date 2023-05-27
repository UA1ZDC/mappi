#ifndef METEO_COMMONS_APPMANAGER_USERSETTINGSSAVER_H
#define METEO_COMMONS_APPMANAGER_USERSETTINGSSAVER_H

#include <meteo/commons/proto/appconf.pb.h>
#include <qstring.h>
#include <sql/nosql/document.h>

namespace meteo {

class UserSettings
{
  public:
    UserSettings() = delete ;

    static bool saveSettings(const google::protobuf::Message& data, const std::string& username );
    static bool saveSettings(const google::protobuf::Message& data, const QString& username );

    static bool loadSettings(google::protobuf::Message* data, const std::string& username);
    static bool loadSettings(google::protobuf::Message* data, const QStringList& usernames);

    static bool loadSettingsMany(QHash<QString, google::protobuf::Message*> settings);
};


}
#endif
