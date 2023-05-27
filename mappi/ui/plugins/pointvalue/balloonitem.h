#ifndef MAPPI_UI_PLUGINS_POINTVALUE_BALLOONITEM_H
#define MAPPI_UI_PLUGINS_POINTVALUE_BALLOONITEM_H

#include <qpen.h>
#include <qmargins.h>
#include <qgraphicsitem.h>

#include <commons/geobasis/geopoint.h>

namespace meteo {
namespace map {

class Document;
class LayerItems;

class BalloonItem : public QGraphicsItem
{
public:
  enum LabelPosition { kBeforeAxis, kMiddleAxis, kAfterAxis, kAutoDetect };

  enum { Type = UserType + 35 };
  int type() const { return Type; }

  BalloonItem(map::LayerItems* layer = 0, QGraphicsItem* parent = 0, QGraphicsScene* scene = 0);
  virtual ~BalloonItem();

  //! Возвращает географическую координату маркера
  GeoPoint geoPos() const;
  //! Устанавливает значение географической координаты
  void setGeoPos(const GeoPoint& pos);

  void addText(const QString& label, const QString& value);
  void clearTexts();

  //! Изменяет значение отступов слева, сверху, справа и снизу.
  //! Если значение равно -1, то значение соответствующего параметра не изменяется.
  void setPadding(int left, int top = -1, int right = -1, int bottom = -1);
  //! Смещение в процентах относительно левого угла.
  void setOffset(int x, int y);

  //! Возвращает экранную координату, рассчитанную по георафической.
  QPointF calcScenePoint() const;

  void setTextColor(const QColor& color)        { colorText_ = color; }
  void setBorderColor(const QColor& color)      { penBorder_.setColor(color); }
  void setBackgroundColor(const QColor& color)  { colorBg_ = color; }

  void setBorder(int width) { penBorder_.setWidth(width); }

  virtual QRectF boundingRect() const;
  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

  void setLayer(map::LayerItems* layer) { layer_ = layer; }

protected:
  QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value);

private:
  map::Document* document() const;
  void calcBoundingRect();

private:
  QStringList labels_;
  QStringList values_;
  GeoPoint geoPos_;

  QMargins padding_;
  double xCenter_ = 0.5;
  double yCenter_ = 0.5;

  QColor colorText_;
  QColor colorBg_;
  QPen penBorder_;

  QRectF boundingRect_;

  map::LayerItems* layer_;

  QList<QRectF> cacheLabelRects_;
  QList<QRectF> cacheValueRects_;
};


} // map
} // meteo

Q_DECLARE_METATYPE( ::meteo::map::BalloonItem* )

#endif // MAPPI_UI_PLUGINS_POINTVALUE_BALLOONITEM_H
