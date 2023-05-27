#ifndef METEO_COMMONS_UI_MAP_LAYER_ISOH
#define METEO_COMMONS_UI_MAP_LAYER_ISOH

#include "layer.h"
#include "geogroup.h"
#include <meteo/commons/proto/map_isoline.pb.h>
#include <meteo/commons/proto/weather.pb.h>
#include <commons/funcs/tcolorgrad.h>
#include <commons/obanal/obanal_struct.h>

class TMeteoParam;

namespace obanal {
  class TField;
}

namespace meteo {

class Projection;

namespace map {

class Map;
class Object;
class GeoText;
class Property;
class Document;

class LayerIso: public Layer
{
  public:
    LayerIso( Document* map, const QString& name = QString() );
    LayerIso( Document* map, const proto::FieldColor& clr, obanal::TField* field );
    ~LayerIso();

    enum {
      Type = meteo::kLayerIso
    };
    int type() const { return Type; }

    void render( QPainter* painter, const QRect& target, Document* document );

    int addIsoLines( bool* ok = 0 ); //!< добавить изолинии в соответствии с field_.descr, field_.level, field_.type_level
    int addIsoLines( const proto::FieldColor& color, bool* ok = 0 ); //!< добавить в соответствии с field_.level, field_.type_level и color
    int addIsoLines( const proto::LevelColor& color ); //!< добавить в соответствии с color
    int addIsoLines( float min, float max, float step ); //!< добавить в соответствии с переданными параметрами

    bool addGradient();
    bool addGradient( const proto::FieldColor& color );
    bool addGradient( const proto::LevelColor& color );

    void removeGradient();
    void removeIsolines();

    const proto::FieldColor& fieldColor() const { return fieldcolor_; }
    void setFieldColor( const proto::FieldColor& fc );
    void setPen( const QPen& pen );

    bool hasGradient() const ;
    bool hasIsoline() const ;

    bool gradientVisible() const ;

    void showGradient();
    void hideGradient();

    int level() const ;
    int typeLevel() const ;
    int descriptor() const ;


    proto::LevelColor levelColor() const ;
    QPen pen() const ;
    float minValue() const ;
    float maxValue() const ;
    QColor minColor() const ;
    QColor maxColor() const ;
    TColorGradList gradient() const ;
    float stepIso() const ;

    void setColor( const TColorGrad& gradlist );
    void setColor( const TColorGradList& gradlist );
    void setStepIso( float step );
    void setMin( float min );
    void setMax( float max );
    void setColorMin( const QColor& clr );
    void setColorMax( const QColor& clr );

    int addExtremums();

    QVector<fieldExtremum> getExtremums(){return extremums_;}

    void setField( obanal::TField* f );
    
    bool hasValue() const { return true; }
    valLabel valueLabel() const ;//{ return QString::fromStdString(fieldcolor_.name()) + QString(" %1 мбар").arg( info_.level() ); }

    static proto::WeatherLayer info( const LayerIso* layer ) ;
    
    bool containGeo( meteo::map::Object * );

    LayerMenu* layerMenu();

    void incGradientCount();
    void decGradientCount();

    int32_t dataSize() const ;
    int32_t serializeToArray( char* data ) const ;
    int32_t parseFromArray( const char* data );
      
  private:
    void setEtrLabel(int tip,TMeteoParam *param );
    void gruppingIsolines();
  //  GeoGroup* objectContain(const meteo::GeoPoint &gp);
    void updateFieldColor();

    LayerIso( Map* map, const QString& name = QString() );
    
    proto::FieldColor fieldcolor_;

    QMap < QString, Object* > extremum_gr_list_;

    QVector<fieldExtremum> extremums_;
    
    QList <GeoGroup*> gr_list;
    int gradientcount_ = 0;
};

}
}

#endif
