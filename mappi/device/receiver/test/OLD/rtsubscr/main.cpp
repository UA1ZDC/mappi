#include "recvimg.h"


#include <mappi/global/global.h>
#include <mappi/settings/mappisettings.h>

#include <qapplication.h>


int main (int argc, char** argv)
{
  TAPPLICATION_NAME("meteo");
    
  QApplication app(argc, argv, false);
  
  meteo::gSettings(mappi::inter::Settings::instance());
  if ( false == meteo::gSettings()->load() ){
    error_log << meteo::msglog::kSettingsLoadFailed;
    return EXIT_FAILURE;
  }

  // ::meteo::gGlobalObj(new ::mappi::MappiGlobal);
  // if( !::meteo::settings::TMeteoSettings::instance()->load() ){
  //   return EXIT_FAILURE;
  // }  
  
  RecvImg recv;
  app.exec();
    

  return 0;
}
