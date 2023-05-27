#include "weatherfigure.h"

#include <qpainter.h>
#include <qdebug.h>
#include <qfile.h>
#include <qbuffer.h>

#include <commons/geom/geom.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/fonts/weatherfont.h>

#include <meteo/commons/global/common.h>
#include "document.h"
#include "layerfigure.h"

namespace meteo {
namespace map {

const int32_t kDefaultRadius = 50;

namespace {
Object* createWeatherFigure( Layer* l )
{
  return new WeatherFigure(l);
}
static const bool res = singleton::PtkppFormat::instance()->registerObjectHandler( WeatherFigure::Type, createWeatherFigure );
Object* createWeatherFigure2( Object* o )
{
  return new WeatherFigure(o);
}
static const bool res2 = singleton::PtkppFormat::instance()->registerChildHandler( WeatherFigure::Type, createWeatherFigure2 );
}

WeatherFigure::WeatherFigure( Layer* l )
  : Object(l)
{
  meteo::internal::WeatherFont* wf = WeatherFont::instance();
  if ( nullptr == wf ) {
      error_log << QObject::tr("Не удалось загрузить библиотеку специальных шрифтов");
    }
}

WeatherFigure::WeatherFigure( Object* p )
  : Object(p)
{
  meteo::internal::WeatherFont* wf = WeatherFont::instance();
  if ( nullptr == wf ) {
      error_log << QObject::tr("Не удалось загрузить библиотеку специальных шрифтов");
    }
}

WeatherFigure::WeatherFigure( Projection* proj )
  : Object(proj)
{
  meteo::internal::WeatherFont* wf = WeatherFont::instance();
  if ( nullptr == wf ) {
      error_log << QObject::tr("Не удалось загрузить библиотеку специальных шрифтов");
    }
}

WeatherFigure::WeatherFigure( const meteo::Property& prop )
  : Object(prop)
{
  meteo::internal::WeatherFont* wf = WeatherFont::instance();
  if ( nullptr == wf ) {
      error_log << QObject::tr("Не удалось загрузить библиотеку специальных шрифтов");
    }
}

WeatherFigure::~WeatherFigure()
{
}

Object* WeatherFigure::copy( Layer* l ) const
{
  if ( nullptr == l ) {
      error_log << QObject::tr("Нулевой указатель на слой");
      return nullptr;
    }
  WeatherFigure* pn = new WeatherFigure(l);
  pn->setProperty(property_);
  pn->setPattern( pattern() );
  pn->meteodata_ = meteodata_;
  pn->setSkelet(skelet());
  foreach(Object* o, objects_){
      o->copy(pn);
    }
  return pn;
}

Object* WeatherFigure::copy( Object* o ) const
{
  if ( nullptr == o ) {
      error_log << QObject::tr("Нулевой указатель на объект");
      return nullptr;
    }
  WeatherFigure* pn = new WeatherFigure(o);
  pn->setProperty(property_);
  pn->setPattern( pattern() );
  pn->meteodata_ = meteodata_;
  pn->setSkelet(skelet());
  foreach(Object* o, objects_){
      o->copy(pn);
    }
  return pn;
}

Object* WeatherFigure::copy( Projection* proj ) const
{
  if ( nullptr == proj ) {
      error_log << QObject::tr("Нулевой указатель на проекцию");
      return nullptr;
    }
  WeatherFigure* pn = new WeatherFigure(proj);
  pn->setProperty(property_);
  pn->setPattern( pattern() );
  pn->meteodata_ = meteodata_;
  pn->setSkelet(skelet());
  foreach(Object* o, objects_){
      o->copy(pn);
    }
  return pn;
}

const tablo::Settings& WeatherFigure::pattern() const
{
  LayerFigure* l = maplayer_cast<LayerFigure*>( layer() );
  if ( nullptr == l ) {
      return pattern_;
    }
  return l->pattern_;
}

TMeteoParam WeatherFigure::paramValue( const QString& name, bool* ok ) const
{
  if ( nullptr != ok ) {
      *ok = false;
    }
  TMeteoParam param = meteodata_.meteoParam(name);
  if ( nullptr != ok ) {
      if ( false == param.isInvalid() ) {
          *ok = true;
        }
      else {
          *ok = false;
        }
    }
  return param;
}

void WeatherFigure::setPattern( const tablo::Settings& p )
{
  cached_pattern_string_.clear();
  LayerFigure* l = maplayer_cast<LayerFigure*>( layer() );
  if ( nullptr == l ) {
      pattern_.CopyFrom(p);
    }
}

void WeatherFigure::setMeteodata( const TMeteoData& md )
{
  QList<QRect> oldlist;
  if ( nullptr != document() && nullptr != document()->eventHandler() ) {
      oldlist = boundingRect( document()->transform() );
    }
  meteodata_ = md;
  if ( nullptr != layer() && nullptr != document() && nullptr != document()->eventHandler() ) {
      QList<QRect> list = boundingRect( document()->transform() );
      QRect bigrect;
      for ( int i = 0, sz = list.size(); i < sz; ++i ) {
          bigrect = bigrect.united(list[i]);
        }
      for ( int i = 0, sz = oldlist.size(); i < sz; ++i ) {
          bigrect = bigrect.united(oldlist[i]);
        }
      LayerEvent* ev = new LayerEvent( layer()->uuid(), LayerEvent::ObjectChanged, bigrect );
      document()->eventHandler()->postEvent(ev);
    }
}

QList<QPoint> WeatherFigure::screenPoints( const QTransform& transform ) const
{
  QList<QPoint> list;
  for ( int i = 0, sz = cartesian_points_.size(); i < sz; ++i ) {
      const QPolygon& poly = cartesian_points_[i];
      for ( int j = 0, jsz = poly.size(); j < jsz; ++j ) {
          list.append( transform.map( poly[j] ) );
        }
    }
  return list;
}

QList<QRect> WeatherFigure::boundingRect( const QTransform& transform ) const
{
  QList<QRect> list;
  if ( false == hasPattern() ) {
      return list;
    }
  int radius = kDefaultRadius;
  if ( nullptr != document() ) {
      float dm = pattern().radius_meter()/(document()->santimeteresScaleProjCenter()/100.0)/2.54;
      QImage px( QSize(1,1), QImage::Format_ARGB32 );
      radius = dm*px.logicalDpiX();
      if ( 0 == radius ) {
          radius = 1;
        }
    }
  QList<QPoint> pntlist = screenPoints(transform);
  for ( int i = 0, sz = pntlist.size(); i < sz; ++i ) {
      const QPoint& pnt = pntlist[i];
      QRect rect( QPoint(0,0), QSize(radius*2, radius*2) );
      rect.moveCenter(pnt);
      list.append(rect);
    }
  return list;
}

QList<GeoVector> WeatherFigure::skeletInRect( const QRect& rect, const QTransform& transform ) const
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
  if ( 0 != gv.size() ) {
      gvlist.append(gv);
    }
  return gvlist;
}

bool WeatherFigure::getPainterPath( QPainterPath* pp, const QRect& target, const QTransform& transform )
{
  wasdrawed_ = false;
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
  tablo::Color clr = color( pattern(), meteodata_ );
  if ( tablo::kNoColor == clr ) {
      return false;
    }
  QColor qclr;
  switch ( clr ) {
    case tablo::kGreen:
      qclr = Qt::green;
      break;
    case tablo::kYellow:
      qclr = Qt::yellow;
      break;
    case tablo::kRed:
      qclr = Qt::red;
      break;
    default:
      return false;
    }
  QBrush qbr( qclr, Qt::SolidPattern );
  QList<QRect> list = boundingRect( transform );
  cached_screen_points_.clear();
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
      const QRect& r = list[i];
      if ( false == target.intersects(r) && false == target.contains(r)  ) {
          continue;
        }
      wasdrawed_ = true;
      QPainterPath p;
      p.addEllipse(r);
      *pp = pp->united(p);
      cached_screen_points_.append( QPolygon(r) );
    }
  return true;
}

