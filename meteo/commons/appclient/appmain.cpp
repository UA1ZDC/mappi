#include <qapplication.h>
#include <qicon.h>

#include <cross-commons/app/helpformatter.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/app/targ.h>
#include <cross-commons/debug/tlog.h>

#include <meteo/commons/ui/appclient/windowappclient.h>
#include <meteo/commons/ui/appclient/controllerappclient.h>

#include <meteo/commons/settings/settings.h>
#include <meteo/commons/global/global.h>

#define HELP(opt, text)  (QStringList() << opt << QObject::tr(text))

const QStringList kHelpOpt = QStringList() << "h" << "help";
const QStringList kFontOpt = QStringList() << "s" << "font-size";
//const QStringList kConnOpt = QStringList() << "c" << "connect";

const HelpFormatter kHelp = HelpFormatter()
  << HELP(kHelpOpt, "Эта справка.")
  << HELP(kFontOpt, "Размер шрифта на вкладке 'Журнал'")
//  << HELP(kConnOpt, "Подключиться к указанному менеджеру (host[:port], по умолчанию подключение осуществляется на порт 34567)")
     ;

int appMain( int argc, char* argv[], const QString & icon )
{
  try {
    TAPPLICATION_NAME("meteo");
    meteo::global::setLogHandler();

    QApplication* app = new QApplication(argc, argv);

    ::meteo::gSettings(new meteo::Settings);

    if( !meteo::gSettings()->load() ){
      return EXIT_FAILURE;
    }

    TArg args( argc, argv );

    if( args.contains(kHelpOpt) ){
      kHelp.print();
      return EXIT_SUCCESS;
    }

//    TLog::setMsgHandler(::meteo::ukaz::logoutToDb);

    meteo::ControllerAppClient* control = new meteo::ControllerAppClient;
    QApplication::setWindowIcon(QIcon(icon));

    meteo::WindowAppClient* view = new meteo::WindowAppClient;
    view->setWindowTitle(QObject::tr("Контроль технического состояния"));
    view->setController(control);

    if ( args.contains(kFontOpt) ) {
      view->setLogDefaultFontSize(args.value(kFontOpt).toInt());
    }

    view->show();

    QTimer::singleShot( 0, view, SLOT(slotInit()) );

    app->exec();

    delete view;
    delete control;
    delete app;
  }
  catch( const std::bad_alloc& ) {
    critical_log << QObject::tr("Недостаточно памяти для работы программы");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

