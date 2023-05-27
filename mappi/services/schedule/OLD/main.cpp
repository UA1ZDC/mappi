#include "scheduleservice.h"


#include <commons/proc_read/daemoncontrol.h>
#include <meteo/commons/rpc/rpc.h>

#include <mappi/global/global.h>
#include <mappi/settings/mappisettings.h>


#include <qcoreapplication.h>


int main(int argc, char *argv[])
{
  TAPPLICATION_NAME("meteo");

  try {
    QCoreApplication app(argc, argv);

    //::meteo::gGlobalObj(new ::mappi::MappiGlobal);
    //TLog::setMsgHandler(meteo::global::stdOutWithTime);
    //    TLog::setMsgHandler(TLog::clearStdOut);

    meteo::gSettings(mappi::inter::Settings::instance());
    
    //    if ( false == meteo::global::Settings::instance()->load() ){
    if ( false == meteo::gSettings()->load()) {
      error_log << meteo::msglog::kSettingsLoadFailed;
      return EXIT_FAILURE;
    }
  
    // if(!::meteo::mappi::TMeteoSettings::instance()->load()) {
    //   error_log << QObject::tr("Не удалось загрузить настройки.");
    //   return EXIT_FAILURE;
    // }

    QString appName = app.applicationFilePath();
    if(ProcControl::isAlreadyRun(appName)) {
      error_log << QObject::tr("Процесс %1 уже запущен.").arg(appName);
      return EXIT_FAILURE;
    }


    // ::meteo::AppStatusThread* status = new ::meteo::AppStatusThread;
    // status->setUpdateLimit(1000);
    // status->setSendLimit(5000);
    // status->start();

    mappi::schedule::ScheduleService* service = new mappi::schedule::ScheduleService(/*status*/);
    
    int msgPort = meteo::global::service( meteo::settings::proto::kSchedule ).port();
    var(msgPort);
    meteo::rpc::Server* server = new meteo::rpc::Server(service, QString("0.0.0.0:%1").arg(msgPort));
    if ( nullptr == server ) {
      error_log << QObject::tr("Не удалось создать сервер scheduler");
      return EXIT_FAILURE;
    }
    server->init();
    info_log << QObject::tr("Сервис опубликован по адресу %1").arg(server->address());
    
    // QObject::connect(server, &meteo::rpc::Server::clientSubscribed,   service, &mappi::schedule::ScheduleService::clientSubscribed);
    // QObject::connect(server, &meteo::rpc::Server::clientDisconnected, service, &mappi::schedule::ScheduleService::clientDisconnected);
    //info_log << QObject::tr("Cервис расписаний запущен успешно");
    
    app.exec();
    
    delete server; server = nullptr;
    delete service; service = nullptr;
  }
  catch(const std::bad_alloc& ) {
    critical_log << QObject::tr("Недостаточно памяти для работы программы");
    return EXIT_FAILURE;
  }


  
  return EXIT_SUCCESS;
}
