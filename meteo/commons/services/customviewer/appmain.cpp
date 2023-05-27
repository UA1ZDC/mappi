#include <qcoreapplication.h>
#include <cross-commons/app/paths.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/appstatusthread.h>

#include <meteo/commons/alphanum/talphanum.h>

#include <commons/proc_read/daemoncontrol.h>
#include <cross-commons/debug/tlog.h>

#include "customviewerservice.h"


int appMain(int argc, char** argv ){
  TAPPLICATION_NAME("meteo");
  ::setlocale(LC_NUMERIC, "C"); // локаль для записи координат в mongo (libbson)

  int result = EXIT_FAILURE;

  try {
    QCoreApplication app(argc, argv);
    meteo::gSettings(meteo::global::Settings::instance());

    if ( false == meteo::global::Settings::instance()->load() ){
      error_log << meteo::msglog::kSettingsLoadFailed;
      return result;
    }

    QString appName = app.applicationFilePath();
    if(ProcControl::isAlreadyRun(appName)) {
      error_log << meteo::msglog::kProcessAlreadyStarted
                   .arg(app.applicationName());
      return result;
    }

    auto service = new meteo::CustomViewerService();
    auto serviceCode = meteo::settings::proto::kCustomViewerService;
    auto serviceAddress = meteo::global::serviceAddress(serviceCode);
    meteo::rpc::Server server(service, serviceAddress);
    if ( false == server.init() ) {
      //auto serviceTitle = meteo::global::serviceTitle(serviceCode);
      error_log << meteo::msglog::kServerCreateFailed.arg("");
      return EXIT_FAILURE;
    }
    
    QList<int> types;
    types << meteo::anc::kWmoFormat << meteo::anc::kIonfoTlg;
    meteo::anc::MetaData::instance()->load(MnCommon::etcPath() + "alphanum.conf", types);

    result = app.exec();
  }
  catch(const std::bad_alloc& ex) {
    critical_log << meteo::msglog::kMemoryAllocFailed.arg(ex.what());
  }
  return result;

}
