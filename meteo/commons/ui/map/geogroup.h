#ifndef METEO_COMMONS_UI_MAP_GEOGROUP_H
#define METEO_COMMONS_UI_MAP_GEOGROUP_H

#include "object.h"

namespace meteo {
namespace map {

class GeoGroup : public Object
{
  public:
    GeoGroup( Layer* layer, const QList<Object*> list = QList<Object*>() );
    GeoGroup( Object* parent, const QList<Object*> list = QList<Object*>() );
    GeoGroup( Projection* proj, const QList<Object*> list = QList<Object*>() );
    GeoGroup( const meteo::Property& prop = meteo::Property(), const QList<Object*> list = QList<Object*>() );
    ~GeoGroup();
    std::unordered_set<Object*> objects();

    enum {
      Type = kGroup
    };
    int type() const { return Type; }

    Object* copy( Layer* l ) const ;
    Object* copy( Object* o ) const ;
    Object* copy( Projection* proj ) const ;

    bool render( QPainter* painter, const QRect& target, const QTransform& transform );
    QList<QRect> boundingRect( const QTransform& transform ) const ;
    virtual QList<GeoVector> skeletInRect( const QRect& rect, const QTransform& transform ) const ;

    QList<Object*> objectsUnderGeoPoint( const GeoPoint& gp ) const;
    bool underGeoPoint( const GeoPoint& gp ) const ;
    int minimumScreenDistance( const QPoint& pos, QPoint* cross = 0 ) const ;

    bool hasValue() const ;
    float value() const ;
    void setValue( float val, const QString& format = "4' '.2'0'", const QString& unit = QString() );
    
    void setProperty( const meteo::Property& prop );

    void add( Object* o );
    void rm( Object* o );
    void ungroup();

    int32_t dataSize() const ;
    int32_t serializeToArray( char* arr ) const ;
    int32_t parseFromArray( const char* data );
};

}
}

#endif
