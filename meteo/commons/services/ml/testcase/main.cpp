#include <stdlib.h>


#include <meteo/commons/services/ml/methods/ml.h>
#include <meteo/commons/services/ml/analyser/analyser.h>
#include <meteo/commons/services/ml/analyser/processes.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/app/options.h>
#include <meteo/commons/settings/settings.h>
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

// #include <meteo/ml/services/obanal/obanalmaker.h>

#include <commons/geobasis/geodata.h>
//#include <meteo/commons/services/fieldata/fieldservicedb.h>

#include <qprocess.h>

#include <QCoreApplication>

using commons::ArgParser;
using commons::Arg;

namespace {


const Arg kHelp      = Arg::make( "-h",  "--help"  );
const Arg kSubstract = Arg::make( "-sb",  "--substract"   );

// координаты верхнего левого и нижнего правого углов
const Arg kTopLat    = Arg::make( "-t",  "--lat-t", true   );
const Arg kTopLon    = Arg::make( "-n",  "--lon-t", true   );
const Arg kBottomLat = Arg::make( "-T",  "--lat-b", true   );
const Arg kBottomLon = Arg::make( "-N",  "--lon-b", true   );

// параметрпо которому будем считать
const Arg kParam     = Arg::make( "-p",  "--param", true   );

// уровень которому будем считать
const Arg kLevel     = Arg::make( "-l",  "--level", true   );

// уровень которому будем считать
const Arg kLevelType     = Arg::make( "-lt",  "--leveltype", true   );

// дата которому будем считать
const Arg kDate      = Arg::make( "-d",  "--date", true   );

// срок которому будем считать
const Arg kSrok      = Arg::make( "-hr",  "--hour", true   );

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
  std::cout << paramhelp( kHelp,              "Показать эту справку и выйти"                ) << '\n';
  std::cout << paramhelp( kSubstract,         "Вырезание фрагмента"           ) << '\n';
  std::cout << paramhelp( kTopLat,            "широта верхней левой точки в градусах"           ) << '\n';
  std::cout << paramhelp( kTopLon,            "долгота верхней левой точки в градусах"           ) << '\n';
  std::cout << paramhelp( kBottomLat,         "широта нижней правой точки в градусах"           ) << '\n';
  std::cout << paramhelp( kBottomLon,         "долгота нижней правой точки в градусах"           ) << '\n';
  std::cout << paramhelp( kParam,             "параметр который вырезаем descrcode"           ) << '\n';
  std::cout << paramhelp( kLevel,             "уровень"           ) << '\n';
  std::cout << paramhelp( kLevelType,         "тип уровеня"           ) << '\n';
  std::cout << paramhelp( kDate,              "дата в формате yyyy-MM-dd"           ) << '\n';
  std::cout << paramhelp( kSrok,              "срок"           ) << '\n';


}
}


/**
 * вырезаем заданный регион из фактических данных
 * за известную дату и срок
 * @param TopLat    [description]
 * @param TopLon    [description]
 * @param BottomLat [description]
 * @param BottomLon [description]
 */
void substractRegion(float TopLat,float TopLon,float BottomLat,float BottomLon, QString date, int srok, int level,int leveltype, int param){
  // Q_UNUSED(param);
  // // определяем дескриптор (номер по имени)
  // meteodescr::TMeteoDescriptor* md = TMeteoDescriptor::instance();
  // int intpar = md->descriptor(param);
  // delete md;

  meteo::ml::Analyser* analyser = new meteo::ml::Analyser();

  // задаем параметры для запроса
  analyser->setDebugOn()
           .setDateTime(   QDateTime::fromString(date, "yyyy-MM-dd") )
           .setHour(   srok )
           .setRegion( double(TopLat), double(TopLon), double(BottomLat), double(BottomLon) )
           .setParam(  param )
           .setLevel(  level );
  //
  // вырезаем регион
  //
  analyser->substractSquareFromSrcData();

  QString surfaero="surface";
  if (leveltype==100){
    surfaero="aero";
  }
  QString argAeroPath = QString( ::MnCommon::binPath()+"pak.obanal "
                             + " --"+surfaero
                             + " --descr "+QString("%1").arg(param)
                             + " --levelP "+QString("%1").arg(level)
                             + " --begin " + analyser->getDateTime().toString(Qt::ISODate)
                             + " --end " + analyser->getDateTime().toString(Qt::ISODate));
  QProcess process;
  process.execute( argAeroPath );

  argAeroPath = QString( ::MnCommon::binPath()+"meteo.ml.app -rs "
                         + " -lvl " + QString("%1").arg(level)
                         + " -lvt " + QString("%1").arg(leveltype)
                         + " -hr " + QString("%1").arg(srok)
                         + " -pm " + QString("%1").arg(param)
                         + " --date " + analyser->getDateTime().toString(Qt::ISODate) );
  process.execute( argAeroPath );

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
  QCoreApplication* app = new QCoreApplication( argc, argv, false );
  Q_UNUSED( app );
  ::setlocale(LC_NUMERIC, "C"); // локаль для записи координат в mongo (libbson)

  ::meteo::gSettings(::meteo::global::Settings::instance());
  if( !::meteo::global::Settings::instance()->load() ){
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


  //
  // вычитаем область и сохраняем в базу
  //
  if (true == options->installed(kSubstract)) {
    float TopLat=0,
          TopLon=0,
          BottomLat=0,
          BottomLon=0;
    // дата за которую забираем данные
    QString date= QDateTime::currentDateTimeUtc().toString("dd.MM.yyyy");
    int param = 12101;
    // срок
    int srok=0,
    // уровень
        level=850,
        leveltype=100;

    if ( options->at(kTopLat   ).value() != QString() ) {
      TopLat    = options->at(kTopLat   ).value().toFloat();
    }
    if ( options->at(kTopLon   ).value() != QString() ) {
      TopLon    = options->at(kTopLon   ).value().toFloat();
    }
    if ( options->at(kBottomLat).value() != QString() ) {
      BottomLat = options->at(kBottomLat).value().toFloat();
    }
    if ( options->at(kBottomLon).value() != QString() ) {
      BottomLon = options->at(kBottomLon).value().toFloat();
    }

    if ( options->at(kSrok).value() != QString() ) {
      srok = options->at(kSrok).value().toInt();
    }

    if ( options->at(kLevel).value() != QString() ) {
      level = options->at(kLevel).value().toInt();
    }

    if ( options->at(kParam).value()!= QString() ) {
      param = options->at(kParam).value().toInt();
    }

    if ( options->at(kLevelType).value() != QString() ) {
      leveltype = options->at(kLevelType).value().toInt();
    }

    if ( options->at(kDate).value() != QString() ) {
      date = options->at(kDate).value();
    }


    error_log << TopLat<< TopLon<< BottomLat<< BottomLon<< date << srok << level << param;
    // вырезаем регион из базы
    substractRegion( TopLat, TopLon, BottomLat, BottomLon, date, srok, level,leveltype, param );


    return EXIT_SUCCESS;
  }




  usage();

}


