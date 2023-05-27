#include "selectaction.h"
#include "../mapview.h"
//#include "savesxf.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include "../menu.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/map/event.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/custom/colorwidget.h>

#include <qcursor.h>

#define MIN_RUBBER_WIDTH 3
#define MIN_RUBBER_HEIGHT 3

namespace meteo {
namespace map {

  /*!
    \param first первая точка выделения
  */
  RectangleRubberItem::RectangleRubberItem(const QPoint & first, SelectAction* pObject /*= 0*/, QGraphicsScene* scene /*= 0*/, QGraphicsItem * parent /*= 0*/):
    QGraphicsRectItem(parent),
    first_(first),
    state_(kEmpty),
    verticResize_(kNoVerticResize),
    gorizResize_(kNoGorizResize),
    cursor_(0),
    parent_(pObject),
    fixed_(false)
  {
    scene->addItem(this);
    setFlag(QGraphicsItem::ItemIsMovable);
    QPen pen(Qt::darkRed);
    pen.setWidth(2);
    setPen(pen);
    setAcceptHoverEvents (true);

    MapScene* mapScene = qobject_cast<MapScene*>(scene);
    if ( 0 == mapScene || 0 == mapScene->document()) {
      debug_log << QObject::tr("Невозможно расчитать географическую координату узла, не создан документ");
      return;
    }

    firstG_ = mapScene->document()->screen2coord(scenePos() + first_);
  }

  RectangleRubberItem::RectangleRubberItem(const QRect & brect, bool fixed,  SelectAction* pObject,
					   QGraphicsScene* scene /*= 0*/, QGraphicsItem * parent /*= 0*/):
    QGraphicsRectItem(parent),
    state_(kEmpty),
    verticResize_(kNoVerticResize),
    gorizResize_(kNoGorizResize),
    cursor_(0),
    parent_(pObject),
    fixed_(fixed)
  {
    scene->addItem(this);
    QPen pen(Qt::darkRed);
    pen.setWidth(2);
    setPen(pen);
    setAcceptHoverEvents (true);

    MapScene* mapScene = qobject_cast<MapScene*>(scene);
    if ( 0 == mapScene || 0 == mapScene->document()) {
      debug_log << QObject::tr("Невозможно расчитать географическую координату узла, не создан документ");
      return;
    }

    setRect(brect.normalized());
    first_ = rect().topLeft();
    firstG_ = mapScene->document()->screen2coord(scenePos() + first_);
    updateGeoPoint();
    setFinished();

  }

  RectangleRubberItem::~RectangleRubberItem()
  {
  }


  QRectF RectangleRubberItem::boundingRect() const
  {
    return QGraphicsRectItem::boundingRect();
  }


  void RectangleRubberItem::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
  {
    if (isValid()) {
      QGraphicsRectItem::paint(p, o, w);
    }
  }


  //! установка второй точки выделения
  void RectangleRubberItem::setSecondPoint(const QPoint& pnt)
  {
    state_ = kDrawing;
    setRect(QRectF(first_, pnt).normalized());
    updateGeoPoint();
  }


  //! Изменение размера выделения.
  /*!

    \param pos новое положение курсора в координатах сцены
    \param lastPos предыдущее положение курсора в координатах сцены
  */
  void RectangleRubberItem::resizeRectangle(const QPointF& pos, const QPointF& lastPos)
  {
    if (verticResize_ == kNoVerticResize && gorizResize_ == kNoGorizResize) {
      return;
    }

    QPointF diff = pos - lastPos;
    QPointF p1 = rect().topLeft();
    QPointF p2 = rect().bottomRight();

    switch (verticResize_) {
    case kTop:
      p1.setY(p1.y() + diff.y());
      if (p1.y() > p2.y()) {
	verticResize_ = kBottom;
      }
      break;
    case kBottom:
      p2.setY(p2.y() + diff.y());
      if (p1.y() > p2.y()) {
	verticResize_ = kTop;
      }
      break;
    default: {}
    }

    switch (gorizResize_) {
    case kLeft:
      p1.setX(p1.x() + diff.x());
      if (p1.x() > p2.x()) {
	gorizResize_ = kRight;
      }
      break;
    case kRight:
      p2.setX(p2.x() + diff.x());
      if (p1.x() > p2.x()) {
	gorizResize_ = kLeft;
      }
      break;
    default: {}
    }

    setRect(QRectF(p1, p2).normalized());
    if (0 != parent_) {
      updateGeoPoint();
    }
  }

