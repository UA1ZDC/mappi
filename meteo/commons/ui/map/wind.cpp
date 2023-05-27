#include "wind.h"

#include <qpainter.h>
#include <qdebug.h>
#include <qlist.h>

#include <commons/geom/geom.h>
#include <meteo/commons/global/global.h>

#include "document.h"
#include "puanson.h"
#include "geotext.h"

namespace meteo {
namespace map {

namespace {
  Object* createWind( Layer* l )
  {
    return new Wind(l);
  }
  static const bool res = singleton::PtkppFormat::instance()->registerObjectHandler( Wind::Type, createWind );
  Object* createWind2( Object* o )
  {
    return new Wind(o);
  }
  static const bool res2 = singleton::PtkppFormat::instance()->registerChildHandler( Wind::Type, createWind2 );
}

Wind::Wind( Layer* l )
  : Object(l),
  pix_(0),
  screenpos_(0),
  dd_(9999),
  ff_(9999),
  draw_bad_dd_(true),
  wind_(this),
  lenft_(40.0)
{
  wind_.setPos( QPoint(0,0) );
  wind_.setLenft(lenft_);
}

Wind::Wind( Object* p )
  : Object(p),
  pix_(0),
  screenpos_(0),
  dd_(9999),
  ff_(9999),
  draw_bad_dd_(true),
  wind_(this),
  lenft_(40.0)
{
  wind_.setPos( QPoint(0,0) );
  wind_.setLenft(lenft_);
}

Wind::Wind( Projection* proj )
  : Object(proj),
  pix_(0),
  screenpos_(0),
  dd_(9999),
  ff_(9999),
  draw_bad_dd_(true),
  wind_(this),
  lenft_(40.0)
{
  wind_.setPos( QPoint(0,0) );
  wind_.setLenft(lenft_);
}

Wind::Wind( const meteo::Property& prop )
  : Object(prop),
  pix_(0),
  screenpos_(0),
  dd_(9999),
  ff_(9999),
  draw_bad_dd_(true),
  wind_(this),
  lenft_(40.0)
{
  wind_.setPos( QPoint(0,0) );
  wind_.setLenft(lenft_);
}

Wind::~Wind()
{
  delete pix_; pix_ = 0;
  delete screenpos_; screenpos_ = 0;
}

Object* Wind::copy( Layer* l ) const
{
  if ( 0 == l ) {
    error_log << QObject::tr("Нулевой указатель на слой");
    return 0;
  }
  Wind* pn = new Wind(l);
  pn->setProperty(property_);
  pn->setDdff( dd_, ff_, ddqual_, ffqual_ );
  pn->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(pn);
  }
  return pn;
}

Object* Wind::copy( Object* o ) const
{
  if ( 0 == o ) {
    error_log << QObject::tr("Нулевой указатель на объект");
    return 0;
  }
  Wind* pn = new Wind(o);
  pn->setProperty(property_);
  pn->setDdff( dd_, ff_, ddqual_, ffqual_ );
  pn->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(pn);
  }
  return pn;
}

Object* Wind::copy( Projection* proj ) const
{
  if ( 0 == proj ) {
    error_log << QObject::tr("Нулевой указатель на проекцию");
    return 0;
  }
  Wind* pn = new Wind(proj);
  pn->setProperty(property_);
  pn->setDdff( dd_, ff_, ddqual_, ffqual_ );
  pn->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(pn);
  }
  return pn;
}
bool Wind::ddok() const
{
  if ( control::DOUBTFUL == ddqual_
    || control::MISTAKEN == ddqual_
    || control::NO_OBSERVE == ddqual_
    ) {
    return false;
  }
  return true;
}

bool Wind::ffok() const
{
  if ( control::DOUBTFUL == ffqual_
    || control::MISTAKEN == ffqual_
    || control::NO_OBSERVE == ffqual_
    ) {
    return false;
  }
  return true;
}

void Wind::setDdff( int d, int f, control::QualityControl dqual, control::QualityControl fqual, int rad )
{
  delete pix_; pix_ = 0;
  dd_ = d;
  ff_ = f;
  ddqual_ = dqual;
  ffqual_ = fqual;

  wind_ = WindPath( this, dd_, ff_, dqual, fqual, rad );
  wind_.setLenft(lenft_);
  cached_rect_ = QRect();
//  preparePixmap();
}

