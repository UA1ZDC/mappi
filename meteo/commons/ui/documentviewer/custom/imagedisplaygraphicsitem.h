#ifndef METEO_COMMONS_DOCUMENTVIEWER_CUSTOM_IMAGEDISPLAYGRAPHICSITEM_H
#define METEO_COMMONS_DOCUMENTVIEWER_CUSTOM_IMAGEDISPLAYGRAPHICSITEM_H

#include <qgraphicsitem.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <commons/geobasis/geovector.h>

namespace meteo {

class ImageDisplayGraphicsItem : public QGraphicsItem
{
public:
  ImageDisplayGraphicsItem( meteo::map::MapView *view );
  ImageDisplayGraphicsItem( meteo::map::MapScene *scene );
  virtual ~ImageDisplayGraphicsItem() override;

  virtual QRectF boundingRect() const override;
  void setRect( const QPolygon& source, const meteo::GeoVector& target );
  void setImage( const QImage& image );


protected:
  virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

  static QTransform createTriangleTransform(const QPolygonF& source, const QPolygonF& target);

private:  
  meteo::map::MapScene *scene_;
  QPolygon source_;
  meteo::GeoVector target_;
  QImage image_;  
};

}
#endif
