#ifndef ANIMATIONCONTROLLER_H
#define ANIMATIONCONTROLLER_H

#include <qobject.h>
#include <qtimer.h>
#include <qlist.h>
#include <qimage.h>
#include <memory>

namespace meteo {
namespace internal {

class AnimationControllerPrivate : public QObject {
  Q_OBJECT
public:
  enum State {
    kStopped,
    kPaused,
    kStarted
  };
  AnimationControllerPrivate(int timeout, bool isRepeated);

  State getInternalState();

  void setSlides( const QList<QImage>& data );
  void animationStart();
  void animationPause();
  void animationResume();
  void animationStop();

  int animationNext();
  int animationPrew();

  void setTimeout( int value );
  int getTimeout();

  void setAnimationSlide( int value );
  int getCurrentAnimationSlide();
  int getAnimationSlidesCount();

  const QList<QImage>& getCurrentImages();

  bool isAnimationRepeated();
  void setAnimationRepeated( bool repeat );
private slots:
  void slotTimeout();
signals:
  void signalAnimationSlideChanged(int value);
  void signalAnimationRepeatChanged( bool newValue );
  void signalAnimationStarted();
  void signalAnimationPaused();
  void signalAnimationResumed();
  void signalAnimationFinished();

private:
  QTimer timer_;
  bool isRepeated_;
  State currentState_;
  QList<QImage> images_;
  int currentSlide_;
};

}
class AnimationController : public QObject
{
  Q_OBJECT
public:
  AnimationController();
  AnimationController(int timeout, bool repeat);
  AnimationController( const AnimationController& other );
  virtual ~AnimationController();

  void updateImageList( const QList<QImage>& images );
  QImage getImageByIndex( int index );
  void animationNext();
  void animationPrew();
  void animationStart();
  void animationStop();
  void animationPause();
  void animationResume();
  void setRepeat( bool repeated );
  void setTimeout( int timeout );

  bool isActive();
  bool isPaused();
  bool isStopped();

signals:
  void signalAnimationSlideChanged( int value );
  void signalRepeatChanged( bool newRepeat );
  void signalAnimationStarted();
  void signalAnimationPaused();
  void signalAnimationResumed();
  void signalAnimationFinished();

private:
  void initConnect();
  std::shared_ptr<internal::AnimationControllerPrivate> privatedata_;
};

}

#endif
