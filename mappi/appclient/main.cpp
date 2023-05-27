#include <meteo/commons/appclient/appmain.h>

int main( int argc, char** argv )
{
  return appMain(argc, argv);
}



// #include <qapplication.h>

// #include <cross-commons/app/paths.h>
// #include <cross-commons/debug/tlog.h>

// #include <meteo/commons/ui/appclient/windowappclient.h>
// #include <meteo/commons/ui/appclient/controllerappclient.h>

// #include <mappi/global/global.h>
// #include <mappi/settings/tmeteosettings.h>

// int main( int argc, char* argv[] )
// {
//   try {
//     TAPPLICATION_NAME("meteo");

//     QApplication* app = new QApplication(argc, argv);
//     //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );

//     ::meteo::gGlobalObj(new ::mappi::MappiGlobal);
//     if( !::meteo::mappi::TMeteoSettings::instance()->load() ){
//       return EXIT_FAILURE;
//     }

//     //   TLog::setMsgHandler(::meteo::ukaz::logoutToDb);

//     meteo::ControllerAppClient* control = new meteo::ControllerAppClient;

//     meteo::WindowAppClient* view = new meteo::WindowAppClient;
//     view->setController(control);
//     view->show();

//     QTimer::singleShot( 0, view, SLOT(slotInit()) );

//     app->exec();

//     delete view;
//     delete control;
//     delete app;
//   }
//   catch( const std::bad_alloc& ) {
//     critical_log << QObject::tr("Недостаточно памяти для работы программы");
//     return EXIT_FAILURE;
//   }

//   return EXIT_SUCCESS;
// }

