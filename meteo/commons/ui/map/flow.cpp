#include "flow.h"

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
  Object* createFlow( Layer* l )
  {
    return new Flow(l);
  }
  static const bool res = singleton::PtkppFormat::instance()->registerObjectHandler( Flow::Type, createFlow );
  Object* createFlow2( Object* o )
  {
    return new Flow(o);
  }
  static const bool res2 = singleton::PtkppFormat::instance()->registerChildHandler( Flow::Type, createFlow2 );
}

Flow::Flow( Layer* l )
  : Object(l),
  pix_(nullptr),
  screenpos_(nullptr),
  dn_(9999),
  cn_(9999),
  flow_(this)
{
  flow_.setPos( QPoint(0,0) );
}


Flow::Flow( Object* p )
  : Object(p),
  pix_(nullptr),
  screenpos_(nullptr),
  dn_(9999),
  cn_(9999),
  flow_(this)
{
  flow_.setPos( QPoint(0,0) );
}

Flow::Flow( Projection* proj )
  : Object(proj),
  pix_(nullptr),
  screenpos_(nullptr),
  dn_(9999),
  cn_(9999),
  flow_(this)
{
  flow_.setPos( QPoint(0,0) );
}

Flow::Flow( const meteo::Property& prop )
  : Object(prop),
  pix_(nullptr),
  screenpos_(nullptr),
  dn_(9999),
  cn_(9999),
  flow_(this)
{
  flow_.setPos( QPoint(0,0) );
}

Flow::~Flow()
{
  delete pix_; pix_ = nullptr;
  delete screenpos_; screenpos_ = nullptr;
}

Object* Flow::copy( Layer* l ) const
{
  if ( nullptr == l ) {
    error_log << QObject::tr("Нулевой указатель на слой");
    return nullptr;
  }
  Flow* pn = new Flow(l);
  pn->setProperty(property_);
  pn->setDncn( dn_, cn_, dnqual_, cnqual_ );
  pn->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(pn);
  }
  return pn;
}

Object* Flow::copy( Object* o ) const
{
  if ( nullptr == o ) {
    error_log << QObject::tr("Нулевой указатель на объект");
    return nullptr;
  }
  Flow* pn = new Flow(o);
  pn->setProperty(property_);
  pn->setDncn( dn_, cn_, dnqual_, cnqual_ );
  pn->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(pn);
  }
  return pn;
}

Object* Flow::copy( Projection* proj ) const
{
  if ( nullptr == proj ) {
    error_log << QObject::tr("Нулевой указатель на проекцию");
    return nullptr;
  }
  Flow* pn = new Flow(proj);
  pn->setProperty(property_);
  pn->setDncn( dn_, cn_, dnqual_, cnqual_ );
  pn->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(pn);
  }
  return pn;
}
bool Flow::dnok() const
{
  if ( control::DOUBTFUL == dnqual_
    || control::MISTAKEN == dnqual_
    || control::NO_OBSERVE == dnqual_
    ) {
    return false;
  }
  return true;
}

bool Flow::cnok() const
{
  if ( control::DOUBTFUL == cnqual_
    || control::MISTAKEN == cnqual_
    || control::NO_OBSERVE == cnqual_
    ) {
    return false;
  }
  return true;
}

void Flow::setDncn( int d, int f, control::QualityControl dqual, control::QualityControl fqual, int rad )
{
  delete pix_; pix_ = nullptr;
  dn_ = d;
  cn_ = f;
  dnqual_ = dqual;
  cnqual_ = fqual;

  flow_ = FlowPath( this, dn_, cn_, dqual, fqual, rad );
  cached_rect_ = QRect();
//  preparePixmap();
}

void Flow::setScreenPos( const QPoint& pos )
{
  if ( nullptr == screenpos_ ) {
    screenpos_ = new QPoint(pos);
  }
  else {
    *screenpos_ = pos;
  }
}

void Flow::removeScreenPos()
{
  delete screenpos_;
  screenpos_ = nullptr;
}

QList<QPoint> Flow::screenPoints( const QTransform& transform ) const
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

QRect Flow::boundingRect( const QPoint& pnt, float* angle, QPoint* oldtopleft ) const
{
  QRect r = boundingRect();
  r.moveTo( r.topLeft() + pnt );
  if ( nullptr != oldtopleft ) {
    *oldtopleft = r.topLeft();
  }
  float locangle = 0.0;
  if ( false == dnok() ) {
    if ( nullptr != angle ) {
      *angle = locangle;
    }
    return r;
  }
  if ( nullptr != document() ) {
    locangle = (-document()->deviationFromNorth(pnt)*RAD2DEG);
  }
  if ( false == MnMath::isZero(locangle) ) {
    QTransform tr;
    tr.translate(pnt.x(), pnt.y());
    tr.rotate(locangle);
    tr.translate(-pnt.x(), -pnt.y());
    r = tr.mapRect(r);
  }
  if ( nullptr != angle ) {
    *angle = locangle;
  }
  return r;
}

QList<QRect> Flow::boundingRect( const QTransform& transform ) const
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

