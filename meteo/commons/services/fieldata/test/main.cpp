#include <stdlib.h>

#include <memory>
#include <iostream>

#include <qcoreapplication.h>

#include <gtest/gtest.h>

//#include "getfielddata.h"
#include "getfieldsmdata.h"

#include <meteo/bank/settings/banksettings.h>
#include <meteo/bank/global/global.h>

int main(int argc, char **argv)
{
  TAPPLICATION_NAME("meteo");
  ::meteo::gGlobalObj(new ::meteo::BankGlobal);

  QCoreApplication app(argc, argv);

  if(false == meteo::bank::Settings::instance()->load()) {
    error_log << QObject::tr("Не удалось загрузить настройки");
    return EXIT_FAILURE;
  }

  testing::InitGoogleTest(&argc, argv);
  debug_log << RUN_ALL_TESTS();

  return app.exec();
}


