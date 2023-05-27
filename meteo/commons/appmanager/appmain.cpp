#include <unistd.h>
#include <signal.h>

#include <qprocess.h>
#include <qstringlist.h>
#include <qcoreapplication.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/app/targ.h>
#include <cross-commons/app/helpformatter.h>
#include <cross-commons/debug/tlog.h>

#include <commons/proc_read/checkps.h>
#include <commons/proc_read/daemoncontrol.h>

#include <meteo/commons/appmanager/tappconf.h>
#include <meteo/commons/appmanager/tappcontrol.h>
#include <meteo/commons/appmanager/tappcontrolservice.h>
#include <meteo/commons/proto/appconf.pb.h>
#include <meteo/commons/proto/services.pb.h>
#include <meteo/commons/rpc/rpc.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

#ifdef T_OS_ASTRA_LINUX

#include <linux/prctl.h>
#include <parsec/mac.h>
#include <parsec/parsec_integration.h>
#include <parsec/parsec_mac.h>
#include <sys/prctl.h>

#endif


#define HELP(opt, text)  (QStringList() << opt << QObject::tr(text))


const QString kAppName = "masloappmanager";

const QStringList kHelpOpt   = QStringList() << "h" << "help";
const QStringList kDaemonOpt = QStringList() << "d" << "daemonize";

const HelpFormatter kHelp = HelpFormatter()
  << HELP(kHelpOpt, "Эта справка.")
  << HELP(kDaemonOpt, "Запуск в режиме демона.")
     ;


meteo::rpc::Server*  server  = nullptr;
meteo::app::internal::Control* ctrl = nullptr;
meteo::app::Service* service = nullptr;

QCoreApplication* start(int argc, char* argv[], bool runDaemon);
void stop();

void hdl(int sig)
{
  Q_UNUSED( sig );

  var(sig);

  stop();
  qApp->quit();
}

int appMain( int argc, char* argv[] )
{
#ifdef T_OS_ASTRA_LINUX
  linux_caps_t lcaps={0,0,0};
  //объявление переменной для установки привилегий PARSEC
  // и ее начальная инициализация
  parsec_caps_t pcaps = {0,0,0};
  // инициализация списка разрешенных привилегий PARSEC
  pcaps.cap_permitted|=CAP_TO_MASK(PARSEC_CAP_SETMAC);
  pcaps.cap_permitted|=CAP_TO_MASK(PARSEC_CAP_PRIV_SOCK);
  pcaps.cap_permitted|=CAP_TO_MASK(PARSEC_CAP_CHMAC);
  // инициализация списка действующих (эффективных) привилегий PARSEC
  pcaps.cap_effective|=CAP_TO_MASK(PARSEC_CAP_SETMAC);
  pcaps.cap_effective|=CAP_TO_MASK(PARSEC_CAP_PRIV_SOCK);
  pcaps.cap_effective|=CAP_TO_MASK(PARSEC_CAP_CHMAC);
  // инициализация списка наследуемых привилегий PARSEC
  pcaps.cap_inheritable|=CAP_TO_MASK(PARSEC_CAP_PRIV_SOCK);
  pcaps.cap_inheritable|=CAP_TO_MASK(PARSEC_CAP_SETMAC);
  pcaps.cap_inheritable|=CAP_TO_MASK(PARSEC_CAP_CHMAC);
  // установка флага наследования привилегий
  if ( -1 == prctl(PR_SET_KEEPCAPS,1) ) {
    error_log << QObject::tr("Ошибка при установке флага наследования привелегий");
    return EXIT_FAILURE;
  }
  // установка привилегий Linux и PARSEC основного процесса сетевого сервиса
  if ( parsec_cur_caps_set(&lcaps,&pcaps) < 0 ) {
    error_log << QObject::tr("Ошибка при установке Linux и PARSEC привелегий");
    return EXIT_FAILURE;
  }
#endif

  try {
    TAPPLICATION_NAME("meteo");
    meteo::global::setLogHandler();

    signal(SIGINT, &hdl);
    signal(SIGTERM, &hdl);

    //QCoreApplication* app = new QCoreApplication(argc,argv);

    TArg args(argc,argv);
    if ( args.contains(kHelpOpt) ) {
      kHelp.print();
      return EXIT_SUCCESS;
    }

   // delete app;
    QCoreApplication* app =  start(argc, argv, args.contains(kDaemonOpt));
    if ( nullptr != app ) {
      app->exec();
      delete app;
    }
  }
  catch( const std::bad_alloc& ) {
    critical_log << QObject::tr("Недостаточно памяти для работы программы");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

QCoreApplication* start(int argc, char* argv[], bool runDaemon)
{
  if ( 0 != ProcControl::daemonizeSock(kAppName,runDaemon) ) {
    //std::cout << QObject::tr("Такой процесс уже запущен").toLocal8Bit().constData()<< std::endl;;
    return nullptr;
  }

  QCoreApplication* app = new QCoreApplication(argc,argv);

  meteo::gSettings(new meteo::Settings);

  if ( !::meteo::gSettings()->load() ) {
    delete app;
    return nullptr;
  }

  // на случай, если appmanager завершился аварийно
  meteo::app::Conf::killProcess();

  ctrl = meteo::app::Control::instance();
  ctrl->loadConf();
  ctrl->startProcs();

  bool ok = false;
  QString addr = meteo::global::serviceAddress( meteo::settings::proto::kDiagnostic, &ok );
  int32_t port;
  QString host;
  if ( false == meteo::rpc::TcpSocket::parseAddress( addr, &host, &port ) ) {
  }
  error_log_if ( !ok ) << QObject::tr("Не удалось получить адрес публикации сервиса.");

  service = new meteo::app::Service();
  server = new meteo::rpc::Server( service, meteo::rpc::TcpSocket::stringAddress("127.0.0.1", port ) );
  server->init();
  if ( nullptr == server ) {
    error_log << QObject::tr("Не удалось создать сетевой сервер управления приложениями");
    delete app;
    return nullptr;
  }
  else {
    QObject::connect( server, SIGNAL( clientSubscribed( meteo::rpc::Controller* ) ),
        ctrl, SLOT( slotClientSubscribed( meteo::rpc::Controller* ) ) );
    QObject::connect( server, SIGNAL( clientUnsubscribed( meteo::rpc::Controller* ) ),
        ctrl, SLOT( slotClientUnsubscribed( meteo::rpc::Controller* ) ) );
  }

  return app;
}

void stop()
{
  // TODO: объединить с meteo::app::Conf::killProcess()
  QList<ProcRead::ProcInfo> list = ProcRead::procChildList(qApp->applicationPid(), true);
  for ( int i=0,isz=list.size(); i<isz; ++i ) {
    QProcess killer;
    killer.start("kill", QStringList() << "-9" << QString::number(list[i].pid));
    killer.waitForFinished();
    killer.kill();
    killer.waitForFinished();
  }

  // на случай если appmanager завершился аварийно
  meteo::app::Conf::killProcess();

  delete ctrl;
  ctrl = nullptr;
  delete service;
  service = nullptr;
  server->shutdown();
  delete server;
  server = nullptr;
}

