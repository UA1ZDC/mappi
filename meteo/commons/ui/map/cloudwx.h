#ifndef METEO_COMMONS_UI_MAP_CLOUDWX_H
#define METEO_COMMONS_UI_MAP_CLOUDWX_H

#include "object.h"

#include <qvector.h>
#include <qpolygon.h>

#include <meteo/commons/proto/sigwx.pb.h>

namespace meteo {
namespace map {

class Document;

class CloudWx : public Object
{
  public:
    CloudWx( Layer* layer );
    CloudWx( Object* parent );
    CloudWx( Projection* proj );
    CloudWx( const meteo::Property& prop = meteo::Property() );
    ~CloudWx();

    enum {
      Type = kCloudWx
    };
    int type() const { return Type; }

    Object* copy( Layer* l ) const ;
    Object* copy( Object* o ) const ;
    Object* copy( Projection* proj ) const ;

    QList<QRect> boundingRect( const QTransform& transform ) const ;
    QList<GeoVector> skeletInRect( const QRect& rect, const QTransform& transform ) const ;
    void setSkelet( const GeoVector& skel );

    bool hasValue() const { return false; };
    float value() const { return 0.0; };
    void setValue( float value, const QString& format = "4' '.2'0'", const QString& unit = QString() );
    
    bool render( QPainter* painter, const QRect& target, const QTransform& transform );

    int minimumScreenDistance( const QPoint& pos, QPoint* cross = 0 ) const ;

    void setProto( const sigwx::Cloud& proto );

    int32_t dataSize() const ;

    int32_t serializeToArray( char* data ) const;
    int32_t parseFromArray( const char* data );


  protected:

  private:
    void paintSmoothPolygon( QPainter* painter, const QPolygon& polygon );
    void paintSmoothLine( QPainter* painter, const QPolygon& polygon );

    void calcDouble( const QPolygon& source, QPair<QPolygon,QPolygon>* target );
};

}
}

#endif
