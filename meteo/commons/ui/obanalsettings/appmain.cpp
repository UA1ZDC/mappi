#include <cross-commons/app/paths.h>
#include <meteo/commons/ui/obanalsettings/obanalsettings.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

#include <QMessageBox>

#include <QApplication>
#include <QProcessEnvironment>
#include <QTextCodec>

int appMain(int argc, char* argv[])
{
  TAPPLICATION_NAME("meteo");
  meteo::global::setLogHandler();
//  ::meteo::gGlobalObj(new ::meteo::PrognozGlobal);

  QApplication app(argc, argv);
//  QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());

  ::meteo::gSettings(new meteo::Settings);
  if ( false == meteo::gSettings()->load() ) {
    error_log << QObject::tr("Не удалось загрузить настройки.");
    QMessageBox::critical(0, "Внимание", QObject::tr("Ошибка. Не удалось загрузить настройки."));
    return EXIT_FAILURE;
  }

  meteo::ObanalSettings settings;
  settings.init();

//  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
//  settings.embedInto(env.value("METEO_XEMBED_WINID").toULong());
  settings.show();

  return app.exec();
}
