#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include <qapplication.h>
#include <Q3Canvas>
//#include <Qt3Support>
#include <qpixmap.h>
#include <qpainter.h>
#include <qpen.h>
#include <qimage.h>
#include <qwmatrix.h>

#include <commons/geobasis/coords.h>
#include <projection_functions.h>
#include <commons/mathtools/mnmath.h>
#include <cross-commons/funcs/mn_errdefs.h>
#include <cross-commons/app/paths.h>

#include <QtDebug>

TAPPLICATION_NAME("test")

using namespace Coords;
using namespace MnMath;

void test_geoDistance()
{ 
  GeoCoord geo1(deg2rad(10), deg2rad(0));
  GeoCoord geo2(deg2rad(-30), deg2rad(0));

  printf("d=%f\n", rad2deg(geoDistance(geo1, geo2)));
  
}

void testFrameForQuad( QApplication& ap ) {
  Screen a[4];
  Screen b[4];

  a[0].x = 50*2;
  a[0].y = 50*2;
  a[1].x = 150*2;
  a[1].y = 45*2;
  a[3].x = 180*2;
  a[3].y = 100*2;
  a[2].x = 70*2;
  a[2].y = 120*2;


/*  a.x = 150;
  a.y = 150;
  b.x = 250;
  b.y = 170;
  c.x = 260;
  c.y = 270;
  d.x = 70;
  d.y = 220;*/

/*  a.x = 50*2;
  a.y = 50*2;
  b.x = 150*2;
  b.y = 50*2;
  c.x = 150*2;
  c.y = 100*2;
  d.x = 50*2;
  d.y = 100*2;*/

/*  a.x = 50*2;
  a.y = 50*2;
  b.x = 150*2;
  b.y = 50*2;
  c.x = 170*2;
  c.y = 100*2;
  d.x = 70*2;
  d.y = 100*2;*/

/*  c.x = 100*2;
  c.y = 100*2;
  d.x = 150*2;
  d.y = 50*2;
  a.x = 200*2;
  a.y = 100*2;
  b.x = 150*2;
  b.y = 150*2;*/

  a[0].x = 100;
  a[0].y = 100;
  a[1].x = 300;
  a[1].y = 100;
  a[2].x = 300;
  a[2].y = 300;
  a[3].x = 100;
  a[3].y = 300;

  if ( ERR_NOERR != findFrameForQuad( a, b ) ) {
    qDebug("Error in calc for frame of quad...");
    return;
  }

  Q3CanvasView w;
  QPen pen(Qt::black, 2);
  w.setGeometry(200,200,800,600);
  QPixmap pix( 800,600 );
  pix.fill(Qt::white);
  Q3Canvas canv(&w);
  canv.resize(800,600);
  w.setCanvas( &canv );
  QPainter p;
  p.begin( &pix );
  p.setPen( pen );
  p.setBackgroundMode(Qt::TransparentMode);
  p.setBrush( Qt::black );
  p.drawLine( a[0].x, a[0].y, a[1].x, a[1].y );
  p.drawLine( a[1].x, a[1].y, a[2].x, a[2].y );
  p.drawLine( a[2].x, a[2].y, a[3].x, a[3].y );
  p.drawLine( a[3].x, a[3].y, a[0].x, a[0].y );
  pen.setColor(Qt::blue);
  p.setPen( pen );
  p.drawLine( b[0].x, b[0].y, b[1].x, b[1].y );
  p.drawLine( b[1].x, b[1].y, b[2].x, b[2].y );
  p.drawLine( b[2].x, b[2].y, b[3].x, b[3].y );
  p.drawLine( b[3].x, b[3].y, b[0].x, b[0].y );
  p.end();
  for ( int i = 0; i < 4; ++i ) {
    qDebug("X[%d] = %d Y[%d] = %d", i, b[i].x, i, b[i].y );
  }
  QPixmap img( abs(b[1].x-b[0].x), abs(b[1].y-b[0].y) );
  QWMatrix mtrx;
  mtrx.rotate( 20 );
  img.fill( Qt::red );
  img = img.xForm( mtrx );
  bitBlt( (QPaintDevice*)&pix, 300, 300, (QPaintDevice*)&img, 0, 0,/*abs(b[1].x-b[0].x), abs(b[1].y-b[0].y),*/img.width(), img.height() /* Qt::AndROP*/, false  );
  canv.setBackgroundPixmap( pix );
  ap.setMainWidget( &w );
  w.show();
  ap.exec();
}

