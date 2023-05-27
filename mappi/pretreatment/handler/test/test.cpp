#include <qcoreapplication.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <sat-commons/satellite/satellite.h>

#include "../handler.h"

#include <unistd.h>

#include <qfile.h>
#include <qdatastream.h>
#include <qimage.h>

#define MAPPIWEATHER_FILE MnCommon::varPath("mappi") + "/weather.txt"

using namespace mappi;
using namespace po;

void testRealTimeParse(const QString& fileName, const QString& satName, mappi::conf::RecvMode mode)
{
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    error_log << QObject::tr("Ошибка открытия файла").arg(fileName);
    return;
  }

  Satellite sat;
  bool ok = sat.readTLE(satName, MAPPIWEATHER_FILE);
  if (!ok) {
    error_log << QObject::tr("Ошибка чтения параметров орбиты спутника %1. Файл=%1").arg(satName).arg(MAPPIWEATHER_FILE);
    return;
  }

  int pos = 0;
  int oneReadSize = 22180;//22180*100; //22180 - одна строка;
  QByteArray data;
  Handler handler;
  handler.setupRT(satName, mode, sat.getTLEParams());

  QVector<QRgb> palette(256);
  for (uint i=0; i< 256; i++) {
    palette[i] = QRgb((i<<16)+ (i<<8) + i);
  }
  
  QByteArray channel;
  int thinning;
  int rows = 0, cols = 0;
  int saveChannel = 1;

  while (pos < file.size()) {
    //debug_log << pos << file.size();
    file.seek(pos);
    data = file.read(oneReadSize);
    
    QByteArray oneres;  
    bool ok = false;
    if (true == (ok = handler.processRT(QDateTime::currentDateTimeUtc(), data, &oneres) && !oneres.isEmpty())) {
      //   var("ok");
      QDataStream ds(oneres);
      int chCnt;
      int number;
      ds >> thinning;
      ds >> chCnt;
      // var(thinning);
      // var(chCnt);
      int rowsOne, colsOne;
      for (int idx = 0; idx < chCnt; idx++) {
	QByteArray one;
	ds >> number;
	ds >> one;
	ds >> rowsOne;
	ds >> colsOne;
	
       	debug_log << number << rowsOne << colsOne << idx << chCnt;
	if (number == saveChannel) {
	  if (number != -1) {
	    // var("append");
	    channel.append(one);
	    rows += rowsOne;
	    cols = colsOne;
	  }
	  break;
	}
      }
    }
    pos += data.size() ;//oneReadSize;

    // int cols = ((2048 - 1) / thinning) + 1;
    // if (cols % 4 != 0) {
    //   cols += 4 - (cols % 4);
    // }

    // int rows = channel.size() / cols;
    if (ok && cols != 0) {
      var(rows);
      var(cols);
      var(channel.size());
      QImage imqt((uchar*)(channel.data()), cols, rows, cols, QImage::Format_Indexed8);
      imqt.setColorCount(256);
      
      
      imqt.setColorTable(palette);
      imqt.save("img_" + QString::number(saveChannel) + ".bmp", "BMP");
  }

    //oneReadSize += 5;
  }

  file.close();

  debug_log << "write" << ( "img_" + QString::number(saveChannel) + ".bmp");


}

int main(int argc, char* argv[])
{
  TAPPLICATION_NAME("meteo");

  int flag = 0, opt;
  QString type;

  QString satName;
  mappi::conf::RecvMode mode = conf::kHiRate;

  while ((opt = getopt(argc, argv, "1hs:l")) != -1) {
    switch (opt) {
    case '1':
      flag = 1;
      break;
    case 's':
      satName = optarg;
      break;
    case 'l':
      mode = mappi::conf::kLowRate;
      break;
    case 'h':
      info_log<<"Usage:"<<argv[0]<<"-1 -s <satName> [-l] <fileName>\n"
	"-1 - test realtime parse\n"
	"-s - satellite\n"
	"-l - low rate [default: high rate]\n"
	"fileName - raw satellite data\n";
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

  if (satName.isEmpty()) {
    error_log << "Need point satellite. \"" << argv[0] << "-h\" for help";
    exit(-1);
  }

  QString fileName;
  if (flag == 1) {
    if (optind >= argc) {
      error_log<<"Need point file name. \""<<argv[0]<<"-h\" for help";
      exit(-1);
    } else {
      fileName = argv[optind];
    }
  }

  QCoreApplication app(argc, argv);

  switch (flag) {
  case 1:
    testRealTimeParse(fileName, satName, mode);
    break;
    
  default: {}
  }

  return 0;
}
