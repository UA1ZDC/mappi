#include "runapp.h"

#include <qfileinfo.h>
#include <qprocess.h>
#include <qmap.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace mappi {

RunApp::RunApp( QObject* p )
  : meteo::PlannerTask(p)
{
}

RunApp::~RunApp()
{
}

void RunApp::run()
{
  QFileInfo fi(path_);
  if ( false == fi.exists() ) {
    error_log << QObject::tr("Приложение для запуска не найдено = %1")
      .arg(path_);
    return;
  }
  if ( false == fi.isExecutable() ) {
    error_log << QObject::tr("У файла приложения нет прав для его выполнения = %1")
      .arg(path_);
    return;
  }
  QStringList arglist;
  QMapIterator<QString, QString> it(args_);
  while ( true == it.hasNext() ) {
    it.next();
    QString first = it.key();
    QString second = it.value();
    if ( false == first.isEmpty() ) {
      arglist << first;
      if ( false == second.isEmpty() ) {
        arglist << second;
      }
    }
  }
  int res = QProcess::execute( path_, arglist );
  switch ( res ) {
    case -1:
      error_log << QObject::tr("Приложение %1 не может быть запущено")
        .arg(path_);
      return;
    case -2:
      error_log << QObject::tr("Приложение %1 упало")
        .arg(path_);
      return;
    default:
      error_log << QObject::tr("Приложение %1 завершилось c кодом возврата %2")
        .arg(path_)
        .arg(res);
      return;
  }
}

}
}
