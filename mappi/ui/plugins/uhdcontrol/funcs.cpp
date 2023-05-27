#include "funcs.h"

#include <cross-commons/debug/tlog.h>

#include <commons/obanal/tfield.h>
#include <commons/geobasis/projection.h>

namespace meteo {

bool graphLinearF2X_one( const Projection& proj, const GeoPoint& geoCoord, QPoint* meterCoord )
{
  double lon = geoCoord.lon() - proj.getMapCenter().lon();
  double lat = geoCoord.lat() - proj.getMapCenter().lat();

  double x = (kRVSG)*(lat*proj.xfactor());
  double y = (kRVSG)*(lon*proj.yfactor());
  meterCoord->setX(x);
  meterCoord->setY(y);

  if ( 0 > x && ::abs(x) == x ) {
    return false;
  }
  if ( 0 > y && ::abs(y) == y ) {
    return false;
  }
  return true;
}

bool graphLinearX2F_one( const Projection& proj, const QPoint& meterCoord, GeoPoint* geoCoord )
{
  if ( 0 == geoCoord ) {
    error_log << QObject::tr("Нулевой указатель GeoPoint*");
    return false;
  }
  *geoCoord = GeoPoint( 0,0,0, LA_GENERAL );

  double x = meterCoord.x();
  double y = meterCoord.y();
  double lat = proj.getMapCenter().lat() + x/( kRVSG*proj.xfactor() );
  double lon = proj.getMapCenter().lon() + y/(kRVSG*proj.yfactor() );
  geoCoord->setLat(lat);
  geoCoord->setLon(lon);

  return true;
}

bool graphLinearF2X_onef( const Projection& proj, const GeoPoint& geoCoord, QPointF* meterCoord )
{
  double lon = geoCoord.lon() - proj.getMapCenter().lon();
  double lat = geoCoord.lat() - proj.getMapCenter().lat();

  double x = kRVSG*lat*proj.xfactor();
  double y = kRVSG*(lon*proj.yfactor());
  meterCoord->setX(x);
  meterCoord->setY(y);

  return true;
}

bool graphLinearX2F_onef( const Projection& proj, const QPointF& meterCoord, GeoPoint* geoCoord )
{
  if ( 0 == geoCoord ) {
    error_log << QObject::tr("Нулевой указатель GeoPoint*");
    return false;
  }
  *geoCoord = GeoPoint( 0,0,0, LA_GENERAL );

  double x = meterCoord.x();
  double y = meterCoord.y();
  double lat = proj.getMapCenter().lat() + x/(kRVSG*proj.xfactor() );
  double lon = proj.getMapCenter().lon() + y/(kRVSG*proj.yfactor() );
  geoCoord->setLat(lat);
  geoCoord->setLon(lon);

  return true;
}

}
