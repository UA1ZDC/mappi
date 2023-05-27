#include <sat-commons/satellite/satellite.h>
#include <sat-commons/satellite/satviewpoint.h>

#include <commons/geobasis/coords.h>
#include <commons/mathtools/mnmath.h>
#include <cross-commons/funcs/mn_funcs.h>
#include <cross-commons/app/paths.h>

#include <cross-commons/debug/tlog.h>

#include <qstringlist.h>
#include <qvector.h>

#include <math.h>
#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <qdebug.h>


#define AVHRR3_FOV_STEP_ANGLE 0.0541
#define AVHRR3_MAX_SCAN_ANGLE 55.37


using namespace std;
using namespace MnMath;
using namespace MnSat;
using namespace Coords;

void test_SGP_SDP();
void test_sgp_sdp_list();
void testAosLos();
void testScheduler();
void testrevolNumber();
void test_scan();
void test_track();
void test_geotrack();
void test_corners();

void test_getpos();

void help(const QString& prog)
{
  info_log << QObject::trUtf8("Использование:\n%1 [-1|-2|-3|-4|-5|-6|-7|-8]\n").arg(prog)
	   << QObject::trUtf8("-1 \t test_SGP_SDP\n")
	   << QObject::trUtf8("-2 \t testAosLos\n")
	   << QObject::trUtf8("-3 \t testrevolNumber\n")
	   << QObject::trUtf8("-4 \t test_scan\n")
	   << QObject::trUtf8("-5 \t test_getpos\n")
	   << QObject::trUtf8("-6 \t test_corners\n")
	   << QObject::trUtf8("-7 \t test_track\n")
	   << QObject::trUtf8("-8 \t test_geotrack");
}


int main(int argc, char** argv)
{
  TAPPLICATION_NAME("test");

  int flag = 0, opt = -1;

  while ((opt = getopt(argc, argv, "123456789h")) != -1) {
    switch(opt) {
    case '1':
      flag = 1;
      test_SGP_SDP();
      break;
    case '2':
      flag = 2;
      testAosLos();
      break;
    case '3':
      flag = 3;
      testrevolNumber();
      break;
    case '4':
      flag = 4;
      test_scan();
      break;
    case '5':
      flag = 5;
      test_getpos();
      break;
    case '6':
      flag = 6;
      test_corners();
      break;    
    case '7':
      flag = 7;
      test_track();
      break;
    case '8':
      flag = 8;
      test_geotrack();
      break;
    case '9':
      flag = 9;
      test_sgp_sdp_list();
      break;

    case 'h':
      help(argv[0]);
      exit(0);
    default: /* '?' */
      error_log << "Option" << opt << "not realized";
      exit(-1);
    }
  }

  if (flag == 0) {
    error_log << "Need select test with option.";
    help(argv[0]);
    exit(-1);
  } 

  return 0;
}



void test_track()
{
  trc;

  QString name="NOAA 19";
  QDateTime beg(QDate(2021, 06, 16), QTime(5,22,0), Qt::UTC);
  QDateTime end(QDate(2021, 06, 16), QTime(5,36,0), Qt::UTC);
  float step = 1;//секунд

  meteo::GeoPoint siteSpb(deg2rad(59.97), deg2rad(30.30), 6);

  Satellite sat;
  bool ok = sat.readTLE(name, MnCommon::projectPath() + "/var/mappi/weather.txt");
   //  bool ok = sat.readTLE(name, "./weather.txt");
  if (!ok) {
    debug_log << QObject::tr("не прочитать tle");
    return;
  }

  QList<TrackTopoPoint> track;
  ok =  sat.trajectory(beg, end, step, siteSpb, track);
  if (!ok) {
    printf("ERR: не рассчитать траекторию\n");
    return;
  }
  printf("count=%d\n", track.count());

  for (auto it = track.begin(); it != track.end(); ++it ) {
    printf("az=%f %f el=%f %f\n", (*it).getAzimut().value().toDeg(), (*it).getAzimut().value().toRad(), (*it).getElevation().value().toDeg(), (*it).getElevation().value().toRad());
  }

  return;
  
  QVector<GeoCoord> track1;
  ok = sat.trajectory(beg, end, step, track1);
  var(ok);
  var(track1.size());
  for (auto it = track1.begin(); it != track1.end(); ++it ) {
    printf("lat=%f lon=%f\n", rad2deg((*it).lat), rad2deg((*it).lon));
  }

}


