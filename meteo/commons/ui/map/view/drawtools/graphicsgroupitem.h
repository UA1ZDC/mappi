#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GRAPHICSGROUPITEM_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GRAPHICSGROUPITEM_H

#include <qgraphicsitem.h>
#include <qobject.h>

namespace meteo {
namespace map {

class GraphicsGroupItem : public QObject, public QGraphicsItemGroup
{
  Q_OBJECT
public:
  GraphicsGroupItem(QGraphicsItem *parent = nullptr);
  ~GraphicsGroupItem();

protected:
  void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
  void wheelEvent(QGraphicsSceneWheelEvent* event);

signals:
  void posChanged(QPointF);

private:
  QPointF delta_ = QPointF(0,0);
};

}
}

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GRAPHICSGROUPITEM_H