QRect Flow::boundingRect() const
{
  if ( false == cached_rect_.isNull() ) {
    return cached_rect_;
  }
  QRect r = flow_.boundingRect();
  if ( nullptr != parent_ && kPuanson == parent_->type() ) {
    Puanson* p = mapobject_cast<Puanson*>(parent_);
    if ( nullptr != p ) {
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
    
QList<GeoVector> Flow::skeletInRect( const QRect& rect, const QTransform& transform ) const
{
  QList<GeoVector> gvlist;
  Projection* proj = projection();
  if ( nullptr == proj ) {
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

bool Flow::render( QPainter* painter, const QRect& target, const QTransform& transform )
{
  wasdrawed_ = false;
  float scale  = 0.0;
  if ( nullptr != curdoc_ ) {
    scale = curdoc_->scale();
  }
  else if ( nullptr != document() ) {
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
    if ( nullptr == pix_ ) {
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



    painter->save();
    QBrush br = qbrush();
    QPen pn = qpen();
    QColor clr = Qt::black;
    if ( nullptr != parent_ && kPuanson == parent_->type() ) {
      Puanson* p = mapobject_cast<Puanson*>(parent_);
      if ( nullptr != p ) {
        const puanson::proto::Puanson& punch = p->punch();
        if ( true ==  punch.has_color() ) {
          clr = QColor::fromRgba( punch.color() );
        }
      }
    }
    pn.setColor(clr);
    br.setColor(clr);
    br.setStyle( Qt::SolidPattern );
    painter->setBrush(br);
    painter->setPen(pn);
    painter->setFont( qfont() );

    QTransform tr;
    tr.translate(pos.x(),pos.y());
    tr.rotate(angle);
    QRectF newr = tr.mapRect(flow_.dfrect());
    painter->drawText( newr, Qt::AlignHCenter | Qt::AlignVCenter, flow_.dfstring() );
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

int Flow::minimumScreenDistance( const QPoint& pos, QPoint* cross ) const
{
  int dist = 10000000;
  QTransform tr;
  if ( nullptr != document() ) {
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

int32_t Flow::dataSize() const
{
  if ( nullptr != parent_ && kPuanson == parent_->type() ) {
    return 0;
  }
  int32_t objsz = Object::dataSize();
  if ( nullptr != screenpos_ ) {
    objsz += sizeof(bool); //есть экранная координата
    objsz += 2*sizeof(int32_t); //размер x и y
  }
  else {
    objsz += sizeof(bool);
  }
  objsz += sizeof(dn_);
  objsz += sizeof(cn_);
  objsz += sizeof(dnqual_);
  objsz += sizeof(cnqual_);
  objsz += sizeof(radius_);
  return objsz;
}

int32_t Flow::serializeToArray( char* arr ) const
{
  if ( nullptr != parent_ && kPuanson == parent_->type() ) {
    return 0;
  }
  int32_t pos = Object::serializeToArray(arr);
  bool hasscreen = false;
  if ( nullptr != screenpos_ ) {
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
  ::memcpy( arr + pos, &dn_, sizeof(dn_) );
  pos += sizeof(dn_);
  ::memcpy( arr + pos, &cn_, sizeof(cn_) );
  pos += sizeof(cn_);
  ::memcpy( arr + pos, &dnqual_, sizeof(dnqual_) );
  pos += sizeof(dnqual_);
  ::memcpy( arr + pos, &cnqual_, sizeof(cnqual_) );
  pos += sizeof(cnqual_);
  ::memcpy( arr + pos, &radius_, sizeof(radius_) );
  pos += sizeof(radius_);
  return pos;
}

int32_t Flow::parseFromArray( const char* arr )
{
  if ( nullptr != parent_ && kPuanson == parent_->type() ) {
    return 0;
  }
  int32_t pos = Object::setData( arr );
  if ( -1 == pos ) {
    return -1;
  }
  delete pix_; pix_ = nullptr;
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
  float c;
  global::fromByteArray( arr + pos, &c );
  pos += sizeof(c);
  control::QualityControl dq;
  global::fromByteArray( arr + pos, &dq );
  pos += sizeof(dq);
  control::QualityControl cq;
  global::fromByteArray( arr + pos, &cq );
  pos += sizeof(cq);
  float rad;
  global::fromByteArray( arr + pos, &rad );
  pos += sizeof(rad);
  setDncn( d, c, dq, cq, rad );
  return pos;
}

QSize Flow::size() const
{
  return boundingRect().size();
}

bool Flow::preparePixmap()
{
  delete pix_; pix_ = nullptr;
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
  flow_.draw(&pntr);
//Аутлайны отключены из-за низкой скорости
//  QPainterPathStroker stroker;
//  stroker.setDashPattern(Qt::SolidLine);
//  stroker.setWidth(3);
//  QPainterPath strpath = stroker.createStroke(path);
//  QPen outpen(Qt::white);
//  pntr.save();
//  pntr.setPen(outpen);
//  QBrush br(outpen.color());
//  pntr.setBrush(br);
//  pntr.drawPath(strpath);
//  pntr.restore();
//  pntr.setBrush( QBrush(Qt::black) );
//  pntr.drawPath(path);
//  pntr.drawRect( pix_.rect() );
  return true;
}

}
}