/////////////////////////////////////////////////////////////////////////////
// Test routine to output position and velocity information
void PrintPosVel(const Satellite & orbit)
{
  //   cOrbit       orbit(tle);
   EciPoint         eci;
   vector<EciPoint> Pos;

   // Calculate position, velocity
   // mpe = "minutes past epoch"
   for (int mpe = 0; mpe <= (360 * 4); mpe += 360)
   {
      // Get the position of the satellite at time "mpe"
      // The coordinates are placed into the local variable "eci".
      orbit.getPosition(mpe, &eci);

      // Push the coordinates object onto the end of the vector.
      Pos.push_back(eci);
   }

   // Print TLE data
//    printf("%s\n",   tle.getName().c_str());
//    printf("%s\n",   tle.getLine1().c_str());
//    printf("%s\n\n", tle.getLine2().c_str());

   // Header
   printf("  TSINCE            X                Y                Z\n\n");

   // Iterate over each of the ECI position objects pushed onto the
   // position vector, above, printing the ECI position information
   // as we go.
   for (unsigned int i = 0; i < Pos.size(); i++)
   {
      printf("%8d.00  %16.8f %16.8f %16.8f\n",
               i * 360,
               Pos[i].pos.x,
               Pos[i].pos.y,
               Pos[i].pos.z);
      GeoCoord geo = Pos[i].posToGeo();
      printf(" time=%f Lon = %f Lat=%f Alt=%f\n", Pos[i].date.getDate(), geo.lon*180/M_PI, geo.lat*180/M_PI, geo.alt );
   }

   printf("\n                    XDOT             YDOT             ZDOT\n\n");

   // Iterate over each of the ECI position objects in the position
   // vector again, but this time print the velocity information.
   for (unsigned int i = 0; i < Pos.size(); i++)
   {
      printf("             %16.8f %16.8f %16.8f\n",
             Pos[i].vel.x,
             Pos[i].vel.y,
             Pos[i].vel.z);
   }

}

void test_SGP_SDP()
{
  trc;

   // Test SGP4
//    string str1 = "SGP4 Test";
//    string str2 = "1 88888U          80275.98708465  .00073094  13844-3  66816-4 0    8";
//    string str3 = "2 88888  72.8435 115.9689 0086731  52.6988 110.5714 16.05824518  105";

//    //   cTle tle1(str1, str2, str3);
//    Satellite satSGP;
//    bool ok = satSGP.readTLE(str1, "tle_testsgp.txt");
//    printf("ok=%d %f\n", ok, satSGP.inclination() );
//    PrintPosVel(satSGP);

//    printf("\n");

   // Test SDP4
//    str1 = "SDP4 Test";
//    str2 = "1 11801U          80230.29629788  .01431103  00000-0  14311-1       8";
//    str3 = "2 11801  46.7916 230.4354 7318036  47.4722  10.4117  2.28537848     6";
  QString str1 = "LANDSAT 8";// "TEST SAT SGP 001";

   Satellite satSDP;
   if (!satSDP.readTLE(str1, "./tle.txt"))  {
    printf("can't read tle\n");
    return;
  }
   printf("epoch=%f\n", satSDP.epoch().getDate());
   printf("revolNumber=%lu\n", satSDP.revol());
   PrintPosVel(satSDP);

   printf("\nExample output:\n");

   // Example: Define a location on the earth, then determine the look-angle
   // to the SDP4 satellite defined above.

   // Create an ECI object to hold the location of the satellite
   EciPoint eciSDP4;

   // Get the location of the satellite. The
   // earth-centered inertial information is placed into eciSDP4.
   // Here we ask for the location of the satellite 90 minutes after
   // the TLE epoch.
   satSDP.getPosition(90.0, &eciSDP4);
   GeoCoord geo = eciSDP4.posToGeo();
   printf(" Lon = %f Lat=%f Alt=%f\n", geo.lon*180/M_PI, geo.lat*180/M_PI, geo.alt );

   // Get the location of the satellite at dt
   QDateTime dt;
   dt.setDate(QDate(2020,9,8));
   dt.setTime(QTime(6,0));
   JullianDate jd(dt);
   var(dt);
   var(jd.toGMST());
   
   satSDP.getPosition(dt, &eciSDP4);
   printf("vel=%f\n", eciSDP4.vel.r);
   printf("time=%f x=%f y=%f z=%f\n", eciSDP4.date.getDate(), eciSDP4.pos.x, eciSDP4.pos.y, eciSDP4.pos.z);
   geo = eciSDP4.posToGeo();
   printf("Lat = %f Lon = %f Alt = %f\n",  geo.lat*180/M_PI, geo.lon*180/M_PI, geo.alt );

   // Now create a site object. Site objects represent a location on the
   // surface of the earth. Here we arbitrarily select a point on the
   // equator.
   // GeoCoord siteEquator(0.0, deg2rad(-100.0), 0); // 0.00 N, 100.00 W, 0 km altitude
   GeoCoord  siteSpb(deg2rad(59.97), deg2rad(30.30), 6/1000.0);

   // Now get the "look angle" from the site to the satellite.
   // Note that the ECI object "eciSDP4" contains a time associated
   // with the coordinates it contains; this is the time at which
   // the look angle is valid.
   //   cCoordTopo topoLook = siteEquator.getLookAngle(eciSDP4);
   TopoCoord topoLook = eciToTopo( siteSpb, eciSDP4 );

   // Print out the results. Note that the Azimuth and Elevation are
   // stored in the cCoordTopo object as radians. Here we convert
   // to degrees using rad2deg()
   printf("AZ: %.1f  EL: %.1f\n",
          rad2deg(topoLook.az),
          rad2deg(topoLook.el));

}

