
//#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <meteo/commons/services/vko/vkocalc.h>

#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/services/climatdata/climataero.h>
#include <meteo/commons/settings/settings.h>


#include <qapplication.h>
#include <qdir.h>

#include <unistd.h>
#include <stdlib.h>

using namespace meteo;

//любой aero
void test_getMonthAero()
{
  VkoCalc calc;
  calc.setStation("26075", 3);
  calc.setDt("2018-01-01", "2021-12-31");
  
  var(meteo::climat::kVkot1);
  calc.getClimatData(meteo::climat::kVkot1);
  calc.calcTableValues();

  var(meteo::climat::kVkoIsoterm);
  calc.getClimatData(meteo::climat::kVkoIsoterm);
  calc.calcTableValues();
  
  var(meteo::climat::kVkoTropo);
  calc.getClimatData(meteo::climat::kVkoTropo);
  calc.calcTableValues();

  var(meteo::climat::kVkoff1);
  calc.getClimatData(meteo::climat::kVkoff1);
  calc.calcTableParamValues(1, 1500);
  
}

void test_getVisible()
{
  VkoCalc calc;
  calc.setStation("26063", 3);
  calc.setDt("2018-01-01", "2021-12-31");
    
  var(meteo::climat::kVkoN);
  calc.getClimatData(meteo::climat::kVkoN);
  calc.calcTableValues();

  var(meteo::climat::kVkoh);
  calc.getClimatData(meteo::climat::kVkoh);
  calc.calcTableParamValues(1, -1);

  var(meteo::climat::kVkohV);
  calc.getClimatData(meteo::climat::kVkohV);
  calc.calcTableParamValues(2, 0);

  var(meteo::climat::kVkoC);
  calc.getClimatData(meteo::climat::kVkoC);
  calc.calcTableValues();

  var(meteo::climat::kVkoV);
  calc.getClimatData(meteo::climat::kVkoV);
  calc.calcTableParamValues(1,-1);
}

void test_getPhenomen()
{
  VkoCalc calc;
  calc.setStation("26063", 3);
  calc.setDt("2018-01-01", "2021-12-31");

  
  // var(meteo::climat::kVkoR);
  // calc.getClimatData(meteo::climat::kVkoR);
  // calc.calcTableValues();
  
  // var(meteo::climat::kVkoYR);
  // calc.getClimatData(meteo::climat::kVkoYR);
  // calc.calcTableValues();

  // var(meteo::climat::kVkow);
  // calc.getClimatData(meteo::climat::kVkow);
  // calc.calcTableValues();

  // var(meteo::climat::kVkoss);
  // calc.getClimatData(meteo::climat::kVkoss);
  // calc.calcTableValues();

  // var(meteo::climat::kVkoYss);
  // calc.getClimatData(meteo::climat::kVkoYss);
  // calc.calcTableValues();

  var(meteo::climat::kVkoYFr);
  calc.getClimatData(meteo::climat::kVkoYFr);
  calc.calcTableValues();  
}

void test_getWind()
{
  VkoCalc calc;
  calc.setStation("26063", 3);
  calc.setDt("2018-01-01", "2021-12-31");

  var(meteo::climat::kVkodd);
  calc.getClimatData(meteo::climat::kVkodd);
  calc.calcTableValues();
  
  var(meteo::climat::kVkoff);
  calc.getClimatData(meteo::climat::kVkoff);
  calc.calcTableValues();

  var(meteo::climat::kVkoYfx);
  calc.getClimatData(meteo::climat::kVkoYfx);
  calc.calcTableValues();
}

void test_getTemp()
{
  VkoCalc calc;
  calc.setStation("26063", 3);
  calc.setDt("2018-01-01", "2021-12-31");
  
  var(meteo::climat::kVkoT);
  calc.getClimatData(meteo::climat::kVkoT);
  calc.calcTableValues();
  
  var(meteo::climat::kVkodT);
  calc.getClimatData(meteo::climat::kVkodT);
  for (int midx = 1; midx < 13; midx++) {
    calc.calcTableParamValues(midx, 21);
  }
    
  var(meteo::climat::kVkoU);
  calc.getClimatData(meteo::climat::kVkoU);
  calc.calcTableValues(); 
}

