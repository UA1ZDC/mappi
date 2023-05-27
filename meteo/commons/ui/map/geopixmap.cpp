#include "geopixmap.h"

#include <qpainter.h>
#include <qdebug.h>
#include <qmath.h>
#include <qbuffer.h>
#include <qbitmap.h>

#include <commons/geom/geom.h>
#include <meteo/commons/global/global.h>

#include "layer.h"
#include "map.h"
#include "document.h"

namespace meteo {
namespace map {

namespace {
  Object* createGeoPixmap( Layer* l )
  {
    return new GeoPixmap(l);
  }
  static const bool res = singleton::PtkppFormat::instance()->registerObjectHandler( GeoPixmap::Type, createGeoPixmap );
  Object* createGeoPixmap2( Object* o )
  {
    return new GeoPixmap(o);
  }
  static const bool res2 = singleton::PtkppFormat::instance()->registerChildHandler( GeoPixmap::Type, createGeoPixmap2 );
}

GeoPixmap::GeoPixmap( Layer* l )
  : Object(l),
  pix_(0),
  bindedtoskelet_(false)
{
  property_.mutable_scalexy()->set_x(1.0);
  property_.mutable_scalexy()->set_y(1.0);
  setPos(kCenter);
}

GeoPixmap::GeoPixmap( Object* p )
  : Object(p),
  pix_(0),
  bindedtoskelet_(false)
{
  setPos(kCenter);
}

GeoPixmap::GeoPixmap( Projection* p )
  : Object(p),
  pix_(0),
  bindedtoskelet_(false)
{
  setPos(kCenter);
}

GeoPixmap::GeoPixmap( const ::meteo::Property& p )
  : Object(p),
  pix_(0),
  bindedtoskelet_(false)
{
}

GeoPixmap::~GeoPixmap()
{
  delete pix_;
  pix_ = 0;
}

Object* GeoPixmap::copy( Layer* l ) const
{
  if ( 0 == l ) {
    error_log << QObject::tr("Нулевой указатель на слой");
    return 0;
  }
  GeoPixmap* gp = new GeoPixmap(l);
  gp->setProperty(property_);
  gp->setSkelet(skelet());
  if ( 0 != pix_ ) {
    gp->setImage(*pix_);
  }
  gp->coordbunch_ = coordbunch_;
  foreach(Object* o, objects_){
    o->copy(gp);
  }
  return gp;
}

Object* GeoPixmap::copy( Object* o ) const
{
  if ( 0 == o ) {
    error_log << QObject::tr("Нулевой указатель на объект");
    return 0;
  }
  GeoPixmap* gp = new GeoPixmap(o);
  gp->setProperty(property_);
  gp->setSkelet(skelet());
  if ( 0 != pix_ ) {
    gp->setImage(*pix_);
  }
  gp->coordbunch_ = coordbunch_;
  foreach(Object* o, objects_){
    o->copy(gp);
  }
  return gp;
}

Object* GeoPixmap::copy( Projection* proj ) const
{
  if ( 0 == proj ) {
    error_log << QObject::tr("Нулевой указатель на проекцию");
    return 0;
  }
  GeoPixmap* gp = new GeoPixmap(proj);
  gp->setProperty(property_);
  gp->setSkelet(skelet());
  if ( 0 != pix_ ) {
    gp->setImage(*pix_);
  }
  gp->coordbunch_ = coordbunch_;
  foreach(Object* o, objects_){
    o->copy(gp);
  }
  return gp;
}

QList<QRect> GeoPixmap::boundingRect( const QTransform& transform ) const
{
  QList<QRect> r;
//  if ( true == bindedToSkelet() ) {
//    QList<QRect> list;
//    if ( 0 != document() ) {
//      list.append( document()->documentRect() );
//    }
//    return list;
//  }
  if ( 0 != coordbunch_.first.size() ) {
    r = boundingRectWithBunch(transform);
  }
  else {
    r = boundingRectWithoutBunch(transform);
  }
  cached_screen_points_.clear();
  for ( int i = 0, sz = r.size(); i < sz; ++i ) {
    cached_screen_points_.append( QPolygon( r[i], true ) );
  }
  return r;
}

QList<QRect> GeoPixmap::boundingRectWithBunch( const QTransform& transform ) const
{
  QList<QRect> list;
  if ( 0 == pix_ ) {
    return list;
  }
  QRect r = pix_->rect();
  QTransform tr = bunchTransform(transform);
  r = tr.mapRect(r);
  list.append(r);

  return list;
}

QList<QRect> GeoPixmap::boundingRectWithoutBunch( const QTransform& transform ) const
{
  QList<QRect> list;
  if ( 0 == pix_ ) {
    return list;
  }
  QSize sz = pixmapRect().size();
  QTransform tr;
  tr.translate( property_.translatexy().x(), property_.translatexy().y() );
  for ( int i = 0, isz = cartesian_points_.size(); i < isz; ++i ) {
    for ( int j = 0, jsz = cartesian_points_[i].size(); j < jsz; ++j ) {
      QPoint pnt = cartesian_points_[i][j];
      pnt = translateByPosition( transform.map( pnt ) );
      QRect r( pnt, sz );
      r = tr.mapRect(r);
      list.append(r);
    }
  }
  return list;
}

QList<GeoVector> GeoPixmap::skeletInRect( const QRect& rect, const QTransform& transform ) const
{
  Q_UNUSED(rect);
  Q_UNUSED(transform);
  QList<GeoVector> list;
  return list;
}

bool GeoPixmap::render( QPainter* painter, const QRect& target, const QTransform& transform )
{
  wasdrawed_ = false;
  if ( 0 == pix_ ) {
    warning_log << QObject::tr("Картинка не установлена. Рисовать нечего.");
    return false;
  }
  if ( true == bindedtoskelet_ && 0 != parent() ) {
    renderOnParentSkelet( painter, target, transform );
  }
  if ( 0 == coordbunch_.first.size() ) {
    return renderWithoutBunch( painter, target, transform );
  }
  return renderWithBunch( painter, target, transform );
}

bool GeoPixmap::renderWithoutBunch( QPainter* painter, const QRect& target, const QTransform& transform )
{
  Q_UNUSED(target);
  if ( 0 == pix_ ) {
    warning_log << QObject::tr("Картинка не установлена. Рисовать нечего.");
    return false;
  }
  QRect pixrect = pix_->rect();
  QTransform tr;
  tr.translate( pixrect.center().x(), pixrect.center().y() );
  tr.rotate(property_.rotateangle());
  tr.translate( pixrect.center().x(), pixrect.center().y() );
  tr.scale( property_.scalexy().x(), property_.scalexy().y() );
  QImage trpix = pix_->transformed( tr, Qt::SmoothTransformation );

  tr.reset();
  tr.translate( property_.translatexy().x(), property_.translatexy().y() );
  painter->save();
  painter->setTransform(tr, true );

  for ( int i = 0, isz = cartesian_points_.size(); i < isz; ++i ) {
    for ( int j = 0, jsz = cartesian_points_[i].size(); j < jsz; ++j ) {
      QPoint pnt = cartesian_points_[i][j];
      pnt = translateByPosition( transform.map( pnt ) );
      painter->drawImage( pnt, trpix );
      wasdrawed_ = true;
    }
  }
  painter->restore();
  return true;
}

bool GeoPixmap::renderWithBunch( QPainter* painter, const QRect& target, const QTransform& transform )
{
  Q_UNUSED(target);
  if ( 0 == pix_ ) {
    warning_log << QObject::tr("Картинка не установлена. Рисовать нечего.");
    return false;
  }
  QList<QRect> list = boundingRect(transform);
  if ( 0 == list.size() ) {
    return false;
  }
  QTransform tr = bunchTransform(transform);
  painter->save();
  painter->setTransform(tr, true);
  painter->drawImage( QPoint(0,0), *pix_ );
  painter->restore();
  wasdrawed_ = true;
  return true;
}

bool GeoPixmap::renderOnParentSkelet( QPainter* painter, const QRect& target, const QTransform& transform )
{
  Q_UNUSED(painter);
  Q_UNUSED(target);
  Q_UNUSED(transform);
  if ( 0 == pix_ || true == pix_->isNull() ) {
    return false;
  }
  if ( false == bindedToSkelet() ) {
    return false;
  }
  if ( 0 == parent() ) {
    return false;
  }

  const QVector<QPolygon>& cached_points = parent()->cachedScreenPoints();
  for ( int i = 0, isz = cartesian_points_.size(); i < isz; ++i ) {
    const QPolygon& poly = cartesian_points_[i];
    for ( int j = 0, jsz = poly.size(); j < jsz; ++j ) {
      QPoint pnt = transform.map( poly[j] );
      int indx = -1;
      int numpoly = -1;
      int dist2 = 1000000000;
      for ( int k = 0, ksz = cached_points.size(); k < ksz; ++k ) {
        const QPolygon& screen = cached_points[k];
        int newdist2 = 0;
        int newindx = nearestPointIndex( pnt, screen, &newdist2 );
        if ( -1 != newindx && newdist2 < dist2 ) {
          indx = newindx;
          dist2 = newdist2;
          numpoly = k;
        }
      }
      if ( dist2 > 10000 ) {
        continue;
      }
      if ( -1 == indx || -1 == numpoly ) {
        continue;
      }
      QPointF pntf(cached_points[numpoly][indx]);
      paintBit( painter, pix_, cached_points[numpoly], indx, &pntf );
    }
  }
  return false;
}

int GeoPixmap::minimumScreenDistance( const QPoint& pos, QPoint* cross ) const
{
  int dist = 10000000;
  QTransform tr;
  if ( 0 != document() ) {
    tr = document()->transform();
  }
  QList<QRect> list  = boundingRect(tr);
  if ( 0 == list.size() ) {
    return dist;
  }
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    dist = meteo::geom::distance( pos, list[i], cross );
    if ( 0 > dist ) {
      dist = 0;
    }
  }
  return dist;
}

void GeoPixmap::bindScreenToCoord( const QPolygon& screen, const GeoVector& coord )
{
  coordbunch_ = qMakePair( screen, coord );
  if ( 0 != layer() && 0 != document() ) {
    if ( 0 != document()->eventHandler() ) {
      QList<QRect> list = boundingRect( document()->transform() );
      QRect bigrect;
      for ( int i = 0, sz = list.size(); i < sz; ++i ) {
        bigrect = bigrect.united(list[i]);
      }
      LayerEvent* ev = new LayerEvent( layer()->uuid(), LayerEvent::ObjectChanged, bigrect );
      document()->eventHandler()->postEvent(ev);
    }
  }
}

void GeoPixmap::unbindFromCoord()
{
  coordbunch_.first.clear();
  coordbunch_.second.clear();
}

void GeoPixmap::setImage( const QImage& pix )
{
  if ( 0 != pix_ ) {
    delete pix_;
  }
  pix_ = new QImage;
  *pix_ = pix.copy();
  if ( 0 != layer() && 0 != document() && 0 != document()->eventHandler() ) {
    QList<QRect> list = boundingRect( document()->transform() );
    QRect bigrect;
    for ( int i = 0, sz = list.size(); i < sz; ++i ) {
      bigrect = bigrect.united(list[i]);
    }
    LayerEvent* ev = new LayerEvent( layer()->uuid(), LayerEvent::ObjectChanged, bigrect );
    document()->eventHandler()->postEvent(ev);
  }
}

void GeoPixmap::setColor( const QColor& clr )
{
  if ( nullptr == pix_ ) {
    return;
  }
  uchar* pixels = pix_->bits();
  QRgb rgba = clr.rgba();
  for ( int h = 0; h < pix_->height(); ++h ) {
    for ( int w = 0; w < pix_->width(); ++w ) {
      QRgb pic_rgba;
      ::memcpy( &pic_rgba, pixels, 4 );
      if ( 0 != qAlpha(pic_rgba) ) {
        ::memcpy( pixels, &rgba, 4 );
      }
      pixels += 4;
    }
  }
}

QImage GeoPixmap::image() const
{
  if ( 0 == pix_ ) {
    return QImage();
  }
  return *pix_;
}

int32_t GeoPixmap::dataSize() const
{
  int32_t objsz = Object::dataSize();
  QByteArray loc;
  if ( 0 != pix_ ) {
    QDataStream stream(&loc, QIODevice::WriteOnly);
    stream << (*pix_);
  }
  int32_t sz = loc.size();
  objsz += sizeof(sz);
  objsz += sz;
  if ( 0 == coordbunch_.first.size() ) {
    objsz += sizeof(bool);
  }
  else {
    objsz += sizeof(bool);
    objsz += 3*2*sizeof(int32_t);
    objsz += coordbunch_.second.dataSize();
  }
  objsz += sizeof(bool);
  return objsz;
}

int32_t GeoPixmap::serializeToArray( char* arr ) const
{
  int32_t pos = Object::data(arr);
  int32_t sz = 0;
  if ( 0 == pix_ ) {
    ::memcpy( arr + pos, &sz, sizeof(sz) );
    pos += sizeof(sz);
  }
  else {
    QByteArray loc;
    QDataStream stream(&loc, QIODevice::WriteOnly);
    stream << (*pix_);
    sz = loc.size();
    ::memcpy( arr + pos, &sz, sizeof(sz) );
    pos += sizeof(sz);
    ::memcpy( arr + pos, loc.data(), sz );
    pos += sz;
  }
  bool hasbunch = false;
  if ( 0 == coordbunch_.first.size() ) {
    ::memcpy( arr + pos, &hasbunch, sizeof(hasbunch) );
    pos += sizeof(hasbunch);
  }
  else {
    hasbunch = true;
    ::memcpy( arr + pos, &hasbunch, sizeof(hasbunch) );
    pos += sizeof(hasbunch);
    const QPolygon& screen = coordbunch_.first;
    int32_t xy = screen[0].x();
    ::memcpy( arr + pos, &xy, sizeof(xy) );
    pos += sizeof(xy);
    xy = screen[0].y();
    ::memcpy( arr + pos, &xy, sizeof(xy) );
    pos += sizeof(xy);
    xy = screen[1].x();
    ::memcpy( arr + pos, &xy, sizeof(xy) );
    pos += sizeof(xy);
    xy = screen[1].y();
    ::memcpy( arr + pos, &xy, sizeof(xy) );
    pos += sizeof(xy);
    xy = screen[2].x();
    ::memcpy( arr + pos, &xy, sizeof(xy) );
    pos += sizeof(xy);
    xy = screen[2].y();
    ::memcpy( arr + pos, &xy, sizeof(xy) );
    pos += sizeof(xy);
    pos +=coordbunch_.second.getData( arr + pos );
  }
  ::memcpy( arr + pos, &bindedtoskelet_, sizeof(bindedtoskelet_) );
  pos += sizeof(bindedtoskelet_);
  return pos;
}

int32_t GeoPixmap::parseFromArray( const char* arr )
{
  int32_t pos = Object::setData(arr);
  if ( -1 == pos ) {
    return pos;
  }
  int32_t sz;
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  if ( 0 != sz ) {
    QByteArray loc( arr + pos, sz );
    QDataStream stream( &loc, QIODevice::ReadOnly );
    QImage p;
    stream >> p;
    setImage(p);
  }
  pos += sz;
  bool hasbunch;
  global::fromByteArray( arr + pos, &hasbunch );
  pos += sizeof(hasbunch);
  if ( true == hasbunch ) {
    QPolygon poly(3);
    int32_t xy;
    global::fromByteArray( arr + pos, &xy );
    pos += sizeof(xy);
    poly[0].setX(xy);
    global::fromByteArray( arr + pos, &xy );
    pos += sizeof(xy);
    poly[0].setY(xy);
    global::fromByteArray( arr + pos, &xy );
    pos += sizeof(xy);
    poly[1].setX(xy);
    global::fromByteArray( arr + pos, &xy );
    pos += sizeof(xy);
    poly[1].setY(xy);
    global::fromByteArray( arr + pos, &xy );
    pos += sizeof(xy);
    poly[2].setX(xy);
    global::fromByteArray( arr + pos, &xy );
    pos += sizeof(xy);
    poly[2].setY(xy);
    coordbunch_.first = poly;
    coordbunch_.second = GeoVector::fromData( arr + pos );
    pos += coordbunch_.second.dataSize();
  }
  global::fromByteArray( arr + pos, &bindedtoskelet_ );
  pos += sizeof(bindedtoskelet_);
  return pos;
}

QRect GeoPixmap::pixmapRect() const
{
  QRect r;
  if ( 0 == pix_ ) {
    return r;
  }
  r = QRect( QPoint(0,0), pix_->size() );
  if ( true == r.isNull() ) {
    return r;
  }
  QTransform tr;
  tr.translate( r.center().x(), r.center().y() );
  tr.rotate( property_.rotateangle());
  tr.translate( -r.center().x(), -r.center().y() );
  tr.scale( property_.scalexy().x(), property_.scalexy().y() );
  r = tr.mapRect(r);
  return r;
};

QPoint GeoPixmap::translateByPosition( const QPoint& posit ) const
{
  if ( 0 == pix_ ) {
    error_log << QObject::tr("Картинка не установлена");
    return posit;
  }
  QSize sz = pixmapRect().size();
  if ( false == sz.isValid() ) {
    error_log << QObject::tr("Размер картинки некорректный");
    return posit;
  }
  QRect r( posit, sz );
  switch ( pos() ) {
    case kCenter:
      r.moveCenter( posit );
      break;
    case kTopCenter:
      r.moveCenter( posit );
      r.moveBottom( posit.y() );
      break;
    case kLeftCenter:
      r.moveCenter( posit );
      r.moveRight( posit.x() );
      break;
    case kTopLeft:
      r.moveBottomRight(posit);
      break;
    case kBottomCenter:
      r.moveCenter( posit );
      r.moveTop( posit.y() );
      break;
    case kBottomLeft:
      r.moveTopRight( posit );
      break;
    case kRightCenter:
      r.moveCenter( posit );
      r.moveLeft( posit.x() );
      break;
    case kTopRight:
      r.moveBottomLeft( posit );
      break;
    case kBottomRight:
      //nothing to do
      //r.moveTopLeft( pos );
      break;
    default:
      warning_log << QObject::tr("Значение положения картинки property_.pos = %1 не удалось обработать.")
        .arg( pos() );
      break;
  }
  return r.topLeft();
}

QTransform GeoPixmap::bunchTransform( const QTransform& transform ) const
{
  QTransform tr;
  Projection* proj = Object::projection();
  if ( 0 == proj ) {
    warning_log << QObject::tr("Параметры проекции не заданы. Невозможно связать координаты картинки с географическими");
    return tr;
  }
  const QPolygon& screen = coordbunch_.first;
  const GeoVector& coord = coordbunch_.second;
  if ( 3 > screen.size() || 3 > coord.size() ) {
    warning_log << QObject::tr("Экранные и географические координаты не связаны или связаны некорректно");
    return tr;
  }
  QPolygon mapscreen;
  for ( int i = 0; i < 3; ++i ) {
    QPoint pnt;
    if ( false == proj->F2X_one( coord[i], &pnt ) ) {
      error_log
        << QObject::tr("Ошибка преобразования географической координаты в экранную")
        << coord[i];
      return tr;
    }
    mapscreen.append( transform.map(pnt) );
  }

  tr = QTransform(
      1, screen[0].x(), screen[0].y(),
      1, screen[1].x(), screen[1].y(),
      1, screen[2].x(), screen[2].y()
      );
  tr = tr.inverted();

  QVector<qreal> a = QVector<qreal>()
    << tr.m11()*mapscreen[0].x() + tr.m12()*mapscreen[1].x() + tr.m13()*mapscreen[2].x()
    << tr.m21()*mapscreen[0].x() + tr.m22()*mapscreen[1].x() + tr.m23()*mapscreen[2].x()
    << tr.m31()*mapscreen[0].x() + tr.m32()*mapscreen[1].x() + tr.m33()*mapscreen[2].x();


  QVector<qreal> b = QVector<qreal>()
    << tr.m11()*mapscreen[0].y() + tr.m12()*mapscreen[1].y() + tr.m13()*mapscreen[2].y()
    << tr.m21()*mapscreen[0].y() + tr.m22()*mapscreen[1].y() + tr.m23()*mapscreen[2].y()
    << tr.m31()*mapscreen[0].y() + tr.m32()*mapscreen[1].y() + tr.m33()*mapscreen[2].y();

  return QTransform( a[1], b[1], a[2], b[2], a[0], b[0] );
}

int GeoPixmap::nearestPointIndex( const QPoint& pnt, const QPolygon& poly, int* d2 ) const
{
  if ( 0 > pnt.x() && ::abs( pnt.x() ) == pnt.x() ) {
    return -1;
  }
  if ( 0 > pnt.y() && ::abs( pnt.y() ) == pnt.y() ) {
    return -1;
  }
  int pnt_indx = -1;
  int dist2 = 1000000000;
  for ( int i = 0, sz = poly.size(); i < sz; ++i ) {
    const QPoint& src = poly[i];
    if ( 0 > src.x() && ::abs( src.x() ) == src.x() ) {
      continue;
    }
    if ( 0 > src.y() && ::abs( src.y() ) == src.y() ) {
      continue;
    }
    QPoint delta = src - pnt;
    if ( 0 > delta.x() && ::abs( delta.x() ) == delta.x() ) {
      continue;
    }
    if ( 0 > delta.y() && ::abs( delta.y() ) == delta.y() ) {
      continue;
    }
    int locdist = delta.x()*delta.x()+delta.y()*delta.y();
    if ( 0 > locdist ) {
      continue;
    }
    if ( locdist < dist2 ) {
      dist2 = locdist;
      pnt_indx = i;
      *d2 = dist2;
    }
  }
  return pnt_indx;
}

}
}
