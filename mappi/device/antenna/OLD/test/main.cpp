#include <cross-commons/debug/tlog.h>
#include <sat-commons/satellite/satellite.h>
#include <sat-commons/satellite/satviewpoint.h>
#include <commons/mathtools/mnmath.h>

#include <commons/geobasis/coords.h>

#include <mappi/device/antenna/prbl-ant.h>
#include <mappi/device/antenna/smallant.h>
#include <mappi/device/antenna/cafant.h>
#include <mappi/device/antenna/mantcoord.h>

#include <qcoreapplication.h>
#include <qtextcodec.h>
#include <qthread.h>


#include <stdio.h>
#include <unistd.h>

using namespace mappi;
using namespace receive;
using namespace MnMath;
using namespace MnSat;
using namespace Coords;

//NOTE у юзера должны быть права на чтение порта (группа dialout)

//возвращает двоичное представление числа (для отладки)
QString db(uint8_t value)
{
  QString value_binary("\0");
  
  if (1 != value && 0 != value) { value_binary = db(value / 2); }
  char a = value % 2 + 48;
  return value_binary += a;
}

void createTrajectory(PrblAnt& ant)
{
  trc;
  // QString name="NOAA 18";
  // QDateTime beg(QDate(2018, 05, 03), QTime(16,38,0), Qt::UTC);
  // QDateTime end(QDate(2018, 05, 03), QTime(16,50,0), Qt::UTC);

  QString name="METOP-A";
  QDateTime beg(QDate(2018, 5, 8), QTime(8,19,0), Qt::UTC);
 QDateTime end(QDate(2018, 5, 8), QTime(8,31,0), Qt::UTC);

  float step = 0.5;//секунд
  meteo::GeoPoint siteSpb(deg2rad(59.97), deg2rad(30.30), 6);

  Satellite sat;
  bool ok = sat.readTLE(name, "./weather.txt");
  if (!ok) {
    error_log << QObject::tr("не прочитать tle\n");
    return;
  }

  QList<TrackTopoPoint> sattraj;
  ok =  sat.trajectory(beg, end, step, siteSpb, sattraj);
  if (!ok) {
    error_log << QObject::tr("не рассчитать траекторию\n");
    return;
  }
  var(sattraj.count());

  // for (auto it = track.begin(); it != track.end(); ++it ) {
  //   printf("az=%f %f el=%f %f\n", rad2deg((*it).az), (*it).az, rad2deg((*it).el), (*it).el);
  // }
  
  MnDevice::MAntCoord track(&ant);
  track.setAngleType(MnDevice::DEGREE);
  track.setTrack(sattraj);
  
  uint trackIdx = 0;
  float zeroAz = -32;
  debug_log << "start" << track.dt(0).toString("hh:mm:ss.zzz") << "azimut" << track.startPointAzimut(zeroAz) << "el" << track[trackIdx].elevat();
  ++trackIdx;
  
  for (; trackIdx < track.count(); trackIdx++) {
    MnDevice::MAntPoint point = track[trackIdx];
    debug_log << trackIdx << "(" << track.count() << ")" 
	      << track.dt(trackIdx - 1).toString("hh:mm:ss.zzz") << point.azimut() << point.elevat()
	      << rad2deg(sattraj.at(trackIdx).az) << rad2deg(sattraj.at(trackIdx).el);
    }

  QString az;
  for (trackIdx = 0; trackIdx < track.count(); trackIdx++) {
    MnDevice::MAntPoint point = track[trackIdx];
    az += QString::number(point.azimut()) + ", ";
  }
  var(az);
  debug_log;

  QString el;
  for (trackIdx = 0; trackIdx < track.count(); trackIdx++) {
    MnDevice::MAntPoint point = track[trackIdx];
    el += QString::number(point.elevat()) + ", ";
  }
  var(el);
}

void help(const QString& progName)
{
  info_log << QObject::tr("\nИспользование:") << progName 
	   << "-1|-2|-3|-4 -a|-e -v <port file>\n"
	   << QObject::tr("-1	     \t Получить текущее значение\n")
    	   << QObject::tr("-2	     \t Двигаться до заданного угла\n")
	   << QObject::tr("-3	     \t Двигаться с постоянной скоростью\n")
    	   << QObject::tr("-4            \t Остановить\n")
	   << QObject::tr("-5            \t Регенерация\n")
    	   << QObject::tr("-6            \t Установить в походное положение\n")
    	   << QObject::tr("-7            \t Установить в рабочее положение\n")
    	   << QObject::tr("-8            \t Получить статус\n")
    
    	   << QObject::tr("-9            \t Создание траектории\n")

	   << QObject::tr("-m            \t номео мотора: 0 - задать/получить азимут; 1 - задать/получить угол места\n")
	   << QObject::tr("-a <angle>    \t Задать азимут\n")
    	   << QObject::tr("-e <angle>    \t Задать угол места\n")
	   << QObject::tr("<port file>   \t Порт (напр, '/dev/ttyUSB0')\n");
}

uint8_t createCrc(uint8_t b1, uint8_t b2, uint8_t b3)
{
    uint8_t sum = ((b1 & 0x0F) + (b1 >> 4) + (b2 & 0x0F) + (b2 >> 4) + (b3 >> 4)) & 0x0F;

    debug_log <<   ((b1 & 0x0F) + (b1 >> 4)) <<  ((b2 & 0x0F) + (b2 >> 4)) << (b3>>4);
    var(sum);
    debug_log << (0x10 - sum);

      return (0x10-sum) & 0xF;
}



