#include <iostream>
#include <fstream>
#include <cstdlib>
#include <qdiriterator.h>
#include <qapplication.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

#include "reservewidget.h"

int appMain (int argc, char** argv)
{
  TAPPLICATION_NAME("meteo");
  meteo::global::setLogHandler();
  QApplication app(argc, argv);
  ::meteo::gSettings(new meteo::Settings);

  if( !meteo::gSettings()->load() ){
    return EXIT_FAILURE;
  }
  meteo::ReserveWidget w;
  w.show();

  app.exec();
  return EXIT_SUCCESS;
}


