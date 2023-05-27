#include <qapplication.h>

#include <cross-commons/app/helpformatter.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/app/targ.h>
#include <cross-commons/debug/tlog.h>

#include <meteo/novost/settings/tmeteosettings.h>

#include <meteo/commons/ui/plugins/meteogram/window.h>
#include <meteo/commons/ui/plugins/meteogram/meteogramplugin.h>

const QStringList kDtOpt = QStringList() << "t" << "dt";

int main(int argc, char** argv)
{
  TAPPLICATION_NAME("meteo");
  TLog::setMsgHandler(TLog::clearStdOut);
  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );

  QApplication app(argc, argv);

  info_log << "LOAD SETTINGS:" << meteo::settings::TMeteoSettings::instance()->load();

  TArg args( argc, argv );

  meteo::MeteogramV2Plugin plg;
  meteo::MeteogramV2Window* w = plg.slotCreateDocument();
  w->checkT()->setChecked(true);
  w->checkTd()->setChecked(true);
  w->checkP()->setChecked(true);
  w->checkU()->setChecked(true);
  w->checkWind()->setChecked(true);
  if ( args.contains(kDtOpt) ) {
    w->setBeginDateTime(QDateTime::fromString(args.value(kDtOpt), Qt::ISODate));
  }

  return app.exec();
}


