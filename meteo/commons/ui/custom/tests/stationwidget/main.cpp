#include <QtGui>

#include <cross-commons/app/targ.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/app/helpformatter.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/debug/tmap.h>

#include <commons/geobasis/geopoint.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/custom/stationwidget.h>

#include <meteo/commons/settings/settings.h>
#include <meteo/product/settings/productsettings.h>


#define HELP(opt, text)  (QStringList() << opt << QObject::tr(text))

const QStringList kHelpOpt  = QStringList() << "?" << "help";

const HelpFormatter kHelp = HelpFormatter()
  << HELP(kHelpOpt, "Эта справка")
     ;


int main(int argc, char** argv)
{
  TAPPLICATION_NAME("meteo");
  TLog::setMsgHandler(TLog::clearStdOut);
  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  QApplication app(argc, argv);

  TArg args(argc, argv);

  if ( args.contains(kHelpOpt, TArg::Flag_Any)  ) {
    kHelp.print();
    return 0;
  }

 ::meteo::gSettings(::meteo::product::Settings::instance());
  if ( !meteo::product::Settings::instance()->load() ) {
    error_log << QObject::tr("Ошибка при загрузке настроек.");
    return EXIT_FAILURE;
  }
  
  QList<meteo::sprinf::MeteostationType> types;
  types// << meteo::sprinf::MeteostationType::kStationSynop
  	<< meteo::sprinf::MeteostationType::kStationAirport
  	<< meteo::sprinf::MeteostationType::kStationAero;
 

  StationWidget w;
  w.setDelaySignal();
  w.loadStation(types, true);
  w.setHorisontal(true);
  //w.setCoordVisible(false);
  // w.setSavedStationsVisible(false);
  w.show();

  var(w.stationIndex());
  
  app.exec();

  return 0;
}
