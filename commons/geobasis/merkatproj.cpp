#include "merkatproj.h"

#include <qmap.h>
#include <qmath.h>

#include <cross-commons/debug/tlog.h>

using namespace MnMath;

namespace meteo {

MerkatProj::MerkatProj()
  : Projection()
{
  type_ = MERCAT;
  projRadius = kRVS;
  projRadius_1 = 1.f/projRadius;
  setMapCenter( GeoPoint( 0.0, 0.0 ) );
  initRamka();
}

MerkatProj::MerkatProj( const GeoPoint& center )
  : Projection( center )
{
  type_ = MERCAT;
  projRadius = kRVS;
  projRadius_1 = 1.f/projRadius;
  projRadius_MPI2 = projRadius * M_PI_2f;         //!< Радиус сферы (м)* M_PI_2

  setMapCenter(center);
  initRamka();
}


MerkatProj::~MerkatProj()
{
}

void MerkatProj::initRamka()
{
  ramka_.clear();
  float leftlon = centerMap_.lon() - M_PIf;
  while ( leftlon < -M_PIf ) {
    leftlon += k2PI;
  }
  //leftlon += 0.00000001;
  leftlon += 0.000001f;
  float rightlon = centerMap_.lon() + M_PIf;
  while ( rightlon > M_PIf ) {
    rightlon -= k2PI;
  }
  //rightlon -= 0.00000001;
  rightlon -= 0.000001f;
  float toplat = M_PI_2f-0.1f;
  float bottomlat = -M_PI_2f+0.1f;
  ramka_.append( GeoPoint( toplat, leftlon ) );
  ramka_.append( GeoPoint( toplat, rightlon ) );
  ramka_.append( GeoPoint( bottomlat, rightlon ) );
  ramka_.append( GeoPoint( bottomlat, leftlon ) );
  ramka_.append( GeoPoint( toplat, leftlon ) );
}

int MerkatProj::countPolygonEntry(const QPolygon& poly, QList<QPolygon> polyList) const
{
  int counter = 0;

  float beginY = poly.first().y();
  float endY   = poly.last().y();
  if( beginY > endY ){
    beginY = endY;
    endY   = poly.first().y();
  }

  for( const QPolygon& p : polyList ) {
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

QPolygon MerkatProj::combineIntersectPolygons(const QPolygon& base, const QList<QPolygon>& polyList) const
{
  QPolygon result = base;

  float beginY = base.first().y();
  float endY   = base.last().y();
  if( beginY > endY ){
    beginY = endY;
    endY   = base.first().y();
  }

  foreach( const QPolygon& p, polyList ){
    float firstY = p.first().y();
    float lastY  = p.last().y();
    if( firstY > lastY ){
      firstY = lastY;
      lastY = p.first().y();
    }

    if( beginY < firstY && beginY < lastY && endY > firstY && endY > lastY ){
      foreach( const QPoint& point, p ){
        result.prepend( point );
      }
    }
  }

  return result;
}

bool MerkatProj::F2X( const GeoVector& srcGeoVector, QVector<QPolygon>* resultPolygonVector ) const
{
  if( srcGeoVector.isEmpty() ){
    return true;
  }

  QMap<int,int> gapYList; // idx, val

  QPoint resultPoint;
  QPolygon srcPolygon;

  GeoPoint* srcPoints = const_cast<GeoPoint*>(srcGeoVector.data());

  bool res = false;

  float midlon = projRadius_MPI2;

  QPoint oldpoint(-999999999,-999999999);
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
          //float y = (endPoint.y() + resultPoint.y()) / 2;
          //gapPoint.setY(static_cast<int>(y));
          int y = (endPoint.y() + resultPoint.y()) >> 1;
          gapPoint.setY(y);

          float x = resultPoint.x();

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
  }


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

bool MerkatProj::F2X(const GeoVector& srcGeoVector, QVector<QPolygon>* resultPolygonVector, bool isClosed) const
{
  if( isClosed ){
    return F2X(srcGeoVector, resultPolygonVector);
  }

  if( srcGeoVector.isEmpty() ){
    return true;
  }

  QPoint resultPoint;
  QPolygon srcPolygon;

  GeoPoint* srcPoints = const_cast<GeoPoint*>(srcGeoVector.data());

  bool res = false;

  float midlon = projRadius_MPI2;

  QPoint oldpoint(-999999999,-999999999);
  for( int i = 0, size = srcGeoVector.size(); i < size; ++i, ++srcPoints ){
    if( F2X_one( *srcPoints, &resultPoint ) ){
      res = true;

      if( !srcPolygon.isEmpty() )
      {
        QPoint endPoint = srcPolygon[srcPolygon.size()-1];

        if( (endPoint.x() < -midlon && resultPoint.x() > midlon ) )
        {
          QPoint gapPoint;
          //int y = (endPoint.y() + resultPoint.y()) / 2.;
          //gapPoint.setY(static_cast<int>(y));
          int y = (endPoint.y() + resultPoint.y())>>1;
          gapPoint.setY(y);

          float x = resultPoint.x();

          // first gap point
          //gapPoint.setX( ftoi_norm( (M_PI-0.0*DEG2RAD)*projRadius ) );
          gapPoint.setX( ftoi_norm( (M_PIf)*projRadius ) );
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
         // float y = (endPoint.y() + resultPoint.y()) / 2.;
         // gapPoint.setY(static_cast<int>(y));
          int y = (endPoint.y() + resultPoint.y()) >>1;
          gapPoint.setY(y);

          float x = resultPoint.x();

          // fisrt gap point
          //gapPoint.setX( ftoi_norm( (M_PI-0.0*DEG2RAD)*projRadius ) );
          gapPoint.setX( ftoi_norm( (M_PIf)*projRadius ) );
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

inline bool MerkatProj::X2F_one(const QPoint& meterCoord, GeoPoint* geoCoord ) const {
  float lon = projRadius_1* meterCoord.x() + centerMap_.lon();
  if ( M_PIf + centerMap_.lon() < lon ) {
    lon = M_PIf + centerMap_.lon();
  }
  else if ( -(M_PIf + centerMap_.lon()) > lon ) {
    lon = -(M_PIf + centerMap_.lon());
  }  
  float lat = projRadius_1*meterCoord.y();
  lat = 2*(::atanf(::expf(lat)) - M_PI_4f);
  geoCoord->setLon(lon);
  geoCoord->setLat(-lat);
  return true;
}

inline bool MerkatProj::X2F_one(const QPointF& meterCoord, GeoPoint* geoCoord ) const {
  float lon = projRadius_1*meterCoord.x()  + centerMap_.lon();
  float lat = projRadius_1*meterCoord.y();
  lat = 2*(::atanf(::expf(lat)) - M_PI_4f);
  geoCoord->setLon(lon);
  geoCoord->setLat(-lat);
  return true;
}

inline bool MerkatProj::F2X_one( const GeoPoint& geoCoord, QPoint* meterCoord ) const {
  float lon = geoCoord.lon() - centerMap_.lon();
  float lat = -geoCoord.lat();
  while( lon < -M_PIf ) {
    lon += 2.0f*M_PIf;
  }
  while( lon > M_PIf ) {
    lon -= 2.0f*M_PIf;
  }
  float dX = projRadius*lon;
  float angle = M_PI_4f + lat*0.5f;
  float tan_ = sinf(angle)/cosf(angle) ;
  float dY = projRadius*::logf(tan_);

  int x = ( dX +0.5 );
  int y = ( dY +0.5 );
  if ( 0 > x && ::abs(x) == x ) {
    return false;
  }
  if ( 0 > y && ::abs(y) == y ) {
    return false;
  }
  meterCoord->setX(x);
  meterCoord->setY(y);

  return true;
}

inline bool MerkatProj::F2X_one( const GeoPoint& geoCoord, QPointF* meterCoord ) const {
  float lon = geoCoord.lon() - centerMap_.lon();
  float lat = -geoCoord.lat();
  while( lon < -M_PIf ) {
    lon += 2.0f*M_PIf;
  }
  while( lon > M_PIf ) {
    lon -= 2.0f*M_PIf;
  }
  float x = projRadius*lon;
  float y = projRadius*::logf(::tanf(M_PI_4f + lat*0.5f));
//  int x = ( dX +0.5 );
//  int y = ( dY +0.5 );
//  if ( 0 > x && ::abs(x) == x ) {
//    return false;
//  }
//  if ( 0 > y && ::abs(y) == y ) {
//    return false;
//  }
  meterCoord->setX(x);
  meterCoord->setY(y);
  return true;
}

GeoPoint MerkatProj::start() const
{
  return GeoPoint( startFi(), startLa() );
}

GeoPoint MerkatProj::end() const
{
  return GeoPoint( endFi(), endLa() );
}

float MerkatProj::startFi() const
{
  return M_PI_2f;
}

float MerkatProj::startLa() const
{
  float startlon = centerMap_.lon() - M_PIf;
  while ( -M_PIf > startlon ) {
    startlon += 2*M_PIf;
  }
  return startlon;
}

float MerkatProj::endFi() const
{
  return -M_PI_2f;
}

float MerkatProj::endLa() const
{
  float endlon = centerMap_.lon() + M_PIf;
  while ( M_PIf < endlon ) {
    endlon -= 2*M_PIf;
  }
  return endlon;
}

void MerkatProj::setMapCenter( const GeoPoint& gpnt )
{
  Projection::setMapCenter(gpnt);
  centerMap_.setLat(0.0);
}

GeoPoint MerkatProj::getProjCenter() const
{
  return centerMap_;
}

void MerkatProj::setStart( const GeoPoint& gp )
{
  Q_UNUSED(gp);
}

void MerkatProj::setEnd( const GeoPoint& gp )
{
  Q_UNUSED(gp);
}

void MerkatProj::setStartFi( float lat )
{
  Q_UNUSED(lat);
}

void MerkatProj::setStartLa( float lon )
{
  Q_UNUSED(lon);
}

void MerkatProj::setEndFi( float lat )
{
  Q_UNUSED(lat);
}

void MerkatProj::setEndLa( float lon )
{
  Q_UNUSED(lon);
}

Projection* MerkatProj::copy() const
{
  MerkatProj* s = new MerkatProj;
  return copyTo(s);
}

}

