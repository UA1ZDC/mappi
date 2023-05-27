#include <QtGui>

#include <cross-commons/app/targ.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/app/helpformatter.h>
#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geopoint.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/services/sprinf/sprinfservice.h>
#include <meteo/commons/ui/custom/stationwidget.h>
#include <meteo/commons/ui/custom/preparewidget.h>

Q_DECLARE_METATYPE( meteo::GeoPoint )

#define HELP(opt, text)  (QStringList() << opt << QObject::tr(text))

const QStringList kHelpOpt  = QStringList() << "?" << "help";
const QStringList kMeteogramOpt  = QStringList() << "m" << "";
const QStringList kTransferOpt  = QStringList() << "t" << "";
const QStringList kSloyOpt  = QStringList() << "s" << "";

const HelpFormatter kHelp = HelpFormatter()
  << HELP(kHelpOpt, "Эта справка")
  << HELP(kMeteogramOpt, "kMeteogram")
  << HELP(kTransferOpt, "kTransfer")
  << HELP(kSloyOpt, "kSloy")
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

  PrepareWidget* w = 0;
  if ( args.contains(kMeteogramOpt) ) {
    w = new PrepareWidget;
    w->setMode(PrepareWidget::kMeteogram);
    w->show();
  }
  else if ( args.contains(kTransferOpt) ) {
    w = new PrepareWidget;
    w->setMode(PrepareWidget::kTransfer);
    w->show();
  }
  else if ( args.contains(kSloyOpt) ) {
    w = new PrepareWidget;
    w->setMode(PrepareWidget::kSloy);
    w->show();
  }
  else {
    kHelp.print();
    return 0;
  }

  w->setBeginDateTime(QDateTime::currentDateTimeUtc());

  app.exec();

  info_log << "BEG DT:" << w->beginDateTime().toString(Qt::ISODate);
  info_log << "END DT:" << w->endDateTime().toString(Qt::ISODate);

  return 0;
}
