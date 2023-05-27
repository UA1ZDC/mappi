#include "geomrl.h"

#include <qpainterpath.h>
#include <qpainter.h>
#include <qdebug.h>
#include <qmath.h>
#include <qelapsedtimer.h>

#include <commons/mathtools/mnmath.h>
#include <commons/geom/geom.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/punchrules/punchrules.h>

#include "layer.h"
#include "layermrl.h"
#include "geotext.h"
#include "puanson.h"
#include "map.h"
#include "document.h"
#include "simple.h"
#include <meteo/commons/global/radarparams.h>
#include <meteo/commons/global/weatherloader.h>
#include "geopolygon.h"

namespace meteo {
namespace map {

namespace {
  Object* createMrl( Layer* l )
  {
    return new GeoMrl(l);
  }
  static const bool res = singleton::PtkppFormat::instance()->registerObjectHandler( GeoMrl::Type, createMrl );
  Object* createMrl2( Object* o )
  {
    return new GeoMrl(o);
  }
  static const bool res2 = singleton::PtkppFormat::instance()->registerChildHandler( GeoMrl::Type, createMrl2 );
}

GeoMrl::GeoMrl( Layer* l, const Radar& r )
  : Object(l),
  radar_(r)
{
  RadarParams grpr( meteo::global::kRadarParamPath() );
  radarcolor_ = grpr.protoParams( radar_.descr() );
  calcSkelet();
}

GeoMrl::GeoMrl( Object* p, const Radar& r )
  : Object(p),
  radar_(r)
{
  RadarParams grpr( meteo::global::kRadarParamPath() );
  radarcolor_ = grpr.protoParams( radar_.descr() );
  calcSkelet();
}

GeoMrl::GeoMrl( Projection* proj, const Radar& r )
  : Object(proj),
  radar_(r)
{
  RadarParams grpr( meteo::global::kRadarParamPath() );
  radarcolor_ = grpr.protoParams( radar_.descr() );
  calcSkelet();
}


GeoMrl::GeoMrl( Layer* l, const Radar& r, const proto::RadarColor& color )
  : Object(l),
  radar_(r),
  radarcolor_(color)
{
  calcSkelet();
}

GeoMrl::GeoMrl( Object* p, const Radar& r, const proto::RadarColor& color )
  : Object(p),
  radar_(r),
  radarcolor_(color)
{
  calcSkelet();
}

GeoMrl::GeoMrl( Projection* proj, const Radar& r, const proto::RadarColor& color )
  : Object(proj),
  radar_(r),
  radarcolor_(color)
{
  calcSkelet();
}

GeoMrl::~GeoMrl()
{
}

Object* GeoMrl::copy( Layer* l ) const
{
  if ( nullptr == l ) {
    error_log << QObject::tr("Нулевой указатель на слой");
    return nullptr;
  }
  GeoMrl* o = nullptr;
  o = new GeoMrl(l, radar_, radarcolor_ );
  o->setProperty(property_);
  o->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(o);
  }
  return o;
}

Object* GeoMrl::copy( Object* o ) const
{
  if ( nullptr == o ) {
    error_log << QObject::tr("Нулевой указатель на объект");
    return nullptr;
  }
  GeoMrl* iso = new GeoMrl(o, radar_, radarcolor_ );
  iso->setProperty(property_);
  iso->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(iso);
  }
  return iso;
}

Object* GeoMrl::copy( Projection* proj ) const
{
  if ( nullptr == proj ) {
    error_log << QObject::tr("Нулевой указатель на проекцию");
    return nullptr;
  }
  GeoMrl* iso = new GeoMrl(proj, radar_, radarcolor_ );
  iso->setProperty(property_);
  iso->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(iso);
  }
  return iso;
}

void GeoMrl::setRadar( const Radar& r )
{
  radar_ = r;
  calcSkelet();
}

TColorGradList GeoMrl::gradient() const
{
  return RadarParams::gradParams( radarcolor_ );
}

void GeoMrl::setGradient( const TColorGradList& grad )
{
  RadarParams::setGradColor( grad, &radarcolor_ );
}

