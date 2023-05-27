#ifndef MAPPI_UI_PLUGINS_UHDCONTROL_LABELITEM_H
#define MAPPI_UI_PLUGINS_UHDCONTROL_LABELITEM_H


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

class LabelItem : public QGraphicsItem
{
public:
  enum LabelPosition { kBeforeAxis, kMiddleAxis, kAfterAxis, kAutoDetect };

  enum { Type = UserType + 29 };
  int type() const { return Type; }

  LabelItem(map::LayerItems* layer = 0, QGraphicsItem* parent = 0, QGraphicsScene* scene = 0);
  virtual ~LabelItem();

  //! Возвращает географическую координату маркера
  GeoPoint geoPos() const;
  //! Устанавливает значение географической координаты
  void setGeoPos(const GeoPoint& pos);

  QString text() const { return text_; }
  void setText(const QString& text);

  //! Изменяет значение отступов слева, сверхе, справа и снизу.
  //! Если значение равно -1, то значение соответствующего параметра не изменяется.
  void setPadding(int left, int top = -1, int right = -1, int bottom = -1);
  //!
  QMargins padding() const { return padding_; }
  //!
  void setOffset(int offsetX, int offsetY);

  //! Возвращает экранную координату, рассчитанную по георафической.
  QPoint calcScenePoint() const;

  //! Возвращает расположение подписи относительно центра.
  Qt::Alignment alignment() const         { return align_;  }
  //! Устанавливает расположение подписи относительно центра.
  void setAlignment(Qt::Alignment align)  { align_ = align; }

  void setTextColor(const QColor& color)        { colorText_ = color; }
  void setBorderColor(const QColor& color)      { penBorder_.setColor(color); }
  void setBackgroundColor(const QColor& color)  { colorBg_ = color; }

  void setBorder(int width) { penBorder_.setWidth(width); }

  QRectF textRect() const;
  virtual QRectF boundingRect() const;
  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

  void setLayer(map::LayerItems* layer) { layer_ = layer; }

protected:
  QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value);

private:
  map::Document* document() const;

private:
  QString text_;
  GeoPoint geoPos_;

  Qt::Alignment align_;
  QMargins padding_;
  QPoint offset_;

  QColor colorText_;
  QColor colorBg_;
  QPen penBorder_;

  QRect boundingRect_;

  map::LayerItems* layer_;
};


} // spectr
} // meteo

Q_DECLARE_METATYPE( ::meteo::spectr::LabelItem* )

#endif // MAPPI_UI_PLUGINS_UHDCONTROL_LABELITEM_H
