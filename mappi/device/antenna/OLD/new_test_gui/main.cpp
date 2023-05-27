#include "mappi/ui/antennactrl_new/antctrl.h"
#include <cross-commons/app/paths.h>
#include <meteo/commons/global/global.h>
#include <mappi/global/global.h>
#include <qapplication.h>


using namespace meteo;

int main(int argc, char** argv)
{
  TAPPLICATION_NAME("meteo");
  QApplication app(argc, argv);

  gSettings(mappi::inter::Settings::instance());

  global::setLogHandler();

  if (!global::Settings::instance()->load() || !gSettings()->load()) {
    error_log.msgBox() << QObject::tr("Настройки приложения не загружены. Приложение будет закрыто.");
    return EXIT_FAILURE;
  }

  mappi::receive::AntCtrl win;
  win.show();

  return app.exec();
}
