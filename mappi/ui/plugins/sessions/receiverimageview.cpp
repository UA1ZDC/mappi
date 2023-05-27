#include "receiverimageview.h"

#include <qevent.h>
#include <qscrollbar.h>
#include <qapplication.h>

#include <cross-commons/debug/tlog.h>

namespace mappi {

ReceiverImageView::ReceiverImageView(QWidget* parent)
  : QGraphicsView(parent)
{}

ReceiverImageView::ReceiverImageView(QGraphicsScene* scene, QWidget* parent)
  : QGraphicsView(parent)
{
  setScene(scene);
}

void ReceiverImageView::mousePressEvent(QMouseEvent* event)
{
  if(event->buttons() & Qt::LeftButton) {
    oldPoint_ = event->pos();
    setCursor(Qt::DragMoveCursor);
  }
}

void ReceiverImageView::mouseReleaseEvent(QMouseEvent* event)
{
  if(false == event->buttons() && Qt::LeftButton) {
    setCursor(Qt::ArrowCursor);
  }
}

void ReceiverImageView::mouseMoveEvent(QMouseEvent* event)
{
  if(event->buttons() & Qt::LeftButton) {
    int offsetX = (oldPoint_.x() - event->pos().x()) * 4,
        offsetY = (oldPoint_.y() - event->pos().y()) * 4;

    oldPoint_ = event->pos();
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() - offsetX);
    verticalScrollBar()->setValue(verticalScrollBar()->value() + offsetY);
  }
}

} // mappi