bool WeatherFigure::render( QPainter* painter, const QRect& target, const QTransform& transform )
{
  wasdrawed_ = false;
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
  tablo::Color clr = color( pattern(), meteodata_ );
  if ( tablo::kNoColor == clr ) {
      return false;
    }
  QColor qclr;
  switch ( clr ) {
    case tablo::kGreen:
      qclr = Qt::green;
      break;
    case tablo::kYellow:
      qclr = Qt::yellow;
      break;
    case tablo::kRed:
      qclr = Qt::red;
      break;
    default:
      return false;
    }
  QBrush qbr( qclr, Qt::SolidPattern );
  QList<QRect> list = boundingRect( transform );
  cached_screen_points_.clear();
  QPainterPath pp;
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
      const QRect& r = list[i];
      if ( false == target.intersects(r) && false == target.contains(r)  ) {
          continue;
        }
      wasdrawed_ = true;
      QPainterPath p;
      p.addEllipse(r);
      pp = pp.united(p);
      cached_screen_points_.append( QPolygon(r) );
    }
  painter->save();
  painter->setPen(Qt::NoPen);
  painter->setBrush(qbr);
  painter->drawPath(pp);
  painter->restore();

  return true;
}


int WeatherFigure::minimumScreenDistance( const QPoint& pos, QPoint* cross ) const
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

const std::string& WeatherFigure::patternString() const
{
  LayerFigure* l = maplayer_cast<LayerFigure*>( layer() );
  if ( nullptr == l ) {
      if ( 0 == cached_pattern_string_.size() ) {
          pattern().SerializeToString(&cached_pattern_string_);
        }
    }
  return cached_pattern_string_;
}

