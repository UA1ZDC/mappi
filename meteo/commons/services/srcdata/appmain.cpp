#include <memory>

#include <qthread.h>
#include <qstringlist.h>
#include <qcoreapplication.h>

#include <commons/proc_read/daemoncontrol.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/services/srcdata/tdata_service.h>
#include <meteo/commons/services/srcdata/dataloader.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

int appMain(int argc, char *argv[])
{
  TAPPLICATION_NAME("meteo");
  meteo::global::setLogHandler();

  int result = EXIT_FAILURE;

  try {
    QCoreApplication app(argc, argv);
    ::meteo::gSettings(meteo::global::Settings::instance());

    if ( false == meteo::global::Settings::instance()->load() ) {
      error_log << QObject::tr("Не удалось загрузить настройки.");
      return EXIT_FAILURE;
    }

    QString appName = app.applicationFilePath();
    if(ProcControl::isAlreadyRun(appName)) {
      error_log << QObject::tr("Процесс %1 уже запущен.").arg(appName);
      return result;
    }
    ::meteo::AppStatusThread* status = new ::meteo::AppStatusThread;

    meteo::surf::TDataService* service = new meteo::surf::TDataService(status);
    meteo::rpc::Server server(service, meteo::global::serviceAddress(meteo::settings::proto::kSrcData));
    if ( false == server.init() ) {
      error_log << QObject::tr("Не создан сервер связи с клиентами");
      return EXIT_FAILURE;
    }
    QObject::connect( &server, &meteo::rpc::Server::ansverSendedError,
                      service,&meteo::surf::TDataService::slotAnsverSendedError );
    QObject::connect( &server, &meteo::rpc::Server::ansverSendedOK,
                      service,&meteo::surf::TDataService::slotAnsverSendedOK );
    status->setUpdateLimit(1000);
    status->setSendLimit(5000);
    status->start();

    ::setlocale(LC_NUMERIC, "C"); // локаль для записи координат в mongo (libbson)

    TSList<meteo::LoaderCard> loaderQueue;
    service->setLoaderQueue(&loaderQueue);

    // DataLoader
    meteo::DataLoader* loader = new meteo::DataLoader;
    loader->setQueue(&loaderQueue);
    loader->setAddress(meteo::gSettings()->serviceAddress(meteo::settings::proto::kSrcDataParent));

    QObject::connect( loader, &meteo::DataLoader::finished, [=](const QString& id, int dataCount, bool result, const QString& comment, const QString& action){
      //trc << var(id) << var(dataCount) << var(result) << var(comment);
      QString title = meteo::global::serviceTitle(meteo::settings::proto::kSrcData);
      if ( 0 == dataCount ) {
        meteo::dbusSendNotification(title, QObject::tr("Нет данных: %1").arg(comment), QString(), 30000);
      }
      else {
        meteo::dbusSendNotification(title, QObject::tr("Получены новые данные: %1").arg(comment), QString(), 30000, id, action);
      }
      if ( false == result ) {
        meteo::dbusSendNotification(title, QObject::tr("Ошибка при обработке запроса: ").arg(comment), QString(), 10000);
      }
    });

    QThread* loaderThread = new QThread;
    QObject::connect( loaderThread, &QThread::started, loader, &meteo::DataLoader::slotInit );

    loader->moveToThread(loaderThread);

    loaderThread->setObjectName("DataLoader");
    loaderThread->start();
    info_log << QObject::tr("%1 запущена").arg(meteo::global::serviceTitle(meteo::settings::proto::kSrcData));

    result = app.exec();
    loaderThread->terminate();
    loaderThread->wait(5000);
    delete loaderThread;
  }
  catch(const std::bad_alloc&) {
    critical_log << QObject::tr("Недостаточно памяти для работы приложения");
  }
  return result;
}
