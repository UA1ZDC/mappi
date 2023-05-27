#include "appmanagerdaemon.h"

#include <qprocess.h>
#include <qstringlist.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <commons/proc_read/checkps.h>

#include <meteo/commons/appmanager/tappconf.h>
#include <meteo/commons/appmanager/tappcontrol.h>
#include <meteo/commons/appmanager/tappcontrolservice.h>
#include <meteo/commons/proto/appconf.pb.h>
#include <meteo/commons/proto/services.pb.h>
#include <meteo/commons/rpc/rpclocation.h>
#include <meteo/commons/rpc/taddress.h>
#include <meteo/commons/rpc/tserver.h>

#include <mappi/global/global.h>
#include <mappi/settings/tmeteosettings.h>

AppManagerDaemon::AppManagerDaemon(int argc, char** argv)
  : QtService<QCoreApplication>(argc, argv, "MappiAppManager")
{
  control_ = 0;
  service_ = 0;
  server_ = 0;

  setServiceDescription(QObject::tr("Диспечер приложений МАППИ"));
  setServiceFlags(QtServiceBase::Default);
  setStartupType(QtServiceController::AutoStartup);
}

void AppManagerDaemon::start()
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

  ::meteo::gGlobalObj(new ::mappi::MappiGlobal);

  if ( !::meteo::mappi::TMeteoSettings::instance()->load() ) { return; }

  control_ = new meteo::app::Control;
  control_->loadConf();
  control_->startProcs();

  bool ok = false;
  rpc::Address addr = meteo::global::serviceAddress( meteo::settings::proto::kDiagnostic, &ok );
  error_log_if ( !ok ) << QObject::tr("Не удалось получить адрес публикации сервиса.");

  service_ = new meteo::app::Service(control_);
  server_ = rpc::Server::create( service_, rpc::Address("127.0.0.1", addr.port()) );
  if ( 0 == server_ ) {
    error_log << QObject::tr("Не удалось создать сетевой сервер управления приложениями");
  }
  else {
    QObject::connect( server_, SIGNAL( clientSubscribed( rpc::Channel*, google::protobuf::Closure* ) ),
        control_, SLOT( slotClientSubscribed( rpc::Channel*, google::protobuf::Closure* ) ) );
    QObject::connect( server_, SIGNAL( clientDisconnected( rpc::Channel* ) ),
        control_, SLOT( slotClientDisconnected( rpc::Channel* ) ) );
  }
}

void AppManagerDaemon::stop()
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

  delete control_;
  control_ = 0;
  delete service_;
  service_ = 0;
  delete server_;
  server_ = 0;
}
