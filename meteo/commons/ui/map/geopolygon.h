#ifndef METEO_COMMONS_UI_MAP_GEOPOLYGON_H
#define METEO_COMMONS_UI_MAP_GEOPOLYGON_H

#include "object.h"

#include <qvector.h>
#include <qpolygon.h>

namespace meteo {
namespace map {
    
class Document;

const float kGeoPolygonInvalidValue(-9999.0);

class GeoPolygon : public Object
{
  public:
    GeoPolygon( Layer* layer );
    GeoPolygon( Object* parent );
    GeoPolygon( Projection* proj );
    GeoPolygon( const meteo::Property& prop = meteo::Property() );
    ~GeoPolygon();

    enum {
      Type = kPolygon
    };
    int type() const { return Type; }

    Object* copy( Layer* l ) const ;
    Object* copy( Object* o ) const ;
    Object* copy( Projection* proj ) const ;

    QList<QRect> boundingRect( const QTransform& transform ) const ;
    QList<GeoVector> skeletInRect( const QRect& rect, const QTransform& transform ) const ;
    void setSkelet( const GeoVector& skel );

    bool hasValue() const ;
    float value() const ;
    void setValue( float value, const QString& format = "4' '.2'0'", const QString& unit = QString() );
    void removeValue();
    
    bool render( QPainter* painter, const QRect& target, const QTransform& transform );

    int minimumScreenDistance( const QPoint& pos, QPoint* cross = 0 ) const ;

    int32_t dataSize() const ;

    int32_t serializeToArray( char* data ) const;
    int32_t parseFromArray( const char* data );


  protected:

  private:
    void paintSmoothPolygon( QPainter* painter, const QPolygon& polygon );
    void paintSmoothLine( QPainter* painter, const QPolygon& polygon );

    void calcDouble( const QPolygon& source, QPair<QPolygon,QPolygon>* target );
    QPoint calcShift( const QPoint& pnt1, const QPoint& pnt2, int shift );
};

}
}

#endif
