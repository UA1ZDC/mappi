#include "view.h"
#include <qscrollbar.h>
#include <QMouseEvent>
#include <QPointF>


namespace mappi {

namespace antenna {

View::View(CoordSystem& coord, QGraphicsScene *scene, QWidget *parent /*=*/) :
    QGraphicsView(scene, parent),
  coord_(coord),
  indicator_(new TargetPoint()),
  dst_({ -1, -1 })
{
  scene->setSceneRect(0, 0, 4096, 3072);        // 4K full frame size
  setScene(scene);
  // setCacheMode(QGraphicsView::CacheBackground);
  setDragMode(QGraphicsView::NoDrag);

  horizontalScrollBar()->setSliderPosition(2500);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // setSceneRect(frameGeometry());
  verticalScrollBar()->setSliderPosition(2500);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  indicator_->setPos(0, 0);
  indicator_->hide();
  scene->addItem(indicator_);

  setCursor(Qt::CrossCursor);
}

View::~View()
{
}

void View::allowPosition(bool isAllow)
{
  if (isAllow) {
    indicator_->show();
    return ;
  }

  indicator_->hide();
}

void View::mousePressEvent(QMouseEvent* event)
{
  switch (event->button()) {
    // начать движение
    case Qt::LeftButton :
      if (indicator_-> isVisible()) {
        QPointF screen = event->localPos();

        indicator_->setPos(mapToScene(screen.x(), screen.y()));
        update();

        dst_ = coord_.toDegree(screen.x(), screen.y());
        emit newPosition(dst_.x, dst_.y);       // signal
      } else
        emit keepMoving();      // signal

      break ;

    // продолжить движение (только для наведения), берутся координаты целеуказателя
    case Qt::MiddleButton :
      if (indicator_-> isVisible()) {
        QPointF screen = mapFromScene(indicator_->pos());
        CoordSystem::point_t point = coord_.toDegree(screen.x(), screen.y());
        emit newPosition(point.x, point.y);     // signal
      }

      break ;

    // остановить движение
    case Qt::RightButton :
      emit stop();      // signal
      break ;

    default :
      break ;
  }

  QGraphicsView::mousePressEvent(event);
}

void View::resizeEvent(QResizeEvent *event)
{
  if (indicator_-> isVisible()) {
    QSize size = this->size();
    coord_.setScope(size.width(), size.height());

    CoordSystem::point_t point = coord_.fromDegree(dst_);
    indicator_->setPos(mapToScene(point.x, point.y));
    // update();
  }

  QGraphicsView::resizeEvent(event);
}

}

}
