#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/proc_read/daemoncontrol.h>

#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/control/tmeteocontrol.h>
#include <meteo/commons/alphanum/talphanum.h>
#include <meteo/commons/primarydb/ancdb.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/services/state/rpctimer.h>

#include <qcoreapplication.h>
#include <qtextcodec.h>

#include <unistd.h>


#define APP_NAME "sbor.alphanum"

typedef TMeteoService<WmoAlphanum> AlphaNumService;

template <> AlphaNumService::TMeteoService(const meteo::ServiceOpt& opt):
  DecodersServiceThread(opt)
{
  clear();
}


template <> void AlphaNumService::fillStatus(meteo::app::OperationStatus* status) const
{
  meteo::ancdb()->setConnectionStatus(status);
  WmoAlphanum::status().fillStatus(status);
  meteo::ancdb()->status().fillStatus(meteo::AncDb::kANCSource, status);
}


template <> uint32_t AlphaNumService::save()
{
  TMeteoData* data = WmoAlphanum::data();
  if (!data) return 0;

  data->set(TMeteoDescriptor::instance()->descriptor("ii_tlg"),
      QString::number(_tlgIdx),
      _tlgIdx,
      control::RIGHT);

  QDateTime dt = WmoAlphanum::dateTime();
  int categ = data->getValue(TMeteoDescriptor::instance()->descriptor("category"), -1, false);

  meteo::StationInfo info;
  meteo::ancdb()->setPtkppId(sourceId());
  meteo::ancdb()->setMeteoSource(WmoAlphanum::code(), meteo::AncDb::kANCSource);
  meteo::ancdb()->setTlgii(_tlgIdx);
  meteo::ancdb()->setCorNumber( corNumber() );

  bool ok = true;
  if (categ >= meteo::surf::kRadarMapType && categ <= meteo::surf::kEndRadarMapType) {
    ok = meteo::ancdb()->saveRadarMap(data, categ, dt, "radarmap");
  } else {
    meteo::ancdb()->fillStationInfo(data, categ, &info);

    uint mask = control::LIMIT_CTRL | control::CONSISTENCY_CTRL;
    if (!TMeteoControl::instance()->control(mask, data)) {
      debug_log << QObject::tr("Ошибка контроля");
      return 1; //TODO код ошибки, видимо, должен возвращаться. не помню для кого
    }

    //TMeteoDescriptor::instance()->modifyForSave(data);

    ok &= meteo::ancdb()->saveReport(*data, categ, dt, info, "meteoreport");
    // var(ok);
  }

  if (!ok) {
    debug_log << QObject::tr("Ошибка сохранения первичной ГМИ idPtkpp=%1\n").arg(_idPtkpp);
    return -1;
  } else {
    //setDecoded();
    //  debug_log<<"Save ok";
  }

  return 0;
}

//-------

void printHelp(const QString& progName)
{
  info_log << QObject::tr("\nИспользование: %1 [option]\n").arg(progName)
     << QObject::tr("Опции:\n")
         << QObject::tr("-c <codec>\t Название кодека\n")
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
  QByteArray codec;

  while ((opt = getopt(argc, argv, "hdc:")) != -1) {
    switch (opt) {
    case 'd':
      isdaemon = true;
      break;
    case 'c':
      codec = optarg;
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
    if ( 0 != ProcControl::daemonizeSock(APP_NAME)) {
      error_log << QObject::tr("Такой процесс уже запущен");
      return -1;
    }
    info_log << QObject::tr("Демон %1 запущен").arg(argv[0]);
  } else {
    if (0 != ProcControl::daemonizeSock(APP_NAME, false)) {
      error_log << QObject::tr("Такой процесс уже запущен");
      return -1;
    }
  }

  QList<int> types;
  types << meteo::anc::kWmoFormat << meteo::anc::kIonfoTlg;
  meteo::anc::MetaData::instance()->load(MnCommon::etcPath() + "alphanum.conf", types);
  meteo::anc::MetaData::instance()->setDbMeta(meteo::ancdb()->dbmeta());

  meteo::settings::proto::Service srv = meteo::global::service( meteo::settings::proto::kDiagnostic );
  meteo::state::RpcTimer stateMng( QString::fromStdString( srv.name() ) );

  optServ.stype = meteo::settings::proto::kAlphanum;
  AlphaNumService* service = new AlphaNumService(optServ);
  if (!codec.isEmpty()) {
    if (! service->setCodec(codec)) {
      exit(-1);
    }
  }
  setlocale(LC_NUMERIC, "C"); // локаль для записи координат в mongo (libbson)
  service->setStatusControl(&stateMng);
  service->start();

  try {
    app.exec();
  }
  catch (const std::bad_alloc &) {
    critical_log << QObject::tr("Недостаточно памяти для функционирования приложения");
    return EXIT_FAILURE;
  }

  if (service) delete service;

  return 0;
}
