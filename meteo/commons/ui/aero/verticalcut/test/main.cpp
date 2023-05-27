#include <qapplication.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/targ.h>
#include <cross-commons/app/helpformatter.h>
#include <cross-commons/app/paths.h>

#include <meteo/novost/settings/tmeteosettings.h>
#include <meteo/novost/global/global.h>

#include "../createcutwidget.h"

const QStringList kBulletinOpt = QStringList() << "b";

int main(int argc, char** argv)
{
  TAPPLICATION_NAME("meteo");
  ::meteo::gGlobalObj(new ::meteo::NovostGlobal);
  TLog::setMsgHandler(TLog::clearStdOut);
  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );

  QApplication app(argc, argv);

  info_log << "LOAD SETTINGS:" << meteo::settings::TMeteoSettings::instance()->load();

  TArg args( argc, argv );

  meteo::CreateCutWidget w(0, args.contains(kBulletinOpt));
  w.show();


  return app.exec();
}


