#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/proc_read/daemoncontrol.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/primarydb/ancdb.h>
#include <meteo/commons/bufr/tbufr.h>
#include <meteo/commons/control/tmeteocontrol.h>
#include <meteo/commons/services/decoders/decservice.h>
#include <meteo/commons/services/decoders/tservicestat.h>
#include <meteo/commons/proto/sigwx.pb.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/services/state/rpctimer.h>

#include <qcoreapplication.h>
#include <qtextcodec.h>

#include <unistd.h>

typedef TMeteoService<TBufr> TBufrService;

template <> TBufrService::TMeteoService(const meteo::ServiceOpt& opt)
  : DecodersServiceThread(opt)
{
  clear();
}

template <> uint32_t TBufrService::save()
{
  QList<TMeteoData> data;
  meteo::bufr::TransfType bt;
  fill(&data, &bt);

  int seqNumber = TBufr::seqNumber();
  clearBM();

  bool oneOk = false;
  for (int i = 0; i < data.size(); i++) {
    data[i].set(TMeteoDescriptor::instance()->descriptor("ii_tlg"),
    QString::number(_tlgIdx),
    _tlgIdx,
    control::RIGHT);

    int categ = data.at(i).getValue(TMeteoDescriptor::instance()->descriptor("category"), -1, false);
    if (categ == -1) continue;

    QDateTime dt = headerDt();
    if (dt.isValid() &&
  !(categ >= meteo::surf::kOceanType && categ <= meteo::surf::kEndOceanType) &&
  categ != meteo::surf::kSatAero) {
      //---
      QDateTime mdDt = TMeteoDescriptor::instance()->dateTime(data.at(i));
      if (fabs(dt.secsTo(mdDt)) > 59*60) {
  //debug_log << "tlg dt =" << dt.toString("hh:mm") << "data dt =" << mdDt.toString("hh:mm") << "categ=" << categ << sourceId();
      }
      //---
      data[i].setDateTime(dt);
    } else if (categ == meteo::surf::kSatAero) {
      dt = TMeteoDescriptor::instance()->dateTime(data.at(i));
      dt.setTime(QTime(dt.time().hour(), 0, 0)); //NOTE пока так, чтоб не делать диапазон времени при выборке из БД
      data[i].setDateTime(dt);
    } else {
      dt = TMeteoDescriptor::instance()->dateTime(data.at(i));
    }
    if (!dt.isValid()) continue;

    //сохранение
    meteo::ancdb()->setPtkppId(sourceId());
    meteo::ancdb()->setMeteoSource(QString("Bufr ") + QString::number(seqNumber), meteo::AncDb::kBufrSource);
    meteo::ancdb()->setTlgii(_tlgIdx);
    meteo::ancdb()->setCorNumber( corNumber() );

    bool ok = true;

    if (categ >= meteo::surf::kRadarMapType && categ <= meteo::surf::kEndRadarMapType) {
      int min = ((dt.time().minute() + 5) / 10) * 10;
      dt.setTime(QTime(dt.time().hour(), min));
      ok = meteo::ancdb()->saveRadarMap(&data.at(i), categ, dt, "radarmap");
    }
    else if (categ == meteo::surf::kFlightSigWx) {
      QList<QByteArray> ba;
      QList<meteo::sigwx::Header> header;
      ok = meteo::decoders::toSigwxStructs(data.at(i), &ba, &header);
      if (ok) {
        for (int idx = 0; idx < header.size(); idx++) {
    ok = meteo::ancdb()->saveSigWx(header.at(idx), ba.at(idx), categ, dt, "flight_wx");
        }
      }
    }
    else {
      meteo::StationInfo info;
      meteo::ancdb()->fillStationInfo(&(data[i]), categ, &info);

      uint mask = control::LIMIT_CTRL | control::CONSISTENCY_CTRL;
      if (!TMeteoControl::instance()->control(mask, &(data[i]))) {
  debug_log << QObject::tr("Ошибка контроля");
      }

      // TMeteoDescriptor::instance()->modifyForSave(&(data[i]));

      //ok &= db->saveMeteoComplex(data[i], categ, dt, stationType, "meteodata");
      ok &= meteo::ancdb()->saveReport(data[i], categ, dt, info, "meteoreport");
      //ok &= db->saveMeteoData(data[i], categ, dt, "bufr");
    }

    if (!ok) {
      debug_log << QObject::tr("Ошибка сохранения первичной ГМИ idPtkpp=%1\n").arg(_idPtkpp);
    }
    else {
      oneOk = true;
    }
  }
  return oneOk ? 0 : -1;
}

template <> void TBufrService::fillStatus(meteo::app::OperationStatus* status) const
{
  meteo::ancdb()->setConnectionStatus(status);
  TBufr::status().fillStatus(status);
  meteo::ancdb()->status().fillStatus(meteo::AncDb::kBufrSource, status);
}

namespace meteo {
namespace bufr {

void printHelp(const QString& progName)
{
  info_log
      << QObject::tr("\nИспользование: %1 [option]\n").arg(progName)
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
    if ( 0 != ProcControl::daemonizeSock("sbor.bufr")) {
      error_log << QObject::tr("Такой процесс уже запущен");
      return -1;
    }

    info_log << QObject::tr("Демон %1 запущен").arg(argv[0]);
  } else {
    if (0 != ProcControl::daemonizeSock("sbor.bufr", false)) {
      error_log << QObject::tr("Такой процесс уже запущен");
      return -1;
    }
  }

  optServ.stype = meteo::settings::proto::kBufr;
  TBufrService* service = new TBufrService(optServ);
  if (!codec.isEmpty()) {
    if (! service->setCodec(codec)) {
      exit(-1);
    }
  }

  setlocale(LC_NUMERIC, "C"); // локаль для записи вещественных чисел в mongo (libbson)
  meteo::settings::proto::Service srv = meteo::global::service( meteo::settings::proto::kDiagnostic );
  meteo::state::RpcTimer stateMng( QString::fromStdString( srv.name() ) );
  service->setStatusControl(&stateMng);
  service->start();

  try {
    app.exec();
  }
  catch (const std::bad_alloc &) {
    critical_log << QObject::tr("Недостаточно памяти для функционирования приложения");
    return EXIT_FAILURE; // exit the application
  }

  if (service) delete service;
  return 0;
}


}
}