void test_sunH()
{
  printf("\nTEST sunH()\n"); 
  GeoCoord geo(deg2rad(43.64986), deg2rad(41.43147) );

  QDateTime dt(QDate(2009, 6,19), QTime(14,0));

  SunParams sp = sun_H(geo, dt);
  printf("aaz=%f h=%f dec=%f\n", rad2deg(sp.az), rad2deg(sp.h), rad2deg(sp.dec));
}


//////Тестирование функций FLtoXY_sphere() и FLtoXY_ellips()

int test_TransCoords()
{
 double fi0=60*PI/180;
 double la0=30*PI/180;
 double R=6371.032; //Радиус Земли
 double a=0.5*6378.160;  //Большая полуось эллипсойда Красовского 
 double b=0.5*6356.777;  //Малая полуось эллипсойда Красовского
 double alpha;   //Направление оси Х прямоугольной топоцентрической СК
 double fi,la,fi1,la1,H;
 double X,Y,Z,M;
 double az,tet,d;


H=200;
alpha=0.5*PI;

 printf("test_TransCoords\n");
 for(la=25.*PI/180;la<36.*PI/180;la+=5.*PI/180)
 {
  for(fi=50.*PI/180;fi<=50.*PI/180;fi+=0.5*PI/180)
  {

    FLtoXY_sphere(R, fi0, la0, fi, la, X, Y, M);
    
    FLtoXY_ellips(a, b, fi0, la0, fi, la, X, Y, M);




    printf("fi=%5.2f la=%5.2f  H=%5.2f\n", fi*180/PI, la*180/PI, H);    
    
    FLHtoXYZ_sphere(R, fi0,la0,alpha, fi, la, H, X,Y,Z);
    printf("X= %5.2f Y= %5.2f Z= %5.2f\n", X, Y, Z);    
    

    az=calcAzimooth_ellips(a,b,fi0,la0,fi,la);
    
    printf("Azimooth_ellips=%5.2f grad\n", az*180./PI);

    az=calcAzimooth_sphere(fi0,la0,fi,la);

    printf("Azimooth_sphere=%5.2f grad\n",az*180./PI);
    printf("Azimooth_sphere=%5.2f rad\n",az);
    
    XYZtoAED_sphere(alpha, X,Y,Z, az,tet,d);
    printf("az=%5.2f tet=%5.2f d=%5.2f\n", az*180/PI, tet*180./PI, d);    	




    AEDtoFLH_sphere(R,fi0,la0, az,tet,d, fi1,la1,H);
    printf("fi1=%5.2f la1=%5.2f H1=%5.2f\n", fi1*180/PI, la1*180./PI, H);    	
  
    printf("+++++++++++++++++++\n");
  }
 }


	return 0;
}

void testDayDuration() {
  QDate date = QDate::currentDate();
  TGis::GeoPoint pos( 30., 60., 0., TGis::DEG );
  pos.toRad();
  Coords::DayDuration dd;
  if ( false == dd.calcLength( pos, date ) ) {
    qDebug() << "error while" << __PRETTY_FUNCTION__;
    return;
  }
  qDebug() << "start time =" << dd.start;
  qDebug() << "stop time =" << dd.start.addSecs(dd.length);
}