int main(int argc, char** argv) 
{
  QCoreApplication app( argc, argv );

  int flag = 0, opt;
  int motor = -1;
  float azimut = -9999, elev = -9999;
  QString val;

  while ((opt = getopt(argc, argv, "123456789m:a:e:h")) != -1) {
    switch (opt) {
    case '1':
      flag = 1;
      break;
    case '2':
      flag = 2;
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
    case '6':
      flag = 6;
      break;
    case '7':
      flag = 7;
      break;
    case '8':
      flag = 8;
      break;
    case '9':
      flag = 9;
      break;
    case 'm':
      val = optarg;
      motor = val.toInt();
      break;
    case 'a':
      val = optarg;
      azimut = val.toFloat();
      break;
    case 'e':
      val = optarg;
      elev = val.toFloat();
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

  uint32_t stat = 0;
  float curAngle = -9999;
  bool ok = false;
  //  PrblAnt ant;
  //SmallAnt* ant = new SmallAnt;
  CafAnt* ant = new CafAnt;
  
  if (flag == 0) {
//    flag = 1;
  }

  
  if (flag == 9) {
    createTrajectory(*ant);
    return 0;
  }

  QString port;
  if (optind >= argc) {
    error_log << QObject::tr("Необходимо указать порт");
    exit(-1);
  } else {
    port = argv[optind];
  }

  if ((flag == 2 || flag == 3) && azimut == -9999 && elev == -9999) {
    error_log << QObject::tr("Необходимо задать угол");
    exit(-1);
  }
  
  if (!ant->openPort(port)) {
    error_log << QObject::tr("Ошибка открытия порта %1").arg(port);
    //    return -1;
  }

  ant->setPower(true);
  usleep(1000);

  /*ok = ant->currentAzimut(&curAngle);
  var(ok);
  var(curAngle);
  ok = ant->currentElevat(&curAngle);
  var(ok);
  var(curAngle);
  
 */

  switch (flag) {
  case 1:
    if (motor == 0) {
      debug_log << QObject::tr("Получение текущего азимута");
      ok = ant->currentAzimut(&curAngle);
      var(curAngle);      
    } else if (motor == 1) {
      debug_log << QObject::tr("Получение текущего угла места");
      ok = ant->currentElevat(&curAngle);
      var(curAngle);
    } else {
      debug_log << QObject::tr("Получение текущих углов");
      // QThread::msleep(1000);
      // ok = ant->currentAzimut(&curAngle);
      // debug_log << "azimut" << curAngle;
      // ok = ant->currentElevat(&curAngle);
      // debug_log << "elev" << curAngle;
    }
  

    break;
  case 2:
    if (motor == 0) {
      debug_log << QObject::tr("Установка азимута %1").arg(azimut);
      ok = ant->setAzimut(azimut);
    } else if (motor == 1) {
      debug_log << QObject::tr("Установка угла места %1").arg(elev);
      ok = ant->setElevat(elev);
    } else {
      debug_log << QObject::tr("Установка азимута %1, угла места %2").arg(azimut).arg(elev);
      ok = ant->setPosition(azimut, elev, false);
    }
    break;
  case 3:
    if (motor == 0) {
      debug_log << QObject::tr("Движение, азимут %1").arg(azimut);
      ok = ant->moveAzimut(azimut);
    } else if (motor == 1) {
      debug_log << QObject::tr("Движение, угол места %1").arg(elev);
      ok = ant->moveElevat(elev);
    } else {
      debug_log << QObject::tr("Движение азимут %1, угол места %2").arg(azimut).arg(elev);
      ok = ant->movePosition(azimut, elev);
    }
    break;
  case 4:
    {
      float curAz, curEl;
      debug_log << QObject::tr("Остановка антенны");
      ok = ant->stop(&curAz, &curEl);
      var(curAz);
      var(curEl);
    }
    break;

  case 5:
    debug_log << QObject::tr("Регенерация");
    //    ok = ant->regenerate();
    break;
  case 6:
    debug_log << QObject::tr("Установка в походное положение");
    ok = ant->setTravelPosition();
    break;
  case 7:
    debug_log << QObject::tr("Установка в рабочее положение");
    //ok = ant->setWorkPosition();
    break;
  case 8:
    debug_log << QObject::tr("Получить статус");
    ok = ant->status(&stat);
    debug_log << QString("0x%1 (%2)").arg(QString::number(stat, 16)).arg(db(stat));
    break;
    
  }
  
  debug_log << "ok=" << ok;

//  sleep(10);

/*  for (int idx = 0; idx < 10; idx++) {
    var(idx);
    ok = ant.currentAzimut(&curAngle);
    var(curAngle);
    ok = ant.currentElevat(&curAngle);
    var(curAngle);

    sleep(1);
  }
*/
  //sleep(10);

  app.exec();

  ant->setPower(false);
  ant->closePort();

  return 0;
}

//инфо о порте
//setserial -G /dev/ttyUSB0
//setserial /dev/ttyUSB0 port 3

// 55553000+09919+09619000012

// 5555 - начало
// 3000 - статус (hex)
// +09919 - азимут 99.19
// +09619 - угол места 96.19
// 0000 - резерв
// 12 - кс
