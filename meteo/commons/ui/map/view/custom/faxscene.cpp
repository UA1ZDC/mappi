#include "faxscene.h"
#include <cross-commons/debug/tlog.h>

FaxScene::FaxScene(QObject *parent) :
  QGraphicsScene(parent)
{
}

void FaxScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  emit faxMousePress();
  QGraphicsScene::mousePressEvent(event);
}

void FaxScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  emit faxMouseRelease();
  QGraphicsScene::mouseReleaseEvent(event);
}

void FaxScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  emit faxMouseMove();
  QGraphicsScene::mouseMoveEvent(event);
}
