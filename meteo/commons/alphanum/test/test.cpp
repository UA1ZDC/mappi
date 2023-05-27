#include <qcoreapplication.h>
#include <qdir.h>
#include <qdatastream.h>
#include <QJsonDocument>


#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/complexmeteo.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include "../stationmsg.h"
#include "../wmomsg.h"
#include "../metadata.h"
#include "../gphdecoder.h"
#include "../cliwaremsg.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/proto/alphanum.pb.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/primarydb/ancdb.h>

#include <unistd.h>

#define _DEBUG

const QString MSG_TEST_READ_CATALOG = "msgtestread/";
const QString khost = "localhost";

bool decode(const QByteArray& ba, const QString&)
{
  QStringList forms;
  forms << "FM15" << "FM16" << "FM20_B" << "FM35"
  << "POLU" << "MAWS" << "AMI2" << "PLV3"
  << "KAPL" << "AMC2_1" << "AMC2AV6" << "AMC2W"
  << "VPTK" << "PIRM" << "MARLA" << "AMIS_bil"
  << "BCM"  << "IPV";

 QList<int> types;
 types << meteo::anc::kWmoFormat << meteo::anc::kTableFormat;

 meteo::anc::MetaData::instance()->load(forms, types);

  QMap<QString, QString> rt;
  meteo::anc::StationMsg sm;
  sm.setCodec(QTextCodec::codecForName("Windows-1251"));
  bool ok = sm.decode(rt, ba);
  var(ok);
  const TMeteoData* data = sm.data();
  if (0 != data) {
    data->printData();
    QDateTime dt = TMeteoDescriptor::instance()->dateTime(*data);
    var(dt);
  }


//   debug_log << "end of decoding" << QTime::currentTime().toString("hh:mm:ss.zzz");

  return true;
}

int decode(const QByteArray& ba)
{
  //  debug_log << "loading:" << QTime::currentTime().toString("hh:mm:ss.zzz");
  QStringList forms;
  forms << "FM12" << "FM13" << "FM14" << "WAREP" << "WAREP_P"
  << "FM15" << "FM16" << "METREPORT" << "SPECIAL"
  << "FM20_A" << "FM20_B"
  << "FM32" << "FM33" << "FM34"
  << "FM35" << "FM36" << "FM37" << "FM38"
  << "FM42"
  << "FM51"
  << "SIGMET" << "GAMET"
    //<< "AIRMET"
  << "AIREP"
  << "ADWRNG" << "WSWRNG"
  << "VAA"
  << "FM62" << "FM71"
  << "KN-02" << "KN-05" << "KN-06"
  << "KN-15" << "FM67" << "KN-19" << "KN-24" << "SLOY"
  << "KN-21"
  << "METEO11" << "METEO44"
  << "IONFO" << "FM18" << "FM64"
  << "AVGM-STORMW" << "AVGM-STORMN" << "AVGM-FORECAST"
  << "GDA1" << "FI4" << "IKO1" << "DOL2" << "MKLM"
  << "GMC"
  << "AFAP"  << "UMAGF" << "TEN" << "TENMN" << "MAGKS" << "SATIN" << "URANJ"
  << "OPPCM" << "UABSE" << "UGEOA" << "UGEOE" << "UGEOI" << "UGEOR"
  << "USIDS" << "USSPS" << "USSPI" << "UFLAE"
  << "IONKA"
  << "KMI-FP" << "KMI-PP" << "KMI-SHP"
  << "RECCO" << "FM65";

  QList<int> types;
  types << meteo::anc::kWmoFormat << meteo::anc::kIonfoTlg;

  meteo::anc::MetaData::instance()->load(forms, types);

  // meteo::anc::MetaData::instance()->load(MnCommon::etcPath("meteo") +"alphanum.conf");


  QMap<QString, QString> rt;
  meteo::anc::WmoMsg wm;
  wm.setCodec(QTextCodec::codecForName("Windows-1251"));
  QDateTime dt;// = QDateTime::currentDateTime();
  int cnt = wm.parse(rt, ba, dt);
  int parsedCnt = 0;
  int idx =0;
  while(wm.hasNext()) {
    var(idx++);
    bool ok = wm.decodeNext();
    if (ok) {
      TMeteoData* data = wm.data();
      //TMeteoDescriptor::instance()->modifyForSave(data);
      if (0 != data) {
	++parsedCnt;
	
	// QString str = data->toString();
	// var(str);
	// debug_log << "--------\n";
	data->printData();
	
	ComplexMeteo cm;
	cm.fill(*data);
	debug_log << "complex";
	cm.printData();
	
	QList<TMeteoData*> fr;
	fr = wm.split();
	
	var(fr.size());
	for (int i = 0; i < fr.size(); i++) {
	  debug_log << "Fragment" << i;
	  fr.at(i)->printData();
	  ComplexMeteo cm;
	  cm.fill(*(fr.at(i)));
	  debug_log << "complex";
	  cm.printData();
	  delete fr[i];
	}
	
	var(TMeteoDescriptor::instance()->dateTime(*data));
      }
    }
  }

  // debug_log << "end of decoding" << QTime::currentTime().toString("hh:mm:ss.zzz");
  debug_log << "count=" << cnt << "parsed" << parsedCnt;
  return 0;
}

