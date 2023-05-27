#include "dataloader.h"
#include <QApplication>
#include <cross-commons/app/paths.h>
#include <meteo/commons/global/global.h>


int main(int argc, char *argv[])
{
  TAPPLICATION_NAME("meteo");
  meteo::global::setLogHandler();
  QApplication a(argc, argv);
  meteo::DataLoader w;
  w.show();
  return a.exec();
}
