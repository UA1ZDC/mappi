#ifndef METEO_COMMONS_UI_MAP_GEOPIXMAP_H
#define METEO_COMMONS_UI_MAP_GEOPIXMAP_H

#include "object.h"

#include <qpair.h>
#include <qimage.h>
#include <qpixmap.h>

namespace meteo {
namespace map {

class Document;

class GeoPixmap : public Object
{
  public:
    GeoPixmap( Layer* layer );
    GeoPixmap( Object* parent );
    GeoPixmap( Projection* proj );
    GeoPixmap( const ::meteo::Property& prop = ::meteo::Property() );
    ~GeoPixmap();

    enum {
      Type = kPixmap
    };
    int type() const { return Type; }

    Object* copy( Layer* l ) const ;
    Object* copy( Object* o ) const ;
    Object* copy( Projection* proj ) const ;

    QList<QRect> boundingRect( const QTransform& transform ) const ;
    QList<QRect> boundingRectWithBunch( const QTransform& transform ) const ;
    QList<QRect> boundingRectWithoutBunch( const QTransform& transform ) const ;

    QList<GeoVector> skeletInRect( const QRect& rect, const QTransform& transform ) const ;

    bool hasValue() const { return false; }
    float value() const { return 0.0; }
    void setValue( float value, const QString& format = "4' '.2'0'", const QString& unit = QString() )
    { Q_UNUSED(value); Q_UNUSED(format); Q_UNUSED(unit); }
    
    bool render( QPainter* painter, const QRect& target, const QTransform& transform );
    bool renderWithoutBunch( QPainter* painter, const QRect& target, const QTransform& transform );
    bool renderWithBunch( QPainter* painter, const QRect& target, const QTransform& transform );
    bool renderOnParentSkelet( QPainter* painter, const QRect& target, const QTransform& transform );

    int minimumScreenDistance( const QPoint& pos, QPoint* cross = 0 ) const ;

    void bindScreenToCoord( const QPolygon& screen, const GeoVector& coord );
    void unbindFromCoord();

    void setImage( const QImage& pixmap );
    void setColor( const QColor& color );

    QImage image() const ;
    const QPair< QPolygon, GeoVector >& coordBunch() const { return coordbunch_; }

    int32_t dataSize() const ;
    int32_t serializeToArray( char* data ) const ;
    int32_t parseFromArray( const char* data );

    bool bindedToSkelet() const { return bindedtoskelet_; }
    void setBindedToSkelet( bool fl ) { bindedtoskelet_ = fl; }


  protected:
    QImage* pix_;

  private:
    QPair< QPolygon, GeoVector > coordbunch_;
    bool bindedtoskelet_;

    QRect pixmapRect() const ;
    QPoint translateByPosition( const QPoint& pnt ) const ;

    QTransform bunchTransform( const QTransform& transform ) const ;

    int nearestPointIndex( const QPoint& pnt, const QPolygon& poly, int* dist2 ) const ;
};

}
}

#endif