void test_sgp_sdp_list()
{
  trc;
  QStringList sats;
  bool ok = Satellite::readSatelliteList("./weather.txt", &sats);
  if (!ok) {
    error_log << "err read file";
    return;
  }

  for (auto str1 : sats) {

    //qDebug() << "\n";
    printf("\n");
    printf("\n%s\n", str1.toStdString().data());
    
    Satellite satSDP;
    if (!satSDP.readTLE(str1, "./weather.txt"))  {
      printf("can't read tle\n");
      return;
    }

    //    qDebug() << "\n";
    //qDebug() << QObject::trUtf8("Возмущенное движение");
    printf("\nВозмущенное движение\n");
    // Example: Define a location on the earth, then determine the look-angle
   // to the SDP4 satellite defined above.
    
   // Create an ECI object to hold the location of the satellite
    EciPoint eciSDP4;
    
    // Get the location of the satellite at dt
    QDateTime dt;
    dt.setDate(QDate(2019,9,17));
    dt.setTime(QTime(6,0));
    JullianDate jd(dt);
    //    qDebug() << "UTC = " << dt.toString(Qt::ISODate);
    printf("UTC = %s\n", dt.toString(Qt::ISODate).toStdString().data());
    printf("S = %f рад\n", jd.toGMST());
   
    satSDP.getPosition(dt, &eciSDP4);
    printf("X = %.2f км  Y = %.2f км  Z = %.2f км\n", eciSDP4.pos.x, eciSDP4.pos.y, eciSDP4.pos.z);
    GeoCoord geo = eciSDP4.posToGeo();
    printf("Lat = %.2f°  Lon = %.2f°  Alt = %.2f км\n", geo.lat*180/M_PI, MnMath::M180To180(geo.lon*180/M_PI), geo.alt);
  }
}

void testAosLos()
{
  trc;
  printf("\nTest AOS and LOS\n");

  Satellite sat;
  QString name="NOAA 19";

  //bool ok = sat.readTLE(name, MnCommon::projectPath() + "/var/mappi/weather.txt");
  bool ok = sat.readTLE(name, "./weather.txt");
  if (!ok) {
    printf("can't read tle\n");
    return;
  }

  //GeoCoord  siteSpb(deg2rad(59.97), deg2rad(30.30), 6/1000.0);
  GeoCoord  siteSpb(deg2rad(59.57), deg2rad(30.17), 10/1000.0);
  //print AOS and LOS
  double curTime = sat.timeFromTLE( QDateTime::currentDateTimeUtc() );
  time_t aos,los;
  printf("curTime=%f\n", curTime*60);
  sat.search_AOS_LOS((time_t)curTime*60, siteSpb, &aos, &los, deg2rad(3.));
  printf("curTime=%f aos=%lu los=%lu\n", curTime*60, aos, los);
  debug_log << "aos=" << QDateTime::currentDateTimeUtc().addSecs((int)(aos - curTime*60)).toString()
	    << "los=" << QDateTime::currentDateTimeUtc().addSecs((int)(los - curTime*60)).toString();

  double maxEl;
  sat.searchMaximum(aos, siteSpb, &maxEl);
  printf("maxEl=%f\n", rad2deg(maxEl));
}



