#ifndef METEO_COMMONS_UI_MAP_VIEW_SELECTACTION_H
#define METEO_COMMONS_UI_MAP_VIEW_SELECTACTION_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include <commons/geobasis/geovector.h>

#include <qgraphicsitem.h>


namespace meteo {
namespace map {

class MapView;
class SaveDoc;
class SelectAction;

  //! Прямоугольное выделение
 class RectangleRubberItem : public QGraphicsRectItem
  {
  public:
    //! Тип изменения выделения по вертикали
    enum VerticResize {
      kNoVerticResize = -1,	//!< Не изменяется
      kTop      = 0,		//!< Изменение верхней границы
      kBottom   = 1,		//!< Изменение нижней границы
    };
    enum GorizontalResize {
      kNoGorizResize = -1,	//!< Не изменяется
      kLeft     = 2,		//!< Изменение левой границы
      kRight    = 3,		//!< Изменение правой границы
    };

    //! Состояние выделения
    enum State {
      kEmpty = -1,   //!< Не сформировано (есть только одна точка)
      kDrawing = 1,  //!< В процессе создания
      kFinished = 2, //!< Сформировано
      kResizing = 3, //!< Изменение размера выделения
      kMoving = 4,   //!< Изменение положения выделения
    };

    RectangleRubberItem(const QPoint & first, SelectAction* pObject, QGraphicsScene* scene = 0, QGraphicsItem * parent = 0);
    RectangleRubberItem(const QRect & rect, bool fixed,  SelectAction* pObject, 
			QGraphicsScene* scene = 0, QGraphicsItem * parent = 0);
    ~RectangleRubberItem();

    QRectF boundingRect() const;
    void paint(QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w);

    //    void setFirstPoint(const QPoint& pnt);
    void setSecondPoint(const QPoint& pnt);
    void setFinished() { state_ = kFinished; }

    bool isValid() { return state_ >= kDrawing; }
    bool isFinished() { return state_ >= kFinished; }

    void updateGeoPoint(bool notice = true);
    void updateScenePoint(bool onlyOutside, const QPointF& pos);
    bool updateScenePoint(const GeoPoint& tl, const GeoPoint& br);
    void resize(int newWidth, int newHeight);

    const GeoPoint& topLeft() { return tl_; }
    const GeoPoint& topRight()  { return tr_; }
    const GeoPoint& bottomLeft() { return bl_; }
    const GeoPoint& bottomRight() {return br_; }
    QRect sceneRect();
    

  private:
    void resizeRectangle(const QPointF& pos, const QPointF& lastPos);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

  private:
    QPointF first_;  //!< Координаты первой точки
    GeoPoint firstG_;

    GeoPoint tl_; //!< Верхний левый угол
    GeoPoint tr_; //!< Верхний правый угол
    GeoPoint br_; //!< Нижний правый угол
    GeoPoint bl_; //!< Нижний левый угол


    State state_; //!< Состояние выделения
    VerticResize verticResize_; //!< Тип изменения выделения по вертикали
    GorizontalResize gorizResize_; //!< Тип изменения выделения по горизонтали
    QCursor* cursor_; //!< Курсор до попадания внутрь области выделения
    SelectAction* parent_;

    bool fixed_; //!< Фиксированная рамка - true, интерактивная false
  };

  //------------------------------------------
  
  //! Интерфейс для объкта, создающим действие - выделение
  class SelectActionIface {
  public:
    SelectActionIface();
    virtual ~SelectActionIface();

    virtual void rubberFinished() {}
    virtual void setPoints(const GeoPoint& nw, const GeoPoint& ne, const GeoPoint& sw, const GeoPoint& se, const QRect& rect) = 0;
    void resizeBand(int newWidth, int newHeight);
    void setBand(const QRect& rect);

    //текст для действия по Enter внутри рамки, если такое необходимо
    virtual QString acceptText() { return QString(); }
    //действие по Enter внутри рамки
    virtual void acceptRubber() {}

  protected:
    bool createAction(map::MapScene* scene, const QRect& rect = QRect());
    void removeAction(bool notice = true);
    
    void setMajorCoords(const GeoPoint& tl, const GeoPoint& br);
    
  private:
    SelectAction* act_;
    
  };

  //------------------------------------------

  //! Действие для создания/удаления выделения на карте
class SelectAction  : public Action
{
  Q_OBJECT
public:
  SelectAction (const QRect& rect, MapScene* scene, SelectActionIface* parent);
  ~SelectAction();

  void setMajorCoords(const GeoPoint& corner1, const GeoPoint& corner2);
  //  void setMinorCoords(const GeoPoint& tr, const GeoPoint& bl);
  void resizeBand(int newWidth, int newHeight);					     
  void setBand(const QRect& rect);
public slots:
  void rubberChanged();
  void removeRubber(bool notice = true);
  void acceptRubber();

private:
  void mousePressEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void wheelEvent(QWheelEvent* event);
  void keyPressEvent( QKeyEvent* event );
  void addActionsToMenu( Menu* m ) const ;

private:
  bool fixed_; //!< true - фиксированный размер рамки; false - интерактивное изменение
  bool mapmove_; //!< true - происходит движение карты (для обновления положения фиксированной рамки)
  RectangleRubberItem* rubber_;

  SelectActionIface* parent_;
};

}
}
#endif // SELECTACTION _H
