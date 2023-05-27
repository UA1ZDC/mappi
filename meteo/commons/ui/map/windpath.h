#ifndef METEO_COMMONS_UI_MAP_WINDPATH_H
#define METEO_COMMONS_UI_MAP_WINDPATH_H

#include <qpoint.h>
#include <qsize.h>
#include <qrect.h>
#include <qpainterpath.h>

#include <commons/meteo_data/meteo_data.h>

class QPolygon;

namespace meteo {
namespace map {

class Wind;

class WindPath
{
  public:
    WindPath(
        Wind* w,
        int dd,
        int ff,
        control::QualityControl ddqual,
        control::QualityControl ffqual,
        int radius = 0 );
    WindPath( Wind* w = 0 );
    ~WindPath();

    QRect boundingRect() const ;

    void draw( QPainter* path ) const ;

    QSize size() const ;

    const QPoint& pos() const { return pos_; }
    int dd() const { return dd_; }
    int ff() const { return ff_; }

    void setLenft( float l );
    float lenft() const { return lenft_; }

    void setPos( const QPoint& pnt ) { pos_ = pnt; clearPrimitivies(); }
    void setDd( float d, control::QualityControl qual ) { dd_ = d; dqual_ = qual; clearPrimitivies(); }
    void setFf( float f, control::QualityControl qual ) { ff_ = f; fqual_ = qual; clearPrimitivies(); }


  private:
    QPoint pos_;
    int dd_;
    int ff_;
    control::QualityControl dqual_;
    control::QualityControl fqual_;
    int radius_;
    QPainterPath path_;
    Wind* wind_;

    mutable QList<QPolygonF> poly_;
    mutable QList<QLineF> line_;
    mutable QPainterPath circle_;
    mutable QRect cached_rect_;
    mutable QString dfrect_;
    float lenft_;

    void clearPrimitivies() const ;
    void buildPrimitivies() const ;

    void buildDF() const ;
    void drawDF( QPainter* p ) const ;
 
    void addLine( float angle, float len, float px, float py ) const ;
    void addLine( QPolygonF* p, float angle, float len, float px, float py) const ;
    void addCircle( const QPoint& pnt, int radius ) const ;
};

}
}

#endif
