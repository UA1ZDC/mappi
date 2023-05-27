#ifndef METEO_COMMONS_UI_MAP_FLOWPATH_H
#define METEO_COMMONS_UI_MAP_FLOWPATH_H

#include <qpoint.h>
#include <qsize.h>
#include <qrect.h>
#include <qpainterpath.h>

#include <commons/meteo_data/meteo_data.h>

class QPolygon;

namespace meteo {
namespace map {

class Flow;

class FlowPath
{
  public:
    FlowPath(
        Flow* fl,
        int dn,
        int cn,
        control::QualityControl dnqual,
        control::QualityControl cnqual,
        int radius = 0 );
    FlowPath( Flow* fl = 0 );
    ~FlowPath();

    QRect boundingRect() const ;
    QRectF dfrect() const ;
    const QString& dfstring() const { return dfrect_; }

    void draw( QPainter* path ) const ;

    QSize size() const ;

    const QPoint& pos() const { return pos_; }
    int dn() const { return dn_; }
    int cn() const { return cn_; }

    void setPos( const QPoint& pnt ) { pos_ = pnt; clearPrimitivies(); }
    void setDd( float d, control::QualityControl qual ) { dn_ = d; dqual_ = qual; clearPrimitivies(); }
    void setFf( float f, control::QualityControl qual ) { cn_ = f; fqual_ = qual; clearPrimitivies(); }


  private:
    QPoint pos_;
    int dn_;
    int cn_;
    control::QualityControl dqual_;
    control::QualityControl fqual_;
    int radius_;
    QPainterPath path_;
    Flow* flow_;

    mutable QList<QPolygonF> poly_;
    mutable QList<QLineF> line_;
    mutable QPainterPath circle_;
    mutable QRect cached_rect_;
    mutable QString dfrect_;
    mutable QPointF finish_;

    void clearPrimitivies() const ;
    void buildPrimitivies() const ;

    void buildDF() const ;
    void drawDF( QPainter* p ) const ;
 
    void addLine( float angle, float len, float px, float py ) const ;
    void addLine( QPolygonF* p, float angle, float len, float px, float py) const ;
    void addCircle( const QPoint& pnt, int radius ) const ;
    void addTriangle( float angle, float width, float len, float px, float py ) const ;
};

}
}

#endif
