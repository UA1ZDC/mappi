#ifndef METEO_COMMONS_UI_MAP_ORNAMENT
#define METEO_COMMONS_UI_MAP_ORNAMENT

#include <qstring.h>
#include <qpolygon.h>
#include <qcolor.h>
#include <qmap.h>

#include <meteo/commons/proto/map_ornament.pb.h>

class QPainter;
class QImage;

namespace meteo {
namespace map {

class GeoPolygon;

class Ornament
{
  public:
    Ornament( const QString& pattern, float linewidth, QRgb color );
    Ornament( const QString& pattern, float linewidth, QRgb color, bool mirror );
    ~Ornament();

    int width( float linewidth ) const ;

    void paintOnDocument( QPainter* painter, const QPolygon& polygon );

    static proto::Ornament ornament( const QString& name );

  private:
    proto::Ornament ornament_;
    QMap< int, QImage > patterns_;
    float factor_;

//    int paintBit( QPainter* painter, QImage* img, const QPolygon& poly, int pnt_indx, QPointF* pnt );
    int step( float step, const QPolygon& poly, int pnt_indx, QPointF* pnt );

    QTransform perelomTransform( const QList<QPointF>& source, const QList<QPointF>& target, bool* ok ) const ;

    void setBitColor( QImage* img, QRgb rgba );

    
};

}
}

#endif
