#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GEOPROXYWIDGET_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GEOPROXYWIDGET_H

#include <qgraphicsproxywidget.h>
#include <commons/geobasis/geopoint.h>

namespace meteo {
namespace map {

class Document;

class GeoProxyWidget : public QGraphicsProxyWidget
{
  Q_OBJECT
public:
  GeoProxyWidget(QGraphicsItem* parent = nullptr);
  void setGeoPos( const GeoPoint& pos );
  void paint( QPainter* painter, const QStyleOptionGraphicsItem *o, QWidget *w  );
  void setLock( bool lock ) { locked_ = lock; }

private:
  Document* doc() const;
  QPoint calcScenePoint() const;

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent* event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
  void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
  void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);

signals:
  void enter();
  void leave();

private:
  GeoPoint geoPos_;
  bool grabed_ = false;
  bool locked_ = false;
  bool wasMoved_ = false;
};

}
}
#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GEOPROXYWIDGET_H
