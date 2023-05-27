#ifndef METEO_COMMONS_UI_MAP_GEOTEXT_H
#define METEO_COMMONS_UI_MAP_GEOTEXT_H

#include "label.h"

#include <qlist.h>
#include <qimage.h>

namespace meteo {
namespace map {

class GeoPolygon;

class GeoText : public Label
{
  public:
    GeoText( Layer* layer );
    GeoText( Object* parent );
    GeoText( Projection* proj );
    GeoText( const meteo::Property& prop = meteo::Property() );
    ~GeoText();

    enum {
      Type = kText
    };
    virtual int type() const { return Type; }

    Object* copy( Layer* l ) const ;
    Object* copy( Object* o ) const ;
    Object* copy( Projection* proj ) const ;

    QList<QRect> boundingRect( const QTransform& transform ) const ;
    QRect boundingRect() const;

    QList<GeoVector> skeletInRect( const QRect& rect, const QTransform& transform ) const ;

    const QString& text() const { return text_; }
    void setText( const QString& text );

    bool hasValue() const { return hasvalue_; }
    float value() const { return value_; }
    void setValue( float value, const QString& format = "4' '.2'0'", const QString& unit = QString() );

    int minimumScreenDistance( const QPoint& pos, QPoint* cross = 0 ) const ;

    QList< QPair< QPoint,float > > screenPoints( const QTransform& transform ) const ;

    int32_t dataSize() const ;
    int32_t serializeToArray( char* arr ) const ;
    int32_t parseFromArray( const char* data );

    void prepareChange();

  protected:
    bool isEmpty() const { return ( true == text_.isEmpty() ); }

  private:
    bool hasvalue_;
    QString text_;
    float value_;
    QString format_;
    QString unit_;
    QString pictext_;

    QRect cached_rect_;

    QList< QPair<QPoint, float> > isolinePoints( const QVector<QPolygon>& parentscreen ) const ;
    QList< QPair<QPoint, float> > geolinePoints( const QVector<QPolygon>& parentscreen ) const ;
    QList< QPair<QPoint, float> > geotextPoints( const QVector<QPolygon>& parentscreen ) const ;
    QList< QPair<QPoint, float> > pointsTextOnLine( const QVector<QPolygon>& parentscreen ) const ;
    QList< QPair<QPoint, float> > pointsTextHorizont( const QVector<QPolygon>& parentscreen ) const ;
    bool preparePixmap();
};

}
}

#endif