void test_getMonth()
{
  VkoCalc calc;
  calc.setStation("26063", 3);
  calc.setDt("2018-01-01", "2021-12-31", 10);
  
  // var(meteo::climat::kVkoNm);
  // calc.getClimatData(meteo::climat::kVkoNm);
  // calc.calcTableValues();

  // var(meteo::climat::kVkohm);
  // calc.getClimatData(meteo::climat::kVkohm);
  // calc.calcTableValues();

  // var(meteo::climat::kVkoVm);
  // calc.getClimatData(meteo::climat::kVkoVm);
  // calc.calcTableValues();

  // var(meteo::climat::kVkohVm);
  // calc.getClimatData(meteo::climat::kVkohVm);
  // calc.calcTableValues();

  // var(meteo::climat::kVkoP0m);
  // calc.getClimatData(meteo::climat::kVkoP0m);
  // calc.calcTableValues();
  
  // var(meteo::climat::kVkoRm);
  // calc.getClimatData(meteo::climat::kVkoRm);
  // calc.calcTableValues();
  
  // var(meteo::climat::kVkoYfxm);
  // calc.getClimatData(meteo::climat::kVkoYfxm);
  // calc.calcTableValues();

  // var(meteo::climat::kVkowm);
  // calc.getClimatData(meteo::climat::kVkowm);
  // calc.calcTableValues();

  // var(meteo::climat::kVkoTm);
  // calc.getClimatData(meteo::climat::kVkoTm);
  // calc.calcTableValues();

  // var(meteo::climat::kVkoUm);
  // calc.getClimatData(meteo::climat::kVkoTm);
  // calc.calcTableValues();
  
  var(meteo::climat::kVkoff1m);
  calc.getClimatData(meteo::climat::kVkoff1m);
  calc.calcTableValues();

  // var(meteo::climat::kVkoTropom);
  // calc.getClimatData(meteo::climat::kVkoTropom);
  // calc.calcTableValues();

  // var(meteo::climat::kVkoTgm);
  // calc.getClimatData(meteo::climat::kVkoTgm);
  // calc.calcTableValues();

  // var(meteo::climat::kVkossm);
  // calc.getClimatData(meteo::climat::kVkossm);
  // calc.calcTableValues();

  // var(meteo::climat::kVkoYssm);
  // calc.getClimatData(meteo::climat::kVkoYssm);
  // calc.calcTableValues();

}

void test_getInfo()
{
  VkoCalc calc;
  calc.setStation("26063", 3);
  calc.setDt("2018-01-01", "2021-12-31");

  
  var(meteo::climat::kVkoddY);
  calc.getClimatData(meteo::climat::kVkoddY);
  calc.calcTableValues();
  
  var(meteo::climat::kVkoffxY);
  calc.getClimatData(meteo::climat::kVkoffxY);
  calc.calcTableValues();

  var(meteo::climat::kVkoNY);
  calc.getClimatData(meteo::climat::kVkoNY);
  calc.calcTableValues(); 

  var(meteo::climat::kVkowRY);  
  calc.getClimatData(meteo::climat::kVkowRY);
  calc.calcTableValues();
  
  var(meteo::climat::kVkoTY);
  calc.getClimatData(meteo::climat::kVkoTY);
  calc.calcTableValues();
  
}
  
void help(const QString& progName)
{
  info_log << QObject::tr("\nИспользование:") << progName;

}

int main (int argc, char* argv[])
{
  TAPPLICATION_NAME("meteo");
  QCoreApplication app(argc, argv);

  QString file;
  int opt;

  while ((opt = getopt(argc, argv, "f:h")) != -1) {
    switch (opt) {
    case 'f':
      file = optarg;
      break;
    case 'h':
      help(argv[0]);
      exit(0);
      break;
    default: /* '?' */
      error_log<<"Option"<<opt<<"not realized";
      exit(-1);
    }
  }

  ::meteo::gSettings(new meteo::Settings);
  if ( false == meteo::gSettings()->load() ) {
    error_log << QObject::tr("Ошибка при загрузке настроек.");
    return EXIT_FAILURE;
  }
  

  // ::meteo::gSettings(::meteo::bank::Settings::instance());

  // if ( !meteo::bank::Settings::instance()->load() ) {
  //   error_log << QObject::tr("Ошибка при загрузке настроек.");
  //   return EXIT_FAILURE;
  // }

  // test_getMonthAero();
  // test_getVisible();
  // test_getPhenomen();
  // test_getWind();
  test_getTemp();

  //test_getMonth();
  //test_getInfo();
  
  return 0;
}
