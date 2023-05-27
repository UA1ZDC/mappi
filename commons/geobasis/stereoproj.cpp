#include "stereoproj.h"

#include <qmath.h>

using namespace MnMath;

namespace meteo {

const float kMinLat = -1.0f*12.01f*DEG2RAD;

StereoProj::StereoProj()
  : Projection()
{
  type_ = STEREO;
  projRadius = 2.0*kRVS;
  setMapCenter( GeoPoint::fromDegree( 90.0, 0.0 ) );
  initRamka();
}

StereoProj::StereoProj( const GeoPoint& center )
  : Projection( center )
{
  type_ = STEREO;
  projRadius = 2.0*kRVS;
  setMapCenter( center );
  initRamka();
}

StereoProj::~StereoProj()
{
}

bool StereoProj::F2X(const GeoVector& dPt, QVector<QPolygon>* apoint, bool isClosed ) const
{
  Q_UNUSED(isClosed);
  if ( dPt.isEmpty() ) {
    return true;
  }


  QPolygon scrPt;
  QPoint pnt;
  GeoPoint* parray = const_cast<GeoPoint*>(dPt.data()+1);
  QPoint oldpnt;
  GeoPoint gp;
  GeoPoint oldgp;
  if ( 1 == dPt.size() ) {
    if ( false == F2X_one( dPt[0], &pnt ) ) {
      return false;
    }
    scrPt.append(pnt);
    apoint->append(scrPt);
    return true;
  }
  bool has_inhemisphere = false;
  for ( int i = 1, size = dPt.size(); i < size; ++i, ++parray ) {
    const GeoPoint& gp = *(parray-1);
    if ( meteo::Northern == hemisphere_ ) {
      if ( gp.lat() > startFi() ) {
        has_inhemisphere = true;
        break;
      }
    }
    else {
      if ( gp.lat() < startFi() ) {
        has_inhemisphere = true;
        break;
      }
    }
  }
  if ( false == has_inhemisphere ) {
    return false;
  }
  parray = const_cast<GeoPoint*>(dPt.data()+1);
  for ( int i = 1, size = dPt.size(); i < size; ++i, ++parray ) {
    const GeoPoint& oldgp = *(parray-1);
    const GeoPoint& gp = *parray;

    if ( false == F2X_one( oldgp, &pnt ) ) {
      continue;
    }
    if ( pnt != oldpnt ) {
      scrPt.append(pnt);
      oldpnt = pnt;
    }
    if ( i+1 == size ) { //добавить последнюю точку
      if ( false == F2X_one( gp, &pnt ) ) {
        continue;
      }
      if ( pnt != oldpnt ) {
        scrPt.append(pnt);
        oldpnt = QPoint();
      }
      if ( false == scrPt.isEmpty() ) {
        apoint->append(scrPt);
      }
      break;
    }
  }
  return true;
}

inline bool StereoProj::X2F_one(const QPoint& meterCoord, GeoPoint* geoCoord ) const
{
  float x = meterCoord.y();
  float y = meterCoord.x();
  if ( hemisphere_ == Southern )
    y = -y;

  float r = sqrtf( x*x + y*y );
  float lon = atan2f( y, x ) + centerMap_.lon();
  float lat = M_PI_2f- 2.0f* atan2f( r, projRadius );
  if ( MnMath::isZero( r ) ) {
    lon = 0.0;
  }
  lat *= hemisphere_;
  geoCoord->setLat(lat);
  geoCoord->setLon(lon);
  return true;
}

inline bool StereoProj::X2F_one(const QPointF& meterCoord, GeoPoint* geoCoord ) const
{
  float x = meterCoord.y();
  float y = meterCoord.x();
  if ( hemisphere_ == Southern )
    y = -y;

  float r = sqrtf( x*x + y*y );
  float lon = atan2f( y, x ) + centerMap_.lon();
  float lat = M_PI_2f- 2.0f* atan2f( r, projRadius );
  if ( MnMath::isZero( r ) ) {
    lon = 0.0;
  }
  lat *= hemisphere_;
  geoCoord->setLat(lat);
  geoCoord->setLon(lon);
  return true;
}

inline bool StereoProj::F2X_one( const GeoPoint& geoCoord, QPoint* meterCoord ) const
{
  float lon = geoCoord.lon() - centerMap_.lon();
  float lat = hemisphere_*geoCoord.lat();
 // float r = projRadius * tan( ( M_PI_2-lat )*0.5 );
  float angle = ( M_PI_2f-lat )*0.5f;
  float r =  sinf( angle );
  r /= cosf(angle);
  r*=projRadius;
  //  float dX = r * cos(lon);
  //  float dY = r * sin(lon);
  float dX = r * cosf(lon);
  float dY = r * sinf(lon);
  int x = ( dX +0.5f );
  int y = ( dY +0.5f );
  if ( 0 > x && ::abs(x) == x ) {
    return false;
  }
  if ( 0 > y && ::abs(y) == y ) {
    return false;
  }
  if ( hemisphere_ == Southern ) {
    y = -y;
  }
  meterCoord->setY(x);
  meterCoord->setX(y);
  return true;
}

inline bool StereoProj::F2X_one( const GeoPoint& geoCoord, QPointF* meterCoord ) const
{
  float lon = geoCoord.lon() - centerMap_.lon();
  float lat = hemisphere_*geoCoord.lat();
  //для проверки
  float angle = ( M_PI_2f-lat )*0.5f;
  float r =  sinf( angle );
  r /= cosf(angle);
  r*=projRadius;
  //float r = projRadius * tan( ( M_PI_2-lat )*0.5 );

//  float x = r * cos(lon);
//  float y = r * sin(lon);
  float x = r * cosf(lon);
  float y = r * sinf(lon);
//  int x = ( dX +0.5 );
//  int y = ( dY +0.5 );
//  if ( 0 > x && ::abs(x) == x ) {
//    return false;
//  }
//  if ( 0 > y && ::abs(y) == y ) {
//    return false;
//  }
  if ( hemisphere_ == Southern ) {
    y = -y;
  }
  meterCoord->setY(x);
  meterCoord->setX(y);

  return true;
}

void StereoProj::setMapCenter( const GeoPoint& gpnt )
{
  GeoPoint ncnt(gpnt);
  if ( 0.0f <= ncnt.lat() ) {
    hemisphere_ = Northern;
  }
  else {
    hemisphere_ = Southern;
  }
  ncnt.setLat( hemisphere_*M_PI_2f );
  Projection::setMapCenter(ncnt);
}

bool StereoProj::checkPolush(int map)
{
  if (( (hemisphere_==meteo::Northern) && (map > 40)) ||
        (( hemisphere_==meteo::Southern) && ((map<31)||(map>61))))  {
    return false;
  }
  return true;
}

GeoPoint StereoProj::getProjCenter() const
{
  return GeoPoint( M_PI_4f*hemisphere(), centerMap_.lon() );
}

float StereoProj::startFi() const {
 if ( meteo::Northern == hemisphere_ ) {
    return -12.0f*DEG2RAD;
 }
 return 12.0f*DEG2RAD;

}

float StereoProj::startLa() const
{
  return centerMap_.lon();
}

float StereoProj::endFi() const {
 if ( meteo::Northern == hemisphere_ ) {
    return 89.0f*DEG2RAD;
 }
 return -89.0f*DEG2RAD;
}

float StereoProj::endLa() const
{
  return centerMap_.lon();
}

void StereoProj::setStart( const GeoPoint& gp )
{
  Q_UNUSED(gp);
}

void StereoProj::setEnd( const GeoPoint& gp )
{
  Q_UNUSED(gp);
}

void StereoProj::setStartFi( float lat )
{
  Q_UNUSED(lat);
}

void StereoProj::setStartLa( float lon )
{
  Q_UNUSED(lon);
}

void StereoProj::setEndFi( float lat )
{
  Q_UNUSED(lat);
}

void StereoProj::setEndLa( float lon )
{
  Q_UNUSED(lon);
}

void StereoProj::setHemisphere( Hemisphere hs )
{
  hemisphere_ = hs;
  centerMap_ = GeoPoint( hemisphere_*M_PI_2f, centerMap_.lon() );
}

void StereoProj::setNorthern()
{
  setHemisphere( Northern );
}

void StereoProj::setSouthern()
{
  setHemisphere( Southern );
}

Projection* StereoProj::copy() const
{
  StereoProj* s = new StereoProj;
  return copyTo(s);
}

void StereoProj::initRamka()
{
  ramka_.clear();
  float lat = startFi();
  int beglon = 180;
  int endlon = -181;
  int step = -1;
  if ( meteo::Southern == hemisphere_ ) {
    beglon = -beglon;
    endlon = -endlon;
    step = 1;
  }
  for ( int i = beglon; i != endlon; i += step ) {
    ramka_.append( GeoPoint( lat, i*DEG2RAD ) );
  }
}

bool StereoProj::isInMap(const GeoPoint& gp){
  float fi = gp.fiDeg();
  if ( meteo::Northern == hemisphere_ ) {
    return ((-12.0f <= fi) && (90.0f >= fi));
  }
  return ((12.0f >= fi) && (-90.f <= fi));
  
}

}

