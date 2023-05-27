#ifndef METEO_COMMONS_UI_MAP_GEOMRL_H
#define METEO_COMMONS_UI_MAP_GEOMRL_H

#include <qvector.h>
#include <qpolygon.h>

#include <commons/funcs/tcolorgrad.h>
#include <meteo/commons/radar/radar.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/proto/map_radar.pb.h>

namespace meteo {

namespace map {

class Document;
class LayerMrl;

class GeoMrl : public Object
{
  public:
    GeoMrl( Layer* layer, const Radar& radar = Radar() );
    GeoMrl( Object* parent, const Radar& radar = Radar() );
    GeoMrl( Projection* proj, const Radar& radar = Radar() );

    GeoMrl( Layer* layer, const Radar& radar, const proto::RadarColor& color );
    GeoMrl( Object* parent, const Radar& radar, const proto::RadarColor& color );
    GeoMrl( Projection* proj, const Radar& radar, const proto::RadarColor& color );

    ~GeoMrl();

    enum {
      Type = kRadar
    };
    int type() const { return Type; }

    Object* copy( Layer* l ) const ;
    Object* copy( Object* o ) const ;
    Object* copy( Projection* proj ) const ;

    void setRadarColor( const proto::RadarColor& color ) { radarcolor_ = color; }
    const proto::RadarColor& radarColor() const { return radarcolor_; }

    void setRadar( const Radar& radar );
    const Radar& radar() const { return radar_; }

    TColorGradList gradient() const ;
    void setGradient( const TColorGradList& gradlist );
    
    bool render( QPainter* painter, const QRect& target, const QTransform& transform );
    QList<QRect> boundingRect( const QTransform& transform ) const ;

    QList<GeoVector> skeletInRect( const QRect& rect, const QTransform& transform ) const;

    int minimumScreenDistance( const QPoint& pos, QPoint* cross = 0 ) const ;

    bool hasValue() const { return false; }
    float value() const { return 0.0; }
    void setValue( float val, const QString& format = "4' '.2'0'", const QString& unit = QString()  );

    float pointValue( const QPoint& pnt, bool* ok ) const ;

    int32_t dataSize() const ;
    int32_t serializeToArray( char* arr ) const ;
    int32_t parseFromArray( const char* data );

    QList< QPair< GeoVector, ::meteo::Property > > getSquares() const ;

  private:
    Radar radar_;
    proto::RadarColor radarcolor_;
    QTransform radartransform_;
    QTransform pnt_transform_;

    void calcSkelet();
    void createPunches();

    void loadFieldColorFromSettings( int descr );
};

}
}

#endif
