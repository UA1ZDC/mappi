#ifndef COMMONS_GEOBASIS_GEOVECTOR_H
#define COMMONS_GEOBASIS_GEOVECTOR_H

#include "geopoint.h"

#include <qvector.h>

namespace meteo {
  enum { INSIDE, OUTSIDE, BOUNDARY };         // положение точки
  //     ВНУТРИ, ВНЕ,     НА ГРАНИЦЕ
class GeoVector : public QVector<GeoPoint> {
  public:
    explicit GeoVector();
    explicit GeoVector(const QVector<GeoPoint>& other);
    explicit GeoVector(int size );
    ~GeoVector();
     
    void to360();
    void to180();
    
    const GeoPoint& maxAlt() const;

    void closeVector();
    void move(const GeoPoint&);

    int contains(const GeoPoint &) const;
    
    inline GeoVector& operator=( const QVector<GeoPoint>& gv );
    inline GeoVector& operator+=( const QVector<GeoPoint>& gv );

    int32_t dataSize() const ;
    QByteArray byteData() const ;
    int32_t getData( QByteArray* arr ) const ;
    int32_t getData( char* arr ) const ;
    static GeoVector fromData( const QByteArray& arr ) ;
    static GeoVector fromData( const char* data ) ;

    GeoVector& operator<<( const GeoPoint& gp );
    GeoVector& operator<<( const GeoVector& gv );
    GeoVector& operator<<( const QVector<GeoPoint>& gv );

  
  friend QString& operator<<( QString& str, const GeoVector& gc ); 
  friend TLog& operator<<( TLog& log, const GeoVector& gc ); 
};




GeoVector& GeoVector::operator=( const QVector<GeoPoint>& gv )
{
  QVector<GeoPoint>::operator=(gv);
  return *this;
}

GeoVector& GeoVector::operator+=( const QVector<GeoPoint>& gv )
{
  QVector<GeoPoint>::operator+=(gv);
  return *this;
}

}

#endif