  void RectangleRubberItem::mousePressEvent ( QGraphicsSceneMouseEvent * event )
  {
    if (verticResize_ != kNoVerticResize || gorizResize_ != kNoGorizResize) {
      state_ = kResizing;
    } else {
      state_ = kMoving;
      QGraphicsRectItem::mousePressEvent(event);
    }
  }

  //! Перемещение или изменение размера области выделеления
  void RectangleRubberItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
  {
    if (0 == event) return;

    if (state_ == kResizing) {
      resizeRectangle(event->scenePos(), event->lastScenePos());
    } else {
      QGraphicsRectItem::mouseMoveEvent(event);
    }
    if (0 != parent_) {
      updateGeoPoint();
    }
  }

  void RectangleRubberItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
  {
    if (state_ == kResizing || state_ == kMoving) {
      state_ = kFinished;
    }
    QGraphicsRectItem::mouseReleaseEvent(event);
  }

  //! Установка курсора внутри области выделения и типа изменения размера
  void RectangleRubberItem::hoverMoveEvent(QGraphicsSceneHoverEvent * event)
  {
    if (0 == event || fixed_) return;

    //FIXME пока так. не запомнить состояние курсора до входа в область выделения, т.к. он нулевой
    QApplication::restoreOverrideCursor();
    //--

    int limit = 20;
    if (event->pos().y() - rect().top() <= limit) {
     if (event->pos().x() - rect().left() <= limit) {
       QApplication::setOverrideCursor(QCursor(Qt::SizeFDiagCursor));
       verticResize_ = kTop;
       gorizResize_ = kLeft;
     } else if (rect().right() - event->pos().x() <= limit) {
       QApplication::setOverrideCursor(QCursor(Qt::SizeBDiagCursor));
       verticResize_ = kTop;
       gorizResize_ = kRight;
     } else {
       QApplication::setOverrideCursor(QCursor(Qt::SizeVerCursor));
       verticResize_ = kTop;
       gorizResize_ = kNoGorizResize;
     }
    } else if (rect().bottom() - event->pos().y() <= limit) {
      if (event->pos().x() - rect().left() <= limit) {
	QApplication::setOverrideCursor(QCursor(Qt::SizeBDiagCursor));
	verticResize_ = kBottom;
	gorizResize_ = kLeft;
      } else if (rect().right() - event->pos().x() <= limit) {
	QApplication::setOverrideCursor(QCursor(Qt::SizeFDiagCursor));
	verticResize_ = kBottom;
	gorizResize_ = kRight;
      } else {
	QApplication::setOverrideCursor(QCursor(Qt::SizeVerCursor));
	verticResize_ = kBottom;
	gorizResize_ = kNoGorizResize;
      }

    } else if (event->pos().x() - rect().left() <= limit) {
      QApplication::setOverrideCursor(QCursor(Qt::SizeHorCursor));
      verticResize_ = kNoVerticResize;
      gorizResize_ = kLeft;

    } else if (rect().right() - event->pos().x() <= limit) {
      QApplication::setOverrideCursor(QCursor(Qt::SizeHorCursor));
      verticResize_ = kNoVerticResize;
      gorizResize_ = kRight;
    } else {
      QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
      verticResize_= kNoVerticResize;
      gorizResize_ = kNoGorizResize;
    }
  }

  void RectangleRubberItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
  {
    if (fixed_) return;
    if (cursor_ == 0) {
      cursor_ = QApplication::overrideCursor();
    }
    QGraphicsItem::hoverEnterEvent(event);
  }

