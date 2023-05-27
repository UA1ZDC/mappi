#include <mappi/global/global.h>
#include <mappi/settings/mappisettings.h>
#include <mappi/ui/antenna/antennawidget.h>
#include <qapplication.h>
#include <stdexcept>


int main(int argc, char* argv[])
{
  TAPPLICATION_NAME("meteo");
  QApplication app(argc, argv);
  gSettings(meteo::global::Settings::instance());
  // global::setLogHandler();

  if (meteo::global::Settings::instance()->load() == false) {
    error_log << meteo::msglog::kSettingsLoadFailed;
    return EXIT_FAILURE;
  }

  mappi::antenna::AntennaWidget w;
  w.show();

 return app.exec();
}
