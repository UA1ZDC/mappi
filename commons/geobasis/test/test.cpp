#include <stdlib.h>

#include <qcoreapplication.h>
#include <qtextcodec.h>

#include <cross-commons/app/paths.h>
#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>
#include <commons/geobasis/projectionfunc.h>
#include <commons/geobasis/obliquemerkator.h>

void test_stereo_polar()
{
  meteo::ProjectionParams pr;
  pr.a = 6378137;
  pr.e = 0.08181919092890624;
  pr.la0 = 0;
  pr.fi0 = 1.57;
  pr.k0 = 0.994;
  pr.fe = 2000000;
  pr.fn = 2000000;

  double lat = 1.274090354;
  double lon = 0.767944871;
  double east, north;
  meteo::f2xy_stereo_polar(pr, lat, lon, &east, &north);
  printf("%f %f\n", east, north);

  //float fi1 = 6.02*M_PI / 180;
  // pr.k0 = (cos(fi1)/pow((1 - pow(pr.e,2)*sin(fi1)*sin(fi1)),0.5));
  // var(pr.k0);
  // meteo::f2xy_stereo_polar(pr, lat, lon, &east, &north);
  // printf("%f %f\n", east, north);

}

void testObliqueMerkat()
{
  meteo::GeoPoint gp1 = meteo::GeoPoint::fromDegree( 0,0 );
//  meteo::GeoPoint gp2 = meteo::GeoPoint::fromDegree( 40,35 );
  meteo::ObliqueMerkat om;
  om.setMapCenter( gp1, meteo::DEG2RAD*0 );
  QPoint scr;
  meteo::GeoPoint src = meteo::GeoPoint::fromDegree( 33, 33 );
  bool res = om.F2X_one( src, &scr );
  meteo::GeoPoint respoint;
  debug_log << "RES =" << res << "SRC =" << src << "SCR = " << scr;
  res = om.X2F_one( scr, &respoint );
  debug_log << "gp =" << respoint;
}

int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("test");
  QCoreApplication app( argc,argv);
  testObliqueMerkat();
//  test_stereo_polar();

  return EXIT_SUCCESS;
}
