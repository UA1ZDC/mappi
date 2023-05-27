#ifndef METEO_COMMONS_UI_MAP_ISOLINE_H
#define METEO_COMMONS_UI_MAP_ISOLINE_H

#include <qvector.h>
#include <qpolygon.h>

#include <commons/funcs/tcolorgrad.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/proto/map_isoline.pb.h>

namespace meteo {
namespace map {

class Document;
class LayerIso;

class IsoLine : public GeoPolygon
{
  public:
    IsoLine( LayerIso* layer, float value = kGeoPolygonInvalidValue );
    IsoLine( Layer* layer, int meteodescr = -1, float value = kGeoPolygonInvalidValue, int lvl = -1, int type_level = -1 );
    IsoLine( Object* parent, int meteodescr = -1, float value = kGeoPolygonInvalidValue, int lvl = -1, int type_level = -1 );
    IsoLine( Projection* proj, int meteodescr, float value = kGeoPolygonInvalidValue, int lvl = -1, int type_level = -1 );
    IsoLine( int meteodescr, float value = kGeoPolygonInvalidValue, int lvl = -1, int type_level = -1 );

    IsoLine( LayerIso* layer, const proto::FieldColor& color, float value = kGeoPolygonInvalidValue );
    IsoLine( Layer* layer, const proto::FieldColor& color, float value = kGeoPolygonInvalidValue, int lvl = -1, int type_level = -1 );
    IsoLine( Object* parent, const proto::FieldColor& color, float value = kGeoPolygonInvalidValue, int lvl = -1, int type_level = -1 );
    IsoLine( Projection* proj, const proto::FieldColor& color, float value = kGeoPolygonInvalidValue, int lvl = -1, int type_level = -1 );
    IsoLine( const proto::FieldColor& color, float value = kGeoPolygonInvalidValue, int lvl = -1, int type_level = -1 );

    ~IsoLine();

    enum {
      Type = kIsoline
    };
    int type() const { return Type; }

    Object* copy( Layer* l ) const ;
    Object* copy( Object* o ) const ;
    Object* copy( Projection* proj ) const ;
    
    QList<GeoVector> skeletInRect( const QRect& rect, const QTransform& transform ) const ;

    void setValue( float value, const QString& format = "4' '.2'0'", const QString& unit = QString() );

    void setMeteodescr( int descr ) { loadFieldColorFromSettings(descr); }
    int meteodescr() const { return fieldcolor_.descr(); }

    void setFieldColor( const proto::FieldColor& color ) { fieldcolor_ = color; updateObjectProto(); }
    const proto::FieldColor& fieldColor() const { return fieldcolor_; }

    proto::LevelColor levelColor() const ;
    QPen pen() const ;
    float minValue() const ;
    float maxValue() const ;
    QColor minColor() const ;
    QColor maxColor() const ;
    TColorGrad gradient() const ;
    float stepIso() const ;

    void setColor( const TColorGrad& gradlist );
    void setStepIso( float step );
    void setMin( float min );
    void setMax( float max );
    void setColorMin( const QColor& clr );
    void setColorMax( const QColor& clr );

    void setLevel( int lvl ) { level_ = lvl; updateObjectProto(); }
    int level() const { return level_; }

    void setTypeLevel( int tlvl ) { type_level_ = tlvl; updateObjectProto(); }
    int typelevel() const { return type_level_; }
    
    bool render( QPainter* painter, const QRect& target, const QTransform& transform );

    int32_t dataSize() const ;
    int32_t serializeToArray( char* arr ) const ;
    int32_t parseFromArray( const char* data );

  private:
    int level_;
    int type_level_;
    proto::FieldColor fieldcolor_;

    void loadFieldColorFromSettings( int descr );

    void updateObjectProto();
};

}
}

#endif
