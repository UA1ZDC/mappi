#include "animationcontroller.h"
#include <qtimer.h>
#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace internal {
AnimationControllerPrivate::AnimationControllerPrivate( int timeout, bool isRepeated )
  : QObject(nullptr),
    isRepeated_(isRepeated)
{
  this->timer_.setInterval(timeout);
  QObject::connect( &this->timer_, &QTimer::timeout, this, &AnimationControllerPrivate::slotTimeout );
  this->currentState_ = kStopped;
}

void AnimationControllerPrivate::animationStart()
{
  if ( kStopped == this->currentState_ ) {
    if ( false == this->images_.isEmpty() ){
      this->setAnimationSlide(0);
      this->timer_.start();
      this->currentState_ = kStarted;
      emit signalAnimationStarted();
    }
    else {
      warning_log << QObject::tr("Ошибка: невозможно запустить анимацию без слайдов");
    }
  }
  else {
    warning_log << QObject::tr("Внимание: попытка запустить анимацию повторно") ;
    return;
  }
}

AnimationControllerPrivate::State AnimationControllerPrivate::getInternalState()
{
  return this->currentState_;
}

int AnimationControllerPrivate::animationNext()
{
  bool cycled = this->isAnimationRepeated();
  int currentIndex = this->getCurrentAnimationSlide();
  int slidesCount = this->getAnimationSlidesCount();

  int newSelectedIndex = -1;

  if (currentIndex == -1) {
    return -1;
  }
  else if ( (currentIndex >= 0) && (currentIndex < slidesCount - 1) ){
      newSelectedIndex = currentIndex + 1;
  }
  else if ( (slidesCount - 1 == currentIndex) && (true == cycled) ) {
    newSelectedIndex = 0;
  }

  if ( (newSelectedIndex >= 0) && (newSelectedIndex < slidesCount) ) {
    this->setAnimationSlide(newSelectedIndex);
  }
  return newSelectedIndex;
}

int AnimationControllerPrivate::animationPrew()
{
  bool cycled = this->isAnimationRepeated();
  int currentIndex = this->getCurrentAnimationSlide();
  int slidesCount = this->getAnimationSlidesCount();

  int newSelectedIndex = -1;
  if ( -1 == currentIndex ) {
    return -1;
  }

  else if ( currentIndex >= 1 && currentIndex < slidesCount ){
      newSelectedIndex = currentIndex - 1;
  }
  else if ( (0 == currentIndex) && (true == cycled ) ) {
    newSelectedIndex = slidesCount - 1;
  }


  if ( (newSelectedIndex >= 0) && (newSelectedIndex < slidesCount) ) {
    this->setAnimationSlide(newSelectedIndex);
  }
  return newSelectedIndex;
}

void AnimationControllerPrivate::animationPause()
{
  if ( kStarted == this->currentState_ ){
    this->timer_.stop();
    this->currentState_ = kPaused;
    emit signalAnimationPaused();
  }
  else {
    error_log << QObject::tr("Ошибка: невозможно поставить на паузу анимацию, которая не запущена");
  }
}

void AnimationControllerPrivate::animationResume()
{
  if ( kPaused == this->currentState_ ){
    this->timer_.start();
    this->currentState_ = kStarted;
    emit signalAnimationResumed();
  }
  else {
    error_log << QObject::tr("Ошибка: невозможно запустить после паузы анимацию, которая не запущена");
  }
}

void AnimationControllerPrivate::animationStop()
{
  if ( kStopped != this->currentState_ ) {
    this->currentState_ = kStopped;
    this->timer_.stop();
    emit signalAnimationFinished();
  }
  else {
    //остановка уже остановленной анимации не требуется
  }
}

void AnimationControllerPrivate::setTimeout(int value)
{
  this->timer_.setInterval(value);
}

int AnimationControllerPrivate::getTimeout()
{
  return this->timer_.interval();
}

void AnimationControllerPrivate::setAnimationSlide( int value )
{
  if ( 0 <= value && value < this->images_.size() ) {
    this->currentSlide_ = value;
    emit signalAnimationSlideChanged(value);
  }
}

int AnimationControllerPrivate::getCurrentAnimationSlide()
{
  return this->currentSlide_;
}

int AnimationControllerPrivate::getAnimationSlidesCount()
{
  return this->images_.size();
}