int32_t WeatherFigure::dataSize() const
{
  int32_t objsz = Object::dataSize();
  const std::string& str = patternString();
  int32_t sz = str.size();
  objsz += sizeof(sz);
  objsz += sz;
  QByteArray loc;
  QDataStream stream( &loc, QIODevice::WriteOnly );
  stream << meteodata_;
  sz = loc.size();
  objsz += sizeof(sz);
  objsz += sz;

  return objsz;
}

int32_t WeatherFigure::serializeToArray( char* arr ) const
{
  int32_t pos = Object::data(arr);
  const std::string& str = patternString();
  int32_t sz = str.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  if ( 0 != sz ) {
      ::memcpy( arr + pos, str.data(), sz );
    }
  pos += sz;

  QByteArray loc;
  QDataStream stream( &loc, QIODevice::WriteOnly );
  stream << meteodata_;
  sz = loc.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, loc.data(), sz );
  pos += sz;

  return pos;
}

int32_t WeatherFigure::parseFromArray( const char* arr )
{
  int32_t pos = Object::setData(arr);
  if ( -1 == pos ) {
      return pos;
    }
  cached_pattern_string_.clear();
  int32_t sz;
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  if ( 0 != sz ) {
      tablo::Settings p;
      p.ParseFromArray( arr + pos, sz );
      setPattern(p);
    }
  pos += sz;
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  if ( 0 != sz ) {
      QByteArray loc( arr + pos, sz );
      QDataStream stream( &loc, QIODevice::ReadOnly );
      TMeteoData md;
      stream >> md;
      setMeteodata(md);
    }
  pos += sz;
  return pos;
}

tablo::Color max(tablo::Color c1, tablo::Color c2)
{
  if ( tablo::kRed == c1    || tablo::kRed == c2 )    { return tablo::kRed; }
  if ( tablo::kYellow == c1 || tablo::kYellow == c2 ) { return tablo::kYellow; }
  if ( tablo::kGreen == c1  || tablo::kGreen == c2 )  { return tablo::kGreen; }

  return tablo::kNoColor;
}

tablo::Color WeatherFigure::color( const tablo::Settings& ptrn, const TMeteoData& md )
{
  tablo::Color clr = tablo::kNoColor;
  for ( int i = 0, sz = ptrn.alert_size(); i < sz; ++i ) {
      const tablo::ColorAlert& alert = ptrn.alert(i);
      if ( false == alert.enabled() ) {
          continue;
        }
      QString descr = QString::fromStdString( alert.descrname() );
      if ( false == md.hasParam(descr) ) {
          continue;
        }
      const TMeteoParam& param = md.meteoParam(descr);
      float value = param.value();
      tablo::Color color = clr;
      for ( int j = 0, jsz = alert.condition_size(); j < jsz; ++j ) {
          const tablo::Condition& cond = alert.condition(j);
          switch ( cond.operation() ) {
            case tablo::kRange:
              if ( value < cond.operand_a() || value > cond.operand_b() ) { continue; }
              break;
            case tablo::kMore:
              if ( value <= cond.operand_a() ) { continue; }
              break;
            case tablo::kLess:
              if ( value >= cond.operand_a() ) { continue; }
              break;
            case tablo::kEqual:
              if ( value != cond.operand_a() ) { continue; }
              break;
            case tablo::kNotEqual:
              if ( value == cond.operand_a() ) { continue; }
              break;
            case tablo::kCifrCode:
              //        TODO
            case tablo::kNoOperation:
              continue;
              break;
            }
          color = max(color, cond.color());
        }
      clr = max( color, clr );
    }

  return clr;
}

QPair< GeoVector, meteo::Property > WeatherFigure::getCircle( bool* ok ) const
{
  if ( nullptr != ok ) {
      *ok = false;
    }
  QPair< GeoVector, meteo::Property > ret;
  tablo::Color clr = color( pattern(), meteodata_ );
  if ( tablo::kNoColor == clr ) {
      return ret;
    }
  QColor qclr;
  switch ( clr ) {
    case tablo::kGreen:
      qclr = Qt::green;
      break;
    case tablo::kYellow:
      qclr = Qt::yellow;
      break;
    case tablo::kRed:
      qclr = Qt::red;
      break;
    default:
      return ret;
    }
  if ( 0 == skelet_.size() ) {
      return ret;
    }
  float radlen = float(pattern().radius_meter())/float(kEarthRadius);
  const GeoPoint& center = skelet_[0];
  for ( int i = 0, sz = 361; i < sz; ++i ) {
      ret.first.append( center.findSecondCoord( radlen, DEG2RAD*i ));
    }
  QBrush qbr( qclr, Qt::SolidPattern );
  meteo::Property prop;
  prop.set_closed(true);
  prop.mutable_pen()->CopyFrom( qpen2pen( QPen(Qt::NoPen) ) );
  prop.mutable_brush()->CopyFrom( qbrush2brush(qbr) );
  ret.second.CopyFrom(prop);
  if ( nullptr != ok ) {
      *ok = true;
    }
  return ret;
}

}
}
