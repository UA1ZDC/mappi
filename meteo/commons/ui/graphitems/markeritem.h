#ifndef METEO_COMMONS_UI_GRAPHITEMS_MARKERITEM_H
#define METEO_COMMONS_UI_GRAPHITEMS_MARKERITEM_H

#include <qgraphicsitem.h>

#include <meteo/commons/ui/map/geopolygon.h>

namespace meteo {
namespace map {
class Document;
class LayerItems;
} // map
} // meteo

namespace meteo {

class MarkerItem;

//! Абстрактный интерфейс для оповещения об изменении состояния MarkerItem.
class MarkerItemClosure
{
public:
  MarkerItemClosure() {}
  virtual ~MarkerItemClosure(){}

  virtual void run(MarkerItem* item) = 0;

private:
  Q_DISABLE_COPY(MarkerItemClosure)
};

//! Графический элемент, предназначенный для отрисовки маркера и
//! выполнения манипуляций с ним (выделение, перемещение).
//!
//! \note: scenePos() не всегда возвращает верные координаты, поэтому лучше пользоваться calcScenePoint().
class MarkerItem : public QGraphicsObject
{
  Q_OBJECT
public:
  //! Перечисление State описывает состояния, в которых может находится маркер, в зависимости
  //! от манипуляций выполняемых пользователем
  enum State {
    kNormal,   //!< обычное состояние, никаких манипуляций не производится
    kHover,    //!< на маркер наведён указатель мыши
    kPress,    //!< над маркером нажата клавиша мыши
    kRelease,  //!< над маркером отпущена клавиша мыши
    kClick,    //!< щелчёк мыши на маркере
    kDrag,     //!< маркер перетаскивается
    kDrop,     //!< маркер брошен (перетаскивание завершено)
    kGroupDrag //!< перетаскивается группа маркеров
  };
  //! Перечистение StyleRole описывает стилевые роли используемые при отрисовки маркера.
  enum StyleRole {
    kNormalStyleRole,   //!< Общий стиль.
    kHoverStyleRole,    //!< Используется когда на маркер наведён указатель мыши.
    kSelectedStyleRole  //!< Используется когда маркер выделен.
  };

  MarkerItem(int size = 10, int round = 5, QGraphicsItem* parent = 0);
  virtual ~MarkerItem();

  virtual QRectF boundingRect() const;
  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

  //! Возвращает географическую координату маркера
  GeoPoint geoPos() const;
  //! Устанавливает значение географической координаты
  void setGeoPos(const GeoPoint& pos);
  //! В соответствии со значением флага flag разрешает или запрещает перемещение маркера
  void setMovable(bool flag);
  //! Возвращает true, если маркер является перемещаемым
  bool isMovable() const;
  //! Возвращает текущее состояние маркера
  void setState(State newState);
  State state() const { return state_; }
  //! Возвращает координату на сцене, к которой привязан маркер
  QPoint calcScenePoint() const;
  //! Устанавливает стиль маркера для роли role.
  void setStyle(StyleRole role, const QPen& pen, const QBrush& brush);
  //! Заменяет функцию, которая будет вызываться при изменении состояния маркера на fn.
  void setCallback(MarkerItemClosure* callback);
  //! Заменяет слой, на котором расположен объект на layer.
  void setLayer(map::LayerItems* layer) { layer_ = layer; }

protected:
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
  virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
  void initStyles();

  map::Document* document() const;

signals:
  void changePos();

private:
  MarkerItemClosure* callback_;
  map::LayerItems* layer_;

  GeoPoint geoPos_;

  bool hovered_;
  bool selected_;

  State state_;
  QDateTime lastMousePress_;

  int round_;
  QRectF boundingRect_;

  QMap<StyleRole,QPen> pen_;
  QMap<StyleRole,QBrush> brush_;
};

} // meteo

#endif // METEO_COMMONS_UI_GRAPHITEMS_MARKERITEM_H
