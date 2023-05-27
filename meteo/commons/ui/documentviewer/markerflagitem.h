#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_MARKERITEM_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_MARKERITEM_H


#include <qgraphicsitem.h>

#include <meteo/commons/ui/map/geopolygon.h>
#include "faxgeoobject.h"


namespace meteo {
namespace map {


//class DrawObject;


//! Графический элемент, предназначенный для отрисовки маркера узла и
//! выполнения манипуляций (выделение, перемещение) с ним.
class MarkerFlagItem : public QGraphicsItem
{
public:
  //! Перечисление State описывает состояния, в которых может находится маркер, в зависимости
  //! от манипуляций выполняемых пользователем
  enum State {
    kNormal,  //!< обычное состояние, никаких манипуляций не производится
//    kHover,   //!< на маркер наведён указатель мыши
    kPress,   //!< над маркером нажата клавиша мыши
    kRelease, //!< над маркером отпущена клавиша мыши
    kClick,   //!< щелчёк мыши на маркере
    kDrag,    //!< маркер перетаскивается
    kDrop     //!< маркер брошен (перетаскивание завершено)
  };
  //! Перечистение StyleRole описывает стилевые роли используемые при отрисовки маркера.
  enum StyleRole {
    kNormalStyleRole,   //!< Общий стиль.
//    kHoverStyleRole,    //!< Используется когда на маркер наведён указатель мыши.
    kSelectedStyleRole  //!< Используется когда маркер выделен.
  };

  MarkerFlagItem(int size = 10, int number = 0);
  virtual ~MarkerFlagItem();

  virtual QRectF boundingRect() const;
  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

  //! Устанавливает центр маркера в позицию pos
  void setScenePoint(const QPoint& pos, bool recalcGeoPoint = true);
  //! Устанавливает значение географической координаты
  void setScenePointForRelease(const QPoint& pos, bool recalcGeoPoint = true);
  //! Устанавливает значение географической координаты
  void setGeoPoint(const GeoPoint& point, bool recalcScenePoint = true);
  //! В соответствии со значением флага flag разрешает или запрещает перемещение маркера
  void setMovable(bool flag);
  //! Возвращает true, если маркер является перемещаемым
  bool isMovable() const;
  //! Возвращает текущее состояние маркера
  State state() const { return state_; }
  //! Возвращает географическую координату, к которой привязан маркер
  GeoPoint geoPoint() const { return geoPoint_; }
  //! Возвращает координату на сцене, к которой привязан маркер
  QPoint scenePoint() const;
  //! Расчитывает значение географической координаты маркера по координате на сцене
  void updateGeoPoint();
  //! Расчитывает значение координаты на сцене по географической координате
  void updateScenePoint();
  void updateScenePointForRelease();
  //! Устанавливает стиль маркера для роли role.
  void setStyle(StyleRole role, const QPen& pen, const QBrush& brush);
  void setColor( const QColor& color );

  void setFaxgeob( FaxGeoObject* geo){ faxgeob = geo;}
//  FaxGeoObject* setFaxgeob( FaxGeoObject* geo){ return faxgeob = geo;}

  void clearFaxgeob();
  FaxGeoObject* geoobject();

protected:
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
//  virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
//  virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
  void initStyles();
  void setState(State newState);

private:
//  DrawObject* object_;
  GeoPoint geoPoint_;

//  bool hovered_;
  bool selected_;

  State state_;
  QDateTime lastMousePress_;

  int size_;
  int number_;

  QRect boundingRect_;

  QMap<StyleRole,QPen> pen_;
  QMap<StyleRole,QBrush> brush_;

  FaxGeoObject* faxgeob;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_MARKERITEM_H
