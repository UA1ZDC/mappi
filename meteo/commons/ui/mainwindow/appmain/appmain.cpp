#include <qapplication.h>
#include <qtranslator.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/settings/tusersettings.h>

#include "startapp.h"

#ifdef T_OS_ASTRA_LINUX

#include <linux/prctl.h>
#include <parsec/mac.h>
#include <parsec/parsec_integration.h>
#include <parsec/parsec_mac.h>
#include <sys/prctl.h>

#endif

int appMain( int argc, char* argv[],const char *app_name, const QString& splashpath, const QString & icon )
{
/*#ifdef T_OS_ASTRA_LINUX
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
*/

//QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

try {
    TAPPLICATION_NAME("meteo");
    QApplication* app = new QApplication(argc, argv);
    QSplashScreen* splash = nullptr;
    QApplication::setWindowIcon(QIcon(icon));
    QApplication::setApplicationName(app_name);
    if ( false == splashpath.isEmpty() ) {
      QPixmap pixmap(":/meteo/splash/splash.png");
      splash = new QSplashScreen(pixmap);
      splash->show();
      app->processEvents();
    }

    QTranslator tr;

    if ( true ==  tr.load( "qt_ru", "/usr/share/qt5/translations"  ) ) {
      app->installTranslator(&tr);
    }
      app->processEvents();

    QApplication::addLibraryPath(MnCommon::projectPath() + "/lib");
    ::setlocale(LC_NUMERIC, "C"); // локаль для записи координат в mongo (libbson)

//    app->processEvents();
    ::meteo::gSettings(new meteo::Settings);

    meteo::global::setLogHandler();

    if ( false == meteo::global::Settings::instance()->load() ){
      if ( nullptr !=splash ) {
        splash->close();
      }
      error_log.msgBox() << QObject::tr("Не удается загрузить настройки приложения. Работа приложения будет завершена");
      delete app;
      return EXIT_FAILURE;
    }

    if ( false == meteo::gSettings()->load() ) {
      if ( nullptr !=splash ) {
        splash->close();
      }
      error_log << QObject::tr("Не удалось загрузить настройки.");
      QMessageBox::critical(0, "Внимание", QObject::tr("Ошибка. Не удалось загрузить настройки."));
      delete app;
      return EXIT_FAILURE;
    }
    app->processEvents();

    if(false == meteo::TUserSettings::instance()->load()) {
      splash->close();
      QString msg = QObject::tr("Не удалось инициализировать админ настройки. Приложение будет работать без авторизации.");
      QMessageBox::warning(0,QObject::tr("Ошибка"),msg,QObject::tr("Закрыть"));
    }
    else if (false == meteo::TUserSettings::instance()->isAuth()) {
      if ( nullptr !=splash ) {
        splash->close();
      }

      QString msg = QObject::tr("Доступ для пользователя <b>")
          + meteo::TUserSettings::instance()->getCurrentUserLogin()
          + QObject::tr("</b> запрещён. <br/> Хотите добавить пользователя?");
      QMessageBox* userQuestion = new QMessageBox;
      userQuestion->setText(msg);
      userQuestion->setWindowTitle(QObject::tr("Ошибка"));
      userQuestion->addButton(QObject::tr("Да"),QMessageBox::AcceptRole);
      userQuestion->addButton(QObject::tr("Нет"),QMessageBox::RejectRole);
      if (0 == userQuestion->exec()) {
        meteo::StartApp* stapp = new meteo::StartApp;
        stapp->startUserSettings();
        app->exec();
        return EXIT_FAILURE;
      }
      else {
        app->quit();
        return EXIT_FAILURE;
      }
    }

    meteo::app::MainWindow* window = WidgetHandler::instance()->mainwindow();
    if( window == nullptr ){
        window = new meteo::app::MainWindow;
    }
    window->setAppName(QObject::tr(app_name));
    //window->setWindowTitle(QObject::tr(" "));
    window->addPluginGroup("meteo.app.widgets");
    app->processEvents();
    window->setupToolbar();
    app->processEvents();
    window->setWindowIcon(QIcon(icon));

    window->show();

    if ( nullptr != splash ) {
      splash->finish(window);
    }

    app->exec();
    if ( nullptr != splash ){
      delete splash;
    }
    //delete splash;
    delete app;
  }
  catch( const std::bad_alloc& ) {
    critical_log << QObject::tr("Недостаточно памяти для работы программы");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}


