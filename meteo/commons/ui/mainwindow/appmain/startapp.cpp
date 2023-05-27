#include "startapp.h"

#include <cross-commons/debug/tlog.h>
//#include <meteo/commons/dbjournal/journal.h>
#include <cross-commons/app/paths.h>
#include <meteo/commons/settings/tusersettings.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/rpc/rpc.h>

#include <qmessagebox.h>
#include <qobject.h>

namespace meteo {

StartApp::StartApp():
  usersettings_(0)
{
}

void StartApp::startUserSettings()
{
   usersettings_ = new QProcess;
   QString appName = GUISU_PATH;
   QStringList appArgs = QStringList();
   appArgs << "-c";
   appArgs << QObject::tr("%1 %2 %3")
              .arg(MnCommon::binPath()+"/maslo.admin")
              .arg( QObject::tr("--username") )
              .arg( meteo::TUserSettings::instance()->getCurrentUserName() );

   QObject::connect( usersettings_, SIGNAL(finished(int)), this, SLOT(slotUserSettingsFinished(int)) );
   debug_log << appName << appArgs;
   QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

   usersettings_->setProcessEnvironment( env );
   usersettings_->start(appName, appArgs );

   if( true != usersettings_->waitForStarted(3000) )
   {

   }
   else
   {
//     journal_info() << QObject::tr("Запуск приложения %1").arg( appName );
   }

   if ( 0 != usersettings_->exitCode() ){
     error_log.msgBox() << QObject::tr("Приложение администрирования завершилось с ошибкой!");
   }
}


void StartApp::slotUserSettingsFinished(int status)
{
  // FIX ME обработка ошибок завершения процесса
  if (0 == status)
  {
    if (false == meteo::TUserSettings::instance()->load())
    {
      error_log << meteo::msglog::kSettingsLoadFailed.arg(QObject::tr("администрирования"));
    }
    else if ( false == meteo::TUserSettings::instance()->isAuth() )
    {
      error_log << meteo::msglog::kUserAccessDenied.arg(meteo::global::currentUserLogin());
      ::exit(EXIT_SUCCESS);
    }
    else
    {
      meteo::app::MainWindow* window = WidgetHandler::instance()->mainwindow();
      if( 0 == window ){
        window = new meteo::app::MainWindow;
        window->addPluginGroup("meteo.app.widgets");
        window->setupToolbar();
        window->setWindowIcon(QIcon(":/meteo/icons/map.png"));
      }
      window->show();
    }
  }
  else
  {
    QProcess* p = qobject_cast<QProcess*>(sender());
    if ( 0 != p ) {
      debug_log << p->readAll();
    }

    error_log <<meteo::msglog::kSettingsIncorrect.arg(QObject::tr("пользователя"));
    ::exit(EXIT_SUCCESS);
  }
}

StartApp::~StartApp()
{

}

} // meteo
