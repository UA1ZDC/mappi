#include "imagedisplaygraphicsitem.h"
#include <qpainter.h>
#include <qapplication.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <qtransform.h>

namespace meteo {

ImageDisplayGraphicsItem::ImageDisplayGraphicsItem( meteo::map::MapView *view )
  : ImageDisplayGraphicsItem(view->mapscene() )
{

}

ImageDisplayGraphicsItem::ImageDisplayGraphicsItem( meteo::map::MapScene *scene )
  : QGraphicsItem (nullptr),
    scene_(scene)
{
  scene_->addItem(this);
  scene_->document()->addItem(this);
}

ImageDisplayGraphicsItem::~ImageDisplayGraphicsItem()
{
  this->scene_->document()->removeItem(this);
}

QRectF ImageDisplayGraphicsItem::boundingRect() const
{
  QPolygonF source;
  QPolygonF target;
  for ( int i = 0; i < this->source_.size(); ++i ){
    source << this->source_[i];
    target << this->scene_->coord2screen(this->target_[i]);
  }

  auto trans = createTriangleTransform(source,target);  
  return trans.mapToPolygon( this->image_.rect()).boundingRect();
}

void ImageDisplayGraphicsItem::setImage( const QImage& image ){
  this->prepareGeometryChange();
  this->image_ = image;
  this->update();
}

void ImageDisplayGraphicsItem::setRect( const QPolygon& source, const meteo::GeoVector& target )
{  
  this->prepareGeometryChange();
  this->source_ = source;
  this->target_ = target;
  this->update();
}

void ImageDisplayGraphicsItem::paint( QPainter* p, const QStyleOptionGraphicsItem* , QWidget*  )
{
  p->save();
  if ( false == this->source_.isEmpty() &&
       this->source_.size() == this->target_.size() ){

    QPolygonF source;
    QPolygonF target;
    for ( int i = 0; i < this->source_.size(); ++i ){
      source << this->source_[i];
      target << this->scene_->coord2screen(this->target_[i]);
    }

    auto trans = createTriangleTransform(source,target);
    p->setTransform( trans, true);
    p->setPen(Qt::green);
    p->drawImage( this->image_.rect(), this->image_ );
  }
  p->restore();
}


QTransform ImageDisplayGraphicsItem::createTriangleTransform(const QPolygonF& source, const QPolygonF& target)
{
  if ( 3 != source.size() && 3 != target.size() ){
    return QTransform();
  }
  auto tr = QTransform(
      1, source[0].x(), source[0].y(),
      1, source[1].x(), source[1].y(),
      1, source[2].x(), source[2].y()
      );
  tr = tr.inverted();

  QVector<qreal> a = QVector<qreal>()
    << tr.m11()*target[0].x() + tr.m12()*target[1].x() + tr.m13()*target[2].x()
    << tr.m21()*target[0].x() + tr.m22()*target[1].x() + tr.m23()*target[2].x()
    << tr.m31()*target[0].x() + tr.m32()*target[1].x() + tr.m33()*target[2].x();


  QVector<qreal> b = QVector<qreal>()
    << tr.m11()*target[0].y() + tr.m12()*target[1].y() + tr.m13()*target[2].y()
    << tr.m21()*target[0].y() + tr.m22()*target[1].y() + tr.m23()*target[2].y()
    << tr.m31()*target[0].y() + tr.m32()*target[1].y() + tr.m33()*target[2].y();

  return QTransform( a[1], b[1], a[2], b[2], a[0], b[0] );
}

}
