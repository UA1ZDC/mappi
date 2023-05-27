#include "generalproj.h"
#include <cross-commons/debug/tlog.h>

using namespace MnMath;

namespace meteo {

void GeneralProj::setFuncTransform( fF2X_one f2x, fX2F_one x2f )
{
  funcF2X_one_ = f2x;
  funcX2F_one_ = x2f;
}

void GeneralProj::setFuncTransform( fF2X_onef f2x, fX2F_onef x2f )
{
  funcF2X_onef_ = f2x;
  funcX2F_onef_ = x2f;
}

bool GeneralProj::linearF2X_one( const Projection& proj, const GeoPoint& geoCoord, QPoint* meterCoord )
{
  float lon = geoCoord.lon() - proj.getMapCenter().lon();
  float lat = geoCoord.lat() - proj.getMapCenter().lat();

  int x = kRVSG*lat*proj.xfactor();
  int y = kRVSG*(lon*proj.yfactor());
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

bool GeneralProj::linearX2F_one( const Projection& proj, const QPoint& meterCoord, GeoPoint* geoCoord )
{
  if ( nullptr == geoCoord ) {
    error_log << QObject::tr("Нулевой указатель GeoPoint*");
    return false;
  }
  *geoCoord = GeoPoint( 0,0,0, LA_GENERAL );

  float x = meterCoord.x();
  float y = meterCoord.y();
  float lat = proj.getMapCenter().lat() + x/( kRVSG*proj.xfactor() );
  float lon = proj.getMapCenter().lon() + y/(kRVSG*proj.yfactor() );
  geoCoord->setLat(lat);
  geoCoord->setLon(lon);

  return true;
}

bool GeneralProj::linearF2X_onef( const Projection& proj, const GeoPoint& geoCoord, QPointF* meterCoord )
{
  float lon = geoCoord.lon() - proj.getMapCenter().lon();
  float lat = geoCoord.lat() - proj.getMapCenter().lat();

  int x = kRVSG*lat*proj.xfactor();
  int y = kRVSG*(lon*proj.yfactor());
  meterCoord->setX(x);
  meterCoord->setY(y);
//  if ( 0 > x && ::abs(x) == x ) {
//    return false;
//  }
//  if ( 0 > y && ::abs(y) == y ) {
//    return false;
//  }
  return true;
}

bool GeneralProj::linearX2F_onef( const Projection& proj, const QPointF& meterCoord, GeoPoint* geoCoord )
{
  if ( nullptr == geoCoord ) {
    error_log << QObject::tr("Нулевой указатель GeoPoint*");
    return false;
  }
  *geoCoord = GeoPoint( 0,0,0, LA_GENERAL );

  float x = meterCoord.x();
  float y = meterCoord.y();
  float lat = proj.getMapCenter().lat() + x/( kRVSG*proj.xfactor() );
  float lon = proj.getMapCenter().lon() + y/(kRVSG*proj.yfactor() );
  geoCoord->setLat(lat);
  geoCoord->setLon(lon);

  return true;
}

GeneralProj::GeneralProj()
  : Projection()
{
  funcF2X_one_ = linearF2X_one;
  funcX2F_one_ = linearX2F_one;
  funcF2X_onef_ = linearF2X_onef;
  funcX2F_onef_ = linearX2F_onef;
  type_ = GENERAL;
  xfactor_ = 1.0;
//  yfactor_ = 0.001;
  yfactor_ = 1.0;
 // initRamka();
}

GeneralProj::GeneralProj( const GeoPoint& center )
  : Projection( center )
{
  funcF2X_one_ = linearF2X_one;
  funcX2F_one_ = linearX2F_one;
  funcF2X_onef_ = linearF2X_onef;
  funcX2F_onef_ = linearX2F_onef;
  type_ = GENERAL;
  xfactor_ = 1.0;
//  yfactor_ = 0.001;
  yfactor_ = 1.0;
  setMapCenter(center);
  initRamka();
}

GeneralProj::~GeneralProj()
{
}

bool GeneralProj::F2X(const GeoVector& dPt, QVector<QPolygon>* apoint, bool /*isClosed*/ ) const
{
  if ( dPt.isEmpty() ) {
    return true;
  }

  QPolygon scrPt;
  QPoint pnt;
  GeoPoint* parray = const_cast<GeoPoint*>(dPt.data());
  GeoPoint gp;
  GeoPoint oldgp;
  for ( int i = 0, size = dPt.size(); i < size; ++i, ++parray ) {
    GeoPoint gp = *parray;
    if ( true == F2X_one( gp, &pnt ) ) {
      scrPt.append(pnt);
    }
  }
  if ( false == scrPt.isEmpty() ) {
    apoint->append(scrPt);
  }
  return true;
}

inline bool GeneralProj::X2F_one(const QPoint& meterCoord, GeoPoint* geoCoord ) const
{
  if ( nullptr == funcX2F_one_ ) {
    error_log << QObject::tr("Функция преобразования географических координат в экранные не установлена");
    return false;
  }
  return funcX2F_one_( *this, meterCoord, geoCoord );
}

inline bool GeneralProj::F2X_one( const GeoPoint& geoCoord, QPoint* meterCoord ) const
{
  if ( nullptr == funcF2X_one_) {
    error_log << QObject::tr("Функция преобразования экранные координат в географические не установлена");
    return false;
  }
  return funcF2X_one_( *this, geoCoord, meterCoord );
}

inline bool GeneralProj::X2F_one(const QPointF& meterCoord, GeoPoint* geoCoord ) const
{
  if ( nullptr == funcX2F_onef_ ) {
    error_log << QObject::tr("Функция преобразования географических координат в экранные не установлена");
    return false;
  }
  return funcX2F_onef_( *this, meterCoord, geoCoord );
}

inline bool GeneralProj::F2X_one( const GeoPoint& geoCoord, QPointF* meterCoord ) const
{
  if ( nullptr == funcF2X_onef_) {
    error_log << QObject::tr("Функция преобразования экранные координат в географические не установлена");
    return false;
  }
  return funcF2X_onef_( *this, geoCoord, meterCoord );
}
//
//void GeneralProj::setRamka(int x0, int y0,int x1, int y1)
//{
//  ramka_.clear();
//  ramka_.append(GeoPoint(x0,y0,0,LA_GENERAL));
//  ramka_.append(GeoPoint(x0,y1,0,LA_GENERAL));
//  ramka_.append(GeoPoint(x1,y1,0,LA_GENERAL));
//  ramka_.append(GeoPoint(x1,y0,0,LA_GENERAL));
//  ramka_.append(GeoPoint(x0,y0,0,LA_GENERAL));
//}

void GeneralProj::setRamka(float x0, float y0,float x1, float y1)
{
  ramka_.clear();
  ramka_.append(GeoPoint(x0,y0,0,LA_GENERAL));
  ramka_.append(GeoPoint(x0,y1,0,LA_GENERAL));
  ramka_.append(GeoPoint(x1,y1,0,LA_GENERAL));
  ramka_.append(GeoPoint(x1,y0,0,LA_GENERAL));
  ramka_.append(GeoPoint(x0,y0,0,LA_GENERAL));
}

void GeneralProj::initRamka()
{
  int x0 =-100;
  int y0 = -100;
  int x1 = 1500;
  int y1 = 1500;
  ramka_.clear();
  ramka_.append(GeoPoint(x0,y0,0,LA_GENERAL));
  ramka_.append(GeoPoint(x1,y0,0,LA_GENERAL));
  ramka_.append(GeoPoint(x1,y1,0,LA_GENERAL));
  ramka_.append(GeoPoint(x0,y1,0,LA_GENERAL));
  ramka_.append(GeoPoint(x0,y0,0,LA_GENERAL));
}

Projection* GeneralProj::copy() const
{
  GeneralProj* s = new GeneralProj;
  return copyTo(s);
}

}

