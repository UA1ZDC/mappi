#include <cross-commons/app/targ.h>
#include <cross-commons/app/helpformatter.h>
#include <cross-commons/debug/tlog.h>

#include <meteo/wrf/global/global.h>
#include <meteo/wrf/ftploader/ftploader.h>
#include <meteo/wrf/settings/wrfsettings.h>

#include "rawloader.h"

#define HELP(opt, text)  (QStringList() << opt << QObject::tr(text))

const QStringList kOptHelp  = QStringList() << "h" << "help";
const QStringList kOptId    = QStringList() << "id";

const HelpFormatter kHelp = HelpFormatter()
    << HELP(kOptHelp, "Эта справка.")
    << HELP(kOptId, "Запуск с параметрами для указанного идентификатора.")
       ;


int main( int argc, char** argv )
{
  try {
    TAPPLICATION_NAME( "meteo" );

    QCoreApplication* app = new QCoreApplication(argc, argv);

    TArg args( argc, argv );

    if ( args.contains(kOptHelp) ) {
      TLog::setMsgHandler( TLog::clearStdOut  );
      kHelp.print();
      delete app;
      return EXIT_SUCCESS;
    }

    if ( !args.contains(kOptId) ) {
      error_log << QObject::tr("Необходимо указать идентификатор");
      delete app;
      return EXIT_FAILURE;
    }

    ::meteo::gGlobalObj(new ::meteo::WrfGlobal);

    if( !::meteo::WrfSettings::instance()->load() ){
//      delete app;
//      return EXIT_FAILURE;
    }

    ::meteo::FtpLoader* loader = new ::mappi::RawLoader;
    QString id = args.value(kOptId);
    loader->setOptions(::meteo::wrf::gSettingsObj()->ftploaderConf(id));
    QTimer::singleShot( 0, loader, SLOT(slotInit()) );

    app->exec();

    delete loader;
    delete app;
  }
  catch (const std::bad_alloc &) {
    critical_log << QObject::tr("Недостаточно памяти для функционирования приложения");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

