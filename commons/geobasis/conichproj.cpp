#include "conichproj.h"

#include <qmap.h>
#include <qmath.h>

using namespace MnMath;

namespace meteo {

ConichProj::ConichProj()
  : Projection()
{
  type_ = CONICH;
  projRadius = kRVS;

  setMapCenter( GeoPoint( 0.0, 0.0 ) );
  initRamka();
}

ConichProj::ConichProj( const GeoPoint& center )
  : Projection( center )
{
  type_ = CONICH;
  projRadius = kRVS;
  setMapCenter(center);
  initRamka();
}


ConichProj::~ConichProj()
{
}

bool ConichProj::isEqual( const Projection& proj ) const
{
  const ConichProj* p = projection_cast<const ConichProj*>(&proj);
  if ( nullptr == p ) {
    return false;
  }
  if ( phi0_     != p->phi0_
    || phi1_     != p->phi1_
    || phi2_     != p->phi2_
    || conich_n_ != p->conich_n_
    || conich_c_ != p->conich_c_ ) {
    return false;
  }
  return Projection::isEqual(proj);
}

void ConichProj::initRamka()
{
  ramka_.clear();
  float lat = startFi();
  int beglon = 170;
  int endlon = -170;
  int step = -1;
  if ( meteo::Southern == hemisphere_ ) {
    beglon = -beglon;
    endlon = -endlon;
    step = 1;
  }
  ramka_.append( GeoPoint(M_PI_2*hemisphere_, centerMap_.lon() ) );
  for ( int i = beglon; i != endlon; i += step ) {
    float curlon = i*DEG2RAD + centerMap_.lon();
    if ( curlon > M_PIf ) {
      curlon = fmodf( curlon, M_PIf ) - M_PIf;
    }
    if ( curlon < -M_PIf ) {
      curlon = fmodf( curlon, M_PIf ) + M_PIf;
    }
//    while ( curlon > M_PIf ) {
//      curlon -= M_PIf*2;
//    }
//    while ( curlon < -M_PIf ) {
//      curlon += M_PIf*2;
//    }
    ramka_.append( GeoPoint( lat, curlon ) );
  }
  ramka_.append( GeoPoint(M_PI_2f*hemisphere_, centerMap_.lon() ) );
}

int ConichProj::countPolygonEntry(const QPolygon& poly, QList<QPolygon> polyList) const
{
  int counter = 0;

  float beginY = poly.first().y();
  float endY   = poly.last().y();
  if( beginY > endY ){
    beginY = endY;
    endY   = poly.first().y();
  }

  foreach( const QPolygon& p, polyList ){
    float firstY = p.first().y();
    float  lastY  = p.last().y();
    if( firstY > lastY ){
      firstY = lastY;
      lastY = p.first().y();
    }

    if( beginY > firstY && beginY < lastY && endY > firstY && endY < lastY ){
      ++counter;
    }
  }

  return counter;
}

QPolygon ConichProj::combineIntersectPolygons(const QPolygon& base, const QList<QPolygon>& polyList) const
{
  QPolygon result = base;

  float beginY = base.first().y();
  float endY   = base.last().y();
  if( beginY > endY ){
    beginY = endY;
    endY   = base.first().y();
  }

  for( const QPolygon& p : polyList ){
    float firstY = p.first().y();
    float lastY  = p.last().y();
    if( firstY > lastY ){
      firstY = lastY;
      lastY = p.first().y();
    }

    if( beginY < firstY && beginY < lastY && endY > firstY && endY > lastY ){
      for( const QPoint& point : p ){
        result.prepend( point );
      }
    }
  }

  return result;
}

bool ConichProj::F2X( const GeoVector& srcGeoVector, QVector<QPolygon>* resultPolygonVector ) const
{
  if( srcGeoVector.isEmpty() ){
    return true;
  }
  bool vhodit = false;
  for ( int i =0, sz = srcGeoVector.size(); i < sz; ++i ) {
    const GeoPoint& geoCoord = srcGeoVector[i];
    if ( Southern == hemisphere() && geoCoord.lat() <= startFi() ) {
      vhodit = true;
    }
    else if ( Northern == hemisphere() && geoCoord.lat() >= startFi() ) {
      vhodit = true;
    }
  }
  if ( false == vhodit ) {
    return false;
  }

  QMap<int,int> gapYList; // idx, val

  QPoint resultPoint;
  QPolygon srcPolygon;

  GeoPoint* srcPoints = const_cast<GeoPoint*>(srcGeoVector.data());

  bool res = false;

  double midlon = projRadius*M_PI_2;

  QPoint oldpoint;
  for( int i = 0, size = srcGeoVector.size(); i < size; ++i, ++srcPoints ) {
    if( F2X_one( *srcPoints, &resultPoint ) ){
      res = true;

      if( !srcPolygon.isEmpty() )
      {
        QPoint endPoint = srcPolygon[srcPolygon.size()-1];

        if(
           (endPoint.x() < -midlon && resultPoint.x() >  midlon) ||
           (endPoint.x() >  midlon && resultPoint.x() < -midlon)
          )
        {
          QPoint gapPoint;
          float y = (endPoint.y() + resultPoint.y()) / 2;
//          int y = (endPoint.y() + resultPoint.y());// >> 1;
          gapPoint.setY(y);

          double x = resultPoint.x();

          gapPoint.setX( ftoi_norm( M_PIf*projRadius) );
          if( 0 < x ){
            gapPoint.setX( -gapPoint.x() );
          }
          srcPolygon.append(gapPoint);

          gapPoint.setX( -gapPoint.x() );
          srcPolygon.append(gapPoint);

          gapYList.insert(srcPolygon.size()-1, gapPoint.y());
          gapYList.insert(srcPolygon.size()-2, gapPoint.y());

          continue;
        }
      }
      if ( oldpoint != resultPoint ) {
        srcPolygon.append(resultPoint);
        oldpoint = resultPoint;
      }
    }
  }

  if( gapYList.isEmpty() )  {
    resultPolygonVector->append(srcPolygon);
    return res;
  }

  QPolygon lPoly;
  QPolygon rPoly;

  int lastGapIndex = gapYList.keys().last();

  QPolygon* poly = nullptr;

  QList<QPolygon> rPolyList;
  QList<QPolygon> lPolyList;

  int j = lastGapIndex;
  bool isHasBeginGapPoint = false;
  bool isHasEndGapPoint   = false;
  // separate poly by left and right side
  while( true ){
    // first gap point
    if( nullptr == poly && gapYList.keys().contains(j) ){
      isHasBeginGapPoint = true;
      isHasEndGapPoint   = false;

      poly = ( srcPolygon.at(j).x() > 0 ) ? &rPoly : &lPoly;
      poly->append(srcPolygon.at(j));
    }
    // next gap point
    else if ( nullptr != poly && gapYList.keys().contains(j) ){
      if( isHasBeginGapPoint ){
        isHasEndGapPoint = true;
      }
      else {
        isHasBeginGapPoint = true;
      }

      poly->append(srcPolygon.at(j));

      // add poly to list
      if( isHasEndGapPoint ){
        if( poly == &rPoly ){
          rPolyList.append( *poly );
        }
        else {
          lPolyList.append( *poly );
        }

        poly->clear();
      }

      // switch poly
      if( isHasEndGapPoint ){
        if( poly == &rPoly ){
          poly = &lPoly;
        }
        else {
          poly = &rPoly;
        }

        isHasBeginGapPoint = false;
        isHasEndGapPoint   = false;
      }
    }
    // next point
    else {
      poly->append(srcPolygon.at(j));
    }

    ++j;

    if( j == srcPolygon.size() ){
      j=0;
    }
    if( j == lastGapIndex ){
      break;
    }
  } // while


  QMap<int,QList<QPolygon> > lLevelMap;
  // separate left side by level
  foreach( const QPolygon& p, lPolyList ){
    int n = countPolygonEntry( p, lPolyList );
    lLevelMap[n].append( p );
  }

  QMap<int,QList<QPolygon> > rLevelMap;
  // separate right side by level
  foreach( const QPolygon& p, rPolyList ){
    int n = countPolygonEntry( p, rPolyList );
    rLevelMap[n].append( p );
  }

  // combine levels of pairwise (left)
  int i=0;
  while( !lLevelMap.value( i ).isEmpty() ){
    QList<QPolygon> curList = lLevelMap.value( i );

    QPolygon res;
    foreach( const QPolygon& p, curList ){
      res.clear();
      res = combineIntersectPolygons( p, lLevelMap.value( i+1 ) );
      resultPolygonVector->append( res );
    }

    i+=2;
  }

  // combine levels of pairwise (right)
  i=0;
  while( !rLevelMap.value( i ).isEmpty() ){
    QList<QPolygon> curList = rLevelMap.value( i );

    QPolygon res;
    foreach( const QPolygon& p, curList ){
      res.clear();
      res = combineIntersectPolygons( p, rLevelMap.value( i+1 ) );
      resultPolygonVector->append( res );
    }

    i+=2;
  }

  return res;
}

bool ConichProj::F2X(const GeoVector& srcGeoVector, QVector<QPolygon>* resultPolygonVector, bool isClosed) const
{
  if( isClosed ){
    return F2X(srcGeoVector, resultPolygonVector);
  }
  bool vhodit = false;
  for ( int i =0, sz = srcGeoVector.size(); i < sz; ++i ) {
    const GeoPoint& geoCoord = srcGeoVector[i];
    if ( Southern == hemisphere() && geoCoord.lat() <= startFi() ) {
      vhodit = true;
    }
    else if ( Northern == hemisphere() && geoCoord.lat() >= startFi() ) {
      vhodit = true;
    }
  }
  if ( false == vhodit ) {
    return false;
  }

  if( srcGeoVector.isEmpty() ){
    return true;
  }

  QPoint resultPoint;
  QPolygon srcPolygon;

  GeoPoint* srcPoints = const_cast<GeoPoint*>(srcGeoVector.data());

  bool res = false;

  double midlon = projRadius*M_PI_2;

  QPoint oldpoint;
  for( int i = 0, size = srcGeoVector.size(); i < size; ++i, ++srcPoints ){
    if( F2X_one( *srcPoints, &resultPoint ) ){
      res = true;

      if( !srcPolygon.isEmpty() )
      {
        QPoint endPoint = srcPolygon[srcPolygon.size()-1];

        if( (endPoint.x() < -midlon && resultPoint.x() > midlon ) )
        {
          QPoint gapPoint;
          float y = (endPoint.y() + resultPoint.y()) / 2;
//          int y = (endPoint.y() + resultPoint.y()) >> 1;
          gapPoint.setY(y);

          float x = resultPoint.x();

          // first gap point
          gapPoint.setX( ftoi_norm( M_PIf*projRadius ) );
          if( 0 < x ){
            gapPoint.setX( -gapPoint.x() );
          }
          srcPolygon.append(gapPoint);
          resultPolygonVector->append(srcPolygon);
          srcPolygon.clear();

          // second gap point
          gapPoint.setX( -gapPoint.x() );
          srcPolygon.append(gapPoint);

          continue;
        }
        else if( ( endPoint.x() > midlon && resultPoint.x() < -midlon ) ){
          QPoint gapPoint;
          float y = (endPoint.y() + resultPoint.y()) / 2;
          //int y = (endPoint.y() + resultPoint.y())/2;// >>1;
          gapPoint.setY(y);

          float x = resultPoint.x();

          // fisrt gap point
          gapPoint.setX( ftoi_norm( M_PIf*projRadius ) );
          if( 0 < x ){
            gapPoint.setX( -gapPoint.x() );
          }
          srcPolygon.append(gapPoint);
          resultPolygonVector->append(srcPolygon);
          srcPolygon.clear();

          // second gap point
          gapPoint.setX( -gapPoint.x() );
          srcPolygon.append(gapPoint);

          continue;
        }
      }
      if ( oldpoint != resultPoint ) {
        srcPolygon.append(resultPoint);
        oldpoint = resultPoint;
      }
    }
  }

  if( !srcPolygon.isEmpty() && true == res ) {
    resultPolygonVector->append(srcPolygon);
  }

  return res;
}

inline bool ConichProj::F2X_one( const GeoPoint& geoCoord, QPoint* meterCoord ) const
{
  float rho;
  if ( true == MnMath::isZero( ::fabs( geoCoord.lat() - M_PI_2f ) ) ) {
    rho = 0.;
  }
  else {
    rho = conich_c_*::pow(::tan(M_PI_4f + 0.5f*geoCoord.lat()), -conich_n_ );
  }
  float lon = geoCoord.lon()-centerMap_.lon();
  if ( lon > M_PIf ) {
    lon = fmodf( lon, M_PIf ) - M_PIf;
  }
  else if ( lon < -M_PIf ) {
    lon = fmodf( lon, M_PIf ) + M_PIf;
  }
  float dX = projRadius*(rho*::sinf( lon*conich_n_ ) );
  float dY = projRadius*( 0 - rho*cosf( lon*conich_n_ ) );
  int x = ( dX +0.5f );
  int y = ( dY +0.5f );
  if ( 0 > x && ::abs(x) == x ) {
    return false;
  }
  if ( 0 > y && ::abs(y) == y ) {
    return false;
  }
  meterCoord->setX(x*hemisphere_);
  meterCoord->setY(-y*hemisphere_);
  return true;
}

inline bool ConichProj::F2X_one( const GeoPoint& geoCoord, QPointF* meterCoord ) const
{
  float rho;
  if ( true == MnMath::isZero( ::fabs( geoCoord.lat() - M_PI_2f ) ) ) {
    rho = 0.;
  }
  else {
    rho = conich_c_*::pow(::tan( M_PI_4f + 0.5f*geoCoord.lat() ), -conich_n_ );
  }
  float lon = geoCoord.lon()-centerMap_.lon();
  while( lon < -M_PIf ) {
    lon += 2.0f*M_PIf;
  }
  while( lon > M_PIf ) {
    lon -= 2.0f*M_PIf;
  }
  float x = projRadius*(rho*sinf( lon*conich_n_ ) );
  float y = projRadius*( 0 - rho*cosf(lon*conich_n_ ) );
  meterCoord->setX(x*hemisphere_);
  meterCoord->setY(-y*hemisphere_);
  return true;
}

inline bool ConichProj::X2F_one(const QPoint& meterCoord, GeoPoint* geoCoord ) const
{
  float x = meterCoord.x()/projRadius*hemisphere_;
  float y = meterCoord.y()/projRadius*hemisphere_;
  float rho = ::hypot(x, -y);
  if ( true == MnMath::isZero(rho) ) {
    geoCoord->setLon(0.0);
    geoCoord->setLat(M_PI_2f*hemisphere_);
    return true;
  }
  if ( 0.0f > conich_n_ ) {
    rho = -rho;
    x = -x;
    y = -y;
  }
  geoCoord->setLat( 2.0f*::atanf(::powf(conich_c_/rho, 1.0f/conich_n_)) - M_PI_2f);
  geoCoord->setLon(::atan2f(x, y)/conich_n_ + centerMap_.lon());
  return true;
}

inline bool ConichProj::X2F_one(const QPointF& meterCoord, GeoPoint* geoCoord ) const
{
  float x = meterCoord.x()/projRadius*hemisphere_;
  float y = meterCoord.y()/projRadius*hemisphere_;
  float rho = ::hypot(x, -y);
  if ( true == MnMath::isZero(rho) ) {
    geoCoord->setLon(0.0);
    geoCoord->setLat(M_PI_2f*hemisphere_);
    return true;
  }
  if ( 0.0f > conich_n_ ) {
    rho = -rho;
    x = -x;
    y = -y;
  }
  geoCoord->setLat( 2.0f*::atanf(::powf(conich_c_/rho, 1.0f/conich_n_)) - M_PI_2f);
  geoCoord->setLon(::atan2f(x, y)/conich_n_ + centerMap_.lon());
  return true;
}

GeoPoint ConichProj::start() const
{
  return GeoPoint( startFi(), startLa() );
}

GeoPoint ConichProj::end() const
{
  return GeoPoint( endFi(), endLa() );
}

float ConichProj::startFi() const {
 if ( meteo::Northern == hemisphere_ ) {
    return -30.0f*DEG2RAD;
 }
 return 30.0f*DEG2RAD;

}

float ConichProj::startLa() const
{
  float startlon = centerMap_.lon() - M_PIf;
  while ( -M_PIf > startlon ) {
    startlon += 2*M_PIf;
  }
  return startlon;
}

float ConichProj::endFi() const
{
 if ( meteo::Northern == hemisphere_ ) {
    return 89.0f*DEG2RAD;
 }
 return -89.0f*DEG2RAD;
}

float ConichProj::endLa() const
{
  float endlon = centerMap_.lon() + M_PIf;
  while ( M_PIf < endlon ) {
    endlon -= 2*M_PIf;
  }
  return endlon;
}

void ConichProj::setMapCenter( const GeoPoint& gpnt )
{
  GeoPoint tpnt = gpnt;
  if ( 0.0f <= tpnt.lat() ) {
    hemisphere_ = Northern;
  }
  else {
    hemisphere_ = Southern;
  }
  tpnt.setLat( M_PI_2f*hemisphere_ );

  phi0_ = tpnt.lat();
  phi1_ = 20.0f*DEG2RAD*hemisphere_;
  phi2_ = 50.0f*DEG2RAD*hemisphere_;

  float cosphi, sinphi;
  conich_n_ = sinphi = ::sinf(phi1_);
  cosphi = ::cosf(phi1_);
  conich_n_ = ::logf( cosphi/::cosf(phi2_) )/::log( ::tanf(M_PI_4f + 0.5f*phi2_)/::tanf(M_PI_4f + 0.5f*phi1_) );
  conich_c_ = cosphi*::powf(::tanf(M_PI_4f + 0.5f*phi1_), conich_n_)/conich_n_;
  Projection::setMapCenter(tpnt);
}

GeoPoint ConichProj::getProjCenter() const
{
  return GeoPoint( M_PI_4f*hemisphere(), centerMap_.lon() );
}

void ConichProj::setStart( const GeoPoint& gp )
{
  Q_UNUSED(gp);
}

void ConichProj::setEnd( const GeoPoint& gp )
{
  Q_UNUSED(gp);
}

void ConichProj::setStartFi( float lat )
{
  Q_UNUSED(lat);
}

void ConichProj::setStartLa( float lon )
{
  Q_UNUSED(lon);
}

void ConichProj::setEndFi( float lat )
{
  Q_UNUSED(lat);
}

void ConichProj::setEndLa( float lon )
{
  Q_UNUSED(lon);
}

Projection* ConichProj::copy() const
{
  ConichProj* s = new ConichProj;
  s->phi0_     = phi0_;
  s->phi1_     = phi1_;
  s->phi2_     = phi2_;
  s->conich_n_ = conich_n_;
  s->conich_c_ = conich_c_;
  return copyTo(s);
}

}

