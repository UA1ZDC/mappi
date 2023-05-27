#ifndef METEO_COMMONS_UI_MAP_RAMKA_H
#define METEO_COMMONS_UI_MAP_RAMKA_H

#include <qpen.h>

class QPainter;

namespace meteo {

class GeoPoint;

namespace map {

class Document;

class Ramka
{
  public:
    Ramka( Document* doc, int halfwidth );
    virtual ~Ramka();

    virtual bool drawOnDocument( QPainter* painter );

    virtual QPolygon calcRamka( int mapindent, int docindent ) const ;

    static void intersectionPoints( const QPolygon& first, const QPolygon& second, QList<QPoint>* intersections );

    void setHalfWidth( int w );
    int halfwidth() const { return halfwidth_; }

    virtual Ramka* copy( Document* d ) const ;

  protected:
    Document* document_;
    QPen pen_;

  private:
    int halfwidth_;

    void drawGridLabels( QPainter* painter );
    void drawLongitudes( QPainter* painter, const QPolygon& ramka );
    void drawLatitides( QPainter* painter, const QPolygon& ramka );

    bool rectIntersectOneOfRects( const QRect& rect, const QList<QRect>& rects );
    QList<QRect> lblrects_;
};

}
}

#endif
