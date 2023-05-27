#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/proc_read/daemoncontrol.h>

#include <meteo/commons/grib/decoder/tgribdecode.h>
#include <meteo/commons/proto/tgribformat.pb.h>
#include <meteo/commons/primarydb/gribmongo.h>

#include <meteo/commons/services/decoders/decservice.h>
#include <meteo/commons/services/decoders/tservicestat.h>
#include <meteo/commons/services/state/rpctimer.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

#include <qtextcodec.h>

#include <unistd.h>



typedef TMeteoService<TDecodeGrib> TGribService;

template <> TGribService::TMeteoService(const meteo::ServiceOpt& opt):
  DecodersServiceThread(opt)
{
  clear();
}

template <> uint32_t TGribService::save()
{
  meteo::grib::dbmongo()->setPtkppId(sourceId());
  //  bool isRepeated = false;
  //  /*uint32_t res =*/ grib::db()->save(TDecodeGrib::grib(), &isRepeated);
  bool ok = meteo::grib::dbmongo()->save(TDecodeGrib::grib());

  if (!ok) {
    error_log << QObject::tr("Ошибка сохранения данных GRIB idPtkpp=%1\n").arg(sourceId());
    return -1;
  }

  //if (0 != res) {
    //setDecoded();
    //return 0;
  //}

  return 0;
}

template <> void TGribService::fillStatus(meteo::app::OperationStatus* status) const
{
  meteo::grib::dbmongo()->setConnectionStatus(status);
  TDecodeGrib::status().fillStatus(status);
  meteo::grib::dbmongo()->status().fillStatus(status);
}

namespace meteo {
namespace grib {

//-----

void printHelp(const QString& progName)
{
  info_log << QObject::tr("\nИспользование: %1 [option] \n").arg(progName)
     << QObject::tr("Опции:\n")
     << QObject::tr("-d\t\t Запустить как демон\n")
     << QObject::tr("-h\t\t Справка\n");
}

int appMain(int argc, char** argv)
{
  TAPPLICATION_NAME("meteo");
  meteo::global::setLogHandler();
  ::meteo::gSettings(meteo::global::Settings::instance());
  QCoreApplication app(argc, argv);

  if ( false == meteo::global::Settings::instance()->load() ) {
    error_log << QObject::tr("Не удалось загрузить настройки.");
    return EXIT_FAILURE;
  }

  bool isdaemon = false;
  int opt;

   meteo::ServiceOpt optServ;

  while ((opt = getopt(argc, argv, "hd")) != -1) {
    switch (opt) {
    case 'd':
      isdaemon = true;
      break;
    case 'h':
      printHelp(argv[0]);
      exit(0);
      break;
    default: /* '?' */
      exit(-1);
    }
  }



  if (isdaemon) {
    if ( 0 != ProcControl::daemonizeSock("sbor.grib")) {
      error_log << QObject::tr("Такой процесс уже запущен");
      return -1;
    }
    info_log << QObject::tr("Демон %1 запущен").arg(argv[0]);
  } else {
    if (0 != ProcControl::daemonizeSock("sbor.grib", false)) {
      error_log << QObject::tr("Такой процесс уже запущен");
      return -1;
    }
  }

  meteo::settings::proto::Service srv = meteo::global::service( meteo::settings::proto::kDiagnostic );
  meteo::state::RpcTimer stateMng( QString::fromStdString( srv.name() ) );

  optServ.stype = meteo::settings::proto::kGrib;
  TGribService* service = new TGribService(optServ);
  service->setStatusControl(&stateMng);
  service->start();

  int res = app.exec();

  if (service) delete service;
  return res;
}

}
}
