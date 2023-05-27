#include <QtGui>

#include <cross-commons/app/targ.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/app/helpformatter.h>
#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geopoint.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/services/sprinf/sprinfservice.h>
#include <meteo/commons/ui/custom/stationwidget.h>
#include <meteo/commons/ui/custom/hoursedit.h>

Q_DECLARE_METATYPE( meteo::GeoPoint )

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

  HoursEdit he;
  he.setMinimum(-1000);
  he.setMaximum(1000);
  he.setDefaultValue(24);
  he.setValue(0);
  he.show();

  return app.exec();
}