  void RectangleRubberItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
  {
    if (fixed_) return;
    cursor_ = 0;
    QApplication::restoreOverrideCursor();
    QGraphicsItem::hoverLeaveEvent(event);
  }

  QRect RectangleRubberItem::sceneRect()
  {
    return QRectF(scenePos() + rect().topLeft(), scenePos() + rect().bottomRight()).toRect();
  }

  //! Обновление координат углов
  /*!
    \param notice - true - предупреждать родителя, false - нет
   */
  void RectangleRubberItem::updateGeoPoint(bool notice /* = true*/)
  {
    MapScene* mapScene = qobject_cast<MapScene*>(scene());
    if ( 0 == mapScene || 0 == mapScene->document()) {
      debug_log << QObject::tr("Невозможно расчитать географическую координату узла, не создан документ");
      return;
    }

    tl_ = mapScene->document()->screen2coord(scenePos() + rect().topLeft());
    tr_ = mapScene->document()->screen2coord(scenePos() + rect().topRight());
    br_ = mapScene->document()->screen2coord(scenePos() + rect().bottomRight());
    bl_ = mapScene->document()->screen2coord(scenePos() + rect().bottomLeft());
    if (notice) {
      parent_->rubberChanged();
    }
  }

  //! Обновление положения на экране (масштаб выделения при масштабировании карты, изменение положения при движении карты)
  /*!
    \param onlyOutside true - изменять только, если курсор снаружи области (чтоб не реагировать на изменения, если
  движение мышки связано с внутренними изменениями)
    \param pos Положение курсора
  */
  void RectangleRubberItem::updateScenePoint(bool onlyOutside, const QPointF& pos)
  {
    if (!onlyOutside || (state_ != kResizing && state_ != kMoving)) {
      MapScene* mapScene = qobject_cast<MapScene*>(scene());
      if ( 0 == mapScene  || 0 == mapScene->document()) {
	debug_log << QObject::tr("Невозможно расчитать координату узла на сцене, не создан документ");
	return;
      }

      if (state_ != kDrawing && state_ != kEmpty) {
	setRect(QRectF(mapScene->document()->coord2screen(tl_) - scenePos(),
		       mapScene->document()->coord2screen(br_)-scenePos()).normalized());
      } else if (state_ == kDrawing) {

	first_ = mapScene->document()->coord2screen(firstG_) - scenePos();

      	setSecondPoint(pos.toPoint());
      }
    }
  }

  bool RectangleRubberItem::updateScenePoint(const GeoPoint& tl, const GeoPoint& br)
  {
    MapScene* mapScene = qobject_cast<MapScene*>(scene());
    if ( 0 == mapScene  || 0 == mapScene->document()) {
      debug_log << QObject::tr("Невозможно расчитать координату узла на сцене, не создан документ");
      return false;
    }

    QRect nrect = QRectF(mapScene->document()->coord2screen(tl) - scenePos(),
			  mapScene->document()->coord2screen(br)-scenePos()).normalized().toRect();
    QRect cur(rect().toRect());

    if (nrect != cur) {
      setRect(nrect);
      return true;
    }

    return false;
  }


  void RectangleRubberItem::resize(int newWidth, int newHeight)
  {
    if (rect().width() == newWidth && rect().height() == newHeight) {
      return;
    }

    QPointF p1 = rect().topLeft();
    QPointF p2 = p1 + QPoint(newWidth, newHeight);
    setRect(QRectF(p1, p2).normalized());

    if (0 != parent_) {
      updateGeoPoint();
    }
  }

//--------------------------------------

  SelectAction ::SelectAction (const QRect& rect,  MapScene* scene, SelectActionIface* parent)
  : Action(scene, "selectaction" ),
    fixed_(false),
    mapmove_(false),
    rubber_(0),
    parent_(parent)
  {
  if (false == hasView()) {
    return;
  }
  if (!rect.isNull()) {
    fixed_ = true;
    // QRect rect = view()->mapscene()->document()->documentRect();
    // int dx = rect.width()*0.2;
    // int dy = rect.height()*0.2;
    // rubber_ = new RectangleRubberItem(QRect(QPoint(rect.topLeft()+QPoint(dx, dy)),
    // 					    QSize(rect.width()-2*dx, rect.height()-2*dy)), fixed_, this, scene_);
    rubber_ = new RectangleRubberItem(rect, fixed_, this, scene_);
    item_ = rubber_;
  }
}

