#include <stdlib.h>
#include <iostream>

#include <qcoreapplication.h>
#include <qtimer.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/global/global.h>
#include "maker.h"

namespace meteo {
namespace obanal {

std::string paramhelp( const commons::Arg& arg, const char* descr )
{
  Q_UNUSED(descr);
  QString str = QObject::tr("    %1")
      .arg( arg.help() ).leftJustified(40);
  return (str+descr).toStdString();
}

void usage()
{
  std::cout << '\n'
            << QObject::tr("Использование: ").toStdString()
            << qApp->applicationName().toStdString()
            << QObject::tr(" [параметры]").toStdString()
            << '\n' << '\n';
  std::cout << QObject::tr("Параметры:").toStdString()
            << '\n';
  std::cout << paramhelp( kAeroArg,     "Проводить вычисление оправдываемости прогнозов Аэрологических данных" ) << '\n';
  std::cout << paramhelp( kSurfaceArg,  "Проводить вычисление оправдываемости прогнозов Приземных данных" ) << '\n';
  std::cout << paramhelp( kOceanArg,    "Проводить вычисление оправдываемости прогнозов Океанографических данных" ) << '\n';
  std::cout << paramhelp( kGribAnalyze, "Проводить вычисление оправдываемости прогнозов по GRIB" ) << '\n';
  std::cout << paramhelp( kBeginDate,   "Дата начала срока" ) << '\n';
  std::cout << paramhelp( kFinishDate,  "Дата окончания срока" ) << '\n';
  std::cout << paramhelp( kDescr,       "Дескриптор величины" ) << '\n';
//  std::cout << paramhelp( kStation,     "Индекс станции" ) << '\n';
  std::cout << paramhelp( kLevelP,      "Уровень по давлению" ) << '\n';
  std::cout << paramhelp( kLevelH,      "Уровень по высоте" ) << '\n';
  std::cout << paramhelp( kCenter,      "Центр обработки (только для GRIB)" ) << '\n';
  std::cout << paramhelp( kHour,        "Срок прогноза (только для GRIB)" ) << '\n';
  std::cout << paramhelp( kModel,       "Модель анализа (только для GRIB )" ) << '\n';
  std::cout << paramhelp( kLevelType,   "Тип уровня" ) << '\n';
  std::cout << paramhelp( kToJSON,      "Сохранить в JSON" ) << '\n';
  std::cout << paramhelp( kFile,        "Выполнить список заданий из указанного файла") << '\n';
  std::cout << paramhelp( kTasksUid,    "Выполнить только задания с указанными uid из списка заданий (используется вместе с --file)") << '\n';
  std::cout << paramhelp( kHelp,        "Показать эту справку и выйти" ) << '\n';
}

}
}


int appMain( int argc, char* argv[] )
{  
  using namespace meteo::obanal;
  try {
    TAPPLICATION_NAME("meteo");
    meteo::global::setLogHandler();
    ::setlocale(LC_NUMERIC, "C"); // локаль для записи координат в mongo (libbson)
    ::meteo::gSettings(meteo::global::Settings::instance());
    if ( false == meteo::global::Settings::instance()->load() ) {
      error_log << QObject::tr("Не удалось загрузить настройки.");
      return EXIT_FAILURE;
    }
    if ( 1 == argc ) {
      info_log << QObject::tr("Внимание! Необходимо указать параметры запуска.");
      usage();
      return EXIT_FAILURE;
    }
    commons::ArgParser* options = commons::ArgParser::instance();
    if ( false == options->parse( argc, argv ) ) {
      usage();
      return EXIT_FAILURE;
    }

    if ( true == options->installed(kHelp)) {
      usage();
      return EXIT_SUCCESS;
    }
    if ( ( true == options->installed(kAeroArg) )
         || ( true == options->installed(kOceanArg))
         || ( true == options->installed(kSurfaceArg))
         || ( true == options->installed(kGribAnalyze) )
         || ( true == options->installed(kFile))) {
      Maker maker(options);
      QCoreApplication* app = new QCoreApplication( argc, argv, false );
      QTimer::singleShot(0, &maker, SLOT(slotMakeAutoObanal()));
      QObject::connect(&maker, SIGNAL(finished(int)), app, SLOT( quit() ));
      return app->exec();
    }
    error_log << QObject::tr("Анализ не был произведён. Нет необходимых параметров запуска.");
    return EXIT_FAILURE;
  }
  catch (const std::bad_alloc & ) {
    critical_log << QObject::tr("Недостаточно памяти для функционирования приложения");
    return EXIT_FAILURE;
  }
}