//ВМО формат
int testWmoDecode(const QString& fileName)
{
  QByteArray ba;
  QFile file(fileName);
  if(!file.open(QIODevice::ReadOnly)) {
    error_log<<"Can't open file"<<fileName;
    return -1;
  }
  ba = file.readAll();
  file.close();

  int res = decode(ba);

  debug_log<<"file"<<fileName;
  return res;
}

void decodeGph(const QByteArray& ba)
{
  QStringList forms;
  forms << "IONEX";

  QList<int> types;
  types << meteo::anc::kGphFormat;

  meteo::anc::MetaData::instance()->load(forms, types);
  meteo::anc::GphDecoder gm(QTextCodec::codecForLocale());

  QMap<QString, QString> type;
  int cnt = gm.parse(type, ba, QDateTime());
  var(cnt);

  const TMeteoData* data = gm.header();
  if (0 != data) {
    data->printData();
  }
  QList<int> keys = gm.mapKeys();
  for (int idx = 0; idx < keys.size(); idx++) {
    debug_log << "номер карты" << keys.at(idx);
    const QDateTime* epoch = gm.epoch(keys.at(idx));
    if (0 != epoch) {
      debug_log << "эпоха" << epoch->toString(Qt::ISODate);
    } else {
      error_log << "no epoch" << keys.at(idx);
    }

    const QByteArray* maps = gm.data(keys.at(idx));
    if (0 != maps) {
      QDataStream ds(*maps);
      uint8_t cnt;
      ds >> cnt;
      debug_log << "карт" << cnt;

      for (int ti = 0; ti < cnt; ti++) {
  int type;
  ds >> type;
  int size;
  ds >> size;
  debug_log << "тип " << type << "точек" << size;

  QList<float> mapdata;
  float val;
  for (int idx = 0; idx < size; idx++) {
    ds >> val;
    mapdata << val;
  }
  var(mapdata.size());
      }
      debug_log;
    } else {
      error_log << "no maps" << keys.at(idx);
    }
  }

}

//геофизика
int testGeophys(const QString& fileName)
{
  QByteArray ba;
  QFile file(fileName);
  if(!file.open(QIODevice::ReadOnly)) {
    error_log<<"Can't open file"<<fileName;
    return -1;
  }
  ba = file.readAll();
  file.close();

  debug_log << "start of decoding" << QTime::currentTime().toString("hh:mm:ss.zzz");
  decodeGph(ba);

  debug_log << "end of decoding" << QTime::currentTime().toString("hh:mm:ss.zzz");
  debug_log<<"file"<<fileName;
  return 0;
}

int testBaikonur(const QString& type, const QString& fileName)
{
  QByteArray ba;
  ba += "999\n\n";
  ba += QDateTime::currentDateTimeUtc().toString(Qt::ISODate) + " 6 " +
    type + " 555555 " + '\n';

  QFile file(fileName);
  if(!file.open(QIODevice::ReadOnly)) {
    error_log<<"Can't open file"<<fileName;
    return -1;
  }
  ba += file.readAll();
  file.close();

  int res = decode(ba,type);

  debug_log<<"file"<<fileName;
  return res;
}

