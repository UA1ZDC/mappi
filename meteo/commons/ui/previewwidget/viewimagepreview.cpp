#include "viewimagepreview.h"
#include <cross-commons/debug/tlog.h>
#include <qevent.h>
#include <qfile.h>
#include <qimage.h>
#include <qscrollbar.h>
#include <meteo/commons/global/global.h>
#include <qrect.h>
#include <qmath.h>
#include <qapplication.h>

namespace meteo {
static const int imageSpacing = 10;
static const int k = 3;

ViewImagePreview::ViewImagePreview(QWidget *parent ) :
  QGraphicsView(parent),
  scene_(new QGraphicsScene()),
  backgroundImages_(QList<QImage>()),
  scale_(0),
  angle_(0)
{
  scene_->setSceneRect( 0,0, 20000, 20000 );  

  this->setScene(scene_);    
  this->setDragMode(QGraphicsView::ScrollHandDrag);
  this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setRenderHint(QPainter::Antialiasing)  ;
  this->setRenderHint(QPainter::SmoothPixmapTransform);
  class VerticalScrollBar:public QScrollBar
  {
    virtual void wheelEvent(QWheelEvent* ) override {

    }
  } *scrollBar = new VerticalScrollBar();
  this->setVerticalScrollBar(scrollBar);

  QObject::connect( this->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SIGNAL(signalDocumentResized()));
  QObject::connect( this->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SIGNAL(signalDocumentResized()));
}

ViewImagePreview::~ViewImagePreview()
{
  this->scene_->deleteLater();
}

void ViewImagePreview::slotDestroyed()
{

}


QTransform ViewImagePreview::createTransform(double awidth, double aheight, double ascale, double offsetX, double offsetY, int angle)
{
  double scale = qPow(1.1, ascale);
  QTransform trans = QTransform::fromTranslate( awidth / 2 , aheight / 2 );
  trans = trans.scale(scale, scale);
  trans = trans.translate(offsetX, offsetY);
  trans = trans.rotate(angle);
  return trans;
}

bool ViewImagePreview::setImage(const QString& path)
{
  QFile file(path);
  if ( false == file.open(QFile::OpenModeFlag::ReadOnly) ){
    error_log << meteo::msglog::kFileNotFound
                 .arg(path);
    return false;
  }
  auto data = file.readAll();
  if ( true == data.isEmpty() ){
    error_log << meteo::msglog::kFileReadFailed
                 .arg(path)
                 .arg("Отсутствуют данные");
    return false;
  }
  auto image = QImage::fromData(data);
  return this->setImage(image);
}

bool ViewImagePreview::setImage( const QImage& image )
{
  QList<QImage> images;
  images << image;
  return this->setImages(images);
}

QRectF ViewImagePreview::imageRect()
{
  return this->imageRect_;
}

QSize ViewImagePreview::imagesTotalSize()
{
  int imagesWidth = 0,
      imagesHeight = 0;
  for ( auto image : backgroundImages_ ){
    imagesWidth = qMax(imagesWidth, image.width() );
    imagesHeight += image.height() + imageSpacing;
  }
  return QSize(imagesWidth, imagesHeight);
}

void ViewImagePreview::resizeSceneToContent()
{
  auto imagesSize = this->imagesTotalSize();
  int width = qMax( imagesSize.width(), this->width()),
      height = qMax( imagesSize.height(), this->width() );
  int viewWidth =  k * width,
      viewHeight = k * height;
  QRectF sceneRect(0, 0, viewWidth, viewHeight );
  QRectF oldSceneRect = this->sceneRect();
  if ( sceneRect.width() > oldSceneRect.width() || sceneRect.height() > oldSceneRect.height() ) {
    this->setSceneRect(sceneRect);
    QTimer::singleShot(0, this, &ViewImagePreview::scrollToCenter);

  }
}

bool ViewImagePreview::setImages( const QList<QImage>& images )
{
  if ( true == this->backgroundImages_.isEmpty() ){
    auto visibleRect = this->getVisibleRect();
    QPointF centerVisiblePoint(visibleRect.x() + 0.5 * visibleRect.width(),
                               visibleRect.y() + 0.5 * visibleRect.height() );
    auto sceneRect = this->sceneRect();
    QPointF sceneCenterPoint ( sceneRect.x() + 0.5 * sceneRect.width(),
                               sceneRect.y() + 0.5 * sceneRect.height() );
    this->currentOffsetX = centerVisiblePoint.x() - sceneCenterPoint.x();
    this->currentOffsetY = centerVisiblePoint.y() - sceneCenterPoint.y();

  }
  this->backgroundImages_ = images;
  this->resizeSceneToContent();
  this->forceRepaint();
  return true;
}

void ViewImagePreview::scrollToCenter()
{
  this->horizontalScrollBar()->setValue( this->horizontalScrollBar()->maximum() /2  );
  this->verticalScrollBar()->setValue(  this->verticalScrollBar()->maximum() /2  );
}

void ViewImagePreview::wheelEvent(QWheelEvent *event)
{    
  QGraphicsView::wheelEvent(event);
  if ( true == event->isAccepted() ){
    return;
  }
  if ( nullptr != event && event->type() == QEvent::Wheel  ) {
    if ( static_cast<QWheelEvent*>(event)->delta() > 0 ) {
      this->setScale( this->getScale() + 1 );
    }
    else {
      this->setScale( this->getScale() - 1 );
    }
    emit signalDocumentResized();
  }    
}

void ViewImagePreview::setScale( double newScale )
{
  auto screct = this->sceneRect();
  auto oldTransform = createTransform(screct.width(), screct.height(), this->getScale(), currentOffsetX, currentOffsetY, 0);
  auto newTransform = createTransform(screct.width(), screct.height(), newScale, currentOffsetX, currentOffsetY, 0);
  QPointF oldPoint = oldTransform.inverted().map(this->mousePos_);
  QPointF newPoint = newTransform.inverted().map(this->mousePos_);
  this->currentOffsetX += newPoint.x() - oldPoint.x();
  this->currentOffsetY += newPoint.y() - oldPoint.y();
  this->forceRepaint();
  this->scale_ = newScale;
}

void ViewImagePreview::forceRepaint()
{
  this->viewport()->update();
}

void ViewImagePreview::mouseMoveEvent(QMouseEvent *event)
{
  QGraphicsView::mouseMoveEvent(event);
  this->mousePos_ = this->mapToScene( event->pos() );  
}

void ViewImagePreview::resizeEvent(QResizeEvent *event)
{
  QGraphicsView::resizeEvent(event);
  emit signalDocumentResized();
}

bool ViewImagePreview::hasImages()
{
  return ( false == this->backgroundImages_.isEmpty() );
}


void ViewImagePreview::drawBackground(QPainter *painter, const QRectF & rect)
{
  auto brush = this->backgroundBrush();
  if ( QBrush() != brush ) {
    painter->fillRect(rect, brush);
  }
  else {
    painter->fillRect(rect, Qt::gray);
  }


  if ( false == backgroundImages_.isEmpty() ) {    
    auto screct = this->sceneRect();
    painter->save();
    auto imageSize = this->imagesTotalSize();
    auto transform = createTransform(screct.width(), screct.height(), this->getScale(), currentOffsetX, currentOffsetY, angle_);
    painter->setTransform( transform, true );
    QPointF topleft ( - 0.5 * imageSize.width(), -0.5 * imageSize.height() );
    QPointF bottomRight ( 0.5 * imageSize.width(), 0.5 * imageSize.height() );
    int posX = -imageSize.width() / 2,
        posY = -imageSize.height() / 2;
    for ( auto image : backgroundImages_ ) {
      painter->drawImage( posX, posY, image);
      posY += image.height() + imageSpacing;
    }

    this->imageRect_ =QRectF( transform.map(topleft), transform.map(bottomRight) );
    painter->restore();
  }
}

void ViewImagePreview::scaleInc()
{
  this->setScale(this->getScale() + 1);
}

void ViewImagePreview::scaleDec()
{
  this->setScale(this->getScale() - 1);
}

void ViewImagePreview::scaleReset()
{
  this->setScale(0);
  this->currentOffsetX = 0;
  this->currentOffsetY = 0;
  emit signalDocumentResized();
  this->forceRepaint();
}

double ViewImagePreview::getScale()
{
  return this->scale_;
}

void ViewImagePreview::scaleOptimal()
{  
  auto rect = this->viewport()->geometry();
  if ( false == this->backgroundImages_.isEmpty() ) {
    auto screct = sceneRect();
    auto transform = this->createTransform(screct.width(), screct.height(), 0, currentOffsetX, currentOffsetY, angle_ );
    auto size = transform.map( QPolygon(this->backgroundImages_.first().rect()) ).boundingRect();    

    double scale = qMin( static_cast<double>( rect.width() ) / size.width(),
                   static_cast<double>( rect.height() ) / size.height());    
    this->setScale( log(scale) / log(1.1) );
    this->currentOffsetX = 0;
    this->currentOffsetY = 0;    
    this->scrollToCenter();
    this->forceRepaint();
    emit signalDocumentResized();
  }
}

QPointF ViewImagePreview::mapToImage( const QPointF& scenePoint )
{
  auto screct = this->sceneRect();
  auto imageSize = this->imagesTotalSize();
  auto transform = createTransform( screct.width(), screct.height(), this->getScale(), currentOffsetX, currentOffsetY, angle_ )
      .inverted();
  auto targetPoint = transform.map(scenePoint);
  return  QPointF( targetPoint.x() + 0.5 * imageSize.width(),
                   targetPoint.y() + 0.5 * imageSize.height() );
}


QPointF ViewImagePreview::mapFromImage( const QPointF& imagePoint )
{
  auto imageSize = this->imagesTotalSize();
  QPointF sourcePoint( imagePoint.x() - 0.5 * imageSize.width(),
                       imagePoint.y() - 0.5  * imageSize.height() );
  auto screct = this->sceneRect();
  auto transform = createTransform( screct.width(), screct.height(), this->getScale(), currentOffsetX, currentOffsetY, angle_ );
  return transform.map(sourcePoint);
}

void ViewImagePreview::rotate(int rangle)
{
  this->angle_ = this->angle_ + rangle;
  if ( this->angle_ >= 360 ) {
    this->angle_ -= 360;
  }
  if ( this->angle_ < 0 ) {
    this->angle_ += 360;
  }
  emit signalDocumentResized();
  this->forceRepaint();
}

void ViewImagePreview::clear()
{
  this->backgroundImages_.clear();
  this->forceRepaint();
}

QRectF ViewImagePreview::getVisibleRect()
{
   return this->mapToScene(this->viewport()->geometry()).boundingRect();
}

}