void Wind::setScreenPos( const QPoint& pos )
{
  if ( 0 == screenpos_ ) {
    screenpos_ = new QPoint(pos);
  }
  else {
    *screenpos_ = pos;
  }
}

void Wind::removeScreenPos()
{
  delete screenpos_;
  screenpos_ = 0;
}

QList<QPoint> Wind::screenPoints( const QTransform& transform ) const
{
  QList<QPoint> points;
  if ( nullptr != screenpos_ ) {
    points.append(*screenpos_);
    return points;
  }
  if ( nullptr == parent_ || false == parent_->isLabel() ) {
    for ( int i = 0, isz = cartesian_points_.size(); i < isz; ++i ) {
      for ( int j = 0, jsz = cartesian_points_[i].size(); j < jsz; ++j ) {
        QPoint pnt  = transform.map(cartesian_points_[i][j]);
        points.append(pnt);
      }
    }
    return points;
  }
  Label* lbl = static_cast<Label*>(parent_);
  auto l = lbl->screenPoints(transform);
  for ( int i = 0, sz = l.size(); i < sz; ++i ) {
    points.append(l[i].first);
  }
  return points;
}

QRect Wind::boundingRect( const QPoint& pnt, float* angle, QPoint* oldtopleft ) const
{
  QRect r = boundingRect();
  r.moveTo( r.topLeft() + pnt );
  if ( 0 != oldtopleft ) {
    *oldtopleft = r.topLeft();
  }
  float locangle = 0.0;
  if ( false == ddok() ) {
    if ( 0 != angle ) {
      *angle = locangle;
    }
    return r;
  }
  Puanson* puanson = mapobject_cast<Puanson*>( parent() );
  if ( 0 != puanson && true == puanson->bindedToSkelet() ) {
    bool res = false;
    locangle = puanson->bindedAngle( pnt, &res )*RAD2DEG;
    if ( false == res ) {
      return QRect();
    }
  }
  else if ( 0 != document() ) {
    locangle = (-document()->deviationFromNorth(pnt)*RAD2DEG);
  }
  if ( false == MnMath::isZero(locangle) ) {
    QTransform tr;
    tr.translate(pnt.x(), pnt.y());
    tr.rotate(locangle);
    tr.translate(-pnt.x(), -pnt.y());
    r = tr.mapRect(r);
  }
  if ( 0 != angle ) {
    *angle = locangle;
  }
  return r;
}

QList<QRect> Wind::boundingRect( const QTransform& transform ) const
{
  QList<QRect> list;
  QList<QPoint> pntlist = screenPoints(transform);
  for ( int i = 0, sz = pntlist.size(); i < sz; ++i ) {
    const QPoint& pnt  = pntlist[i];
    QRect r = boundingRect(pnt);
    list.append(r);
  }
  return list;
}

QRect Wind::boundingRect() const
{
  if ( false == cached_rect_.isNull() ) {
    return cached_rect_;
  }
  QRect r = wind_.boundingRect();
  if ( 0 != parent_ && kPuanson == parent_->type() ) {
    Puanson* p = mapobject_cast<Puanson*>(parent_);
    if ( 0 != p ) {
      const puanson::proto::Puanson& punch = p->punch();
      if ( false == MnMath::isEqual( 1.0, punch.scale() ) ) {
        QSize sz = r.size()*punch.scale();
        QRect nr( r.topLeft()*punch.scale(), sz );
        r = nr;
      }
    }
  }
  cached_rect_ = r;
  return r;
}
    
QList<GeoVector> Wind::skeletInRect( const QRect& rect, const QTransform& transform ) const
{
  QList<GeoVector> gvlist;
  Projection* proj = projection();
  if ( 0 == proj ) {
    return gvlist;
  }
  QList<QPoint> list = screenPoints(transform);
  GeoVector gv;
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    const QPoint& scr = list[i];
    if ( false == rect.contains(scr) ) {
      continue;
    }
    GeoPoint gp;
    QPoint cartpoint = transform.inverted().map(scr);
    proj->X2F_one( cartpoint, &gp );
    gv.append(gp);
  }
  gvlist.append(gv);
  return gvlist;
}

