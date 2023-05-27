#include "usersettings.h"
#include <cross-commons/app/paths.h>
#include <cross-commons/app/options.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/tusersettings.h>
#include <qapplication.h>


using namespace meteo;

static const commons::Arg kUsernameArg = commons::Arg::makeLong("--username" , true);

QString paramhelp(const commons::Arg& arg, const QString& descr)
{
  return QObject::tr("    %1 %2")
    .arg( arg.help() )
    .arg(descr).leftJustified(40);
}

void usage()
{
  error_log << QObject::tr("Использование^");
  error_log << paramhelp(kUsernameArg, QObject::tr("Логин пользователя"));
}

int appMain(int argc, char** argv)
{
  TAPPLICATION_NAME("meteo");
  QApplication app(argc, argv);
  gSettings(new meteo::Settings);

  global::setLogHandler();

  if (!global::Settings::instance()->load() || !gSettings()->load()) {
    error_log.msgBox() << QObject::tr("Настройки приложения не загружены. Приложение будет закрыто.");
    return EXIT_FAILURE;
  }

  commons::ArgParser* options = commons::ArgParser::instance();
  if (!options->parse(argc, argv)) {
    usage();
    return EXIT_FAILURE;
  }

  if (options->installed(kUsernameArg)) {
    auto originalLogin = options->at(kUsernameArg).value();
    TUserSettings::instance()->setCurrentUserLogin(originalLogin);
  }

  if (!TUserSettings::instance()->load()) {
    error_log.msgBox() << QObject::tr("Служба диагностики недоступна. Приложение будет закрыто.");
    return EXIT_FAILURE;
  }

  // убрано за ненадобностью - лишняя проверка авторизации, когда поьлзователь в системе уже есть, а восмользоваться СПО не может
// #ifdef T_OS_ASTRA_LINUX
//   if (!TUserSettings::instance()->isRoot()&&!TUserSettings::instance()->isVgmdaemon()) {
//     error_log.msgBox() << QObject::tr("Ошибка доступа. Выполнение команды возможно только от администратора. Обратитесь к администратору");
//     return EXIT_FAILURE;
//   }
// #endif

  ui::UserSettings win;
  win.show();

  return app.exec();
}