void AnimationControllerPrivate::setSlides( const QList<QImage>& data )
{
  this->images_ = data;
}

const QList<QImage>& AnimationControllerPrivate::getCurrentImages()
{
  return this->images_;
}

bool AnimationControllerPrivate::isAnimationRepeated()
{
  return this->isRepeated_;
}

void AnimationControllerPrivate::setAnimationRepeated( bool repeat )
{
  this->isRepeated_ = repeat;
  emit signalAnimationRepeatChanged(repeat);
}

void AnimationControllerPrivate::slotTimeout()
{
  if ( -1 == this->animationNext() ) {
    this->animationStop();
  }
}

}

AnimationController::AnimationController()
  : AnimationController(1000, true)
{

}

AnimationController::AnimationController(int timeout, bool repeat)
{
  this->privatedata_ = std::shared_ptr<internal::AnimationControllerPrivate>( new internal::AnimationControllerPrivate(timeout, repeat) );
  this->initConnect();
}

AnimationController::AnimationController( const AnimationController& other ) : QObject (nullptr)
{
  this->privatedata_ = other.privatedata_;
  this->initConnect();
}

AnimationController::~AnimationController()
{

}

bool AnimationController::isActive()
{
  return this->privatedata_->getInternalState() == internal::AnimationControllerPrivate::kStarted;
}

bool AnimationController::isPaused()
{
  return this->privatedata_->getInternalState() == internal::AnimationControllerPrivate::kPaused;
}

bool AnimationController::isStopped()
{
  return this->privatedata_->getInternalState() == internal::AnimationControllerPrivate::kStopped;
}

void AnimationController::animationNext()
{
  this->privatedata_->animationNext();
}

void AnimationController::setTimeout( int timeout )
{
  this->privatedata_->setTimeout(timeout);
}

void AnimationController::animationPrew()
{
  this->privatedata_->animationPrew();
}

void AnimationController::animationStart()
{  
  this->privatedata_->animationStart();
}

void AnimationController::animationPause()
{
  this->privatedata_->animationPause();
}

void AnimationController::animationResume()
{
  this->privatedata_->animationResume();
}

void AnimationController::animationStop()
{
  this->privatedata_->animationStop();
}

void AnimationController::setRepeat( bool repeated )
{
  this->privatedata_->setAnimationRepeated(repeated);
}

void AnimationController::initConnect()
{
  QObject::connect( this->privatedata_.get(), &internal::AnimationControllerPrivate::signalAnimationRepeatChanged, this, &AnimationController::signalRepeatChanged );
  QObject::connect( this->privatedata_.get(), &internal::AnimationControllerPrivate::signalAnimationSlideChanged, this, &AnimationController::signalAnimationSlideChanged );
  QObject::connect( this->privatedata_.get(), &internal::AnimationControllerPrivate::signalAnimationStarted, this, &AnimationController::signalAnimationStarted );
  QObject::connect( this->privatedata_.get(), &internal::AnimationControllerPrivate::signalAnimationFinished, this, &AnimationController::signalAnimationFinished );
  QObject::connect( this->privatedata_.get(), &internal::AnimationControllerPrivate::signalAnimationResumed, this, &AnimationController::signalAnimationResumed );
  QObject::connect( this->privatedata_.get(), &internal::AnimationControllerPrivate::signalAnimationPaused, this, &AnimationController::signalAnimationPaused );
}

void AnimationController::updateImageList( const QList<QImage>& images )
{  
  switch (this->privatedata_->getInternalState()) {
  case internal::AnimationControllerPrivate::kStopped:{
    this->privatedata_->setSlides(images);
    break;
  }
  case internal::AnimationControllerPrivate::kPaused:{
    if ( images.size() == this->privatedata_->getAnimationSlidesCount() ){
      this->privatedata_->setSlides(images);
    }
    else {
      error_log << QObject::tr("Ошибка: во время паузы можно изменять только слайды, изменять их количество запрещено");
    }
    break;
  }
  case internal::AnimationControllerPrivate::kStarted:{
    error_log << QObject::tr("Ошибка: изменять слайды во время воспроизведения анимации запрещено");
    break;
  }
  }
}

QImage AnimationController::getImageByIndex( int index )
{
  auto images = this->privatedata_->getCurrentImages();
  if ( 0 <= index && index <= images.size() ){
    return images[index];
  }
  return QImage();
}

}