//////////////////////////////////////////////////////////////////////////////

void testrevolNumber()
{
  trc;

  Satellite sat;
  QString name="NOAA 19";
  //if (!sat.readTLE(name, MnCommon::projectPath() + "/var/mappi/weather.txt") ){
  if (!sat.readTLE(name, "./weather.txt") ){
    printf("can't read tle\n");
    return;
  }
  printf("\nepoch=%f\n", sat.epoch().getDate());
  double curTime = sat.timeFromTLE( QDateTime::currentDateTimeUtc() );
  printf("curTime=%f\n", curTime);
  printf("tleRev=%lu curRev=%lu\n",  sat.revol(),sat.revolNumber(QDateTime::currentDateTimeUtc()));
 //дней с 01.01.2000
  double t=( sat.epoch().getDate()- 2451545.0 - fmod(sat.epoch().getDate()  + 0.5, 1.0 ))  / 365.25;
  QDateTime dt(QDate(2000,1,1));
  dt=dt.addYears(int(t));
  dt=dt.addSecs(int(fmod(t,1)* 365.25*SEC_PER_DAY ));
  debug_log << "t=" << fmod(t,1) 
	    << dt.toString();

//   sat.readTLE(name, "weather1.txt");
//   printf("\nepoch=%f\n", sat.epoch().getDate());
//   curTime = sat.timeFromTLE( QDateTime::currentDateTime(Qt::UTC) );
//   printf("curTime=%f\n", curTime);
//   printf("tleRev=%lu curRev=%lu\n",  sat.revol(),sat.revolNumber(QDateTime::currentDateTime(Qt::UTC)));

//   //дней с 01.01.2000
//   t=( sat.epoch().getDate()- 2451545.0 - fmod(sat.epoch().getDate()  + 0.5, 1.0 ))  / 365.25;
//   dt = QDateTime(QDate(2000,1,1));
//   dt=dt.addYears(int(t));
//   dt=dt.addSecs(int(fmod(t,1)* 365.25*SEC_PER_DAY ));
//   printf("t=%f %s\n", (fmod(t,1)* 365.25*SEC_PER_DAY), dt.toString().latin1() );
}


