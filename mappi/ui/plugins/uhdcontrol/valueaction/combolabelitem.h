#ifndef COMBOLABELITEM_H
#define COMBOLABELITEM_H

#include "spectrvalueaction.h"

#include <commons/geobasis/geopoint.h>

#include <qpen.h>
#include <qmargins.h>
#include <qgraphicsitem.h>

#include <commons/geobasis/geopoint.h>

namespace meteo {
namespace map {
class Document;
class LayerItems;
} // map
} // meteo

namespace meteo {
namespace spectr {

class ComboLabelItem : public QGraphicsItem
{
public:
  enum LabelPosition { kBeforeAxis, kMiddleAxis, kAfterAxis, kAutoDetect };

  enum { Type = UserType + 30 };
  int type() const { return Type; }

  ComboLabelItem(map::LayerItems* layer = 0, QGraphicsItem* parent = 0, QGraphicsScene* scene = 0);
  virtual ~ComboLabelItem();

  //! Возвращает географическую координату маркера
  GeoPoint geoPos() const;
  //! Устанавливает значение географической координаты
  void setGeoPos(const GeoPoint& pos);

  void setTitle(const QString& text);
  void addText(const QString& label, const QString& value);
  void clearTexts();

  //! Изменяет значение отступов слева, сверхе, справа и снизу.
  //! Если значение равно -1, то значение соответствующего параметра не изменяется.
  void setPadding(int left, int top = -1, int right = -1, int bottom = -1);
  //!
  void setOffset(int offsetX, int offsetY);

  //! Возвращает экранную координату, рассчитанную по георафической.
  QPoint calcScenePoint() const;

  //! Возвращает расположение подписи относительно центра.
  Qt::Alignment alignment() const         { return align_;  }
  //! Устанавливает расположение подписи относительно центра.
  void setAlignment(Qt::Alignment align);

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
  QString title_;
  QStringList labels_;
  QStringList values_;
  GeoPoint geoPos_;

  Qt::Alignment align_;
  QMargins padding_;
  QPoint offset_;

  QColor colorText_;
  QColor colorBg_;
  QPen penBorder_;

  QRectF boundingRect_;

  map::LayerItems* layer_;

  QRectF cacheTitleRect_;
  QList<QRectF> cacheLabelRects_;
  QList<QRectF> cacheValueRects_;

};


} // spectr
} // meteo

Q_DECLARE_METATYPE( ::meteo::spectr::ComboLabelItem* )

#endif // COMBOLABELITEM_H
