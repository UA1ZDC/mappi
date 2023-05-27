#include <mappi/schedule/sat_schedule.h>
#include <commons/geobasis/coords.h>
#include <commons/mathtools/mnmath.h>

#include <cross-commons/app/paths.h>

#include <mappi/proto/reception.pb.h>

 #define MAPPISCHEDULE_FILE      MnCommon::varPath("mappi") + "weather.txt"

using namespace mappi;
using namespace schedule;

int test1()
{

 printf("\nTest scheduler\n");
 Coords::GeoCoord siteSpb(MnMath::deg2rad(59.97), MnMath::deg2rad(30.30), 6/1000);
  
  //Coords::GeoCoord siteSpb(MnMath::deg2rad(59.57), MnMath::deg2rad(30.17), 10/1000.0);
  //MOSCOW!!!!!!
 //  Coords::GeoCoord siteSpb(MnMath::deg2rad(55.98), MnMath::deg2rad(37.5), 0);

  //praga!!!!!!
 //  Coords::GeoCoord siteSpb(MnMath::deg2rad(50.09), MnMath::deg2rad(14.42), 0);


 //SpSettings settings;
 //  QPtrList< Settings::PretrSatParam > satParam;
 QStringList satParam;
 satParam << "NOAA 18" << "NOAA 19" << "NOAA 15";// << "METEOR-M 2" << "METOP-A" << "FENGYUN 3A" << "FENGYUN 3B" <<  "FENGYUN 3C";
 //satParam << "METEOR-M 2" << "METEOR-M2 2";

  // if ( settings.getSettings( satParam ) != 0 ) {
  //   printf("ERROR\n");
  //   return -1;
  // }
   //satParam.setAutoDelete(false);
   
 QList<satellite::Schedule> schedule;
  SatSchedule sched(MAPPISCHEDULE_FILE, siteSpb);


  float hours = 24*6;
  float thr = MnMath::deg2rad(1.);
  float culmin = MnMath::deg2rad(30.);
  QDateTime dt = QDateTime::currentDateTimeUtc();

  // dt.setDate(QDate(2015, 06, 05));
  // dt.setTime(QTime(0, 0));


  sched.getSchedule( dt.addDays(-2), /*sats*/satParam, hours, thr, culmin, schedule);
  qSort( schedule );

  //sched.settlementConflicts(schedule, mappi::conf::kBeginPrior);
  //sched.settlementConflicts(schedule, mappi::conf::kContinPrior);
  //sched.settlementConflicts(schedule, mappi::conf::kWestPrior);
  sched.settlementConflicts(schedule, mappi::conf::kEastPrior);
  for (int i=0; i<schedule.count(); i++ ) {
    printf("%s\t\t%s\t %s\t %f\t %lu dir=%d confl=%d\n", schedule[i].name.toUtf8().data(),
      schedule[i].aos.toString("yyyy-MM-dd hh:mm").toUtf8().data(),
      schedule[i].los.toString("yyyy-MM-dd hh:mm").toUtf8().data(), MnMath::rad2deg(schedule[i].maxEl), schedule[i].revol,
      schedule[i].direction, schedule[i].state
    );
  };
  printf("%d\n", schedule.count());
  return 0;

}


