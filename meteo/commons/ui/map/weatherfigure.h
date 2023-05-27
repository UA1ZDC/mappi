#ifndef METEO_COMMONS_UI_MAP_WEATHERFIGURE_H
#define METEO_COMMONS_UI_MAP_WEATHERFIGURE_H

#include "object.h"

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <meteo/commons/proto/meteotablo.pb.h>

namespace meteo {
namespace map {

class WeatherFigure : public Object
{
  public:
    WeatherFigure( Layer* layer );
    WeatherFigure( Object* parent );
    WeatherFigure( Projection* proj );
    WeatherFigure( const meteo::Property& prop = meteo::Property() );
    ~WeatherFigure();

    enum {
      Type = kFigure
    };
    virtual int type() const { return Type; }

    Object* copy( Layer* l ) const ;
    Object* copy( Object* o ) const ;
    Object* copy( Projection* proj ) const ;

    const tablo::Settings& pattern() const ;
    bool hasPattern() const { return ( true == pattern().IsInitialized() ); }
    TMeteoParam paramValue( const QString& name, bool* ok = 0 ) const ;

    const TMeteoData& meteodata() const { return meteodata_; }

    void setPattern( const tablo::Settings& p );

    void setMeteodata( const TMeteoData& m );

    //прямоугольники фигуры на документе
    QList<QRect> boundingRect( const QTransform& transform ) const ;
    QList<QPoint> screenPoints( const QTransform& transform ) const ;

    QList<GeoVector> skeletInRect( const QRect& rect, const QTransform& transform ) const ;

    bool render( QPainter* painter, const QRect& target, const QTransform& transform );

    bool getPainterPath(QPainterPath *, const QRect& target, const QTransform& transform );

    bool hasValue() const { return false; }
    float value() const { return 0.0; }
    void setValue( float value, const QString& format = "4' '.2'0'", const QString& unit = QString() )
    { Q_UNUSED(value); Q_UNUSED(format); Q_UNUSED(unit); }

    int minimumScreenDistance( const QPoint& pos, QPoint* cross = 0 ) const ;

    const std::string& patternString() const ;
    int32_t dataSize() const ;
    int32_t serializeToArray( char* arr ) const ;
    int32_t parseFromArray( const char* data );

    static tablo::Color color( const tablo::Settings& pattern, const TMeteoData& md );

    QPair< GeoVector, meteo::Property > getCircle( bool* ok = 0 ) const ;

  private:
    tablo::Settings pattern_;
    TMeteoData meteodata_;
    mutable std::string cached_pattern_string_;
};

}
}

#endif