  SelectAction::~SelectAction()
  {
    removeRubber();
    if (scene_ != 0) {
      scene_->removeAction(this);
    }
  }

  //! Начинаем рисовать рамку, если ее ещё нет
  void SelectAction::mousePressEvent(QMouseEvent* event)
  {
    if( 0 == parent_ ||  false == hasView() ) {
      Action::mousePressEvent(event);
      return;
    }
    if (rubber_ == 0) {
      view()->setDragMode(QGraphicsView::NoDrag);

      QPoint pnt = view()->mapToScene(event->pos()).toPoint();
      rubber_ = new RectangleRubberItem(pnt, this, scene_);
      item_ = rubber_;
    } else if (fixed_) {
      mapmove_ = true;
    }
  }

  //! Процесс рисования рамки, если она ещё не закончена
void SelectAction::mouseMoveEvent(QMouseEvent* event)
{
  if ( false == hasView() ) {
    return;
  }
  if (rubber_ != 0 && !rubber_->isFinished()) {
    rubber_->setSecondPoint(view()->mapToScene(event->pos()).toPoint());
    return;
  } else if (mapmove_ && fixed_ && 0 != rubber_) {
    rubber_->updateGeoPoint(true);
  }

  Action::mouseMoveEvent(event);
}

  //! Окончание рисования рамки, если она не закончена. Иначе для интерактивной рамки обновление экранных координат (обработка движения карты,
  //рамка сохраняет географические координаты), для фиксированной обновление географических координат
void SelectAction::mouseReleaseEvent(QMouseEvent* event)
{
  if ( false == hasView() || 0 == parent_) {
    Action::mousePressEvent(event);
    return;
  }

  if(0 != rubber_) {
    if (!rubber_->isFinished()) {
      //FIXME пока такое ограничение, будет зависеть от уишки и удобства использования
      if (rubber_->rect().width() >= MIN_RUBBER_WIDTH &&
	  rubber_->rect().height() >= MIN_RUBBER_HEIGHT) {
	rubber_->setFinished();
	parent_->rubberFinished();
	QApplication::restoreOverrideCursor();
	view()->setDragMode(QGraphicsView::QGraphicsView::ScrollHandDrag);
      } else {
	removeRubber();
      }
    } else {
      if (fixed_) {
	rubber_->updateGeoPoint(true);
	mapmove_ = false;
      } else {
	rubber_->updateScenePoint(true, view()->mapToScene(event->pos()));
	rubberChanged();
      }
    }
  }

  Action::mouseReleaseEvent(event);
}

  //! Изменение масштаба карты: интрерактивная рамка сохраняет географические координаты, фиксированная экранные
  void SelectAction::wheelEvent(QWheelEvent* event)
  {
    if ( false == hasView() || 0 == parent_ ){
      Action::wheelEvent(event);
      return;
    }

    if(0 != rubber_) {
      if (fixed_) {
	rubber_->updateGeoPoint(true);
      } else {
	rubber_->updateScenePoint(false, view()->mapToScene(event->pos()));
      }
      rubberChanged();
    }
  }


  void SelectAction::addActionsToMenu( Menu* menu ) const
  {
    if(0 == menu ) return;
    if( !menu->isEmpty()){  menu->addSeparator();  }

    QPoint pos = view()->mapFromGlobal( QCursor::pos() );
    if (0 == rubber_ || !hasView() || !rubber_->contains( view()->mapToScene(pos) ) ) { //TODO Только внутри рамки или везде на карте?
      return ;
    }
    if (0 != parent_ && !parent_->acceptText().isNull()) {
      menu->addAction(QObject::tr("%1 (Enter)").arg(parent_->acceptText()), this, SLOT(acceptRubber()));
    }
    menu->addAction(QObject::tr("Удалить рамку (Esc)"), this, SLOT(removeRubber()));
    menu->addSeparator();
  }