bool GeoMrl::render( QPainter* painter, const QRect& target, const QTransform& transform )
{
  wasdrawed_ = false;
  float scale = 0.0;
  if ( nullptr != curdoc_ ) {
    scale = curdoc_->scale();
  }
  else if ( nullptr != document() ) {
    scale = document()->scale();
  }
  if ( false == visible(scale) ) {
    return false;
  }
  cached_screen_points_.clear();

  TColorGradList grad = RadarParams::gradParams( radarcolor_ );


  pnt_transform_ = (radartransform_*transform).inverted();

  QList<QRect> rects = boundingRect( transform );

  painter->save();

  painter->setRenderHint( QPainter::Antialiasing, false );
  painter->setRenderHint( QPainter::HighQualityAntialiasing, true );

  for ( int i = 0, sz = rects.size(); i < sz; ++i ) {
    QRect r = rects[i];
    bool need_draw = false;
    bool screenrect_contains_target_whole = r.contains( target, true );
    if ( false == screenrect_contains_target_whole && true == target.intersects(r) ) {
      need_draw = true;
    }
    if ( false == need_draw && false == screenrect_contains_target_whole && true == r.intersects(target) ) {
      need_draw = true;
    }
    if ( false == need_draw && true == target.contains(r) ) {
      need_draw = true;
    }
    if ( false == need_draw ) {
      QPainterPath path;
      path.addRect(r);
      bool screenpathcontains_target = path.contains(target);
      QBrush br = qbrush();
      if ( false == screenpathcontains_target && true == path.intersects(target) ) {
        need_draw = true;
      }
      else {
        if ( true == screenpathcontains_target ) {
          need_draw = true;
        }
      }
    }
    if ( false == need_draw ) {
      continue;
    }
    r = r.intersected( document()->documentRect() );
    QPoint topleft = r.topLeft();
    QImage img( r.size(), QImage::Format_ARGB32 );
    img.fill( QColor(0,0,0,0).rgba() );
    unsigned char* bits = img.bits();
    int32_t imgsizem1 = img.width()*img.height() - 1;
    for ( int j = 0, jsz = r.width(); j < jsz; ++j ) {
      for ( int k = 0, ksz = r.height(); k < ksz; ++k ) {
        bool ok = false;
        float v = pointValue( topleft + QPoint(j,k), &ok );
        if ( true == ok ) {
          QColor c = grad.color( v, &ok );
          if ( true == ok ) {
            uint32_t trgt_pnt = qRgba( c.red(), c.green(), c.blue(), c.alpha()*0.8 );
            int32_t shift = j + k*img.width();
            if ( imgsizem1 < shift ) {
              //warning_log << QObject::tr("Выход за границы изображения");
              continue;
            }
            ::memcpy( bits + shift*4, &trgt_pnt, 4  );
//            img.setPixel( j, k, trgt_pnt );
          }
        }
      }
    }
    cached_screen_points_.append( QPolygon(r) );
    painter->save();
    painter->setRenderHint( QPainter::Antialiasing, false );
    painter->setRenderHint( QPainter::SmoothPixmapTransform, false );
    painter->setRenderHint( QPainter::HighQualityAntialiasing, false );
    painter->setPen ( Qt::NoPen );
    painter->drawImage( r.topLeft(), img );
    painter->restore();
    wasdrawed_ = true;
  }
  painter->restore();
  foreach(Object* o, objects_){
    painter->save();
    o->render( painter, target, transform );
    painter->restore();
  }
  return true;
  
}
    
QList<QRect> GeoMrl::boundingRect( const QTransform& transform ) const
{
  QList<QRect> list;
  QVector<QPolygon>::const_iterator it = cartesian_points_.constBegin();
  QVector<QPolygon>::const_iterator end = cartesian_points_.constEnd();
  for ( ; end != it; ++it ) {
    QRect r = transform.map(*it).boundingRect();
    list.append(r);
  }
  return list;
}
    
QList<GeoVector> GeoMrl::skeletInRect( const QRect& rect, const QTransform& transform ) const
{
  QList<GeoVector> gvlist;
  Projection* proj = projection();
  if ( nullptr == proj ) {
    return gvlist;
  }
  QList<QPoint> list;
  for ( int i = 0, isz = cartesian_points_.size(); i < isz; ++i ) {
    QPolygon screen = transform.map(cartesian_points_[i]);
    if ( 2 > screen.size() ) {
      continue;
    }
    bool need_draw = false;
    QRect screenrect = screen.boundingRect();
    bool screenrect_contains_target_whole = screenrect.contains( rect, true );
    if ( false == screenrect_contains_target_whole && true == rect.intersects(screenrect) ) {
      need_draw = true;
    }
    if ( false == need_draw && false == screenrect_contains_target_whole && true == screenrect.intersects(rect) ) {
      need_draw = true;
    }
    if ( false == need_draw && true == rect.contains(screenrect) ) {
      need_draw = true;
    }
    if ( false == need_draw ) {
      QPainterPath path;
      path.addPolygon(screen);
      bool screenpathcontains_target = path.contains(rect);
      QBrush br = qbrush();
      if ( false == screenpathcontains_target && true == path.intersects(rect) ) {
        need_draw = true;
      }
      else if ( Qt::NoBrush != br.style() && true == closed() ) {
        if ( true == screenpathcontains_target ) {
          need_draw = true;
        }
      }
    }
    if ( true == need_draw ) {
      gvlist.append(skelet_);
      return gvlist;
    }
  }
  return gvlist;
//  Q_UNUSED(rect);
//  Q_UNUSED(transform);
//  return QList<GeoVector>();
}