bool Wind::render( QPainter* painter, const QRect& target, const QTransform& transform )
{
  wasdrawed_ = false;
  float scale  = 0.0;
  if ( 0 != curdoc_ ) {
    scale = curdoc_->scale();
  }
  else if ( 0 != document() ) {
    scale = document()->scale();
  }
  if ( false == visible(scale) ) {
//    delete pix_; pix_ = 0;
    return false;
  }
  QList<QPoint> list  = screenPoints( transform );
  painter->setRenderHint( QPainter::TextAntialiasing, false );
//  bool wasrender = false;
  cached_screen_points_.clear();
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    float angle = 0;
    QPoint oldtopleft;
    QPoint pos = list[i];
    QRect rect = boundingRect( pos, &angle, &oldtopleft );
    if ( false == target.intersects(rect) && false == target.contains(rect)  ) {
      continue;
    }
    QPainterPath path = painter->clipPath();
    if ( false == path.isEmpty() ) {
      if ( false == path.contains(rect) && false == path.intersects(rect) ) {
        continue;
      }
    }
    if ( 0 == pix_ ) {
      if ( false == preparePixmap() ) {
        return false;
      }
    }
//    wasrender = true;
    painter->save();
    if ( false == MnMath::isZero(angle)  ) {
      painter->translate( pos.x(), pos.y() );
      painter->rotate(angle);
      painter->translate( -pos.x(), -pos.y() );
    }
    painter->drawImage( oldtopleft, *pix_ );
    wasdrawed_ = true;
    painter->restore();
    cached_screen_points_.append( QPolygon(rect) );
//    painter->drawRect(rect);
  }
//  if ( false == wasrender ) {
//    delete pix_; pix_ = 0;
//  }
  foreach(Object* o, objects_){
    painter->save();
    o->render( painter, target, transform );
    painter->restore();
  }
 return true;
  
}

int Wind::minimumScreenDistance( const QPoint& pos, QPoint* cross ) const
{
  int dist = 10000000;
  QTransform tr;
  if ( 0 != document() ) {
    tr = document()->transform();
  }
  QList<QPoint> list  = screenPoints(tr);
  if ( 0 == list.size() ) {
    return dist;
  }
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    const QPoint& p = list[i];
    dist = meteo::geom::distance( pos, QPolygon() << p, cross );
  }
  return dist;
}
void Wind::setLenft( float l )
{
  lenft_ = l;
  wind_.setLenft(lenft_);
  cached_rect_ = QRect();
}

int32_t Wind::dataSize() const
{
  if ( 0 != parent_ && kPuanson == parent_->type() ) {
    return 0;
  }
  int32_t objsz = Object::dataSize();
  if ( 0 != screenpos_ ) {
    objsz += sizeof(bool); //есть экранная координата
    objsz += 2*sizeof(int32_t); //размер x и y
  }
  else {
    objsz += sizeof(bool);
  }
  objsz += sizeof(dd_);
  objsz += sizeof(ff_);
  objsz += sizeof(ddqual_);
  objsz += sizeof(ffqual_);
  objsz += sizeof(radius_);
  objsz += sizeof(draw_bad_dd_);
  objsz += sizeof(lenft_);
  return objsz;
}

int32_t Wind::serializeToArray( char* arr ) const
{
  if ( 0 != parent_ && kPuanson == parent_->type() ) {
    return 0;
  }
  int32_t pos = Object::serializeToArray(arr);
  bool hasscreen = false;
  if ( 0 != screenpos_ ) {
    hasscreen = true;
    ::memcpy( arr + pos, &hasscreen, sizeof(hasscreen) );
    pos += sizeof(hasscreen);
    int32_t xy = screenpos_->x();
    ::memcpy( arr + pos, &xy, sizeof(xy) );
    pos += sizeof(xy);
    xy = screenpos_->y();
    ::memcpy( arr + pos, &xy, sizeof(xy) );
    pos += sizeof(xy);
  }
  else {
    ::memcpy( arr + pos, &hasscreen, sizeof(hasscreen) );
    pos += sizeof(hasscreen);
  }
  ::memcpy( arr + pos, &dd_, sizeof(dd_) );
  pos += sizeof(dd_);
  ::memcpy( arr + pos, &ff_, sizeof(ff_) );
  pos += sizeof(ff_);
  ::memcpy( arr + pos, &ddqual_, sizeof(ddqual_) );
  pos += sizeof(ddqual_);
  ::memcpy( arr + pos, &ffqual_, sizeof(ffqual_) );
  pos += sizeof(ffqual_);
  ::memcpy( arr + pos, &radius_, sizeof(radius_) );
  pos += sizeof(radius_);
  ::memcpy( arr + pos, &draw_bad_dd_, sizeof(draw_bad_dd_) );
  pos += sizeof(draw_bad_dd_);
  ::memcpy( arr + pos, &lenft_, sizeof(lenft_) );
  pos += sizeof(lenft_);
  return pos;
}

