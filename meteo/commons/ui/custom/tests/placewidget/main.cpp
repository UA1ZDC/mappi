#include <QtGui>

#include <cross-commons/app/targ.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/app/helpformatter.h>
#include <cross-commons/debug/tlog.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/custom/placewidget.h>

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

  PlaceWidget w;
  w.show();

  app.exec();

  info_log << QObject::tr("--- СПИСОК ПУНКТОВ ---");
  foreach ( const PlaceInfo& i, w.placeList() ) {
    info_log  << QString::number(i.id)
              << i.index
              << i.name
              << i.echelon
                 ;
  }

  return 0;
}
