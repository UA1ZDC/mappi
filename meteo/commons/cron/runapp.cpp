#include "runapp.h"

#include <qfileinfo.h>
#include <qprocess.h>
#include <qmap.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {

RunApp::RunApp( QObject* p )
  : meteo::PlannerTask(p)
{
  id_ = 0;
}

RunApp::~RunApp()
{
}

void RunApp::run()
{
  QMapIterator<int,QString> it(apps_);
  while ( true == it.hasNext() ) {
    it.next();
    runApp(it.value(), args_[it.key()]);
  }
}

void RunApp::runApp(const QString& appPath, const QStringList& args) const
{
  debug_log << "run:" << appPath << args.join(" ");

  QFileInfo fi(appPath);
  if ( false == fi.exists() ) {
    error_log << QObject::tr("Приложение для запуска не найдено = %1")
      .arg(appPath);
    return;
  }
  if ( false == fi.isExecutable() ) {
    error_log << QObject::tr("У файла приложения нет прав для его выполнения = %1")
      .arg(appPath);
    return;
  }
  int res = QProcess::execute( appPath, args );
  switch ( res ) {
    case -1:
      error_log << QObject::tr("Приложение %1 не может быть запущено")
        .arg(appPath);
      return;
    case -2:
      error_log << QObject::tr("Приложение %1 завершило работу аварийно")
        .arg(appPath);
      return;
    default:
      info_log << QObject::tr("Приложение %1 завершилось c кодом возврата %2")
        .arg(appPath)
        .arg(res);
      return;
  }
}

}
