#include "obliquemerkator.h"

namespace meteo {

static const float kRadius = kRVS;

ObliqueMerkat::ObliqueMerkat()
  : Projection()
{
  type_ = OBLIQUEMERKAT;
}

ObliqueMerkat::~ObliqueMerkat()
{
}
//
//void ObliqueMerkat::setStartEnd( const GeoPoint& s, const GeoPoint& e )
//{
//  if ( s.lon() < e.lon() ) {
//    start_ = s;
//    end_ = e;
//  }
//  else {
//    start_ = e;
//    end_ = s;
//  }
//  float sin_fi0 = ::sin( start_.lat() );
//  float cos_fi0 = ::cos( start_.lat() );
//  float sin_fi1 = ::sin( end_.lat() );
//  float cos_fi1 = ::cos( end_.lat() );
//  float sin_la0 = ::sin( start_.lon() );
//  float cos_la0 = ::cos( start_.lon() );
//  float sin_la1 = ::sin( end_.lon() );
//  float cos_la1 = ::cos( end_.lon() );
//
//  float lon_p = ::atan2( ( cos_fi0*sin_fi1*cos_la0 - sin_fi0*cos_fi1*cos_la1 ), ( sin_fi0*cos_fi1*sin_la1 - cos_fi0*sin_fi1*sin_la0 ) );
//  float lat_p = ::atan( -::cos( lon_p - start_.lon() )/::tan( start_.lat() ) );
//  pole_ = GeoPoint( lat_p, lon_p );
//  sin_fip_ = ::sin( pole_.lat() );
//  cos_fip_ = ::cos( pole_.lat() );
//  origin_.setLat(0.0);
//  origin_.setLon( lon_p + M_PI_2 );
//}

void ObliqueMerkat::setMapCenter( const GeoPoint& cntr, float azimuth )
{
  centerMap_ = cntr;
  azimuth_ = azimuth;
  float lat_p = ::asin( ::cos( cntr.lat() )*::sin(azimuth) );
  float lon_p = ::atan2( -::cos(azimuth), -::sin( cntr.lat() )*::sin(azimuth) ) + cntr.lon();
  pole_ = GeoPoint( lat_p, lon_p );
  sin_fip_ = ::sin( pole_.lat() );
  cos_fip_ = ::cos( pole_.lat() );
  origin_.setLat(0.0);
  origin_.setLon( lon_p + M_PI_2 );
  start_ = GeoPoint( M_PI_2, centerMap_.lon() - M_PI );
  end_ = GeoPoint( -M_PI_2, centerMap_.lon() + M_PI );
  initRamka();
}

bool ObliqueMerkat::F2X(const GeoVector& dPt, QVector<QPolygon>* apoint, bool isClosed ) const
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

bool ObliqueMerkat::F2X_one(const GeoPoint& geoCoord, QPoint* meterCoord ) const
{
  float sin_delta_lon = ::sin( geoCoord.lon() - origin_.lon() );
  float cos_delta_lon = ::cos( geoCoord.lon() - origin_.lon() );
  float A = sin_fip_*::sin( geoCoord.lat() ) - cos_fip_*::cos( geoCoord.lat() )*sin_delta_lon;
  float xd = (kRadius)*::atan2( ::tan( geoCoord.lat() )*cos_fip_ + sin_fip_*sin_delta_lon,cos_delta_lon );
  float yd = (kRadius)/2.0*::log( (1.0 + A)/(1.0 - A) );
  int x = MnMath::ftoi_norm(xd);
  if ( 0 > x && ::abs(x) == x ) {
    return false;
  }
  int y = MnMath::ftoi_norm(yd);
  if ( 0 > y && ::abs(y) == y ) {
    return false;
  }
  meterCoord->setX(x);
  meterCoord->setY(y);
  return true;
}

bool ObliqueMerkat::X2F_one(const QPoint& meterCoord, GeoPoint* geoCoord ) const
{
  float x_div_R = meterCoord.x()/(kRadius);
  float y_div_R = meterCoord.y()/(kRadius);
  float exp_y_div_R = ::exp(y_div_R);
  float exp_minus_1 = ::exp(-y_div_R);
  float sinh = (exp_y_div_R - exp_minus_1)/2.0;
  float cosh = (exp_y_div_R + exp_minus_1)/2.0;
  float tanh = sinh/cosh;
  float fi= ::asin( sin_fip_*tanh + cos_fip_*::sin(meterCoord.x()/(kRadius))/cosh );
  float la = origin_.lon() + ::atan2( sin_fip_*::sin(x_div_R) - cos_fip_*sinh,::cos(x_div_R) );
  while ( fi > M_PI_2 ) {
    fi -= M_PI;
  }
  while ( fi < -M_PI_2 ) {
    fi += M_PI;
  }
  while ( la > M_PI ) {
    la -= M_PI*2;
  }
  while ( la < -M_PI ) {
    la += M_PI*2;
  }
  geoCoord->setLat(fi);
  geoCoord->setLon(la);
  return true;
}

bool ObliqueMerkat::F2X_one(const GeoPoint& geoCoord, QPointF* meterCoord ) const
{
  float sin_delta_lon = ::sin( geoCoord.lon() - origin_.lon() );
  float cos_delta_lon = ::cos( geoCoord.lon() - origin_.lon() );
  float A = sin_fip_*::sin( geoCoord.lat() ) - cos_fip_*::cos( geoCoord.lat() )*sin_delta_lon;
  float x = (kRadius)*::atan2( ::tan( geoCoord.lat() )*cos_fip_ + sin_fip_*sin_delta_lon, cos_delta_lon );
  float y = (kRadius)/2.0*::log( (1.0 + A)/(1.0 - A) );
  meterCoord->setX(x);
  meterCoord->setY(y);
  return true;
}

bool ObliqueMerkat::X2F_one(const QPointF& meterCoord, GeoPoint* geoCoord ) const
{
  float x_div_R = meterCoord.x()/(kRadius);
  float y_div_R = meterCoord.y()/(kRadius);
  float exp_y_div_R = ::exp(y_div_R);
  float exp_minus_1 = ::exp(-y_div_R);
  float sinh = (exp_y_div_R - exp_minus_1)/2.0;
  float cosh = (exp_y_div_R + exp_minus_1)/2.0;
  float tanh = sinh/cosh;
  float fi = ::asin( sin_fip_*tanh + cos_fip_*::sin(meterCoord.x()/(kRadius))/cosh );
  float la = origin_.lon() + ::atan2( sin_fip_*::sin(x_div_R) - cos_fip_*sinh, ::cos(x_div_R) );
  while ( fi > M_PI_2 ) {
    fi -= M_PI;
  }
  while ( fi < -M_PI_2 ) {
    fi += M_PI;
  }
  while ( la > M_PI ) {
    la -= M_PI*2;
  }
  while ( la < -M_PI ) {
    la += M_PI*2;
  }
  geoCoord->setLat(fi);
  geoCoord->setLon(la);
  return true;
}

Projection* ObliqueMerkat::copy() const
{
  ObliqueMerkat* p = new ObliqueMerkat;
  p->setMapCenter( centerMap_, azimuth_ );
  return p;
}

void ObliqueMerkat::initRamka()
{
  float delta = 5.0*DEG2RAD;
  ramka_.clear();
  GeoPoint top = centerMap_.findSecondCoord( M_PI_2-delta, azimuth_ );
  GeoPoint bottom = centerMap_.findSecondCoord( M_PI_2-delta, azimuth_+M_PI );
  GeoPoint left= centerMap_.findSecondCoord( M_PI-delta, azimuth_ - M_PI_2 );
  GeoPoint right= centerMap_.findSecondCoord( M_PI-delta, azimuth_ + M_PI_2 );
  GeoPoint lefttop = GeoPoint( top.lat(), left.lon() );
  GeoPoint righttop = GeoPoint( top.lat(), right.lon() );
  GeoPoint leftbot = GeoPoint( bottom.lat(), left.lon() );
  GeoPoint rightbot = GeoPoint( bottom.lat(), right.lon() );
  ramka_.append(lefttop);
  ramka_.append(leftbot);
  ramka_.append(rightbot);
  ramka_.append(righttop);
  ramka_.append(lefttop);

  QVector<QPolygon> poly;
  F2X(ramka_, &poly, true);
}

}
