
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/services/climatdata/climataero.h>
#include <meteo/commons/services/climatdata/climatsurf.h>
#include <meteo/commons/settings/settings.h>


#include <qcoreapplication.h>
#include <qdir.h>

#include <unistd.h>
#include <stdlib.h>

using namespace meteo;

void test_getMonthAero()
{
  meteo::climat::DataRequest req;
  req.set_station("26075");
  req.set_station_type(sprinf::kStationAero);
  req.set_levtype(100);
  req.add_descrname("ff");
  req.add_descrname("dd");
  req.set_date_start("2020-07-01");
  req.set_date_end("2020-08-06");
  
  climat::AeroLayerListReply allres;

  climat::ClimatAeroService aero;
  bool ok = aero.getMonthValue(&req, &allres);
  var(ok);
  var(allres.Utf8DebugString());

}

void test_getSurfDay()
{
  meteo::climat::DataRequest req;
  req.set_station("26063");
  req.set_station_type(sprinf::kStationSynop);
  //req.add_descrname("R");
  req.add_descrname("ff");
  //  req.set_month(7);
  req.set_date_start("2020-07-01");
  req.set_date_end("2020-08-06");
  
  meteo::climat::SurfDayReply allres;

  climat::ClimatSurfService surf;
  bool ok = surf.getDayValue(&req, &allres);
  var(ok);
  var(allres.Utf8DebugString());

}


void test_getMonthValue()
{
  meteo::climat::DataRequest req;
  req.set_station("26063");
  req.set_station_type(sprinf::kStationSynop);
  //req.add_descrname("R");
  req.add_descrname("ff");
  //  req.set_month(7);
  req.set_date_start("2020-07-01");
  req.set_date_end("2020-08-06");
  
  meteo::climat::SurfMonthReply allres;

  climat::ClimatSurfService surf;
  bool ok = surf.getMonthValue(&req, &allres);
  var(ok);
  var(allres.Utf8DebugString());

}

void test_getMonthAvgValue()
{
  meteo::climat::DataRequest req;
  req.set_station("26063");
  req.set_station_type(sprinf::kStationSynop);
  //req.add_descrname("R");
  req.add_descrname("ff");
  //  req.set_month(7);
  req.set_date_start("2020-07-01");
  req.set_date_end("2020-08-06");
  
  meteo::climat::SurfMonthAvgReply allres;

  climat::ClimatSurfService surf;
  bool ok = surf.getAvgMonthValue(&req, &allres);
  var(ok);
  var(allres.Utf8DebugString());
}

void test_getYear()
{
  meteo::climat::DataRequest req;
  req.set_station("26063");
  req.set_station_type(sprinf::kStationSynop);
  //req.add_descrname("R");
  req.add_descrname("ff");
  //  req.set_month(7);
  req.set_date_start("2020-07-01");
  req.set_date_end("2020-08-06");
  
  meteo::climat::SurfYearReply allres;

  climat::ClimatSurfService surf;
  bool ok = surf.getYearValue(&req, &allres);
  var(ok);
  var(allres.Utf8DebugString());
}


void test_getDecade()
{
  meteo::climat::DataRequest req;
  req.set_station("26063");
  req.set_station_type(sprinf::kStationSynop);
  req.add_descrname("sss");
  //  req.set_month(7);
  req.set_date_start("2020-01-01");
  req.set_date_end("2020-08-06");
  
  meteo::climat::SurfDecadeReply allres;

  climat::ClimatSurfService surf;
  bool ok = surf.getDecadeValue(&req, &allres);
  var(ok);
  var(allres.Utf8DebugString());
}


void test_getSurfAll()
{
  meteo::climat::DataRequest req;
  req.set_station("26063");
  req.set_station_type(sprinf::kStationSynop);
  req.add_descrname("ff");
  req.add_descrname("dd");
  //  req.set_month(7);
  req.set_date_start("2020-01-01");
  req.set_date_end("2020-08-06");
  
  meteo::climat::SurfAllListReply allres;

  climat::ClimatSurfService surf;
  bool ok = surf.getAllValueList(&req, &allres);
  var(ok);
  var(allres.Utf8DebugString());
}



void test_getDate()
{
  meteo::climat::DataRequest req;
  req.set_station("26063");
  req.set_station_type(sprinf::kStationSynop);
  req.add_descrname("T");
  //  req.set_month(7);
  req.set_date_start("2019-01-01");
  req.set_date_end("2021-09-06");
  req.set_collection("surf_hour");
  req.set_limit(20);
  req.set_less_then(true);
  req.set_lmonth_start(7);
  req.set_lmonth_end(8);
  
  meteo::climat::SurfDateReply allres;

  climat::ClimatSurfService surf;
  bool ok = surf.getDateValue(&req, &allres);
  var(ok);
  var(allres.Utf8DebugString());
}


void help(const QString& progName)
{
  info_log << QObject::tr("\nИспользование:") << progName << "-f <file>\n"
    "-f <file> Файл со сводкой\n";
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
  

  test_getMonthAero();
  test_getSurfDay();
  test_getMonthValue();
  test_getMonthAvgValue();
  test_getYear();
  test_getDecade();
  test_getSurfAll();
  test_getDate();
  
  return 0;
}
