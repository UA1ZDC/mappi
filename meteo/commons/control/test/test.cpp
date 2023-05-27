#include <qcoreapplication.h>
#include <qdir.h>

#include <commons/meteo_data/tmeteodescr.h>
#include <commons/meteo_data/meteo_data.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <meteo/commons/alphanum/stationmsg.h>
#include <meteo/commons/alphanum/wmomsg.h>
#include <meteo/commons/alphanum/metadata.h>
#include <meteo/commons/control/tmeteocontrol.h>
#include <meteo/commons/bufr/tbufr.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/primarydb/ancdb.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>


#include <commons/mathtools/mnmath.h>

#include <unistd.h>

void fillCoord(TMeteoData* data);

bool decode(const QByteArray& ba, const QString&)
{
  QList<int> types;
  types << meteo::anc::kWmoFormat << meteo::anc::kIonfoTlg;
  meteo::anc::MetaData::instance()->load(MnCommon::etcPath("meteo") +"alphanum.conf", types);

  QMap<QString, QString> rt;
  meteo::anc::StationMsg sm;
  bool ok = sm.decode(rt, ba);
  var(ok);
  TMeteoData* data = sm.data();
  ok = TMeteoControl::instance()->control(control::LIMIT_CTRL | control::CONSISTENCY_CTRL, data);

  if (0 != data) {
    data->printData();
  }

  return true;
}

int decodeWmo(const QByteArray& ba)
{
  //  debug_log << "loading:" << QTime::currentTime().toString("hh:mm:ss.zzz");
  QStringList forms;
  forms << "FM12" << "FM13" << "FM14" << "WAREP"
  << "FM15" << "FM16" << "METREPORT" << "SPECIAL"
  << "FM20_A" << "FM20_B"
  << "FM32" << "FM33" << "FM34"
  << "FM35" << "FM36" << "FM37" << "FM38"
  << "FM51"
  << "SIGMET" << "GAMET"
    //<< "AIRMET"
  << "AIREP"
  << "ADWRNG" << "WSWRNG"
  << "VAA"
  << "FM62" << "FM71"
  << "KN-02" << "KN-05" << "KN-06"
  << "KN-15" << "KN-19" << "KN-24" << "SLOY"
  << "KN-21"
  << "METEO11" << "METEO44"
  << "IONFO" << "FM18" << "FM64";

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
      if (0 != data) {
  ++parsedCnt;

  int categ = data->getValue(TMeteoDescriptor::instance()->descriptor("category"), -1, false);
  meteo::StationInfo info;
  bool okCoord = meteo::ancdb()->fillStationInfo(data, categ, &info);
  var(okCoord);

  var("control");
  TMeteoControl::instance()->control(control::LIMIT_CTRL | control::CONSISTENCY_CTRL, data);
  var(TMeteoDescriptor::instance()->dateTime(*data));
  data->printData();
      }
    }
  }

  // debug_log << "end of decoding" << QTime::currentTime().toString("hh:mm:ss.zzz");
  debug_log << "count=" << cnt << "parsed" << parsedCnt;
  return 0;
}

int decodeAndControl(const QString& type, const QString& fileName)
{
  QByteArray ba;

  ba += QDateTime::currentDateTimeUtc().toString(Qt::ISODate) + " 555555  " + " TEST " + type + '\n';

  QFile file(fileName);
  if(!file.open(QIODevice::ReadOnly)) {
    error_log<<"Can't open file"<<fileName;
    return -1;
  }
  ba += file.readAll();
  file.close();

  int res;
  if (!type.isEmpty()) {
    res = decode(ba,type);
  } else {
    res = decodeWmo(ba);
  }

  debug_log<<"file"<<fileName;
  return res;
}


int decodeAndControlBufr(const QString& fileName)
{
  QByteArray ba;

  QFile file(fileName);
  if(!file.open(QIODevice::ReadOnly)) {
    error_log<<"Can't open file"<<fileName;
    return -1;
  }
  ba += file.readAll();
  file.close();

  QMap<QString, QString> type;

  TBufr bufr;
  bufr.setCodec(QTextCodec::codecForName("Windows-1251"));
  int err_cnt =  bufr.decode(type, ba);
  var(err_cnt);

  QList<TMeteoData> data;

  meteo::bufr::TransfType bt;
  bufr.fill(&data, &bt);
  var(data.count());
  // bufr.removeBad(data);
  // var(data.count());

  //return;

  printvar(data.size());

  if (data.size() == 0) return -1;

  for (int idx=0; idx < data.size(); idx++) {
    printvar(idx);
    TMeteoData& one = data[idx];
    TMeteoControl::instance()->control(control::LIMIT_CTRL /*| control::CONSISTENCY_CTRL*/, &one);
    one.printData();
    QDateTime dt = TMeteoDescriptor::instance()->dateTime(one);
    var(dt);
    debug_log;
  }


  debug_log<<"file"<<fileName;
  return 0;
}



int main (int argc, char* argv[])
{
  TAPPLICATION_NAME("meteo");
  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  QCoreApplication app(argc, argv, false);

  int flag = 0, opt;
  QString type;

  while ((opt = getopt(argc, argv, "1:23h")) != -1) {
    switch (opt) {
    case '1':
      flag = 1;
      type = optarg;
      var(type);
      break;
    case '2':
      flag = 2;
      break;
    case '3':
      flag = 3;
      break;
    case 'h':
      info_log << "Usage:" << argv[0] << "\n -1 <type> <fileName> - станции\n" << "-2 <fileName> - ВМО\n" << "-3 <fileName> - BUFR";
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
  if (flag == 1 || flag == 2 || flag == 3) {
    if (optind >= argc) {
      error_log<<"Need point file name. \""<<argv[0]<<"-h\" for help";
      exit(-1);
    } else {
      fileName = argv[optind];
    }
  }


  ::meteo::gSettings(::meteo::global::Settings::instance());


  if ( !meteo::global::Settings::instance()->load() ) {
    error_log << QObject::tr("Ошибка при загрузке настроек.");
    return EXIT_FAILURE;
  }

  switch (flag) {
  case 1:
    decodeAndControl(type, fileName);
    break;
  case 2:
    decodeAndControl("", fileName);
    break;
  case 3:
    decodeAndControlBufr(fileName);
    break;

  default: {}
  }

  return 0;
}