void testCoordSys()
{
 GeoCoord pos[10], posSatellite, posIIO, posSurface, posBeam;
 double termLon, termLat;
 double x,y,z;
 double L1, L2;
 double x1, x2, x3, x4;
 bool ok1,ok2;

//coords translations

printf("Coords Translations From GEODESY(Lat, Lon, Heght) to UNIVERSAL RECTANGULAR(X,Y,Z) and back\n");
 for(int i=0;i<10;i++)
 {
  pos[i].setGeoPoint(i*5./180.*PI, i*5./180.*PI, i*5000);
///  pos[i].lat = i*5./180.*PI;
///  pos[i].lon = i*5./180.*PI;
///  pos[i].alt = i*5000;
  UsrCoord rc = geoToUsr(pos[i]);// pos[i].llhxyz(&x,&y,&z);
  printf("X=%e\n",rc.x);
  printf("Y=%e\n",rc.y);
  printf("Z=%e\n\n",rc.z);
  GeoCoord gc = usrToGeo(rc); //pos[i].xyzllh(x,y,z);
  printf("Fi=%e\n",gc.lat*180./PI);
  printf("La=%e\n",gc.lon*180./PI);
  printf("H=%e\n\n", gc.alt);
  printf("**********\n");
 }

//unlinear equations 
/*
 if(squareEquation(2.,3.,1.,x1,x2)) printf("squareEquation: x1 = %e , x2 = %e\n", x1, x2);
 if(cubeEquation(1.,0.,-4.,0.,x1,x2,x3)) printf("cubeEquation: x1 = %e , x2 = %e, x3 = %e\n", x1, x2, x3);
 else printf("cubeEquation: x1 = %e , x2 = Im, x3 = Im\n", x1);
 fourPowerEquation(1.,100.,2.,2.,-1., x1,x2,x3,x4, ok1, ok2); // 1, 3, 3, -1 ,-6  real roots: 1, -2   
 if(ok1) printf("fourPowerEquation: x1 = %e, x2 = %e\n", x1, x2);
 if(ok2) printf("fourPowerEquation: x3 = %e, x4 = %e\n", x3, x4);
 if(ok1) printf("fourPowerEquation control = %e\n", 1.*x1*x1*x1*x1+100.*x1*x1*x1+2.*x1*x1+2.*x1-1.);
 if(ok2) printf("fourPowerEquation control = %e\n", 1.*x3*x3*x3*x3+100.*x3*x3*x3+2.*x3*x3+2.*x3-1.);
*/

//beam point at surface
printf("Find Surface Point of Beam from SATELLITE (60 deg,30 deg,5000 km) to IIO (70 deg, 25 deg, 300 km)\n");
 posSatellite.setGeoPoint(60./180.*PI, 30./180.*PI, 5000000.);
 posIIO.setGeoPoint(70./180.*PI, 25./180.*PI, 300000. );
 ok1= surfacePoint(posSatellite, posIIO, posSurface, L1, L2);
 if(ok1) printf("surfacePoint: Fi=%8.5f, La=%8.5f, H =%8.5f, L_IIO =%8.5f, L_Surface =%8.5f\n", posSurface.lat*180./PI, posSurface.lon*180./PI, posSurface.alt, L1, L2 );

//beam point at distance
printf("Find Point of Beam from SATELLITE (60 deg,30 deg,5000 km) to IIO (70 deg, 25 deg, 300 km) at DISTANCE to IIO (L_IIO)\n");
 beamPoint(posSatellite, posSurface, posBeam, L1);
 printf("beamPoint: Fi=%8.5f, La=%8.5f, H =%8.5f\n", posBeam.lat*180./PI, posBeam.lon*180./PI, posBeam.alt );

//Sun terminator
printf("Find Points of Sun Terminstor for LONGITUDES=-180...180(10) deg\n");
 for(termLon=-180.*PI/180.; termLon<180.*PI/180.; termLon+=10.*PI/180.)
 {
  termLat = sunTerminatorLat(2010, 1, 27, 10, 34, 10, termLon);
  printf("sunTermLon = %8.5f\n", termLon*180./PI);
  printf("sunTermLat = %8.5f\n", termLat*180./PI);
 }
}



int main( int argc, char* argv[] )
{
  testDayDuration();
  testCoordSys();
//  QApplication ap( argc, argv );
//  test_geoDistance();
//  testFrameForQuad( ap );
//  test_TransCoords();
  
//  test_sunH();
  return 0;
}
