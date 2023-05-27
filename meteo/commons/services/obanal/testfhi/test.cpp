#include <stdlib.h>

#include <qapplication.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qdatetime.h>
#include <qlist.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/proc_read/daemoncontrol.h>

#include <meteo/commons/rpc/rpc.h>

#include <meteo/novost/global/global.h>
#include <meteo/novost/settings/tmeteosettings.h>
#include <meteo/novost/proto/meteosettings.pb.h>

#include <meteo/commons/services/obanal/tcalcfhi.h>

int main(int argc, char* argv[] )
{
  TAPPLICATION_NAME("meteo");
  ::meteo::gGlobalObj(new ::meteo::NovostGlobal);
  QApplication app( argc, argv );
  Q_UNUSED(app);

  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  if( false == meteo::settings::TMeteoSettings::instance()->load() ){
    error_log << meteo::settings::TMeteoSettings::instance()->errorString();
    return EXIT_FAILURE;
  }
  meteo::obanal::calcAndSaveFHI( QDate::currentDate().addDays(-1) );

  return EXIT_SUCCESS;
}