void test_scan()
{
  trc;

  SatViewPoint sat;
  QString name="NOAA 19";

  //bool ok = sat.readTLE(name, MnCommon::projectPath() + "/var/mappi/weather.txt");
  bool ok = sat.readTLE(name, "./weather.txt");
  if (!ok) {
    printf("can't read tle\n");
    return;
  }
//   time_t aos,los;
//   GeoCoord  siteSpb(deg2rad(59.97), deg2rad(30.30), 6/1000.0);
//   double curTime = sat.timeFromTLE( QDateTime::currentDateTime(Qt::UTC) );
//   ok = sat.search_AOS_LOS((time_t)curTime*60, siteSpb, &aos, &los);
//   QDateTime beg = QDateTime::currentDateTime(Qt::UTC).addSecs((int)(aos - curTime*60));
//   QDateTime end = QDateTime::currentDateTime(Qt::UTC).addSecs((int)(los - curTime*60));

//NOAA 17
//   QDateTime beg(QDate(2008, 4, 15), QTime(9,27,11));
//   QDateTime end(QDate(2008, 4, 15), QTime(9,39,55));
//NOAA 18
 //  QDateTime beg(QDate(2008, 4, 15), QTime(12,34,26));
//   QDateTime end(QDate(2008, 4, 15), QTime(12,46,33));
  QDateTime beg(QDate(2009, 3, 17), QTime(9,36,13));
  QDateTime end(QDate(2009, 3, 17), QTime(9,42,9));
  sat.setDateTime(beg, end);

    //printf("beg=%s end=%s\n", beg.toString().latin1(), end.toString().latin1());
  debug_log << "beg=" << beg.toString("dd.MM.yyyy hh:mm:ss.zz")
	    << "end=" <<end.toString("dd.MM.yyyy hh:mm:ss.zz");

  float step = 400*deg2rad(AVHRR3_FOV_STEP_ANGLE);
  // uint size=2048;
  uint size=400;

  //  GeoCoord* pp = new(std::nothrow)GeoCoord[size] ;//MnCommon::allocateMem<GeoCoord>(size,1);
  QVector<Coords::GeoCoord> pp(size);
  // ok = sat.countGrid(beg, end, step, 1, deg2rad(AVHRR3_MAX_SCAN_ANGLE), pp, size);

  printvar(step);
  printvar(400*deg2rad(AVHRR3_FOV_STEP_ANGLE));
  ok = sat.countGrid(step, 60, 400*deg2rad(AVHRR3_FOV_STEP_ANGLE), pp);
  if (!ok) printf("\n!!!Размер массива меньше реального количества значений!!!!\n");
  for (uint i=0; i< size; i++) {
    double lon= rad2deg(pp[i].lon);
    if (lon > 180) {
      lon -= 360;
    }
    //    printf("%3d lon=%6.2f lat=%6.2f\n",i, rad2deg(pp[i]->lon), rad2deg(pp[i]->lat));
    printf("%3d lon=%7.2f lat=%7.2f\n",i, lon, rad2deg(pp[i].lat));
  }
  //delete[] pp;
//  MnCommon::deallocateMem(pp, size);

//крайние точки изображения
//   GeoCoord geo;
//   if (!sat.getPosition( sat.timeFromTLE(beg), &geo) ) {
//   }
//   printf("raan=%f\n", sat.raan());
//   float sigma = deg2rad(55.37);
//   double v_lat, delta_lon;
//   sat.countViewedPoint(geo.lat, geo.alt, sigma, &v_lat, &delta_lon, 1);
//   printf("lat=%f lon=%f delta=%f\n",  rad2deg(geo.lat), rad2deg(geo.lon), rad2deg(delta_lon));
//   printf("вп lat=%f lon=%f\n", rad2deg(v_lat), rad2deg(fmod2p(geo.lon + delta_lon)));
//   sat.countViewedPoint(geo.lat, geo.alt, sigma, &v_lat, &delta_lon, -1);
//   printf("lat=%f lon=%f delta=%f\n",  rad2deg(geo.lat), rad2deg(geo.lon), rad2deg(delta_lon));
//   printf("вл lat=%f lon=%f\n", rad2deg(v_lat), rad2deg(fmod2p(geo.lon - delta_lon)));


//   if (!sat.getPosition( sat.timeFromTLE(end), &geo) ) {
//   }
//   printf("raan=%f\n", sat.raan());
//   sat.countViewedPoint(geo.lat, geo.alt, sigma, &v_lat, &delta_lon, 1);
//   printf("lat=%f lon=%f delta=%f\n",  rad2deg(geo.lat), rad2deg(geo.lon), rad2deg(delta_lon));
//   printf("нп lat=%f lon=%f\n", rad2deg(v_lat), rad2deg(fmod2p(geo.lon + delta_lon)));
//   sat.countViewedPoint(geo.lat, geo.alt, sigma, &v_lat, &delta_lon, -1);
//   printf("lat=%f lon=%f delta=%f\n",  rad2deg(geo.lat), rad2deg(geo.lon), rad2deg(delta_lon));
//   printf("нл lat=%f lon=%f\n", rad2deg(v_lat), rad2deg(fmod2p(geo.lon - delta_lon)));

//   printf("incl=%f\n", rad2deg(sat.inclination()));


//   EciPoint ecip;
//   sat.getPosition( sat.timeFromTLE(beg), &ecip);
//   printf("\npos.x=%f pos.y=%f poz.z=%f poz.r=%f\n", ecip.pos.x, ecip.pos.y, ecip.pos.z, ecip.pos.r);
//   printf("vel.x=%f vel.y=%f poz.z=%f poz.r=%f\n\n",  ecip.vel.x, ecip.vel.y, ecip.vel.z, ecip.vel.r);
//   sat.getPosition( sat.timeFromTLE(end), &ecip);
//   printf("pos.x=%f pos.y=%f poz.z=%f poz.r=%f\n", ecip.pos.x, ecip.pos.y, ecip.pos.z, ecip.pos.r);
//   printf("vel.x=%f vel.y=%f poz.z=%f poz.r=%f\n",  ecip.vel.x, ecip.vel.y, ecip.vel.z, ecip.vel.r);
}


