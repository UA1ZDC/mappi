#ifndef METEO_COMMONS_UI_MAP_FLOW_H
#define METEO_COMMONS_UI_MAP_FLOW_H

#include "object.h"

#include <qimage.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <meteo/commons/proto/puanson.pb.h>

#include "flowpath.h"

namespace meteo {
namespace map {

class Flow : public Object
{
  public:
    Flow( Layer* layer );
    Flow( Object* parent );
    Flow( Projection* proj );
    Flow( const ::meteo::Property& prop = ::meteo::Property() );
    ~Flow();
    enum {
      Type = kFlow
    };
    virtual int type() const { return Type; }

    Object* copy( Layer* l ) const ;
    Object* copy( Object* o ) const ;
    Object* copy( Projection* proj ) const ;

    float dn() const { return dn_; }
    control::QualityControl dnqual() const { return dnqual_; }
    float cn() const { return cn_; }
    control::QualityControl cnqual() const { return cnqual_; }

    bool dnok() const ;
    bool cnok() const ;

    void setDn( float d, control::QualityControl dnqual = control::RIGHT );
    void setCn( float c, control::QualityControl cnqual = control::RIGHT );

    void setDncn( int dn, int cn, control::QualityControl dnqual = control::RIGHT, control::QualityControl cnqual = control::RIGHT, int rad = 0 );

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

    int32_t dataSize() const ;
    int32_t serializeToArray( char* data ) const ;
    int32_t parseFromArray( const char* data );

  private:
    QImage* pix_;
    QPoint* screenpos_;
    float dn_;
    float cn_;
    control::QualityControl dnqual_;
    control::QualityControl cnqual_;
    float radius_;
    
    FlowPath flow_;
    mutable QRect cached_rect_;

    QSize size() const ;
    bool preparePixmap();
};

}
}

#endif
