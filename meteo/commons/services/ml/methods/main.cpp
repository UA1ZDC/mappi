#include <stdlib.h>

#include <forecast_ml.h>

#include <QCoreApplication>

using commons::ArgParser;
using commons::Arg;

namespace {


const Arg kHelp                       =     Arg::make( "-h",  "--help"  );
const Arg kForecastOpr                =     Arg::make( "-o",  "--opr"   );
const Arg kForecastML                 =     Arg::make( "-m",  "--machinelearning"   );

std::string paramhelp( const Arg& arg, const char* descr )
{
  Q_UNUSED(descr);
  QString str = QObject::tr("    %1")
      .arg( arg.help() ).leftJustified(40);
  return (str+descr).toStdString();
}

void usage()
{
  std::cout
      << '\n'
      << QObject::tr("Использование: ").toStdString()
      << qApp->applicationName().toStdString()
      << QObject::tr(" [параметры]").toStdString()
      << '\n' << '\n';
  std::cout
      << QObject::tr("Параметры:").toStdString()
      << '\n';
  std::cout << paramhelp( kHelp,                      "Показать эту справку и выйти"                ) << '\n';
  std::cout << paramhelp( kForecastOpr,               "Расчет оправдываемости прогнозов"           ) << '\n';
  std::cout << paramhelp( kForecastML,         "Машинное обучение прогнозов"        ) << '\n';

}
}



void startMLCalculate(){
  meteo::prognoz::ForecastML* forecastml = new meteo::prognoz::ForecastML();

  forecastml->trainModel();

  delete forecastml;
  return;
}







/**
 * Главная функция
 * @param  argc [description]
 * @param  argv [description]
 * @return      [description]
 */
int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("meteo");
  meteo::global::setLogHandler();
  QCoreApplication* app = new QCoreApplication( argc, argv, false );
  Q_UNUSED( app );
  ::setlocale(LC_NUMERIC, "C"); // локаль для записи координат в mongo (libbson)

  ::meteo::gSettings(::meteo::prognoz::Settings::instance());
  if( !::meteo::prognoz::Settings::instance()->load() ){
    return EXIT_FAILURE;
  }
  if ( 1 == argc ) {
    usage();
    return EXIT_FAILURE;
  }
  ArgParser* options = ArgParser::instance();
  if ( false == options->parse( argc, argv ) ) {
    usage();
    return EXIT_FAILURE;
  }
  if ( true == options->installed(kHelp)) {
    usage();
    return EXIT_SUCCESS;
  }
  if (true == options->installed(kForecastML)) {
      startMLCalculate();
    return EXIT_SUCCESS;
  }



  usage();

}