void test_corners()
{
  trc;

  SatViewPoint sat;
  QString name="NOAA 18";

  //bool ok = sat.readTLE(name, MnCommon::projectPath() + "/var/mappi/weather.txt");
  bool ok = sat.readTLE(name, "./weather.txt");
  if (!ok) {
    printf("can't read tle\n");
    return;
  }
  //NOAA 17
 //  QDateTime beg(QDate(2008, 4, 15), QTime(9,27,11));
//   QDateTime end(QDate(2008, 4, 15), QTime(9,39,55));
  //NOAA 18
   QDateTime beg(QDate(2008, 4, 15), QTime(12,34,26));
   QDateTime end(QDate(2008, 4, 15), QTime(12,46,33));
   sat.setDateTime(beg,end);

   GeoCoord bl, br, bm, el, er, em;
   sat.getPosition( sat.timeFromTLE(beg), &bm);
   sat.getPosition( sat.timeFromTLE(end), &em);
   ok = sat.countGridCorners(deg2rad(AVHRR3_MAX_SCAN_ANGLE), &bl, &br, &el, &er);
  if (!ok) printf("Error countGridCorners!!\n");
  else  {
    printf("верх лев  lat=%f lon=%f\n",  rad2deg(bl.lat), rad2deg(bl.lon));
    printf("верх сер  lat=%f lon=%f\n",  rad2deg(bm.lat), rad2deg(bm.lon));
    printf("верх прав lat=%f lon=%f\n",  rad2deg(br.lat), rad2deg(br.lon));
    printf("нижн лев  lat=%f lon=%f\n",  rad2deg(el.lat), rad2deg(el.lon));
    printf("нижн сер  lat=%f lon=%f\n",  rad2deg(em.lat), rad2deg(em.lon));
    printf("нижн прав lat=%f lon=%f\n",  rad2deg(er.lat), rad2deg(er.lon));
  }

}

void test_geotrack()
{
  trc;

  Satellite sat;
  QString name="NOAA 18";
  //bool ok = sat.readTLE(name, MnCommon::projectPath() + "/var/mappi/weather.txt");
  bool ok = sat.readTLE(name, "./weather.txt");
  if (!ok) {
    printf("не прочитать tle\n");
    return;
  }
  //NOAA 18
  QDateTime beg(QDate(2008, 4, 15), QTime(12,34,26));

  GeoCoord  siteSpb(deg2rad(59.97), deg2rad(30.30), 6/1000.0);
  GeoCoord  siteNoth(deg2rad(90.), deg2rad(30.), 0.);

  QVector<GeoCoord> futur;
  QVector<GeoCoord> past;
  ok =  sat.trajectory(beg, 60, deg2rad(110.), siteSpb, futur, past);
  if (!ok) {
    printf("не расчитать траекторию\n");
    return;
  }

  printf("PAST count=%d\n", past.count());
  for (QVector<GeoCoord>::Iterator it = past.begin(); it != past.end(); ++it ) {
    printf("lat=%f lon=%f\n", rad2deg((*it).lat), rad2deg((*it).lon));
  }

  printf("\nFUTUR count=%d\n", futur.count());
  for (QVector<GeoCoord>::Iterator it = futur.begin(); it != futur.end(); ++it ) {
    printf("lat=%f lon=%f\n", rad2deg((*it).lat), rad2deg((*it).lon));
  }

}

void test_getpos() 
{
  trc;

  Satellite sat;
  QString name="METEOSAT-11 (MSG-4)";
  //bool ok = sat.readTLE(name, MnCommon::projectPath() + "/var/mappi/weather.txt");
  bool ok = sat.readTLE(name, "./weather.txt");
  if (!ok) {
    printf("не прочитать tle\n");
    return;
  }
  GeoCoord gc;
  TopoCoord tc;
  GeoCoord gc2;
  gc.lat = MnMath::deg2rad(60.);
  gc.lon = MnMath::deg2rad(30.);
  sat.getPosition( QDateTime::currentDateTimeUtc(), gc, &tc, &gc2 );
  //debug("TOPO EL: %f TOPO AZ %f", tc.az, tc.el);
  fprintf(stderr,"TOPO EL: %f TOPO AZ %f", tc.az, tc.el);

}
