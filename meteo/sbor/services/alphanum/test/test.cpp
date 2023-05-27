#include <commons/meteo_data/meteo_data.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/primarydb/ancdb.h>
#include <meteo/commons/alphanum/talphanum.h>
#include <meteo/commons/alphanum/metadata.h>
#include <meteo/commons/control/tmeteocontrol.h>
#include <meteo/commons/services/decoders/decservice.h>
#include <meteo/commons/services/decoders/tservicestat.h>

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
      _byLine = false;
    }
    virtual ~FileParser() {}

    virtual ulong processMsg(const meteo::tlg::MessageNew& tlg) = 0;

    //читать файл построчно (true) или целиком (false)
    void setReadByLine(bool line) {
      _byLine = line;
    }

    void start(QString fileName, QString type, QString index, QString name, bool withCption = true) {
      info_log << QObject::tr("Запуск обработки телеграмм");
      QByteArray ba;
      QByteArray caption;
      if (withCption) {
  caption += QDateTime::currentDateTimeUtc().toString(Qt::ISODate) + " 0 " + index +
    "  " + " " + name + " " + type + '\n';
      }

      QFile file(fileName);
      if(!file.open(QIODevice::ReadOnly)) {
  error_log<<"Can't open file"<<fileName;
  return;
      }
      ba += file.readAll();
      file.close();

      if (! _byLine) {
  sendAll(caption + ba, fileName);
      } else {
  sendByLine(caption, ba, fileName);
      }
    }

    //отправка данных
    void sendAll(const QByteArray& ba, QString /*fileName*/) {
      meteo::tlg::MessageNew tlg;
      tlg.set_msg(ba.data(), ba.size());
      //  tlg.set_size(ba.size());

      tlg.set_format(meteo::tlg::kGMS);
      tlg.mutable_header()->set_t1("o");
      tlg.mutable_header()->set_t2("b");
      tlg.mutable_header()->set_a1("m");
      tlg.mutable_header()->set_a2("n");
      tlg.mutable_header()->set_ii(91);

      tlg.mutable_header()->set_ii(99);
      tlg.mutable_metainfo()->set_id(0);
      //tlg.mutable_metainfo()->set_converted_dt("2018-01-01T12:12");
      //tlg.set_error_string(fileName.toStdString());

      processMsg(tlg);
    }

    //отправка построчно
    void sendByLine(const QByteArray& caption, const QByteArray& body, const QString& fileName) {
      QByteArray ba;
      int pos = 0;
      int prev = 0;

      while( -1 != pos ) {
  pos = body.indexOf("X", pos + 1);
  if (pos != -1) {
    ba = body.mid(prev, pos - prev);
  } else if (prev < body.size() - 1) {
    ba = body.mid(prev, ba.size() - 1 - prev);
  }
  if (!ba.isEmpty()) {
    sendAll(caption + ba, fileName);
  }
  prev = pos + 1;
      }
    }

    // const rpc::Address& sprinfAddress()
    // {
    //   bool ok;
    //   _sprinfAddr = meteo::global::serviceAddress( meteo::settings::proto::kSprinf, &ok );
    //   if (_sprinfAddr.type() == rpc::Address::UNKNOWN) {
    // 	error_log << QObject::tr("Сервис доступа к справочной информации не найден");
    //   }

    //   return _sprinfAddr;
    // }

  private:
    bool _byLine;
    //rpc::Address _sprinfAddr;
  };

}


typedef TMeteoService<WmoAlphanum, test::FileParser> TAlphaNumService;


template <> TAlphaNumService::TMeteoService()
{
}

template <> uint32_t TAlphaNumService::save()
{
  trc;
  TMeteoData* data = WmoAlphanum::data();
  if (!data) return 0;

  data->set(TMeteoDescriptor::instance()->descriptor("ii_tlg"),
      QString::number(_tlgIdx),
      _tlgIdx,
      control::RIGHT);

  int categ = data->getValue(TMeteoDescriptor::instance()->descriptor("category"), -1);

  meteo::StationInfo info;
  bool okCoord = meteo::ancdb()->fillStationInfo(data, categ, &info);
  var(okCoord);

  uint mask = control::LIMIT_CTRL | control::CONSISTENCY_CTRL;
  if (!TMeteoControl::instance()->control(mask, data)) {
    error_log << QObject::tr("Ошибка контроля");
    return 1;
  }
  //data->printData();

  QDateTime dt = WmoAlphanum::dateTime();
  var(dt);
  // dt.setDate(QDate(2018,9,27));
  // dt.setTime(QTime(15,0));

  meteo::ancdb()->setPtkppId(sourceId());
  meteo::ancdb()->setMeteoSource(WmoAlphanum::code(), meteo::AncDb::kANCSource);
  meteo::ancdb()->setTlgii(_tlgIdx);

  debug_log << "save";

  bool ok = true;
  if (categ >= meteo::surf::kRadarMapType && categ <= meteo::surf::kEndRadarMapType) {
    ok = meteo::ancdb()->saveRadarMap(data, categ, dt, "test_radarmap");
  } else {


    
//    TMeteoDescriptor::instance()->modifyForSave(data);
//    debug_log << "DATA =" << data->toString();
    ok &= meteo::ancdb()->saveReport(*data, categ, dt, info, "test_meteoreport");
  }
  if (!ok) {
    error_log << QObject::tr("Ошибка сохранения первичной ГМИ idPtkpp=%1\n").arg(_idPtkpp);
  } else {
    debug_log<<"Save ok";
  }

  return 0;
}


//----

void help(const QString& progName)
{
  info_log << QObject::tr("\nИспользование:") << progName << "-f <file>\n"
    "-f <file> Файл со сводкой\n";
}

int main (int argc, char* argv[])
{
  TAPPLICATION_NAME("meteo");
  //  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );

  //  ::meteo::gGlobalObj(new ::meteo::SborGlobal);
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

  if (file.isEmpty()) {
    error_log << "Необходимо указать файл, содержащий сводку (<file>)\n";
    help(argv[0]);
    exit(-1);
  }

  ::meteo::gSettings(::meteo::global::Settings::instance());
  meteo::global::Settings::instance()->load();

  QList<int> types;
  setlocale(LC_NUMERIC, "C"); // локаль для записи координат в mongo (libbson)

  types << meteo::anc::kWmoFormat << meteo::anc::kIonfoTlg;
  meteo::anc::MetaData::instance()->load(MnCommon::etcPath() + "alphanum.conf", types);
  //const ConnectProp conf = meteo::Global::instance()->mongodbConfMeteo();
  //meteo::anc::MetaData::instance()->setConnectProp(conf);
  meteo::anc::MetaData::instance()->setDbMeta(meteo::ancdb()->dbmeta());
  //db->setSaveDubl(false);

  TAlphaNumService* service = new TAlphaNumService;
  service->setCodec("Windows-1251");
  service->start(file, "", "", "");

  delete service;
  return 0;// app.exec();
}
