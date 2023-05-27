#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/settings/tusersettings.h>
#include <meteo/commons/ui/settings/settingsform.h>
#include <meteo/commons/global/global.h>

#include <QApplication>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QTextCodec>


meteo::TSettingsForm *settings = nullptr;

void init() {
  static const auto permErrorMessage = QObject::tr("Недостаточно прав у пользователя. Приложение будет закрыто.");  

  /*if ( false == meteo::TAdminSettings::instance()->isRunAsRoot() ){
    error_log.msgBox() << permErrorMessage;
    return qApp->exit(EXIT_FAILURE);
  }*/

  if ( nullptr != settings ){
    error_log << QObject::tr("Почему-то settings не NULL. ");
    return;
  }

  meteo::TUserSettings::instance()->load();

  settings = new meteo::TSettingsForm();
  settings->init();
  settings->showMaximized();
}

int appMain( int argc, char** argv )
{
  TAPPLICATION_NAME( "meteo" );
  meteo::global::setLogHandler();
  ::meteo::gSettings(new meteo::Settings);
  if ( false == ::meteo::gSettings()->load() ){
    auto emessage = QObject::tr("Ошибка при загрузке настроек пользователя. "
                                    "Часть функций могут работать неправильно.");
    error_log.msgBox() << emessage;

  }

  if ( false == meteo::global::Settings::instance()->load() ){
    error_log << QObject::tr("Не загружено");
  }  

  QApplication::setSetuidAllowed(true);

  QApplication app( argc, argv );
  QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());

  QTimer::singleShot(0, &init);

  return app.exec();
}
