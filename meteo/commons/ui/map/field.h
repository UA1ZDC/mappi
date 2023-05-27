#ifndef METEO_COMMONS_UI_MAP_FIELD_H
#define METEO_COMMONS_UI_MAP_FIELD_H

#include <qglobal.h>

#include <commons/geobasis/geopoint.h>
#include <commons/funcs/tcolorgrad.h>
#include <meteo/commons/proto/meteo.pb.h>

#include "object.h"

namespace obanal {
class TField;
}

namespace meteo {
namespace map {

class Document;

class Field : public Object
{
  public:
    Field( Layer* layer );
    ~Field();

    Object* copy( Layer* l ) const { Q_UNUSED(l); return nullptr; };
    Object* copy( Object* o ) const { Q_UNUSED(o); return nullptr; };
    Object* copy( Projection* p ) const { Q_UNUSED(p); return nullptr; };
    
    QList<QRect> boundingRect( const QTransform& transform ) const { Q_UNUSED(transform); return QList<QRect>() << QRect(); }
    virtual QList<GeoVector> skeletInRect( const QRect& rect, const QTransform& transform ) const ;

    void setData( obanal::TField* field );

    void render( QPainter* painter, const QRect& target, Document* document );
    int minimumScreenDistancePow2( const QPoint& pos, double scalepower ) const;
    bool hasValue() const { return false; }
    float value() const { return 999999; }
    void setValue( double val, const QString& format = "4' '.2'0'", const QString& unit = QString()  );

  private:
    obanal::TField* field_;
    TColorGrad grad_;
};


}
}

#endif
