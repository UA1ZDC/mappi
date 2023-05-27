#include <meteo/commons/appmanager/appmain.h>

int main( int argc, char* argv[] )
{
  return appMain(argc, argv);
}



// #include <qtextcodec.h>

// #include <cross-commons/app/paths.h>
// #include <cross-commons/debug/tlog.h>

// #include "appmanagerdaemon.h"

// int main( int argc, char* argv[] )
// {
//   try {
//     TAPPLICATION_NAME("meteo");

//     QCoreApplication(argc,argv);

//     AppManagerDaemon* daemon = new AppManagerDaemon(argc, argv);

//     //TLog::setMsgHandler(::meteo::ukaz::logoutToDb);

//     daemon->exec();

//     delete daemon;
//   }
//   catch(const std::bad_alloc& ) {
//     critical_log << QObject::tr("Недостаточно памяти для работы программы");
//     return EXIT_FAILURE;
//   }

//   return EXIT_SUCCESS;
// }
