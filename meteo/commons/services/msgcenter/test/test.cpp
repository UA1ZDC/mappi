#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <meteo/commons/proto/msgcenter.pb.h>
#include <meteo/commons/services/msgcenter/msgcenter.h>
#include <meteo/commons/settings/settings.h>

#include <qcoreapplication.h>
#include <qdir.h>

#include <unistd.h>
#include <stdlib.h>

using namespace meteo;

void test_getReport(const QString& station, const QString& name, int64_t tlg_id)
{
  meteo::msgcenter::GetReportRequest req;
  req.set_station(station.toStdString());
  req.add_tlg_id(tlg_id);
  req.set_station_name(name.toStdString());
  
  meteo::msgcenter::GetReportResponse res;

  meteo::MethodBag mb(nullptr);
  mb.GetReport(&req, &res);

  var(res.Utf8DebugString());

}

void test_getReports()
{
  test_getReport("UTSA", "name", 11745843);
  test_getReport("ЬХСБ", "Варфоломеевка", 11753596);
  test_getReport("76741", "name", 14181144);
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

  while ((opt = getopt(argc, argv, "h")) != -1) {
    switch (opt) {
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
  

  test_getReports();

  return 0;
}