//5, 11, 17, 23
void test2(const Coords::GeoCoord& site, const QDateTime& dtBeg, const QString& satName, const QString& text, int h1, int h2, int h3, int h4)
{
  return;
  QList<satellite::Schedule> schedule;
  SatSchedule sched(MAPPISCHEDULE_FILE, site);

  float thr = MnMath::deg2rad(1.);
  float culmin = MnMath::deg2rad(1.);

  printf("%s ", text.toUtf8().data());

  //  int countD = 2;

  QDateTime dt = dtBeg;
  float hours = 24*28;
  sched.getSchedule( dt, satName, hours, thr, culmin, schedule);

  int cnt1 = 0, cnt2 = 0, cnt3 = 0, cnt4 = 0;
  for (int i=0; i<schedule.count(); i++ ) {

    // printf("h=%d h1=%d h2=%d h3=%d h4=%d\n", schedule[i].aos.time().hour(), h1, h2, h3, h4);
    
    if ((h1 < h2 && schedule[i].aos.time().hour() >= h1 && schedule[i].aos.time().hour() < h2) ||
	(h1 > h2 && (schedule[i].aos.time().hour() >= h1 || schedule[i].aos.time().hour() < h2))) {
      cnt1++;
      // printf("1\n");
    } else if ((h2 < h3 && schedule[i].aos.time().hour() >= h2 && schedule[i].aos.time().hour() < h3 ) ||
	       (h2 > h3 && (schedule[i].aos.time().hour() >= h2 || schedule[i].aos.time().hour() < h3)) ){
      cnt2++;
      //printf("2\n");
    } else if ((h3 < h4 && schedule[i].aos.time().hour() >= h3 && schedule[i].aos.time().hour() < h4 ) ||
	       (h3 > h4 && (schedule[i].aos.time().hour() >= h3 || schedule[i].aos.time().hour() < h4))) {
      cnt3++;
      // printf("3\n");
    } else if ( (h4 < h1 && schedule[i].aos.time().hour() >= h4 && schedule[i].aos.time().hour() < h1) ||
		(h4 > h1 && (schedule[i].aos.time().hour() >= h4 || schedule[i].aos.time().hour() < h1))) {
      cnt4++;
      //printf("4\n");
    }
    
 
    // printf("\n%s\t %s\t %s\t", 
    // 	   schedule[i].name.ascii(),
    // 	   schedule[i].aos.toString().ascii(),
    // 	   schedule[i].los.toString().ascii()
    // 	   );
    
    
  }

  // printf("\n");

  printf("%d %d %d %d  %d\n", cnt1, cnt2, cnt3, cnt4, schedule.count());
  int cnt = cnt1 + cnt2 + cnt3 + cnt4;
  if (cnt != schedule.count()) {
    printf("error cnt=%d %d\n", cnt, schedule.count());
  }

  // dt = dtBeg;
  // dt = dt.addSecs(h2*3600);
  // cnt = 0;
  // for (int d = 0; d < countD; d++) {
  //   printf("%s\n", dt.toString().ascii());
  //   sched.getSchedule( dt, satName, 6, thr, culmin, schedule);
  //   cnt += schedule.count();

  //   for (uint i=0; i<schedule.count(); i++ ) {
  //     printf("%s\t %s\t %s\t %f\t %lu dir=%d confl=%d\n", schedule[i].name.ascii(),
  // 	     schedule[i].aos.toString().ascii(),
  // 	     schedule[i].los.toString().ascii(), MnMath::rad2deg(schedule[i].maxEl), schedule[i].revol,
  // 	     schedule[i].direction, schedule[i].state
  // 	     );
  //   }
  //   dt = dt.addDays(1);
  // }
  // printf("cnt = %d\n", cnt);


  // dt = dtBeg;
  // dt = dt.addSecs(h3*3600);
  // cnt = 0;
  // for (int d = 0; d < countD; d++) {
  //   printf("%s\n", dt.toString().ascii());
  //   sched.getSchedule( dt, satName, 6, thr, culmin, schedule);
  //   cnt += schedule.count();
  //   dt = dt.addDays(1);
  //   for (uint i=0; i<schedule.count(); i++ ) {
  //     printf("%s\t %s\t %s\t %f\t %lu dir=%d confl=%d\n", schedule[i].name.ascii(),
  // 	     schedule[i].aos.toString().ascii(),
  // 	     schedule[i].los.toString().ascii(), MnMath::rad2deg(schedule[i].maxEl), schedule[i].revol,
  // 	     schedule[i].direction, schedule[i].state
  // 	     );
  //   }
  // }
  // printf("cnt=%d\n", cnt);

  // dt = dtBeg;
  // dt = dt.addSecs(h4*3600);
  // cnt = 0;
  // for (int d = 0; d < countD; d++) {
  //   printf("%s\n", dt.toString().ascii());
  //   sched.getSchedule( dt, satName, 6, thr, culmin, schedule);
  //   cnt += schedule.count();
  //   dt = dt.addDays(1);
  //   for (uint i=0; i<schedule.count(); i++ ) {
  //     printf("%s\t %s\t %s\t %f\t %lu dir=%d confl=%d\n", schedule[i].name.ascii(),
  // 	     schedule[i].aos.toString().ascii(),
  // 	     schedule[i].los.toString().ascii(), MnMath::rad2deg(schedule[i].maxEl), schedule[i].revol,
  // 	     schedule[i].direction, schedule[i].state
  // 	     );
  //   }
  // }
  // printf("cnt = %d\n", cnt);


  // for (uint i=0; i<schedule.count(); i++ ) {
  //   printf("%s\t %s\t %f\t\n",
  //     schedule[i].aos.toString().ascii(),
  //     schedule[i].los.toString().ascii(), 
  // 	   MnMath::rad2deg(schedule[i].maxEl));
  // };

}

int main(int argc, char** argv) 
{
  TAPPLICATION_NAME("meteo");


  test1();
  return 0;


  if (argc < 11) {
    printf("Не достаточно аргументов\n");
    return -1;
  }

  // printf("%s\n", argv[1]);
  // printf("%s\n", argv[5]);
  // printf("%s\n", argv[3]);

  QString sat = argv[1];
  QString text = argv[2];
  float lat = QString(argv[3]).toFloat();
  float lon = QString(argv[4]).toFloat();
  float alt = QString(argv[5]).toFloat();
  int h1 = QString(argv[6]).toInt();
  int h2 = QString(argv[7]).toInt();
  int h3 = QString(argv[8]).toInt();
  int h4 = QString(argv[9]).toInt();
  int dayofMonth = QString(argv[10]).toInt();
  
  //  printf("%d\n", dayofMonth);

  Coords::GeoCoord site(MnMath::deg2rad(lat), MnMath::deg2rad(lon), alt);
  QDateTime dt;
  dt.setDate(QDate(2015, 9, dayofMonth));
  dt.setTime(QTime(0, 0));

  test2(site, dt, sat, text, h1, h2, h3, h4);

  printf("\n!!!\n");
  
  test1();
  
  return 0;
}
