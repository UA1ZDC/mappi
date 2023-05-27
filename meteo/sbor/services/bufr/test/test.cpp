#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/primarydb/ancdb.h>
#include <meteo/commons/bufr/tbufr.h>
#include <meteo/commons/services/decoders/decservice.h>
#include <meteo/commons/services/decoders/tservicestat.h>
#include <meteo/commons/control/tmeteocontrol.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/services/decoders/meteo_convert.h>
#include <meteo/commons/proto/sigwx.pb.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

//#include <meteo/sbor/settings/settings.h>

#include <qcoreapplication.h>
#include <qdir.h>

#include <unistd.h>
#include <stdlib.h>

/*!
  \file  test.cpp
  \brief Проверка работы раскодирования
*/

namespace test {
  //Чтение файла и отправка раскодировщику
  class FileParser {
  public:
    FileParser() {
    }
    virtual ~FileParser() {}

    virtual ulong processMsg(const meteo::tlg::MessageNew& tlg) = 0;

    void start(QString fileName, QString type, QString index, QString name) {
      QByteArray ba;
      QByteArray caption;
      caption += QDateTime::currentDateTimeUtc().toString(Qt::ISODate) + " 0 " + index +
  "  " + " " + name + " " + type + '\n';

      QFile file(fileName);
      if(!file.open(QIODevice::ReadOnly)) {
  error_log<<"Can't open file"<<fileName;
  return;
      }
      ba += file.readAll();
      file.close();

      sendAll(caption + ba, fileName);

    }

    //отправка данных
    void sendAll(const QByteArray& ba, QString /*fileName*/) {
      meteo::tlg::MessageNew tlg;
      tlg.set_msg(ba.data(), ba.size());
      tlg.mutable_header()->set_ii(99);
      tlg.mutable_metainfo()->set_id(0);
      // tlg.set_error_string(fileName.toStdString());
      tlg.mutable_metainfo()->set_converted_dt("2021-02-24T06:00");
      QTime ttt; ttt.start();
      processMsg(tlg);
      var(ttt.elapsed());
    }

    // const rpc::Address& sprinfAddress()
    // {
    //   _sprinfAddr = meteo::global::serviceAddress( meteo::settings::proto::kSprinf );
    //   if (_sprinfAddr.type() == rpc::Address::UNKNOWN) {
    // 	error_log << QObject::tr("Сервис доступа к справочной информации не найден");
    //   }

    // 	return _sprinfAddr;
    // }

  private:

//rpc::Address _sprinfAddr;
  };

}

typedef TMeteoService<TBufr, test::FileParser> TBufrService;

template <> TBufrService::TMeteoService()
{
}


