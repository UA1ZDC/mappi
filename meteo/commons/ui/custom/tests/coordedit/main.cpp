#include <QtGui>

#include <qstandarditemmodel.h>

#include <cross-commons/app/targ.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/app/helpformatter.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/debug/tmap.h>

#include <commons/geobasis/geopoint.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/custom/coordedit.h>


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


  CoordEdit e;
  e.setMaximum(180);
  e.show();

  app.exec();

  return 0;
}