int testBaikonur(QString& fileName)
{
  QByteArray ba;

  QFile file(fileName);
  if(!file.open(QIODevice::ReadOnly)) {
    error_log<<"Can't open file"<<fileName;
    return -1;
  }
  ba += file.readAll();
  file.close();

  int res = decode(ba, "");

  debug_log<<"file"<<fileName;
  return res;
}


void testCliware(QString& fileName)
{
  QByteArray ba;

  QFile file(fileName);
  if(!file.open(QIODevice::ReadOnly)) {
    error_log<<"Can't open file"<<fileName;
    return;
  }
  ba += file.readAll();
  file.close();

  QList<int> types;
  types << meteo::anc::kCliwareFormat;
  QMap<QString, QString> rt;
  
  meteo::anc::CliwareMsg cm;
  int cnt = cm.parse(rt, QJsonDocument::fromJson(ba));
  int parsedCnt = 0;
  int idx =0;
  while(cm.hasNext()) {
    var(idx++);
    bool ok = cm.decodeNext();
    if (ok) {
      TMeteoData* data = cm.data();
      if (0 != data) {
	++parsedCnt;
	data->printData();
      }
    }
  }
  
  debug_log << "count=" << cnt << "parsed" << parsedCnt;

  debug_log<<"file"<<fileName;
}

int main (int argc, char* argv[])
{
  TAPPLICATION_NAME("meteo");
  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );

  int flag = 0, opt;
  QString type;
  bool setdbmeta = false;

  while ((opt = getopt(argc, argv, "12:3h45m")) != -1) {
    switch (opt) {
    case '1':
      flag = 1;
      break;
    case '2':
      flag = 2;
      type = optarg;
      var(type);
      break;
    case '3':
      flag = 3;
      break;
    case '4':
      flag = 4;
      break;
    case '5':
      flag = 5;
      break;
    case 'm':
      setdbmeta = true;
      break;
    case 'h':
      info_log<<"Usage:"<<argv[0]<<"-1|-2|-3|-4 <type> <fileName>\n"
	"-1 <fileName> - FM\n"
	"-2 <type> - Baikonur whithout caption\n"
	"-3 <fileName> - Baikonur whit caption\n"
	"-4 <fileName> - geophysic\n"
	"-5 <fileName> - cliware\n"
	"-m - загружать данные из БД";
      exit(0);
      break;
    default: /* '?' */
      error_log<<"Option"<<opt<<"not realized";
      exit(-1);
    }
  }

  if (flag == 0) {
    error_log<<"Need select test with option. \""<<argv[0]<<"-h\" for help";
    exit(-1);
  }

  QString fileName;
  if (flag >= 1) {
    if (optind >= argc) {
      error_log<<"Need point file name. \""<<argv[0]<<"-h\" for help";
      exit(-1);
    } else {
      fileName = argv[optind];
    }
  }

  QCoreApplication app(argc, argv);


  //  meteo::AncDb* db = nullptr;
  //для fillTypes нужны sbor::Settings
  if (setdbmeta) {
    // db = new meteo::AncDb("meteodb", khost, 27017);
    // meteo::anc::MetaData::instance()->setDbMeta(db->dbmeta());
    ::meteo::gSettings(meteo::global::Settings::instance());
    if ( false == meteo::global::Settings::instance()->load() ) {
      error_log << QObject::tr("Не удалось загрузить настройки.");
      return EXIT_FAILURE;
    }
    meteo::anc::MetaData::instance()->setDbMeta(meteo::ancdb()->dbmeta());
  }

  switch (flag) {
  case 1:
    testWmoDecode(fileName);
    break;
  case 2:
    testBaikonur(type, fileName);
    break;
  case 3:
    testBaikonur(fileName);
    break;
  case 4:
    testGeophys(fileName);
    break;
  case 5:
    testCliware(fileName);
    break;
  default: {}
  }


  //  delete db;

  return 0;
}