template <> uint32_t TBufrService::save()
{
  QTime ttt; ttt.start();
  QList<TMeteoData> data;
  meteo::bufr::TransfType bt;
  fill(&data, &bt);
  var(ttt.elapsed());

  int seqNumber = TBufr::seqNumber();

  var(data.size());
  QTime ttt1;
  ttt1.start();

  for (int i = 0; i < data.size(); i++) {
    data[i].set(TMeteoDescriptor::instance()->descriptor("ii_tlg"),
    QString::number(_tlgIdx),
    _tlgIdx,
    control::RIGHT);

    uint mask = control::LIMIT_CTRL | control::CONSISTENCY_CTRL;
    if (!TMeteoControl::instance()->control(mask, &(data[i]))) {
      error_log << QObject::tr("Ошибка контроля");
      return 1;
    }

    //data[i].printData();
    
    int categ = data.at(i).getValue(TMeteoDescriptor::instance()->descriptor("category"), -1);
    if (categ == -1) continue;

    QDateTime dt = headerDt(); //TMeteoDescriptor::instance()->dateTime(data.at(i));
    if (!dt.isValid()) {
      dt = QDateTime::currentDateTime();
      data[i].setDateTime(QDateTime::currentDateTime());
      // continue;
    }
    dt = QDateTime(dt.date(), QTime(dt.time().hour(), 0, 0), Qt::UTC);

    if (categ == meteo::surf::kSatAero) {
      data[i].setDateTime(dt);
    }

    //  dt = QDateTime(QDate(2021, 02,24), QTime(6,0));
    var(dt);


    //сохранение
    meteo::ancdb()->setPtkppId(sourceId());
    meteo::ancdb()->setMeteoSource(QString("Bufr ") + QString::number(seqNumber), meteo::AncDb::kBufrSource);
    meteo::ancdb()->setTlgii(_tlgIdx);


    bool ok = true;
    //    if (bt == meteo::bufr::kRadarMap) {
    if (categ >= meteo::surf::kRadarMapType && categ <= meteo::surf::kEndRadarMapType) {
      ok = meteo::ancdb()->saveRadarMap(&data.at(i), categ, dt, "test_radarmap");
    } else if (categ == meteo::surf::kFlightSigWx) {
      QList<QByteArray> ba;
      QList<meteo::sigwx::Header> header;
      ok = meteo::decoders::toSigwxStructs(data.at(i), &ba, &header);
      var(header.size());
      if (ok) {
  for (int idx = 0; idx < header.size(); idx++) {
    ok = meteo::ancdb()->saveSigWx(header.at(idx), ba.at(idx), categ, dt, "test_flight_wx");
  }
      }
    } else {
      meteo::StationInfo info;
      meteo::ancdb()->fillStationInfo(&(data[i]), categ, &info);
      //TMeteoDescriptor::instance()->modifyForSave(&(data[i]));
      ok &= meteo::ancdb()->saveReport(data[i], categ, dt, info, "test_meteoreport");
    }
    if (!ok) {
      error_log << QObject::tr("Ошибка сохранения первичной ГМИ idPtkpp=%1\n").arg(_idPtkpp);
    } else {
      debug_log<<"Save ok";
    }

  }
  var(ttt1.elapsed());

  return 0;
}


void readSigwx(const QString& fileName)
{
  QFile file(fileName);
  if(!file.open(QIODevice::ReadOnly)) {
    error_log<<"Can't open file"<<fileName;
    return;
  }

  QByteArray ba = file.readAll();
  file.close();
  std::string str(ba.data(), ba.size());

  meteo::sigwx::SigWx sig;

  sig.ParseFromString(str);
  var(sig.DebugString());
}

//----

void help(const QString& progName)
{
  info_log << QObject::tr("\nИспользование:") << progName << "-f <file>\n"
    "-f <file> Файл со сводкой\n"
    "-r <file> Прочитать сохраненный sigwx файл \n";
}


int main (int argc, char* argv[])
{
  TAPPLICATION_NAME("meteo");
  //  ::meteo::gGlobalObj(new ::meteo::SborGlobal);

  QCoreApplication app(argc, argv);

  QString file;
  int opt;
  int flag = 0;

  while ((opt = getopt(argc, argv, "f:hr:")) != -1) {
    switch (opt) {
    case 'f':
      file = optarg;
      flag = 0;
      break;
    case 'r':
      file = optarg;
      flag = 1;
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

  if (file.isEmpty()) {
    error_log << "Необходимо указать файл, содержащий сводку (<file>)\n";
    help(argv[0]);
    exit(-1);
  }

  if (flag == 1) {
    readSigwx(file);
  } else {

    setlocale(LC_NUMERIC, "C"); // локаль для записи вещественных чисел в mongo (libbson)

    ::meteo::gSettings(::meteo::global::Settings::instance());

    if ( !meteo::global::Settings::instance()->load() ) {
      error_log << QObject::tr("Ошибка при загрузке настроек.");
      return EXIT_FAILURE;
    }

    //db->setSaveDubl(false);

    TBufrService* service = new TBufrService;
    service->setCodec("Windows-1251");

    service->start(file, "", "", "");
    // app.exec();
    delete service;
  }

  return 0;
}