int GeoMrl::minimumScreenDistance( const QPoint& pos, QPoint* cross ) const
{
  int dist = 10000000;
  QTransform tr;
  if ( nullptr != document() ) {
    tr = document()->transform();
  }
  QList<QRect> list  = boundingRect(tr);
  if ( 0 == list.size() ) {
    return dist;
  }
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    dist = meteo::geom::distance( pos, list[i], cross );
  }
  return dist;
}

void GeoMrl::setValue( float val, const QString& frmt, const QString& unt )
{
  Q_UNUSED(val);
  Q_UNUSED(frmt);
  Q_UNUSED(unt);
}

float GeoMrl::pointValue( const QPoint& pnt, bool* ok ) const
{
  QPointF pntf = pnt_transform_.map(pnt);
  return radar_.cellValue( pntf.x(), pntf.y(), ok );
}

void GeoMrl::calcSkelet()
{
  GeoVector gv;
  float mw = float(radar_.cellWidth()*radar_.columnSize())/2.0;
  float mh = float(radar_.cellHeight()*radar_.rowSize())/2.0;
  float radlen = ::sqrt( mw*mw+mh*mh )/(kEarthRadius);
  GeoPoint gp = radar_.center().findSecondCoord( radlen, - M_PI_4 - M_PI_2 );
  gv.append(gp);
  gp = radar_.center().findSecondCoord( radlen, M_PI_4 + M_PI_2 );
  gv.append(gp);
  gp = radar_.center().findSecondCoord( radlen, M_PI_4 );
  gv.append(gp);
  gp = radar_.center().findSecondCoord( radlen, -M_PI_4 );
  gv.append(gp);
  setSkelet(gv);
  if ( 1 < cartesian_points_.size() ) {
    warning_log << QObject::tr("Данные мрл попали на обрез карты");
  }
  if ( 0 == cartesian_points_.size() ) {
    warning_log << QObject::tr("Не удалось посчитать координаты мрл-данных");
  }
  QPolygonF poly;
  poly.append( QPointF(0,0) );
  poly.append( QPointF( radar_.columnSize(), 0) );
  poly.append( QPointF( radar_.columnSize(), radar_.rowSize() ) );
  poly.append( QPointF( 0, radar_.rowSize() ) );
  bool res = QTransform::quadToQuad( poly, cartesian_points_[0], radartransform_ );
  if ( false == res ) {
    error_log << QObject::tr("Не удалось посчитать матрицу!!!");
  }
  if ( true == radarcolor_.has_punch_id() ) {
    createPunches();
  }
//  getSquares();
}

void GeoMrl::createPunches()
{
  QString punch_id = QString::fromStdString( radarcolor_.punch_id() );
  if ( false == WeatherLoader::instance()->punchlibraryspecial().contains(punch_id) ) {
    error_log << QObject::tr("Шаблон наноски знаков %1 не найден");
    return;
  }
  puanson::proto::Puanson punch = WeatherLoader::instance()->punchlibraryspecial()[punch_id];
  puanson::proto::CellRule rule;
  bool ok = false;
  for ( int i = 0, sz = punch.rule_size(); i < sz; ++i ) {
    if ( punch.rule(i).id().descr() == radarcolor_.descr() ) {
      rule.CopyFrom( punch.rule(i) );
      ok = true;
      break;
    }
  }
  if ( false == ok ) {
    error_log << QObject::tr("В шаблоне наноски нет правила для дескриптора %1").arg(radarcolor_.descr() );
    return;
  }
  QString paramname = QString::fromStdString( rule.id().name() );
  for ( int i = 0, sz = radar_.columnSize(); i < sz; ++i ) {
    for ( int j = 0, jsz = radar_.rowSize(); j < jsz; ++j ) {
      ok = false;
      float val = radar_.cellValue( i,j, &ok );
      if ( true == ok ) {
        QString str = stringFromRuleValue( val, rule );
        if ( false == str.isEmpty() ) {
          GeoPoint gp;
          ok = projection()->X2F_one( radartransform_.map(QPoint(i,j)), &gp );
          if ( false == ok ) {
            continue;
          }
          TMeteoParam param( "", val, control::NO_CONTROL );
          TMeteoData md;
          md.add( paramname, param );
          Puanson* p = new Puanson(this);
          p->setPunch(punch);
          p->setMeteodata(md);
          p->setSkelet(gp);
        }
      }
    }
  }
}

