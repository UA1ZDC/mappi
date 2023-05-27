#include <stdlib.h>
#include <QGuiApplication>

#include <memory>
#include <iostream>

#include <qcoreapplication.h>
#include <qstringlist.h>
#include <qtextcodec.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/proc_read/daemoncontrol.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/services/map/mapservice.h>

#include <meteo/commons/settings/settings.h>
#include <meteo/commons/settings/tusersettings.h>
#include <meteo/commons/global/global.h>

int appMain(int argc, char* argv[])
{
  try {
    TAPPLICATION_NAME("meteo");

    QGuiApplication* app = new QGuiApplication(argc, argv);
    gSettings(meteo::global::Settings::instance());
    if ( false == meteo::global::Settings::instance()->load() ) {
      error_log << QObject::tr("Не удалось загрузить настройки.");
      return EXIT_FAILURE;
    }
    QString appName = app->applicationFilePath();
    if(ProcControl::isAlreadyRun(appName)) {
      error_log << QObject::tr("Процесс %1 уже запущен.").arg(appName);
      return EXIT_FAILURE;
    }

    meteo::global::setLogHandler();

   if (!meteo::TUserSettings::instance()->load()) {
     //QString msg = QObject::tr("Не удалось инициализировать админ настройки. Приложение будет работать без авторизации.");
//      QMessageBox::warning(0,QObject::tr("Ошибка"),msg,QObject::tr("Закрыть"));
   }
   else if (!meteo::TUserSettings::instance()->isAuth()) {
     debug_log << "OSHIBKA";
//      QString msg = QObject::tr("Доступ для пользователя <b>")
//          + meteo::TAdminSettings::instance()->getCurrentUserLogin()
//          + QObject::tr("</b> запрещён. <br/> Хотите добавить пользователя?");
//      QMessageBox* userQuestion = new QMessageBox;
//      userQuestion->setText(msg);
//      userQuestion->setWindowTitle(QObject::tr("Ошибка"));
//      userQuestion->addButton(QObject::tr("Да"),QMessageBox::AcceptRole);
//      userQuestion->addButton(QObject::tr("Нет"),QMessageBox::RejectRole);
//      int answer = userQuestion->exec();
//      if (0 == answer) {
//        meteo::StartApp* stapp = new meteo::StartApp;
//        stapp->startUserSettings();
//        app->exec();
//        return EXIT_FAILURE;
//      }
//      else {
//        app->quit();
//        return EXIT_FAILURE;
//      }
   }

    meteo::map::Service* service = new meteo::map::Service;

    meteo::rpc::Server* netserver = new meteo::rpc::Server(
      service, QString("127.0.0.1:%1").arg(meteo::global::service(meteo::settings::proto::kMap).port()));

    if ( nullptr == netserver ) {
      error_log << QObject::tr("Не создан сервер связи с клиентами");
      return EXIT_FAILURE;
    }
    if ( nullptr != service->methodbag() ) {
      QObject::connect( netserver, SIGNAL( clientConnected(meteo::rpc::Channel*) ),
          service->methodbag(), SLOT( slotClientConnected(meteo::rpc::Channel*) ) );
      QObject::connect( netserver, SIGNAL( clientDisconnected(meteo::rpc::Channel*) ),
          service->methodbag(), SLOT( slotClientDisconnected(meteo::rpc::Channel*) ) );
    }

    netserver->init();

    info_log << QObject::tr("Cервис генерации документов запущен успешно");
    app->exec();
  }
  catch (const std::bad_alloc &) {
    critical_log << QObject::tr("Недостаточно памяти для функционирования приложения");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
