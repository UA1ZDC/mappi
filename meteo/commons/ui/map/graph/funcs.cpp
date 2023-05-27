#include "funcs.h"

#include <cross-commons/debug/tlog.h>

#include <commons/obanal/tfield.h>
#include <commons/geobasis/projection.h>

namespace meteo {

bool graphLinearF2X_one( const Projection& proj, const GeoPoint& geoCoord, QPoint* meterCoord )
{
  if ( 0 == meterCoord ) { return false; }

  float lat = geoCoord.lat() - proj.getMapCenter().lat();
  float lon = geoCoord.lon() - proj.getMapCenter().lon();

  float x = kRVSG*lat*proj.xfactor();
  float y = kRVSG*lon*proj.yfactor();
  meterCoord->setX(x);
  meterCoord->setY(-y);
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
  if ( 0 == geoCoord ) { return false; }

  *geoCoord = GeoPoint( 0,0,0, LA_GENERAL );

  float x = meterCoord.x();
  float y = meterCoord.y();
  float lat = proj.getMapCenter().lat() + x/(kRVSG*proj.xfactor());
  float lon = proj.getMapCenter().lon() + y/(kRVSG*proj.yfactor());
  geoCoord->setLat(lat);
  geoCoord->setLon(-lon);

  return true;
}

bool graphLinearF2X_onef( const Projection& proj, const GeoPoint& geoCoord, QPointF* meterCoord )
{
  if ( 0 == meterCoord ) { return false; }

  float lon = geoCoord.lon() - proj.getMapCenter().lon();
  float lat = (geoCoord.lat() - proj.getMapCenter().lat());

  float x = kRVSG*lat*proj.xfactor();
  float y = kRVSG*lon*proj.yfactor();
  meterCoord->setX(x );
  meterCoord->setY(-y);

  return true;
}

bool graphLinearX2F_onef( const Projection& proj, const QPointF& meterCoord, GeoPoint* geoCoord )
{
  if ( 0 == geoCoord ) { return false; }

  *geoCoord = GeoPoint( 0,0,0, LA_GENERAL );

  float x = meterCoord.x();
  float y = meterCoord.y();
  float lat = proj.getMapCenter().lat() + x/(kRVSG*proj.xfactor());
  float lon = proj.getMapCenter().lon() + y/(kRVSG*proj.yfactor());
  geoCoord->setLat(lat);
  geoCoord->setLon(-lon);

  return true;
}

} // meteo
