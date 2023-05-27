#include "label.h"

#include <qpainter.h>

#include <cross-commons/debug/tlog.h>

#include "document.h"

namespace meteo {
namespace map {
Label::Label( Layer* l )
  : Object(l),
  pix_(nullptr),
  screenpos_(nullptr)
{
}

Label::Label( Object* p )
  : Object(p),
  pix_(nullptr),
  screenpos_(nullptr)
{
}

Label::Label( Projection* proj )
  : Object(proj),
  pix_(nullptr),
  screenpos_(nullptr)
{
}

Label::Label( const meteo::Property& prop )
  : Object(prop),
  pix_(nullptr),
  screenpos_(nullptr)
{
}

Label::Label()
  : Object(),
  pix_(nullptr),
  screenpos_(nullptr)
{
}

Label::~Label()
{
  delete screenpos_; screenpos_ = nullptr;
  delete pix_; pix_ = nullptr;
}

bool Label::render( QPainter* painter, const QRect& target, const QTransform& transform )
{
  if ( true == freezed_ ) { //флаг freezed обеспечивает возможность двойного обхода всех меток во время отрисовки
    freezed_ = false;
    return false;
  }
  else if ( nullptr != document() ) {
    freezed_ = true;
  }
  wasdrawed_ = false;
  if ( true == isEmpty() ) {
    delete pix_; pix_ = 0;
    return false;
  }
  float scale = 0.0;
  if ( nullptr != curdoc_ ) {
    scale = curdoc_->scale();
  }
  else if ( nullptr != document() ) {
    scale = document()->scale();
  }
  if ( false == drawAlways() ) {
    if ( false == visible(scale) ) {
      return false;
    }
  }
  else {
    if ( false == property_.visible() ) {
      return false;
    }
  }
  cached_screen_points_.clear();
  painter->setRenderHint( QPainter::SmoothPixmapTransform, true );
  painter->setPen( qpen() );
  painter->setBrush( qbrush() );
  painter->setFont( qfont() );
  QList< QPair< QPoint,float > > list = screenPoints(transform);
  Position orient = pos();
  drawedrects_.clear();
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    const QPoint& pnt = list[i].first;
    if ( false == target.contains(pnt) && nullptr == screenpos_ ) {
      continue;
    }
    float angle = list[i].second*RAD2DEG;
    QPoint oldtopleft;
    QRect r = boundingRect( pnt, orient, angle, &oldtopleft );
    if ( false == target.intersects(r) && false == target.contains(r)  ) {
      continue;
    }
    if ( nullptr != layer() ) {
      layer()->addToDrawedLabels(this);
    }
    cached_screen_points_.append( QPolygon(r) );
    if ( nullptr != document() && true == document()->textFreezed() ) {
      if ( true == document()->addToFreeze( target, this, r, pnt, oldtopleft, angle ) ) {
        drawedrects_.append(r);
      }
      continue;
    }

    render( painter, r, pnt, oldtopleft, angle );
  }
  if ( true == wasdrawed_ ) {
    for ( auto o : childs() ) {
      o->render( painter, target, transform );
    }
  }
  return true;
}

bool Label::render( QPainter* painter, const QRect& r, const QPoint& pnt, const QPoint& oldtopleft, float angle )
{
  TextRamka rmk = ramka();
  if ( kNoRamka != rmk ) {
    painter->save();
    QPen pen = painter->pen();
    QBrush brush = painter->brush();
    if ( kBorder  != (kBorder & rmk ) ) {
      pen.setStyle( Qt::NoPen );
      painter->setPen(pen);
    }
    if ( kBrush != (kBrush & rmk ) ) {
      brush.setStyle( Qt::NoBrush );
      painter->setBrush(brush);
    }
    painter->drawRect(r);
    painter->restore();
  }
  if ( nullptr == pix_ || true == pix_->isNull() ) {
    preparePixmap();
    if ( nullptr == pix_ || true == pix_->isNull() ) {
      return false;
    }
  }
  if ( true == MnMath::isZero(angle) ) {
    painter->drawImage( r.topLeft(), *pix_ );
  }
  else {
    painter->save();
    painter->setRenderHint( QPainter::SmoothPixmapTransform, true );
    painter->translate( pnt.x(), pnt.y() );
    painter->rotate(angle);
    painter->translate( -pnt.x(), -pnt.y() );
    painter->drawImage( oldtopleft, *pix_ );
    painter->restore();
  }
  wasdrawed_ = true;
  return true;
}

void Label::setScreenPos( const QPoint& pos )
{
  if ( 0 == screenpos_ ) {
    screenpos_ = new QPoint(pos);
  }
  else {
    *screenpos_ = pos;
  }
}

void Label::removeScreenPos()
{
  delete screenpos_;
  screenpos_ = 0;
}

QRect Label::boundingRect( const QPoint& pnt, Position orient, float angle, QPoint* oldtopleft ) const
{
  QRect r = boundingRect();
  r.moveTo( r.topLeft() + pnt );
  r = rectByOrient( r, orient );
  if ( 0 != oldtopleft ) {
    *oldtopleft = r.topLeft();
  }
  if ( false == MnMath::isZero(angle) ) {
    QTransform tr;
    tr.translate(pnt.x(), pnt.y());
    tr.rotate(angle);
    tr.translate(-pnt.x(), -pnt.y());
    r = tr.mapRect(r);
  }
  return r;
}

}
}
