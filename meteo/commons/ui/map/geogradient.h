#ifndef METEO_COMMONS_UI_MAP_GEOGRADIENT_H
#define METEO_COMMONS_UI_MAP_GEOGRADIENT_H

#include <qvector.h>
#include <qpolygon.h>

#include <commons/funcs/tcolorgrad.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/proto/map_isoline.pb.h>

namespace obanal {
  class TField;
}

namespace meteo {
namespace map {

class Document;
class LayerIso;

class GeoGradient : public Object
{
  public:
    GeoGradient( LayerIso* layer );
    GeoGradient( Layer* layer, obanal::TField* field = 0 );
    GeoGradient( Object* parent, obanal::TField* field = 0 );
    GeoGradient( Projection* proj, obanal::TField* field );

    GeoGradient( LayerIso* layer, const proto::FieldColor& color );
    GeoGradient( Layer* layer, obanal::TField* field, const proto::FieldColor& color );
    GeoGradient( Object* parent, obanal::TField* field, const proto::FieldColor& color );
    GeoGradient( Projection* proj, obanal::TField* field, const proto::FieldColor& color );

    ~GeoGradient();

    enum {
      Type = kGradient
    };
    int type() const { return Type; }

    Object* copy( Layer* l ) const ;
    Object* copy( Object* o ) const ;
    Object* copy( Projection* proj ) const ;

    void setFieldColor( const proto::FieldColor& color ) { fieldcolor_ = color; }
    const proto::FieldColor& fieldColor() const { return fieldcolor_; }

    proto::LevelColor levelColor() const ;
    TColorGradList gradient() const ;
    void setGradient( const TColorGradList& gradlist );
    
    bool render( QPainter* painter, const QRect& target, const QTransform& transform );
    QList<QRect> boundingRect( const QTransform& transform ) const ;

    QList<GeoVector> skeletInRect( const QRect& rect, const QTransform& transform ) const;

    int minimumScreenDistance( const QPoint& pos, QPoint* cross = 0 ) const ;

    bool hasValue() const { return false; }
    float value() const { return 0.0; }
    void setValue( float val, const QString& format = "4' '.2'0'", const QString& unit = QString()  );

    int32_t dataSize() const ;
    int32_t serializeToArray( char* arr ) const ;
    int32_t parseFromArray( const char* data );

  private:
    obanal::TField* field_;
    proto::FieldColor fieldcolor_;

    void loadFieldColorFromSettings( int descr );
};

}
}

#endif
