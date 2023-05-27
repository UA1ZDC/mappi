#include "rawloader.h"

#include <qprocess.h>
#include <qfileinfo.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

namespace mappi {

RawLoader::RawLoader(QObject* parent)
  : ::meteo::FtpLoader(parent)
{
}

void RawLoader::downloadDone(const QString& tempName, const QString& origName)
{
  Q_UNUSED( origName );

  QString suffix = QFileInfo(origName).suffix().toLower();
  if ( "raw16" != suffix && "cif" != suffix ) {
    ::meteo::FtpLoader::downloadDone(tempName,origName);
    return;
  }

  const QString appName = MnCommon::binPath() + "/mappi.pretreatment.files";

  QProcess proc;

  if ( "cif" == suffix ) {
    proc.start(appName, {"-i","-f","-o",tempName});
  }
  else {
    proc.start(appName, {"-o",tempName});
  }

  if ( false == proc.waitForStarted() ) {
    error_log << QObject::tr("Ошибка при запуске '%1'").arg(appName);
    return;
  }

  QString log;
  while ( false == proc.waitForFinished(3000) ) {
    log = proc.readAll();
    if ( !log.isEmpty() ) { none_log << log.remove(log.size() - 1, 1); }
  }
  log = proc.readAll();
  if ( !log.isEmpty() ) { none_log << log.remove(log.size() - 1, 1); }

  if ( proc.state() != QProcess::NotRunning || proc.exitStatus() != QProcess::NormalExit ) {
    error_log << QObject::tr("Процесс '%1' завершился с ошибкой %2").arg(appName).arg(proc.exitCode());
    return;
  }
}

} // mappi
