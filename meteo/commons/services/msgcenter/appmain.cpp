#include <signal.h>

#include <qthread.h>
#include <qcoreapplication.h>
#include <qprocess.h>
#include <qobject.h>

#include <cross-commons/app/targ.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/app/helpformatter.h>
#include <cross-commons/debug/tlog.h>

#include <cross-commons/debug/tmap.h>

#include "msgcenter.h"
#include "notconfirmedchecker.h"

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/appstatusthread.h>
#include <meteo/commons/settings/settings.h>

#include <meteo/commons/settings/settings.h>

#define HELP(opt, text)  (QStringList() << opt << QObject::tr(text))

const QStringList kHelpOpt      = QStringList() << "h" << "help";
const QStringList kMsgPortOpt   = QStringList() << "M" << "msg-srv-port";
const QStringList kPathOpt      = QStringList() << "p" << "path";
const QStringList kExpireOpt    = QStringList() << "e" << "msg-expire";

const HelpFormatter kHelp = HelpFormatter()
                            << HELP(kHelpOpt, "Эта справка.")
                            << HELP(QStringList(), "")
                            << HELP(kMsgPortOpt, "Порт для публикации сервиса msgcenter.")
                            << HELP(kPathOpt, "Путь сохранения телеграмм.");


int appMain( int argc, char** argv )
{
  TAPPLICATION_NAME( "meteo" );
  meteo::global::setLogHandler();
  gSettings(meteo::global::Settings::instance());
  QCoreApplication app( argc, argv );

  TArg args(argc, argv);

  if( args.contains(kHelpOpt) ){
    kHelp.print();
    return EXIT_SUCCESS;
  }

  if ( false == meteo::global::Settings::instance()->load() ){
    error_log << QObject::tr("Не удается загрузить настройки приложения. Работа приложения будет завершена");
    return EXIT_FAILURE;
  }

  meteo::AppStatusThread* status = new meteo::AppStatusThread;
  status->setUpdateLimit(1);
  status->setSendLimit(50);
  status->start();
  //
  //  // очередь входящих телеграмм
  //  meteo::MsgQueue queue;
  //
  //  TSList<meteo::GetTelegramCard> getRequests;
  //  TSList<meteo::msgcenter::DecodeRequest> decodeRequests;
  //  // Эта очередь предназначена для синхронизации при работе с multiresponse запросами,
  //  // которые обрабатываются в отдельном потоке.
  //  // Когда необходимо отправить данные клиенту, мы помещаем в очередь соответствующий done
  //  // и оповещаем MsgCenterService о пополнении очереди. После вызова Run() MsgCenterService
  //  // удаляет done из очереди и после этого можно приступать к заполнению новой порции данных.
  //  TSList<google::protobuf::Closure*> doneQueue;

  // Service
  meteo::MsgCenter* service = new meteo::MsgCenter(status);
  //  service->setStatusSender(status);
  //  service->setMsgQueue(&queue);
  //  service->setRequestQueue(&decodeRequests);
  //  service->setRequestQueue(&getRequests);
  //  service->setSyncQueue(&doneQueue);

  int msgPort = meteo::global::service( meteo::settings::proto::kMsgCenter ).port();
  msgPort = args.value(kMsgPortOpt, QString::number(msgPort)).toInt();

  meteo::rpc::Server* srv = new meteo::rpc::Server(service, QString("0.0.0.0:%1").arg(msgPort));
  if ( nullptr == srv ) {
    error_log << QObject::tr("Не удалось создать сервер msgcenter");
    return EXIT_FAILURE;
  }
  srv->init();

  info_log << QObject::tr("Сервис опубликован по адресу %1").arg(srv->address());

  meteo::NotConfirmedChecker notconfirmchecker(service);
  // Receiver
  //  meteo::MsgReceiver* receiver = new meteo::MsgReceiver;
  //  receiver->setMsgQueue(&queue);
  //  receiver->setMsgPath(args.value(kPathOpt, meteo::global::telegramsPath()));
  //  receiver->setStatusSender(status);
  //  receiver->setSelfIds({ wmoId, hmsId });

  //  QObject::connect( receiver, SIGNAL(msgProcessed(int)), service, SLOT(slotMsgProcessed(int)) );
  //  QObject::connect( receiver, SIGNAL(msgProcessed(int,QString,qint64,bool)), service, SLOT(slotMsgProcessed(int,QString,qint64,bool)) );
  //
  //  QThread* recvThread = new QThread;
  //  QObject::connect( recvThread, SIGNAL(started()), receiver, SLOT(slotInit()) );
  //
  //  receiver->moveToThread(recvThread);
  //
  //  recvThread->setObjectName("Receiver");
  //  recvThread->start();
  //
  //
  //  // Router
  //  meteo::MsgRouter* router = new meteo::MsgRouter;
  //  router->setRouteOpt(::meteo::global::Settings::instance()->route());
  //  router->setWmoId(wmoId);
  //  router->setHmsId(hmsId);
  //  router->setMsgQueue(&queue);
  //  router->setStatusSender(status);
  //
  //  QObject::connect( router, SIGNAL(msgProcessed(int)), service, SLOT(slotMsgProcessed(int)) );
  //  QObject::connect( router, SIGNAL(msgProcessed(int,QString,qint64,bool)), service, SLOT(slotMsgProcessed(int,QString,qint64,bool)) );
  //  QObject::connect( srv, &meteo::rpc::Server::clientDisconnected, router, &meteo::MsgRouter::slotClientDisconnected );
  //  QObject::connect( srv, &meteo::rpc::Server::clientSubscribed, router, &meteo::MsgRouter::slotClientSubscribed );
  //  QObject::connect( service, &meteo::MsgCenterService::subscribeDistributeMsg, router, &meteo::MsgRouter::slotNewSubscribe );
  //
  //  QTimer::singleShot( 0, router, SLOT(slotInit()) );
  //
  //
  //  // Updater
  //  meteo::MsgUpdater* updater = new meteo::MsgUpdater;
  //  updater->setRequestQueue(&decodeRequests);
  //  updater->setStatusSender(status);
  //
  //  QThread* updaterThread = new QThread;
  //  QObject::connect( updaterThread, &QThread::started, updater, &meteo::MsgUpdater::slotInit );
  //
  //  updater->moveToThread(updaterThread);
  //
  //  updaterThread->setObjectName("Updater");
  //  updaterThread->start();
  //
  //
  //  // Getter
  //  meteo::MsgGetter* getter = new meteo::MsgGetter;
  //  getter->setRequestQueue(&getRequests);
  //  getter->setStatusSender(status);
  //  getter->setSyncQueue(&doneQueue);
  //
  //  QObject::connect( getter, SIGNAL(syncQueueChanged()), service, SLOT(slotSendResponse()) );
  //
  //  QThread* getterThread = new QThread;
  //  QObject::connect( getterThread, &QThread::started, getter, &meteo::MsgGetter::slotInit );
  //
  //  getter->moveToThread(getterThread);
  //
  //  getterThread->setObjectName("Getter");
  //  getterThread->start();
  //
  //
  //  // GetterMulti
  //  meteo::MsgGetterMulti* getterMulti = new meteo::MsgGetterMulti;
  //    getterMulti->setStatusSender(status);
  //  getterMulti->setSyncQueue(&doneQueue);
  //
  //  QObject::connect( getterMulti, SIGNAL(syncQueueChanged()), service, SLOT(slotSendResponse()) );
  //  QObject::connect( service, SIGNAL(newGetTelegramMulti(const meteo::GetTelegramCard&)),
  //                    getterMulti, SLOT(slotNewGetTelegramMulti(const meteo::GetTelegramCard&)) );
  //  QObject::connect( srv, &meteo::rpc::Server::clientSubscribed, getterMulti, &meteo::MsgGetterMulti::slotClientSubscribed );
  //  QObject::connect( srv, &meteo::rpc::Server::clientUnsubscribed, getterMulti, &meteo::MsgGetterMulti::slotClientUnsubscribed );
  //
  //
  //  QThread* getterMultiThread = new QThread;
  //  QObject::connect( getterMultiThread, &QThread::started, getterMulti, &meteo::MsgGetterMulti::slotInit );
  //
  //  getterMulti->moveToThread(getterMultiThread);
  //
  //  getterMultiThread->setObjectName("GetterMulti");
  //  getterMultiThread->start();
  //
  //  // Processor
  //  meteo::MsgProcessor* processor = new meteo::MsgProcessor;
  //  processor->setWmoId(wmoId);
  //  processor->setHmsId(hmsId);
  //  processor->setQueue(&queue);
  //
  //
  //  QObject::connect( processor, SIGNAL(msgProcessed(int)), service, SLOT(slotMsgProcessed(int)) );
  //
  //  QThread* processorThread = new QThread;
  //  QObject::connect( processorThread, &QThread::started, processor, &meteo::MsgProcessor::slotInit );
  //
  //  processor->moveToThread(processorThread);
  //
  //  processorThread->setObjectName("Processor");
  //  processorThread->start();
  //
  //
  //  meteo::NotConfirmedChecker notconfirmchecker(service);

  app.exec();
  delete srv; srv = nullptr;
  delete service; service = nullptr;

  //  nodeServiceProc->kill();
  //  nodeServiceProc->deleteLater();
  //  status->terminate();
  //  recvThread->terminate();
  //  getterThread->terminate();
  //  getterMultiThread->terminate();
  //  updaterThread->terminate();
  //  processorThread->terminate();
  //
  //  status->wait(5000);
  //  recvThread->wait(5000);
  //  getterThread->wait(5000);
  //  getterMultiThread->wait(5000);
  //  updaterThread->wait(5000);
  //  processorThread->wait(5000);
  //  delete status;
  //  delete service;
  //  delete router;
  //  delete recvThread;
  //  delete getterThread;
  //  delete getterMultiThread;
  //  delete updaterThread;
  //  delete processorThread;

  return EXIT_SUCCESS;
}
