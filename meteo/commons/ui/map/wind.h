#ifndef METEO_COMMONS_UI_MAP_WIND_H
#define METEO_COMMONS_UI_MAP_WIND_H

#include "object.h"

#include <qimage.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <meteo/commons/proto/puanson.pb.h>

#include "windpath.h"

namespace meteo {
namespace map {

class Wind : public Object
{
  public:
    Wind( Layer* layer );
    Wind( Object* parent );
    Wind( Projection* proj );
    Wind( const ::meteo::Property& prop = ::meteo::Property() );
    ~Wind();
    enum {
      Type = kWind
    };
    virtual int type() const { return Type; }

    Object* copy( Layer* l ) const ;
    Object* copy( Object* o ) const ;
    Object* copy( Projection* proj ) const ;

    float dd() const { return dd_; }
    control::QualityControl ddqual() const { return ddqual_; }
    float ff() const { return ff_; }
    control::QualityControl ffqual() const { return ffqual_; }

    bool draw_bad_dd() const { return draw_bad_dd_; }
    void set_draw_bad_dd( bool fl ) { draw_bad_dd_ = fl; }

    bool ddok() const ;
    bool ffok() const ;

    void setDd( float d, control::QualityControl ddqual = control::RIGHT );
    void setFf( float f, control::QualityControl ffqual = control::RIGHT );

    void setDdff( int dd, int ff, control::QualityControl ddqual = control::RIGHT, control::QualityControl ffqual = control::RIGHT, int rad = 0 );

    void setScreenPos( const QPoint& pos );
    void removeScreenPos();

    QList<QPoint> screenPoints( const QTransform& transform ) const ;

    //! возвращает прямоугольник ориентированного ветра
    //! angle - угол на который повернулся ветер в градУсах
    //! oldtopleft - топлефт неповернутого ветра
    QRect boundingRect( const QPoint& pnt, float* angle = 0, QPoint* oldtopleft = 0 ) const ;

    //прямоугольники ветра на документе
    QList<QRect> boundingRect( const QTransform& transform ) const ;
    //прямоугольник ветра относительно центра пуансона
    QRect boundingRect() const ;

    QList<GeoVector> skeletInRect( const QRect& rect, const QTransform& transform ) const ;


    bool render( QPainter* painter, const QRect& target, const QTransform& transform );

    bool hasValue() const { return false; }
    float value() const { return 0.0; }
    void setValue( float value, const QString& format = "4' '.2'0'", const QString& unit = QString() )
    { Q_UNUSED(value); Q_UNUSED(format); Q_UNUSED(unit); }

    int minimumScreenDistance( const QPoint& pos, QPoint* cross = 0 ) const ;

    void setLenft( float l );
    float lenft() const { return lenft_; }

    int32_t dataSize() const ;
    int32_t serializeToArray( char* data ) const ;
    int32_t parseFromArray( const char* data );

  private:
    QImage* pix_;
    QPoint* screenpos_;
    float dd_;
    float ff_;
    control::QualityControl ddqual_;
    control::QualityControl ffqual_;
    float radius_;
    bool draw_bad_dd_;
    
    WindPath wind_;
    mutable QRect cached_rect_;
    float lenft_;

    QSize size() const ;
    bool preparePixmap();
};

}
}

#endif
