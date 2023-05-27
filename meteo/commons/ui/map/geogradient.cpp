#include "geogradient.h"

#include <qpainter.h>
#include <qdebug.h>
#include <qmath.h>
#include <qelapsedtimer.h>

#include <commons/obanal/interpolorder.h>
#include <commons/mathtools/mnmath.h>
#include <commons/geom/geom.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/common.h>

#include "layer.h"
#include "layeriso.h"
#include "geotext.h"
#include "map.h"
#include "document.h"
#include "simple.h"
#include <meteo/commons/global/gradientparams.h>

#define start_time QElapsedTimer _timer__name; _timer__name.start()
#define reset_time(text) debug_log << text << _timer__name.restart() << "msec"

namespace meteo {
namespace map {

namespace {
  Object* createGrad( Layer* l )
  {
    return new GeoGradient(l);
  }
  static const bool res = singleton::PtkppFormat::instance()->registerObjectHandler( GeoGradient::Type, createGrad );
  Object* createGrad2( Object* o )
  {
    return new GeoGradient(o);
  }
  static const bool res2 = singleton::PtkppFormat::instance()->registerChildHandler( GeoGradient::Type, createGrad2 );
}

GeoGradient::GeoGradient( LayerIso* l )
  : Object(l),
  field_( l->field() ),
  fieldcolor_( l->fieldColor() )
{
  if ( 0 != l ) {
    l->incGradientCount();
  }
}

GeoGradient::GeoGradient( Layer* l, obanal::TField* field )
  : Object(l),
  field_(field)
{
  GradientParams grpr( meteo::global::kIsoParamPath() );
  if ( nullptr != field_ ) {
    fieldcolor_ = grpr.protoParams( field_->getDescr() );
  }
  LayerIso* li = maplayer_cast<LayerIso*>( layer() );
  if ( nullptr != li ) {
    li->incGradientCount();
  }
}

GeoGradient::GeoGradient( Object* p, obanal::TField* field )
  : Object(p),
  field_(field)
{
  GradientParams grpr( meteo::global::kIsoParamPath() );
  if ( nullptr != field_ ) {
    fieldcolor_ = grpr.protoParams( field_->getDescr() );
  }
  LayerIso* li = maplayer_cast<LayerIso*>( layer() );
  if ( nullptr != li ) {
    li->incGradientCount();
  }
}

GeoGradient::GeoGradient( Projection* proj, obanal::TField* field )
  : Object(proj),
  field_(field)
{
  GradientParams grpr( meteo::global::kIsoParamPath() );
  if ( nullptr != field_ ) {
    fieldcolor_ = grpr.protoParams( field_->getDescr() );
  }
  LayerIso* li = maplayer_cast<LayerIso*>( layer() );
  if ( nullptr != li ) {
    li->incGradientCount();
  }
}

GeoGradient::GeoGradient( LayerIso* l, const proto::FieldColor& color )
  : Object(l),
  field_( l->field() ),
  fieldcolor_(color)
{
  if ( nullptr != l ) {
    l->incGradientCount();
  }
}


GeoGradient::GeoGradient( Layer* l, obanal::TField* field, const proto::FieldColor& color )
  : Object(l),
  field_(field),
  fieldcolor_(color)
{
  LayerIso* li = maplayer_cast<LayerIso*>( layer() );
  if ( nullptr != li ) {
    li->incGradientCount();
  }
}

GeoGradient::GeoGradient( Object* p, obanal::TField* field, const proto::FieldColor& color )
  : Object(p),
  field_(field),
  fieldcolor_(color)
{
  LayerIso* li = maplayer_cast<LayerIso*>( layer() );
  if ( nullptr != li ) {
    li->incGradientCount();
  }
}

GeoGradient::GeoGradient( Projection* proj, obanal::TField* field, const proto::FieldColor& color )
  : Object(proj),
  field_(field),
  fieldcolor_(color)
{
  LayerIso* li = maplayer_cast<LayerIso*>( layer() );
  if ( nullptr != li ) {
    li->incGradientCount();
  }
}

GeoGradient::~GeoGradient()
{
  LayerIso* l = maplayer_cast<LayerIso*>( layer() );
  if ( nullptr != l ) {
    l->decGradientCount();
  }
}

Object* GeoGradient::copy( Layer* l ) const
{
  if ( nullptr == l ) {
    error_log << QObject::tr("Нулевой указатель на слой");
    return 0;
  }
  GeoGradient* o = nullptr;
  LayerIso* li = maplayer_cast<LayerIso*>(l);
  if ( nullptr != li ) {
    o = new GeoGradient(li);
  }
  else {
    o = new GeoGradient(l, field_, fieldcolor_ );
  }
  o->setProperty(property_);
  o->setSkelet(skelet());
  foreach(Object* ob, objects_){
    ob->copy(o);
  }
  return o;
}

Object* GeoGradient::copy( Object* o ) const
{
  if ( nullptr == o ) {
    error_log << QObject::tr("Нулевой указатель на объект");
    return nullptr;
  }
  GeoGradient* iso = new GeoGradient(o, field_, fieldcolor_ );
  iso->setProperty(property_);
  iso->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(iso);
  }
  return iso;
}

Object* GeoGradient::copy( Projection* proj ) const
{
  if ( nullptr == proj ) {
    error_log << QObject::tr("Нулевой указатель на проекцию");
    return nullptr;
  }
  GeoGradient* iso = new GeoGradient(proj, field_, fieldcolor_ );
  iso->setProperty(property_);
  iso->setSkelet(skelet());
  foreach(Object* o, objects_){
    o->copy(iso);
  }
  return iso;
}

TColorGradList GeoGradient::gradient() const
{
  if ( nullptr == field_ ) {
    return TColorGradList();
  }
  return GradientParams::gradParams( field_->getLevel(), field_->getLevelType(), fieldcolor_ );
}

void GeoGradient::setGradient( const TColorGradList& grad )
{
  if ( nullptr == field_ ) {
    return;
  }
  GradientParams::setGradColor( field_->getLevel(), field_->getLevelType(), grad, &fieldcolor_ );
}

bool GeoGradient::render( QPainter* painter, const QRect& target, const QTransform& transform )
{
  wasdrawed_ = false;
  if ( false == visible() ) {
      return false;
  }
  if ( nullptr == field_ ) {
    return false;
  }
  if ( nullptr == projection() ) {
    return false;
  }
//  Q_UNUSED(target);

  int min_x = target.topLeft().x();
  int min_y = target.topLeft().y();
  int max_x = target.bottomRight().x();
  int max_y = target.bottomRight().y();
  qreal op = painter->opacity();
  int alpha = int(op*250.);
  start_time;
//  int min_x = document()->documentTopLeft().x();
//  int min_y = document()->documentTopLeft().y();
//  int max_x = document()->documentBottomRight().x();
//  int max_y = document()->documentBottomRight().y();
  int kolx = max_x - min_x;
  int koly = max_y - min_y;
  if ( kolx < 1 || koly < 1 ) {
    return false;
  }
  QRgb* cvals = new QRgb[kolx*koly];

  TColorGradList gradlist = gradient();
  
  for ( auto g : gradlist ) {
    auto minclr = g.begcolor();
    auto maxclr = g.endcolor();
    debug_log << "COLOR =" << minclr.red() << minclr.green() << minclr.blue() << maxclr.red() << maxclr.green() << maxclr.blue() << g.begval() << g.endval() ;
  }

  QTransform invtr = transform.inverted();
  int step_k = 10;
  QImage img( kolx,koly,QImage::Format_ARGB32 );
  for ( int x=min_x; x<max_x; x+=step_k ) {
    int xx = x-min_x;
    for ( int y=min_y; y<max_y; y+=step_k ) {
      int yy = y-min_y;
      uint32_t* trgt_pnt = cvals + (xx*koly +yy);
      *trgt_pnt = qRgba( 255, 255, 255,0 );
      QPoint cp = invtr.map( QPoint(x,y) );
      float v;
      GeoPoint gp;
      bool ok = projection()->X2F_one( cp, &gp );
      if ( false  == ok ) {
        continue;
      }
//      debug_log << "gp =" << gp;
      v = field_->pointValueF( gp, &ok );
      if ( false == ok ) {
        continue;
      }
//      debug_log << gp;
//      debug_log << "valF=" << v;
//      debug_log << "val =" << field_->pointValue( gp, &ok );
      QColor c = gradlist.color(v, &ok );
      if ( false == ok ) {
        continue;
      }
      if ( c.alpha() != 255 ) {
        *trgt_pnt = qRgba( c.red(), c.green(), c.blue(), c.alpha() );
      }
      else {
        *trgt_pnt = qRgba( c.red(), c.green(), c.blue(), alpha );
      }
    }
  }
  unsigned char* bits = img.bits();
  int32_t imgsizem1 = img.width()*img.height() - 1;
  for ( int xx=0; xx<kolx-step_k; xx+=step_k ) {
    for ( int yy=0; yy<koly-step_k; yy+=step_k ) {
      QRgb color0_0 = cvals[xx*koly +yy];
      QRgb color0_1 = cvals[xx*koly +yy+step_k];
      QRgb color1_0 = cvals[(xx+step_k)*koly +yy];
      QRgb color1_1 = cvals[(xx+step_k)*koly +yy+step_k];
      if(color0_0 == color0_1 && color0_0==color1_0&& color0_0== color1_1){
        for ( int y_vs = 0; y_vs <step_k; ++y_vs ) {
          for ( int x_vs =0; x_vs < step_k; ++x_vs ) {
            int32_t shift = xx+x_vs + (yy+y_vs)*img.width();
            if ( imgsizem1 < shift ) {
              //warning_log << QObject::tr("Выход за границы изображения");
              continue;
            }
            ::memcpy( bits + shift*4, &color0_0, 4 );
//            img.setPixel ( xx+x_vs,yy+y_vs,color0_0);
          }
        }
        continue;
      }

      for ( int y_vs = 0; y_vs <step_k; ++y_vs ) {
        for ( int x_vs = 0; x_vs < step_k; ++x_vs ) {
          uint32_t icolor = qRgba ( 255, 255, 255, 0 );
          QPoint cp = invtr.map( QPoint(min_x+ xx+x_vs,min_y+yy+y_vs) );
          float v;
          GeoPoint gp;
          bool ok = projection()->X2F_one( cp, &gp );
          int32_t shift = xx+x_vs + (yy+y_vs)*img.width();
          if ( imgsizem1 < shift ) {
            //warning_log << QObject::tr("Выход за границы изображения");
            continue;
          }
          if ( false  == ok ) {
            ::memcpy( bits + shift*4, &icolor, 4 );
//            img.setPixel ( xx+x_vs,yy+y_vs,icolor );
            continue;
          }
          v = field_->pointValueF( gp, &ok );
          if ( false == ok ) {
            ::memcpy( bits + shift*4, &icolor, 4 );
//            img.setPixel ( xx+x_vs,yy+y_vs,icolor );
            continue;
          }
          QColor c = gradlist.color(v, &ok );
          if ( false == ok ) {
            ::memcpy( bits + shift*4, &icolor, 4 );
//            img.setPixel ( xx+x_vs,yy+y_vs,icolor );
            continue;
          }
          icolor = qRgba( c.red(), c.green(), c.blue(), alpha );
//          img.setPixel ( xx+x_vs,yy+y_vs,icolor );
          ::memcpy( bits + shift*4, &icolor, 4 );
        }
      }
    }
  }

  painter->save();
  painter->setRenderHint( QPainter::Antialiasing, false );
  painter->setRenderHint( QPainter::SmoothPixmapTransform, false );
  painter->setRenderHint( QPainter::HighQualityAntialiasing, false );
  painter->setPen ( Qt::NoPen );
//  painter->drawImage( document()->documentRect().topLeft(), img );
  painter->drawImage( target.topLeft(), img );
  painter->restore();
  wasdrawed_ = true;
  delete[] cvals;
  return true;
}
    
QList<QRect> GeoGradient::boundingRect( const QTransform& transform ) const
{
  Q_UNUSED(transform);
  if ( nullptr == document() ) {
    return QList<QRect>();
  }
  return QList<QRect>() << document()->documentRect();
}
    
QList<GeoVector> GeoGradient::skeletInRect( const QRect& rect, const QTransform& transform ) const
{
  Q_UNUSED(rect);
  Q_UNUSED(transform);
  return QList<GeoVector>();
}

int GeoGradient::minimumScreenDistance( const QPoint& pos, QPoint* cross ) const
{
  Q_UNUSED(pos);
  Q_UNUSED(cross);
  return 0.0;
}

void GeoGradient::setValue( float val, const QString& frmt, const QString& unt )
{
  Q_UNUSED(val);
  Q_UNUSED(frmt);
  Q_UNUSED(unt);
}

void GeoGradient::loadFieldColorFromSettings( int descr )
{
  GradientParams params( meteo::global::kIsoParamPath() );
  fieldcolor_ = params.protoParams(descr);
}

int32_t GeoGradient::dataSize() const
{
  int32_t objsz = Object::dataSize();
  std::string str;
  if ( true == fieldcolor_.IsInitialized() ) {
    fieldcolor_.SerializeToString(&str);
  }
  int32_t sz = str.size();
  objsz += sizeof(sz);
  objsz += sz;
  
  return objsz;
}

int32_t GeoGradient::serializeToArray( char* arr ) const
{
  int32_t pos = Object::data(arr);
  std::string str;
  if ( true == fieldcolor_.IsInitialized() ) {
    fieldcolor_.SerializeToString(&str);
  }
  int32_t var = str.size();
  ::memcpy( arr + pos, &var, sizeof(var) );
  pos += sizeof(var);
  ::memcpy( arr + pos, str.data(), var );
  pos += var;
  return pos;
}

int32_t GeoGradient::parseFromArray( const char* arr )
{
  LayerIso* l = maplayer_cast<LayerIso*>( layer() );
  if ( nullptr != l ) {
    field_ = l->field();
  }
  int32_t pos = Object::setData(arr);
  if ( -1 == pos ) {
    return pos;
  }
  int32_t var;
  global::fromByteArray( arr + pos, &var );
  pos += sizeof(var);
  if ( 0 != var ) {
    fieldcolor_.ParseFromArray( arr + pos, var );
  }
  pos += var;
  return pos;
}

}
}