  void SelectAction::acceptRubber()
  {
    if (0 != parent_) {
      parent_->acceptRubber();
    }
  }

  //! Передача новых координат родительскому объекту
  void SelectAction::rubberChanged()
  {
    if (0 ==  parent_) {
      return;
    }

    if (0 != rubber_) {
      parent_->setPoints(rubber_->topLeft(), rubber_->topRight(), rubber_->bottomRight(), rubber_->bottomLeft(), rubber_->sceneRect());
    } else {
      if ( false == hasView() || 0 == scene_->document() ||
	   0 == view()->mapscene() || 0 == view()->mapscene()->document()) {
	return;
      }

      int w = view()->mapscene()->document()->ramkaWidth();
      QRect docrect = view()->mapscene()->document()->documentRect();
      docrect.adjust(w, w, -w, -w);
      GeoPoint nw = scene_->document()->screen2coord(docrect.topLeft());
      GeoPoint ne = scene_->document()->screen2coord(docrect.topRight());
      GeoPoint se = scene_->document()->screen2coord(docrect.bottomRight());
      GeoPoint sw = scene_->document()->screen2coord(docrect.bottomLeft());
      parent_->setPoints(nw, ne, se, sw, docrect);
    }
  }

  //! Удаление рамки
  void SelectAction::removeRubber(bool notice /* = true*/)
  {
    if (hasView()) {
      view()->setDragMode(QGraphicsView::QGraphicsView::ScrollHandDrag);
    }

    if (0 != rubber_) {
      item_ = 0;
      delete rubber_;
      rubber_ = 0;
      if (notice) {
	rubberChanged();
      }
    }

    QApplication::restoreOverrideCursor();
    QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));

  }

  void SelectAction::keyPressEvent( QKeyEvent* event )
  {
    if (event->key() == Qt::Key_Escape) {
      removeRubber();
    } else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
      acceptRubber();
    }

  }

  //! Перерисовка рамки в соответствии с двумя противоположными углами (по диагонали)
  void SelectAction::setMajorCoords(const GeoPoint& corner1, const GeoPoint& corner2)
  {
    if (0 == rubber_ || !rubber_->isFinished()) return;

    if (rubber_->updateScenePoint(corner1, corner2)) {
      rubber_->updateGeoPoint(false);
    }
  }

  void SelectAction::resizeBand(int newWidth, int newHeight)
  {
    if (0 == rubber_ || !rubber_->isFinished()) return;
    rubber_->resize(newWidth, newHeight);
  }

  void SelectAction::setBand(const QRect& rect)
  {
    if (0 == rubber_ || !rubber_->isFinished()) return;
    rubber_->setRect(rect);
    rubber_->updateGeoPoint(false);
  }

  //---------------

  SelectActionIface::SelectActionIface():act_(0)
  {
  }

  SelectActionIface::~SelectActionIface()
  {
    removeAction();
  }

  bool SelectActionIface::createAction(map::MapScene* scene, const QRect& rect /* = QRect()*/)
  {
    if (scene == 0) {
      return false;
    }

//    removeAction(); //временно убрал, чтоб не падало

    act_ = new SelectAction(rect, scene, this);
    scene->addAction(act_);
    return true;
  }

  void SelectActionIface::removeAction(bool notice /* = true*/)
  {
    if (0 != act_) {
      act_->removeRubber(notice);
      delete act_;
      act_ = 0;
    }
  }

  void SelectActionIface::setMajorCoords(const GeoPoint& tl, const GeoPoint& br)
  {
    if (0 == act_) return;
    act_->setMajorCoords(tl, br);
  }


  void SelectActionIface::resizeBand(int newWidth, int newHeight)
  {
    if (0 == act_) return;
    act_->resizeBand(newWidth, newHeight);
  }

  void SelectActionIface::setBand(const QRect& rect)
  {
    if (0 == act_) return;
    act_->setBand(rect);
  }

}
}
