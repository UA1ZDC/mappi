#ifndef COMMONS_GEOBASIS_GEOPOINT_H
#define COMMONS_GEOBASIS_GEOPOINT_H

#include <stdio.h>
#include <math.h>

#include <qvector.h>
#include <qpair.h>
#include <qdatastream.h>

#include <commons/mathtools/mnmath.h>
#include <commons/mathtools/mathematical_constants.h>

class TLog;

namespace meteo
{

constexpr float RAD2DEG = 180.0f / M_PIf;
constexpr float DEG2RAD = M_PIf / 180.0f;
constexpr float k2PI    = 2 * M_PIf;
constexpr float kPI180  = M_PIf / 180.0f;
constexpr float kPI360  = M_PIf / 360.0f;

enum CoordType
{
  RAD = 0,
  DEG = 1
};

enum typeGeoCoord {
  LA360         = 0,    //!< широта от 0 до 180 долгота от 0 до 360
  LA180         = 1,    //!< широта от -90 до 90 долгота от -180 до 180
  LA_GENERAL    = 2     //!< широта и долгота не ограничены
};

class  GeoPoint {
  public:
    GeoPoint();
    GeoPoint( float lat, float lon,  float alt = 0., typeGeoCoord type_coord = LA180 );

    static GeoPoint fromDegree( float lat, float lon, float alt = 0., typeGeoCoord type_coord = LA180 );
    static const GeoPoint Invalid;

    const GeoPoint& to360();
    const GeoPoint& to180();
    float lat() const { return lat_; }
    float lon() const { return lon_; }

    float latDeg() const {return lat_*RAD2DEG; }
    float lonDeg() const { return lon_*RAD2DEG; }

    float fi() const { return lat_; }
    float la() const { return lon_; }

    float fiDeg() const { return lat_*RAD2DEG; }
    float laDeg() const { return lon_*RAD2DEG; }

    float alt() const { return alt_; }
    typeGeoCoord type() const { return type_coord_; }

    QString toString(bool withAlt = false, const QString& templ = QString()) const;

    QString strLat() const ;
    QString strLon() const ;
    QString strAlt() const ;

    bool isValid() const ;

    void setPoint( float lt, float ln, float a = 0., typeGeoCoord type_coord = LA180 );

    void setLat( float lt );
    void setLon( float ln ) ;
    void setLatDeg( float lt ) ;
    void setLonDeg( float ln ) ;

    void setFi( float lt );
    void setLa( float ln ) ;
    void setFiDeg( float lt ) ;
    void setLaDeg( float ln ) ;

    void setAlt( float a ) { alt_ = a; }

    void setIn();

    float calcDistLoks(const GeoPoint &g, float angle) const ;
    float calcUgolOrt(const GeoPoint &g) const;
    void calcFLOrt(GeoPoint *g, float rast, float ugol);
    float calcUgolLoks(const  GeoPoint &g);

    float calcDistance(const GeoPoint& p2) const;
    float radianDistance(const GeoPoint& p2) const;
    float radianDistanceFast(const GeoPoint& p2) const;

    float radianAzimuth(const GeoPoint& p2) const;
    float simpAzimut(const GeoPoint& p2) const;
    float orthodromicDirection(const GeoPoint& p2) const;

    GeoPoint findSecondCoord( const float d, const float a ) const ;
    GeoPoint findPerpendicularIntersection(const QPair<GeoPoint, GeoPoint>& line) const;

    GeoPoint& operator+(const GeoPoint& src);
    GeoPoint& operator+=(const GeoPoint& src);
    bool operator==(const GeoPoint& src );
    GeoPoint& operator-(const GeoPoint& src);
    GeoPoint& operator-=(const GeoPoint& src);
    bool operator <(const GeoPoint& rhs) const;

    //! Проверка на равенство с точкой point с точностью delta.
    bool compare(const GeoPoint& point) const;
    //! Проверка на равенство с точкой point с точностью delta.
    bool compareLatLon(const GeoPoint& point) const;
    
    float ugolA(const GeoPoint& B, const GeoPoint& C) const;
    
    static int32_t size();
    QByteArray data() const ;
    int32_t getData( QByteArray* arr ) const ;
    int32_t getData( char* arr ) const ;
    static GeoPoint fromData( const QByteArray& arr ) ;
    static GeoPoint fromData( const char* data ) ;

  private:
    float lat_;
    float lon_;

    typeGeoCoord type_coord_;
    float alt_;

  friend QString& operator<<( QString& str, const GeoPoint& gc );
  friend TLog& operator<<( TLog& log, const GeoPoint& gc );
  friend QDataStream& operator<<(QDataStream& in,const GeoPoint& gc);
  friend QDataStream& operator>>(QDataStream& out, GeoPoint& gc);

};

QVector<GeoPoint> orthodromicLine(const GeoPoint& from, const GeoPoint& to, float radianStep);
QVector<GeoPoint> divideLineBySteps(const GeoPoint& from, const GeoPoint& to, float radianStep);

inline GeoPoint GeoPoint::fromDegree( float lat, float lon, float alt,typeGeoCoord type_coord )
{
  return GeoPoint( lat*DEG2RAD, lon*DEG2RAD, alt, type_coord );
}

inline bool GeoPoint::isValid() const {
  if ( (M_PI_2f < ::fabs(lat_)
      || M_PIf < ::fabs(lon_)) && LA_GENERAL != type() ) {
    return false;
  }
  return true;
}

inline bool operator==(const GeoPoint &p1, const GeoPoint &p2)
{
  return MnMath::isEqual(p1.lat(), p2.lat())
   && MnMath::isEqual(p1.lon(), p2.lon())&&
    MnMath::isEqual(p1.alt(), p2.alt());
}

inline bool operator!=(const GeoPoint &p1, const GeoPoint &p2)
{
  return (false == operator==(p1,p2));
}

inline const GeoPoint operator-(const GeoPoint& p1, const GeoPoint& p2)
{
  return GeoPoint(p1.lat() - p2.lat(), p1.lon() - p2.lon(), p1.alt() - p2.alt(), p1.type());
}

} //meteo

#endif

