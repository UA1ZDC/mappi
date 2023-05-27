#include "animationgraphicsitem.h"

namespace meteo {

AnimationGraphicsItem::AnimationGraphicsItem( meteo::map::MapScene* scene, const QList<QImage>& images, int timeout ) :
  QObject (nullptr),
  ImageDisplayGraphicsItem (scene),
  controller_(AnimationController(timeout, true))
{
  controller_.updateImageList(images);
  controller_.animationStart();
  this->initConnect();
}

AnimationGraphicsItem::AnimationGraphicsItem( meteo::map::MapScene* scene, const AnimationController &controller) :
  QObject (nullptr),
  ImageDisplayGraphicsItem (scene),
  controller_(controller)
{
  this->initConnect();
}

AnimationGraphicsItem::~AnimationGraphicsItem()
{
}

void AnimationGraphicsItem::initConnect()
{
  QObject::connect( &this->controller_, &AnimationController::signalAnimationSlideChanged, this, &AnimationGraphicsItem::slotSlideChanged );
}

void AnimationGraphicsItem::slotSlideChanged( int index )
{
  auto image = this->controller_.getImageByIndex(index);
  this->setImage(image);
}

}