void GeoMrl::loadFieldColorFromSettings( int descr )
{
  RadarParams params( meteo::global::kRadarParamPath() );
  radarcolor_ = params.protoParams(descr);
}

int32_t GeoMrl::dataSize() const
{
  int32_t objsz = Object::dataSize();
  surf::OneMrlValueReply md = radar_.proto();
  std::string str;
  if ( true == md.IsInitialized() ) {
    md.SerializeToString(&str);
  }
  int32_t sz = str.size();
  objsz += sizeof(sz);
  objsz += sz;
  if ( true == radarcolor_.IsInitialized() ) {
    radarcolor_.SerializeToString(&str);
  }
  sz = str.size();
  objsz += sizeof(sz);
  objsz += sz;

  return objsz;
}

int32_t GeoMrl::serializeToArray( char* arr ) const
{
  int32_t pos = Object::data(arr);
  surf::OneMrlValueReply md = radar_.proto();
  std::string str;
  if ( true == md.IsInitialized() ) {
    md.SerializeToString(&str);
  }
  int32_t sz = str.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  if ( 0 != sz ) {
    ::memcpy( arr + pos, str.data(), sz );
  }
  pos += sz;
  if ( true == radarcolor_.IsInitialized() ) {
    radarcolor_.SerializeToString(&str);
  }
  sz = str.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  if ( 0 != sz ) {
    ::memcpy( arr + pos, str.data(), sz );
  }
  pos += sz;

  return pos;
}

int32_t GeoMrl::parseFromArray( const char* arr )
{
  int32_t pos = Object::setData(arr);
  if ( -1 == pos ) {
    return pos;
  }
  int32_t sz;
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  Radar r;
  if ( 0 != sz ) {
    surf::OneMrlValueReply md;
    md.ParseFromArray( arr + pos, sz );
    r.setProto(md);
    radar_ = r;
  }
  pos += sz;
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  if ( 0 != sz ) {
    radarcolor_.ParseFromArray( arr + pos, sz );
  }
  pos += sz;
  QPolygonF poly;
  poly.append( QPointF(0,0) );
  poly.append( QPointF( radar_.columnSize(), 0) );
  poly.append( QPointF( radar_.columnSize(), radar_.rowSize() ) );
  poly.append( QPointF( 0, radar_.rowSize() ) );
  bool res = QTransform::quadToQuad( poly, cartesian_points_[0], radartransform_ );
  if ( false == res ) {
    error_log << QObject::tr("Не удалось посчитать матрицу!!!");
  }
  return pos;
}

QList< QPair< GeoVector, ::meteo::Property > > GeoMrl::getSquares() const
{

  TColorGradList grad = RadarParams::gradParams( radarcolor_ );
  QList< QPair< GeoVector, ::meteo::Property > > list;
  for ( int i = 0, sz = radar_.columnSize(); i < sz; ++i ) {
    for ( int j = 0, jsz = radar_.rowSize(); j < jsz; ++j ) {
      bool ok = false;
      float val = radar_.cellValue( i,j, &ok );
      if ( true == ok ) {
        GeoPoint center;
        ok = projection()->X2F_one( radartransform_.map(QPoint(i,j)), &center );
        if ( false == ok ) {
          continue;
        }
        QColor c = grad.color( val, &ok );
        c.setAlpha( c.alpha()*0.8 );
        if ( true == ok ) {
          GeoVector gv;
          float mw = float(radar_.cellWidth())/2.0;
          float mh = float(radar_.cellHeight())/2.0;
          float radlen = ::sqrt( mw*mw+mh*mh )/(kEarthRadius);
          GeoPoint gp = center.findSecondCoord( radlen, -M_PI_4 - M_PI_2 );
          gv.append(gp);
          gp = center.findSecondCoord( radlen, M_PI_4 + M_PI_2 );
          gv.append(gp);
          gp = center.findSecondCoord( radlen, M_PI_4 );
          gv.append(gp);
          gp = center.findSecondCoord( radlen, -M_PI_4 );
          gv.append(gp);
          gv.append( gv.first() );
          ::meteo::Property prop;
          prop.mutable_pen()->CopyFrom( qpen2pen( QPen(Qt::NoPen) ) );
          prop.mutable_brush()->CopyFrom( qbrush2brush( QBrush(c) ) );
          prop.set_closed(true);
          list.append( qMakePair( gv, prop ) );
//          GeoPolygon* geopol = new GeoPolygon( layer() );
//          geopol->setProperty(prop);
//          geopol->setSkelet(gv);
        }
      }
    }
  }
  return list;
}

}
}