int32_t Wind::parseFromArray( const char* arr )
{
  if ( 0 != parent_ && kPuanson == parent_->type() ) {
    return 0;
  }
  int32_t pos = Object::setData( arr );
  if ( -1 == pos ) {
    return -1;
  }
  delete pix_; pix_ = 0;
  cached_rect_ = QRect();
  bool hasscreen = false;
  global::fromByteArray( arr + pos, &hasscreen );
  pos += sizeof(hasscreen);
  if ( true == hasscreen ) {
    int32_t x; int32_t y;
    global::fromByteArray( arr + pos, &x );
    pos += sizeof(x);
    global::fromByteArray( arr + pos, &y );
    pos += sizeof(y);
    setScreenPos( QPoint(x,y) );
  }
  float d;
  global::fromByteArray( arr + pos, &d );
  pos += sizeof(d);
  float f;
  global::fromByteArray( arr + pos, &f );
  pos += sizeof(f);
  control::QualityControl dq;
  global::fromByteArray( arr + pos, &dq );
  pos += sizeof(dq);
  control::QualityControl fq;
  global::fromByteArray( arr + pos, &fq );
  pos += sizeof(fq);
  float rad;
  global::fromByteArray( arr + pos, &rad );
  pos += sizeof(rad);
  global::fromByteArray( arr + pos, &draw_bad_dd_ );
  pos += sizeof(draw_bad_dd_);
  global::fromByteArray( arr + pos, &lenft_ );
  pos += sizeof(lenft_);
  setDdff( d, f, dq, fq, rad );
  return pos;
}

QSize Wind::size() const
{
  return boundingRect().size();
}

bool Wind::preparePixmap()
{
  delete pix_; pix_ = 0;
//  cached_rect_ = QRect();
  QRect r = boundingRect();
//  cached_rect_ = r;
  QPoint rtopleft = r.topLeft();
  QSize sz = r.size();
  if ( true == sz.isEmpty() ) {
    return false;
  }
  pix_ = new QImage( sz+QSize(4,4), QImage::Format_ARGB32 );
  pix_->fill( QColor(0,0,0,0) );
//  pix_.fill( Qt::red );
  QPainter pntr(pix_);
//  pntr.setBrush( qbrush() );
//  pntr.setPen( qpen() );
  QBrush br = qbrush();
  QPen pn = qpen();
  QColor clr = Qt::black;
  if ( 0 != parent_ && kPuanson == parent_->type() ) {
    Puanson* p = mapobject_cast<Puanson*>(parent_);
    if ( 0 != p ) {
      const puanson::proto::Puanson& punch = p->punch();
      if ( true ==  punch.has_color() ) {
        clr = QColor::fromRgba( punch.color() );
      }
    }
  }
  pn.setColor(clr);
  br.setColor(clr);
  br.setStyle( Qt::SolidPattern );
  pntr.setBrush(br);
  pntr.setPen(pn);
  pntr.setRenderHint( QPainter::Antialiasing, true );
  pntr.setRenderHint( QPainter::HighQualityAntialiasing, true );
  pntr.translate( -rtopleft );
  if ( 0 != parent_ && kPuanson == parent_->type() ) {
    Puanson* p = mapobject_cast<Puanson*>(parent_);
    if ( 0 != p ) {
      const puanson::proto::Puanson& punch = p->punch();
      if ( false == MnMath::isEqual( 1.0, punch.scale() ) ) {
        pntr.scale( punch.scale(), punch.scale() );
      }
    }
  }
//  QPainterPath path;
  wind_.draw(&pntr);
  return true;
}

}
}
