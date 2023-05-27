#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/grib/decoder/tgribdecode.h>
#include <meteo/commons/proto/tgribformat.pb.h>
#include <meteo/commons/services/decoders/decservice.h>
#include <meteo/commons/primarydb/gribmongo.h>
#include <meteo/commons/proto/surface.pb.h>
#include <meteo/commons/grib/iface/tgribiface.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

#include <qcoreapplication.h>
#include <qdir.h>

#include <unistd.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

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
      
    //   return _sprinfAddr;
    // }

  private:

    //    rpc::Address _sprinfAddr;
  };

}


typedef TMeteoService<TDecodeGrib, test::FileParser> TGribService;

template <> TGribService::TMeteoService()
{
  //  addDirectory(el.text().replace("$PRJ_DIR", MnCommon::projectPath()));
  // addDirectory(QString(getenv("HOME")) + "/datalink");
  // setExts(QStringList("tlg"));
  // setMark(true);
  // setUnk(false);
}


template <> uint32_t TGribService::save() 
{
  meteo::grib::dbmongo()->setPtkppId("-1");
  //  bool ok = meteo::grib::dbmongo()->save(TDecodeGrib::grib(), "testgrib");
  bool ok = meteo::grib::dbmongo()->save(TDecodeGrib::grib());
  debug_log << QObject::tr("Сохранение в grib") << ok;
    
  return 0;
}

void readFile(const QString& fileName)
{
  QByteArray ba;
  
  QFile file(fileName);
  file.open(QIODevice::ReadOnly);
  ba = file.readAll();  
  file.close();

  //var(ba.size());
  QByteArray uba = ba; //qUncompress(ba);
  //var(uba.size());
  
  std::string str(uba.data(), uba.size());
  ::grib::TGribData grib;
  grib.ParseFromString(str);

  //var(grib.DebugString());
  
  {
    TGribIface iface(&grib);
    if (grib.has_product1()) {
      debug_log << iface.isBitmap() << "ver = 1 number" << grib.product1().number();
    } else {
      debug_log << iface.isBitmap() << "ver = 2 discipline" << grib.discipline()
		<< "category" << grib.product2().category() << "number" << grib.product2().number();
    }
    meteo::surf::GribData gdata;
    iface.fillData(gdata.mutable_value());
    double min = DBL_MAX;
    double max = -DBL_MAX;
    for (int64_t vi = 0; vi < gdata.value_size(); vi++) {
      //debug_log << min << max << gdata.value(vi);
      if (gdata.value(vi) > max) max = gdata.value(vi);
      if (gdata.value(vi) < min) min = gdata.value(vi);
    }
    if (fabs(min) < 0.00001 || max > 99999999) {
      debug_log << "min=" << min << "max=" << max;
    }
  }
}

//----

void help(const QString& progName)
{
  info_log << QObject::tr("\nИспользование:") << progName << "-1 <file> | -2 <file> \n"
    "-1 <file> - раскодировать file сохранить в БД и в ФС path\n"
    "-2 <file> - прочитать из сохраненного файла file\n";
}


int main (int argc, char* argv[])
{
  TAPPLICATION_NAME("meteo");
  //  ::meteo::gGlobalObj(new ::meteo::SborGlobal);
  
  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  QCoreApplication app(argc, argv);

  QString file;
  int opt;


  int flag = 0;
  while ((opt = getopt(argc, argv, "1:2:hr:")) != -1) {
    switch (opt) {
    case '1':
      flag = 1;
      file = optarg;
      break;
    case '2':
      flag = 2;
      file = optarg;
      break;      
    case 'h':
      help(argv[0]);
      exit(0);
      break;
    default: /* '?' */
      error_log << "Option" << opt << "not realized";
      exit(-1);
    }
  }

  if (flag == 0) {
      error_log << "Need point option\n";
      help(argv[0]);
      exit(-1);
  }

  if (flag == 2) {
    readFile(file);
    return 0;
  }
  

  ::meteo::gSettings(meteo::global::Settings::instance());

  if ( false == meteo::global::Settings::instance()->load() ) {
    error_log << QObject::tr("Не удалось загрузить настройки.");
    return EXIT_FAILURE;
  }
  
  TGribService* service = new TGribService;

  service->start(file, "", "", "");
 
  
  return 0; //app.exec();
}
