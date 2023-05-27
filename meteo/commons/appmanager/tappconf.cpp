#include "tappconf.h"

#include <qbytearray.h>
#include <qfile.h>
#include <qobject.h>
#include <qprocess.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <commons/textproto/tprototext.h>
#include <commons/textproto/pbtools.h>

#include <meteo/commons/proto/appconf.pb.h>
#include <meteo/commons/global/global.h>

namespace meteo {
namespace app {

bool Conf::load( AppConfig* conf )
{
  bool result = false;
  if ( nullptr == conf ) {
    error_log << meteo::msglog::kNullPointer.arg(QObject::tr("AppConfig"));//FIXME AppConfig -> человеческое название
    return result;
  }
  QDir confDir(MnCommon::etcPath() + "/app.conf.d");
  QStringList confFiles = confDir.entryList(QDir::Files);
  AppConfig tmpConf;
  for( const QString& name : confFiles ) {
    QString fname = confDir.path() +  "/" + name;
    if( QFile::exists(fname) ) {
      QFile file(fname);
      if( false == file.open(QIODevice::ReadOnly) ) {
        error_log << QObject::tr("Не удалось открыть %1 для чтения").arg(fname);
        continue;
      }
      QString strconf = QString::fromUtf8(file.readAll());
      file.close();
      TProtoText::fillProto(strconf, &tmpConf);
      result = true;
    }
    for (const auto& app : tmpConf.app()) {
      conf->add_app()->CopyFrom(app);
    }
  }

  return result;
}

void Conf::killProcess()
{
  meteo::app::AppConfig config;
  meteo::app::Conf::load(&config);
  for ( int i = 0, sz = config.app_size(); i < sz; ++i ) {
    QString appName = QString::fromStdString(config.mutable_app(i)->path());
    QProcess killall;
#ifndef WIN32
    killall.start("killall", QStringList() << "-9" << appName);
#else
    appName = QFileInfo(appName).fileName();
    killall.start("taskkill", QStringList() << "/F" << "/T" << "/IM" << appName);
#endif // WIN32
    killall.waitForFinished();
    killall.kill();
    killall.waitForFinished();
  }
}

}
}
