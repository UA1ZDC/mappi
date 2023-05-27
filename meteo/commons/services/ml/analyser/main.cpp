#include <stdlib.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/app/options.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/prognoz/settings/prognozsettings.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/forecast.pb.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/services/obanal/tobanalservice.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/field.pb.h>

#include <meteo/commons/proto/surface.pb.h>
#include <commons/textproto/pbtools.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/complexmeteo.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/mathtools/mnmath.h>
#include <meteo/commons/services/obanal/tobanalservice.h>
#include <meteo/commons/services/obanal/tobanaldb.h>
#include <commons/obanal/func_obanal.h>

#include <commons/geobasis/geodata.h>

#include <QCoreApplication>

using commons::ArgParser;
using commons::Arg;

namespace {


const Arg kHelp                       =     Arg::make( "-h",  "--help"  );
const Arg kForecastOpr                =     Arg::make( "-o",  "--opr"   );
const Arg kForecastCalculate          =     Arg::make( "-c",  "--calc"   );

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
  std::cout << paramhelp( kForecastCalculate,         "Расчет и заполнение весов с грибами"        ) << '\n';

}
}




void startAnalyse(){

  meteo::field::DataAnalyseRequest request;

  request.add_centers(4);
  request.add_centers(34);
  request.add_centers(74);
  request.set_days_interval(-10);
  request.set_step(-3600*6);

  meteo::rpc::Channel* field_ch = meteo::global::serviceChannel( meteo::settings::proto::kField );
  if ( 0 == field_ch ) {
      error_log << QObject::tr("Не удалось подключиться к сервису объективного анализа данных.");
      return;
    }

  meteo::field::SimpleDataReply * reply =
      field_ch->remoteCall( &meteo::field::FieldService::CalcForecastOpr, request,  30000000, true);
  if ( nullptr == reply ) {
      error_log << QObject::tr("При попытке получить анализ данных ответ от сервиса данных не получен");
      delete field_ch;
      return;
    }

  // debug_log << reply->Utf8DebugString();
  delete field_ch;
  delete reply;
}



void startForecastCalculate(){

  // int i, n;
  // double cma=0, avg=0, avg2=0;

  // for ( i=0, n=0; i<10; i++ ){
  //   error_log<< i;
  //   MnMath::calculateCumulateDeviation( cma, n, avg, avg2, i );
  //   error_log<<"СКО "<< cma << " n=" << n;
  // }
  // error_log<<"СКО "<< cma;

  meteo::prognoz::GribAnalyser* analyser = new meteo::prognoz::GribAnalyser();

  //
  // Все прогностические модели,по которым будем считать
  //
  QList<int> models = QList<int>() << 45
                                   << 4
                                   << 96
                                   << 15
                                   << 11;

  //
  // Все прогностические центры,по которым будем считать
  //
  QList<int> centers = QList<int>() << 4
                                    << 5
                                    << 34;

  //
  // Стандартные изобарические уровни, для которых рассчитываем поля
  //
  QList<int>  levels = QList<int>() << 950
                                    << 900
                                    << 850
                                    << 700
                                    << 600
                                    << 500
                                    << 400
                                    << 300
                                    << 250
                                    << 200
                                    << 150
                                    << 100
                                    << 50 ;

  //
  // Метеопараметры, по которым анализируем поля
  //
  QList<int>  params = QList<int>() << 10051
                                    << 10004
                                    << 12101
                                    << 12108
                                    << 12103
                                    << 20001
                                    << 20010
                                    << 11001
                                    << 11002;

  //
  // Сроки за которые анализируем
  //
  QList<int> hours = QList<int>() << 00;
                                  // << 12;

  // устанаваливаем отладку
  analyser->setDebugOn()
          // устанавливаем множитель десктриптора под которым будем сохранять
          // в 0 (т.е. не используем)
          .setFieldDescription(0);


  for (int m : models) {
    // модель
    analyser->setModel(m);

    for (int c : centers ) {
      // центр
      analyser->setCenter(c);

      for (int l : levels ){
        // уровень
        analyser->setLevel(l);

        for (int p : params ){
          // метеопараметр
          analyser->setParam(p);

          for (int h : hours ){
            error_log << analyser->getErrorMsg();
            // час и запускаем
            analyser->setHour(h)
                    .clearData()
                    .startForecastLostDataCalculate();
            error_log<<"============================= END CALCULATE ===============================================";
          }
        }
      }
    }
  }
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
  if (true == options->installed(kForecastOpr)) {
      startAnalyse();
    return EXIT_SUCCESS;
  }
  if (true == options->installed(kForecastCalculate)) {
      startForecastCalculate();
    return EXIT_SUCCESS;
  }



  usage();

}


