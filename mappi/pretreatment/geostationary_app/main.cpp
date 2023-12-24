#include "mainwindow.h"
#include <mappi/global/global.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TAPPLICATION_NAME("meteo");
    MainWindow w;

    meteo::gSettings(mappi::inter::Settings::instance());
    if ( false == meteo::gSettings()->load() ) {
      error_log << meteo::msglog::kSettingsLoadFailed;
      return -1;
    }

    w.show();
    return a.exec();
}
