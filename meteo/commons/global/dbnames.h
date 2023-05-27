#ifndef METEO_COMMONS_GLOBAL_DBNAMES_H
#define METEO_COMMONS_GLOBAL_DBNAMES_H

#include <qobject.h>

namespace meteo {
namespace db {

  static const QString& kDbDocuments = QObject::tr("documentdb");
  static const QString& kDbInter = QObject::tr("interdb");
  static const QString& kDbMeteo = QObject::tr("meteodb");
  static const QString& kDbTelegrams = QObject::tr("telegramsdb");
  static const QString& kDbUsers = QObject::tr("usersdb");
  static const QString& kDbClimat = QObject::tr("climatdb");

  namespace usersdb {
    static const QString& kDepartments = QObject::tr("departments");
    static const QString& kRanks = QObject::tr("ranks");
    static const QString& kRoles = QObject::tr("roles");
    static const QString& kUsers = QObject::tr("users");
    static const QString& kAppointments = QObject::tr("user_appointments");
    static const QString& kPositions = QObject::tr("positions");
  }

  namespace interdb {
    static const QString& kMessages = QObject::tr("messages");
    static const QString& tasks = QObject::tr("tasks");    
    static const QString& kFileUpDownloads = QObject::tr("up_down_loads");
    static const QString& kFileChunks = QObject::tr("up_down_loads.parts");
    static const QString& kWebFiles = QObject::tr("webfiles");
  }

}

}

#endif
