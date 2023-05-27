#include "puanson.h"

#include <qpainter.h>
#include <qdebug.h>
#include <qfile.h>
#include <qbuffer.h>

#include <commons/geom/geom.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/fonts/weatherfont.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/punchrules/punchrules.h>

#include "document.h"
#include "wind.h"
#include "flow.h"
#include "layerpunch.h"
#include <meteo/commons/global/weatherloader.h>

namespace meteo {
namespace map {

namespace {
Object* createPuanson( Layer* l )
{
  return new Puanson(l);
}
static const bool res = singleton::PtkppFormat::instance()->registerObjectHandler( Puanson::Type, createPuanson );
Object* createPuanson2( Object* o )
{
  return new Puanson(o);
}
static const bool res2 = singleton::PtkppFormat::instance()->registerChildHandler( Puanson::Type, createPuanson2 );

int nearestPointIndex( const QPoint& pnt, const QPolygon& poly, int* d2 )
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

Puanson::Puanson( Layer* l )
  : Label(l),
    wind_(nullptr),
    flow_(nullptr),
    bindedtoskelet_(false),
    lenft_(40.0)
{
  setPos( kCenter );
}

Puanson::Puanson( Object* p )
  : Label(p),
    wind_(nullptr),
    flow_(nullptr),
    bindedtoskelet_(false),
    lenft_(40.0)
{
  setPos( kCenter );
}

Puanson::Puanson( Projection* proj )
  : Label(proj),
    wind_(nullptr),
    flow_(nullptr),
    bindedtoskelet_(false),
    lenft_(40.0)
{
  setPos( kCenter );
}

Puanson::Puanson( const meteo::Property& prop )
  : Label(prop),
    wind_(nullptr),
    flow_(nullptr),
    bindedtoskelet_(false),
    lenft_(40.0)
{
  setPos( kCenter );
}

Puanson::Puanson()
  : Label(),
    wind_(nullptr),
    flow_(nullptr),
    bindedtoskelet_(false),
    lenft_(40.0)
{
  setPos( kCenter );
}

Puanson::~Puanson()
{
  delete flow_; flow_ = nullptr;
  delete wind_; wind_ = nullptr;
}

Object* Puanson::copy( Layer* l ) const
{
  if ( nullptr == l ) {
    error_log << QObject::tr("Нулевой указатель на слой");
    return nullptr;
  }
  Puanson* pn = new Puanson(l);
  pn->setProperty(property_);
  pn->setPunch( punch() );
  pn->meteodata_ = meteodata_;
  pn->setSkelet(skelet());
  foreach(Object* o, objects_){
    if ( o != flow_ && o != wind_ ) {
      o->copy(pn);
    }
  }
  return pn;
}

Object* Puanson::copy( Object* o ) const
{
  if ( nullptr == o ) {
    error_log << QObject::tr("Нулевой указатель на объект");
    return nullptr;
  }
  Puanson* pn = new Puanson(o);
  pn->setProperty(property_);
  pn->setPunch( punch() );
  pn->meteodata_ = meteodata_;
  pn->setSkelet(skelet());
  foreach(Object* o, objects_){
    if ( o != flow_ && o != wind_ ) {
      o->copy(pn);
    }
  }
  return pn;
}

Object* Puanson::copy( Projection* proj ) const
{
  if ( nullptr == proj ) {
    error_log << QObject::tr("Нулевой указатель на проекцию");
    return nullptr;
  }
  Puanson* pn = new Puanson(proj);
  pn->setProperty(property_);
  pn->setPunch( punch() );
  pn->meteodata_ = meteodata_;
  pn->setSkelet(skelet());
  foreach(Object* o, objects_){
    if ( o != flow_ && o != wind_ ) {
      o->copy(pn);
    }
  }
  return pn;
}

bool Puanson::visible( float scalepower ) const
{
  return Object::visible(scalepower);
  //  if ( false == Object::visible(scalepower) ) {
  //    return false;
  //  }
  //  if ( true == property_.draw_always() ) {
  //    return true;
  //  }
  //  if ( nullptr == layer() || nullptr == document() ) {
  //    return true;
  //  }
  //  if ( true == layer()->isObjectInDrawedList(this) ) {
  //    return true;
  //  }
  //  QList< QPair< QPoint, float > > list  = screenPoints( document()->transform() );
  //  bool fl = false;
  //  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
  //    auto pos = list[i];
  //    QRect r = Label::boundingRect( pos.first, kCenter, pos.second );
  //    fl = layer()->isLabelIntersectedWithDrawed(r);
  //    if ( fl == false ) {
  //      return true;
  //    }
  //  }
  //  return false;
}

const puanson::proto::Puanson& Puanson::punch() const
{
  LayerPunch* l = maplayer_cast<LayerPunch*>( layer() );
  if ( nullptr == l ) {
    return punch_;
  }
  return l->punch_;
}

bool Puanson::hasWind() const
{
  descr_t dddescr = TMeteoDescriptor::instance()->descriptor("dd");
  descr_t ffdescr = TMeteoDescriptor::instance()->descriptor("ff");
  if ( false == meteodata_.hasParam(dddescr) || false == meteodata_.hasParam(ffdescr) ) {
    return false;
  }
  return true;
}

bool Puanson::hasFlow() const
{
  descr_t dndescr = TMeteoDescriptor::instance()->descriptor("dndn");
  descr_t cndescr = TMeteoDescriptor::instance()->descriptor("cncn");
  if ( false == meteodata_.hasParam(dndescr) || false == meteodata_.hasParam(cndescr) ) {
    return false;
  }
  return true;
}

TMeteoParam Puanson::paramValue( const QString& name, bool* ok ) const
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

TMeteoParam Puanson::paramValue( const puanson::proto::Id& id, bool* ok ) const
{
  QString name = QString::fromStdString(id.name());
  return paramValue( name, ok );
}

TMeteoParam Puanson::dd( bool* ok ) const
{
  return paramValue( "dd", ok );
}

TMeteoParam Puanson::ff( bool* ok ) const
{
  return paramValue( "ff", ok );
}

TMeteoParam Puanson::dn( bool* ok ) const
{
  return paramValue( "dndn", ok );
}

TMeteoParam Puanson::cn( bool* ok ) const
{
  return paramValue( "cncn", ok );
}

void Puanson::loadTestMeteoData()
{
  TMeteoData md;
  const puanson::proto::Puanson& p = punch();
  for ( int i = 0, sz = p.rule_size(); i < sz; ++i ) {
    const puanson::proto::CellRule& rule = p.rule(i);
    const puanson::proto::Id& id = rule.id();
    float val = rule.testval();
    int qual = rule.testqual();
    TMeteoParam param( "", val, (control::QualityControl)qual );
    setMeteoParam( QString::fromStdString(id.name()), param, &md );
  }
  setMeteodata(md);
}

void Puanson::setPunch( const puanson::proto::Puanson& p )
{
  cached_punch_string_.clear();
  LayerPunch* l = maplayer_cast<LayerPunch*>( layer() );
  if ( nullptr == l ) {
    punch_.CopyFrom(p);
  }
  property_.set_priority( p.priority() );
  if ( true == p.wind() && ( true == meteodata_.hasParam("dd") || true == meteodata_.hasParam("ff") ) ) {
    descr_t dddescr = TMeteoDescriptor::instance()->descriptor("dd");
    descr_t ffdescr = TMeteoDescriptor::instance()->descriptor("ff");
    float dd;
    control::QualityControl ddqual;
    float ff;
    control::QualityControl ffqual;
    bool ddok = meteodata_.getValue( dddescr, &dd, &ddqual );
    bool ffok = meteodata_.getValue( ffdescr, &ff, &ffqual );
    if ( false == ddok ) {
      ddqual = control::NO_OBSERVE;
    }
    if ( false == ffok ) {
      ffqual = control::NO_OBSERVE;
    }
    setDdff( MnMath::ftoi_norm(dd), MnMath::ftoi_norm(ff), ddqual, ffqual );
    if ( nullptr == wind_ ) {
      wind_ = new Wind(this);
    }
    wind_->setLenft(lenft_);
    wind_->set_draw_bad_dd( p.draw_bad_dd() );
    wind_->setFont( p.windfont() );
  }
  else {
    delete wind_; wind_ = nullptr;
  }
  if ( true == p.flow() && ( true == meteodata_.hasParam("dndn") || true == meteodata_.hasParam("cncn") ) ) {
    descr_t dndescr = TMeteoDescriptor::instance()->descriptor("dndn");
    descr_t cndescr = TMeteoDescriptor::instance()->descriptor("cncn");
    float dn;
    control::QualityControl dnqual;
    float cn;
    control::QualityControl cnqual;
    bool dnok = meteodata_.getValue( dndescr, &dn, &dnqual );
    bool cnok = meteodata_.getValue( cndescr, &cn, &cnqual );
    if ( false == dnok ) {
      dnqual = control::NO_OBSERVE;
    }
    if ( false == cnok ) {
      cnqual = control::NO_OBSERVE;
    }
    setDncn( MnMath::ftoi_norm(dn), MnMath::ftoi_norm(cn), dnqual, cnqual );
    if ( nullptr == flow_ ) {
      flow_ = new Flow(this);
    }
    flow_->setFont( p.windfont() );
  }
  else {
    delete flow_; flow_ = nullptr;
  }
  cached_rect_ = QRect();
  delete pix_; pix_ = nullptr;
  //  preparePixmap();
}

bool Puanson::setPunchById( const QString& id )
{
  auto wl = meteo::map::WeatherLoader::instance();
  auto punches = wl->punchlibrary();
  if ( true == punches.contains(id) ) {
    setPunch( punches[id] );
    return true;
  }
  punches = wl->punchlibraryspecial();
  if ( true == punches.contains(id) ) {
    setPunch( punches[id] );
    return true;
  }
  error_log << QObject::tr("Шаблон пуансона '%1' не найден")
               .arg(id);
  return false;
}

bool Puanson::setPunch( const QString& path )
{
  QFile file(path);
  if ( false == file.exists() ) {
    error_log << QObject::tr("Не найден файл %1 c шаблоном пуансона")
                 .arg( file.fileName());
    return false;
  }
  if ( false == file.open( QIODevice::ReadOnly ) ) {
    error_log << QObject::tr("Не удалось открыть файл %1 c шаблоном пуансона")
                 .arg( file.fileName());
    return false;
  }
  if ( 1024*1024 < file.size() ) {
    warning_log << QObject::tr("Размер файла %1 c шаблоном пуансона большой %2 б")
                   .arg( file.fileName() )
                   .arg( file.size() );
  }

  QByteArray arr = file.readAll();
  puanson::proto::Puanson p;
  if ( false == TProtoText::fillProto( arr, &p ) ) {
    error_log << QObject::tr("Не удалось загрузить шаблон %1").arg( file.fileName() );
    return false;
  }
  file.close();
  setPunch(p);
  return true;
}

void Puanson::setUV( float u, float v)
{
  float dd=0.;
  float ff=0.;
  MnMath::preobrUVtoDF(u, v, &dd, &ff);
  setDdff(dd,ff,control::ABSENT_CORRECTED,control::ABSENT_CORRECTED);
}

void Puanson::setDdff( int dd, int ff, control::QualityControl ddqual, control::QualityControl ffqual )
{
  //  descr_t dddescr = TMeteoDescriptor::instance()->descriptor("dd");
  //  descr_t ffdescr = TMeteoDescriptor::instance()->descriptor("ff");
  QString ddcode = QString::number(dd);
  QString ffcode = QString::number(ff);
  TMeteoParam ddparam( ddcode, dd, ddqual );
  TMeteoParam ffparam( ffcode, ff, ffqual );
  setMeteoParam("dd", ddparam, &meteodata_ );
  setMeteoParam("ff", ffparam, &meteodata_ );
  //  meteodata_.add( "dd", ddparam );
  //  meteodata_.add( "ff", ffparam );
  if ( true == punch().wind() ) {
    if ( nullptr == wind_ ) {
      wind_ = new Wind(this);
    }
    wind_->setLenft(lenft_);
    wind_->set_draw_bad_dd( punch().draw_bad_dd() );
    wind_->setFont( punch().windfont() );
    wind_->setDdff( dd, ff, ddqual, ffqual, punch().radius() );
  }
  else {
    delete wind_;
    wind_ = nullptr;
  }
}

void Puanson::setDncn( int dn, int cn, control::QualityControl dnqual, control::QualityControl cnqual )
{
  //  descr_t dddescr = TMeteoDescriptor::instance()->descriptor("dd");
  //  descr_t ffdescr = TMeteoDescriptor::instance()->descriptor("ff");
  QString dncode = QString::number(dn);
  QString cncode = QString::number(dn);
  TMeteoParam dnparam( dncode, dn, dnqual );
  TMeteoParam cnparam( cncode, cn, cnqual );
  setMeteoParam("dndn", dnparam, &meteodata_ );
  setMeteoParam("cncn", cnparam, &meteodata_ );
  //  meteodata_.add( "dd", ddparam );
  //  meteodata_.add( "ff", ffparam );
  if ( true == punch().flow() ) {
    if ( nullptr == flow_ ) {
      flow_ = new Flow(this);
    }
    flow_->setFont( punch().windfont() );
    flow_->setDncn( dn, cn, dnqual, cnqual, punch().radius() );
  }
  else {
    delete flow_;
    flow_ = nullptr;
  }
}

void Puanson::setParamValue( const QString& name, const TMeteoParam& param )
{
  setMeteoParam( name, param, &meteodata_ );
  if ( name == "dd" || name == "ff" ) {
    descr_t dddescr = TMeteoDescriptor::instance()->descriptor("dd");
    descr_t ffdescr = TMeteoDescriptor::instance()->descriptor("ff");
    float dd;
    control::QualityControl ddqual;
    float ff;
    control::QualityControl ffqual;
    bool ddok = meteodata_.getValue( dddescr, &dd, &ddqual );
    bool ffok = meteodata_.getValue( ffdescr, &ff, &ffqual );
    if ( false == ddok ) {
      ddqual = control::NO_OBSERVE;
    }
    if ( false == ffok ) {
      ffqual = control::NO_OBSERVE;
    }
    setDdff( MnMath::ftoi_norm(dd), MnMath::ftoi_norm(ff), ddqual, ffqual );
  }
  else if ( true == meteodata_.hasParam("u") && true == meteodata_.hasParam("v") ) {
    descr_t udescr = TMeteoDescriptor::instance()->descriptor("u");
    descr_t vdescr = TMeteoDescriptor::instance()->descriptor("v");
    float u=0.;
    control::QualityControl uqual;
    float v=0.;
    control::QualityControl vqual;
    bool uok = meteodata_.getValue( udescr, &u, &uqual );
    bool vok = meteodata_.getValue( vdescr, &v, &vqual );
    if (( true == uok ) && ( false == vok )) {
      setUV(u, v);
    }
  }
  else if ( name == "dndn" || name == "cncn" ) {
    descr_t dndescr = TMeteoDescriptor::instance()->descriptor("dndn");
    descr_t cndescr = TMeteoDescriptor::instance()->descriptor("cncn");
    float dn;
    control::QualityControl dnqual;
    float cn;
    control::QualityControl cnqual;
    bool dnok = meteodata_.getValue( dndescr, &dn, &dnqual );
    bool cnok = meteodata_.getValue( cndescr, &cn, &cnqual );
    if ( false == dnok ) {
      dnqual = control::NO_OBSERVE;
    }
    if ( false == cnok ) {
      cnqual = control::NO_OBSERVE;
    }
    setDncn( MnMath::ftoi_norm(dn), MnMath::ftoi_norm(cn), dnqual, cnqual );
  }
  else {
    cached_rect_ = QRect();
    //    delete pix_; pix_ = nullptr;
    //    preparePixmap();
  }
}

void Puanson::setParamValue( const puanson::proto::Id& id, const TMeteoParam& param )
{
  setParamValue( QString::fromStdString(id.name()), param );
}

void Puanson::setMeteodata( const TMeteoData& md )
{
  QList<QRect> oldlist;
  if ( nullptr != document() && nullptr != document()->eventHandler() ) {
    oldlist = boundingRect( document()->transform() );
  }
  meteodata_ = md;
  bool has_wind = false;
  if ( true == punch().wind()) {
    if ( true == meteodata_.hasParam("dd") || true == meteodata_.hasParam("ff") ) {
      descr_t dddescr = TMeteoDescriptor::instance()->descriptor("dd");
      descr_t ffdescr = TMeteoDescriptor::instance()->descriptor("ff");
      float dd;
      control::QualityControl ddqual;
      float ff;
      control::QualityControl ffqual;
      bool ddok = meteodata_.getValue( dddescr, &dd, &ddqual );
      bool ffok = meteodata_.getValue( ffdescr, &ff, &ffqual );
      if ( false == ddok ) {
        ddqual = control::NO_OBSERVE;
      }
      if ( false == ffok ) {
        ffqual = control::NO_OBSERVE;
      }
      setDdff( MnMath::ftoi_norm(dd), MnMath::ftoi_norm(ff), ddqual, ffqual );
      has_wind = true;
    }
    else {
      if ( true == meteodata_.hasParam("u") && true == meteodata_.hasParam("v") ) {
        descr_t udescr = TMeteoDescriptor::instance()->descriptor("u");
        descr_t vdescr = TMeteoDescriptor::instance()->descriptor("v");
        float u=0.;
        control::QualityControl uqual;
        float v=0.;
        control::QualityControl vqual;
        bool uok = meteodata_.getValue( udescr, &u, &uqual );
        bool vok = meteodata_.getValue( vdescr, &v, &vqual );
        if (( true == uok ) && ( true == vok )) {
          setUV(u, v);
          has_wind = true;
        }
      }
    }
  }
  if(false == has_wind ){
    delete wind_; wind_ = nullptr;
  }
  if ( true == punch().flow() && ( true == meteodata_.hasParam("dndn") || true == meteodata_.hasParam("cncn") ) ) {
    descr_t dndescr = TMeteoDescriptor::instance()->descriptor("dndn");
    descr_t cndescr = TMeteoDescriptor::instance()->descriptor("cncn");
    float dn;
    control::QualityControl dnqual;
    float cn;
    control::QualityControl cnqual;
    bool dnok = meteodata_.getValue( dndescr, &dn, &dnqual );
    bool cnok = meteodata_.getValue( cndescr, &cn, &cnqual );
    if ( false == dnok ) {
      dnqual = control::NO_OBSERVE;
    }
    if ( false == cnok ) {
      cnqual = control::NO_OBSERVE;
    }
    setDncn( MnMath::ftoi_norm(dn), MnMath::ftoi_norm(cn), dnqual, cnqual );
  }
  else {
    delete flow_; flow_ = nullptr;
  }
  cached_rect_ = QRect();
  delete pix_; pix_ = nullptr;
  if ( nullptr != layer() && nullptr != document() && nullptr != document()->eventHandler() && false == document()->eventHandler()->muted() ) {
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

void Puanson::windCorrection( float angle )
{
  if ( punch().wind() && nullptr != wind_ ) {
    float dd = wind_->dd() + angle;
    float ff = wind_->ff();
    auto ffqual = wind_->ffqual();
    auto ddqual = wind_->ddqual();
    wind_->setDdff(dd ,ff, ddqual, ffqual);
  }
}

QList<QRect> Puanson::boundingRect( const QTransform& transform ) const
{
  QList<QRect> list;
  if ( false == hasPunch() ) {
    return list;
  }
  auto pntlist = screenPoints(transform);
  for ( int i = 0, sz = pntlist.size(); i < sz; ++i ) {
    list.append( Label::boundingRect( pntlist[i].first, kCenter, pntlist[i].second ) );
  }
  return list;
}

float Puanson::bindedAngle( const QPoint& pnt, bool* res ) const
{
  *res = false;
  if ( false == bindedToSkelet() ) {
    return 0.0;
  }
  if ( nullptr == parent() ) {
    return 0.0;
  }
  if ( nullptr == document() ) {
    return 0.0;
  }
  QVector<QPolygon> cached_points;
  for ( auto p : parent()->cartesianSkelet() ) {
    cached_points.append( document()->transform().map(p) );
  }
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
    return 0.0;
  }
  if ( -1 == indx || -1 == numpoly ) {
    return 0.0;
  }
  QPointF pntf(cached_points[numpoly][indx]);
  float angle;
  *res = angleOnPolygon( boundingRect(), cached_points[numpoly], indx, &pntf, &angle );
  if ( true == *res ) {
    return angle;
  }
  return 0.0;
}

void Puanson::setLenft( float l )
{
  lenft_ = l;
  if ( nullptr != wind_ ) {
    wind_->setLenft(lenft_);
  }
}

QRect Puanson::boundingRect() const
{
  if ( false == cached_rect_.isNull() ) {
    return cached_rect_;
  }
  QRect r;
  if ( false == hasPunch() ) {
    cached_rect_ = r;
    return r;
  }
  for ( int i = 0, sz = punch().rule_size(); i < sz; ++i ) {
    const puanson::proto::CellRule& rule = punch().rule(i);
    QString id = QString::fromStdString( rule.id().name() );
    if ( false == meteodata_.hasParam(id) && false == rule.has_absent_rule() ) {
      continue;
    }
    r = r.united( boundingRect(rule) );
  }
  if ( false == MnMath::isEqual( 1.0, punch().scale() ) ) {
    QSize sz = r.size()*punch().scale();
    QRect nr( r.topLeft()*punch().scale(), sz );
    r = nr;
  }
  if ( nullptr != wind_ ) {
    r = r.united(wind_->boundingRect());
  }
  if ( nullptr != flow_ ) {
    r = r.united(flow_->boundingRect());
  }
  if ( true == r.isNull() ) {
    cached_rect_ = r;
    return r;
  }
  switch ( punch().center() ) {
    case puanson::proto::kNoCenter:
      break;
    case puanson::proto::kCircle:
    case puanson::proto::kSquare: {
      QRect r2 =  QRect( QPoint(0,0), QSize( punch().radius()*2*punch().scale(), punch().radius()*2*punch().scale() ) );
      r2.moveTo( r2.topLeft() - QPoint( r2.width()/2, r2.height()/2) );
      r = r.united(r2);
    }
    break;
  }
  if ( true == punch().crisscross() ) {
    r = r.united(
          QRect( QPoint( -punch().radius()*punch().scale(), -punch().radius()*punch().scale() ),
                 QSize( punch().radius()*punch().scale()*2, punch().radius()*punch().scale()*2 ) )
          );
  }
  cached_rect_ = r;
  return r;
}

QRect Puanson::boundingRect( const puanson::proto::CellRule& rule ) const
{
  QRect r;
  QString id = QString::fromStdString( rule.id().name() );
  if ( false == meteodata_.hasParam(id) && false == rule.has_absent_rule() ) {
    return r;
  }
  QPoint pnt( rule.pos().x(), rule.pos().y() );
  if ( nullptr != wind_ && false == wind_->ddok() && true == rule.y_plus_bad_dd() ) {
    pnt.setY( pnt.y() - wind_->boundingRect().height() );
  }
  if ( false == meteodata_.hasParam(id) ) {
    QImage* pix = new QImage( QSize(1,1),QImage::Format_ARGB32 );
    QPainter p(pix);
    TMeteoParam param;
    float val = param.value();
    QString str = stringFromRuleValue( val, rule );
    QFont f = fontFromRuleValue( val, rule );
    QColor c = colorFromRuleValue( val, rule );
    p.setFont(f);
    p.setPen(c);
    r = p.boundingRect( QRect(), Qt::AlignHCenter | Qt::AlignVCenter, str );
    p.end();
    delete pix;
    r.moveTo(pnt);
    switch ( rule.align() ) {
    case puanson::proto::kCenter:
      r.moveTo( r.topLeft() - QPoint( r.width()/2, r.height()/2) );
    break;
    case puanson::proto::kTopCenter:
      r.moveTo( r.topLeft() - QPoint( r.width()/2, r.height() ) );
    break;
    case puanson::proto::kLeftCenter:
      r.moveTo( r.topLeft() - QPoint( r.width(), r.height()/2) );
    break;
    case puanson::proto::kTopLeft:
      r.moveTo( r.topLeft() - QPoint( r.width(), r.height()) );
    break;
    case puanson::proto::kBottomCenter:
      r.moveTo( r.topLeft() - QPoint( r.width()/2, 0 ) );
    break;
    case puanson::proto::kBottomLeft:
      r.moveTo( r.topLeft() - QPoint( r.width(), 0 ) );
    break;
    case puanson::proto::kRightCenter:
      r.moveTo( r.topLeft() - QPoint( 0, r.height()/2 ) );
    break;
    case puanson::proto::kTopRight:
      r.moveTo( r.topLeft() - QPoint( 0, r.height() ) );
    break;
    case puanson::proto::kNoPosition:
      r = QRect();
    break;
    case puanson::proto::kBottomRight:
    break;
    }

    return r;
  }
  switch ( rule.type() ) {
  case puanson::proto::kNumber:
    r = boundingRect( rule.id(), pnt, rule.align(), rule.number() );
  break;
  case puanson::proto::kSymbol:
    r = boundingRect( rule.id(), pnt, rule.align(), rule.symbol() );
  break;
  case puanson::proto::kSymNumber:
    r = boundingRect( rule.id(), pnt, rule.align(), rule.symbol() );
    r = r.united( boundingRect( rule.id(), pnt, rule.align(), rule.number() ) );
  break;
  }
  return r;
}

QRect Puanson::boundingRect( const puanson::proto::Id& id, const QPoint& pos, puanson::proto::Align align, const puanson::proto::CellRule::NumbeRule& rule ) const
{
  meteo::internal::WeatherFont* wf = WeatherFont::instance();
  if ( nullptr == wf ) {
    error_log << QObject::tr("Не удалось загрузить библиотеку специальных шрифтов");
  }
  QString strid = QString::fromStdString( id.name() ) ;
  QRect r;
  bool res = false;
  const TMeteoParam& param = meteodata_.meteoParam(strid, &res);
  if ( false == res ) {
    return r;
  }
  QImage* pix = new QImage( QSize(1,1),QImage::Format_ARGB32 );
  QPainter p(pix);
  float val = param.value();
  QString str = stringFromRuleValue( val, rule );
  QFont f = fontFromRuleValue( val, rule );
  QColor c = colorFromRuleValue( val, rule );
  p.setFont(f);
  p.setPen(c);
  r = p.boundingRect( QRect(), Qt::AlignHCenter | Qt::AlignVCenter, str );
  p.end();
  delete pix;
  r.moveTo(pos);
  switch ( align ) {
  case puanson::proto::kCenter:
    r.moveTo( r.topLeft() - QPoint( r.width()/2, r.height()/2) );
  break;
  case puanson::proto::kTopCenter:
    r.moveTo( r.topLeft() - QPoint( r.width()/2, r.height() ) );
  break;
  case puanson::proto::kLeftCenter:
    r.moveTo( r.topLeft() - QPoint( r.width(), r.height()/2) );
  break;
  case puanson::proto::kTopLeft:
    r.moveTo( r.topLeft() - QPoint( r.width(), r.height()) );
  break;
  case puanson::proto::kBottomCenter:
    r.moveTo( r.topLeft() - QPoint( r.width()/2, 0 ) );
  break;
  case puanson::proto::kBottomLeft:
    r.moveTo( r.topLeft() - QPoint( r.width(), 0 ) );
  break;
  case puanson::proto::kRightCenter:
    r.moveTo( r.topLeft() - QPoint( 0, r.height()/2 ) );
  break;
  case puanson::proto::kTopRight:
    r.moveTo( r.topLeft() - QPoint( 0, r.height() ) );
  break;
  case puanson::proto::kNoPosition:
    r = QRect();
  break;
  case puanson::proto::kBottomRight:
  break;
  }

  return r;
}

QRect Puanson::boundingRect( const puanson::proto::Id& id, const QPoint& pos, puanson::proto::Align align, const puanson::proto::CellRule::SymbolRule& rule ) const
{
  meteo::internal::WeatherFont* wf = WeatherFont::instance();
  if ( nullptr == wf ) {
    error_log << QObject::tr("Не удалось загрузить библиотеку специальных шрифтов");
  }
  QString strid = QString::fromStdString( id.name() ) ;
  QRect r;
  bool res = false;
  const TMeteoParam& param = meteodata_.meteoParam(strid, &res);
  if ( false == res ) {
    return r;
  }
  float val = param.value();
  QString code = param.code();

  QString str = stringFromRuleValue( val, rule, code );
  QFont f = fontFromRuleValue( val, rule );
  QColor c = colorFromRuleValue( val, rule );

  QImage* pix = new QImage( QSize(1,1), QImage::Format_ARGB32);
  QPainter p(pix);
  p.setFont(f);
  p.setPen(c);
  r = p.boundingRect( QRect(), Qt::AlignHCenter | Qt::AlignVCenter, str );
  p.end();
  delete pix;
  r.moveTo(pos);
  switch ( align ) {
  case puanson::proto::kCenter:
    r.moveTo( r.topLeft() - QPoint( r.width()/2, r.height()/2) );
  break;
  case puanson::proto::kTopCenter:
    r.moveTo( r.topLeft() - QPoint( r.width()/2, r.height() ) );
  break;
  case puanson::proto::kLeftCenter:
    r.moveTo( r.topLeft() - QPoint( r.width(), r.height()/2) );
  break;
  case puanson::proto::kTopLeft:
    r.moveTo( r.topLeft() - QPoint( r.width(), r.height()) );
  break;
  case puanson::proto::kBottomCenter:
    r.moveTo( r.topLeft() - QPoint( r.width()/2, 0 ) );
  break;
  case puanson::proto::kBottomLeft:
    r.moveTo( r.topLeft() - QPoint( r.width(), 0 ) );
  break;
  case puanson::proto::kRightCenter:
    r.moveTo( r.topLeft() - QPoint( 0, r.height()/2 ) );
  break;
  case puanson::proto::kTopRight:
    r.moveTo( r.topLeft() - QPoint( 0, r.height() ) );
  break;
  case puanson::proto::kNoPosition:
    r = QRect();
  break;
  case puanson::proto::kBottomRight:
  break;
  }

  return r;
}

QList<GeoVector> Puanson::skeletInRect( const QRect& rect, const QTransform& transform ) const
{
  QList<GeoVector> gvlist;
  Projection* proj = projection();
  if ( nullptr == proj ) {
    return gvlist;
  }
  auto list = screenPoints(transform);
  GeoVector gv;
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    const QPoint& scr = list[i].first;
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

int Puanson::minimumScreenDistance( const QPoint& pos, QPoint* cross ) const
{
  int dist = 10000000;
  QTransform tr;
  if ( nullptr != document() ) {
    tr = document()->transform();
  }
  QList<QRect> rects = boundingRect(tr);
  QList<QPolygon> polys;
  for ( auto r : rects ) {
    polys.append( QPolygon( r, true ) );
  }
  for ( auto p : polys ) {
    int d = meteo::geom::distance( pos, p, cross );
    if ( d < dist ) {
      dist = d;
    }
  }
  return dist;
}

QString Puanson::stringForParam( const QString& name ) const
{
  const TMeteoParam& param = meteodata_.meteoParam( name );
  puanson::proto::Id id;
  id.set_name( name.toStdString() );
  return stringFromRuleValue( param.value(), id, punch(), param.code() );
}

QStringList Puanson::stringForParamList(const QString &name) const
{
  auto paramList = meteodata_.meteoParamList(name);
  QStringList str;
  for ( auto param : paramList ) {
    puanson::proto::Id id;
    id.set_name( name.toStdString() );
    auto value = stringFromRuleValue( param.value(), id, punch(), param.code() );
    if ( false == str.contains(value)) {
      str.append(value);
    }
  }
  return str;
}

QFont Puanson::fontForParam( const QString& name ) const
{
  if ( false == meteodata_.hasParam(name) ) {
    return QFont();
  }
  const TMeteoParam& param = meteodata_.meteoParam( name );
  puanson::proto::Id id;
  id.set_name( name.toStdString() );
  return fontFromRuleValue( param.value(), id, punch() );
}

QColor Puanson::colorForParam( const QString& name ) const
{
  if ( false == meteodata_.hasParam(name) ) {
    return Qt::black;
  }
  const TMeteoParam& param = meteodata_.meteoParam( name );
  puanson::proto::Id id;
  id.set_name( name.toStdString() );
  return colorFromRuleValue( param.value(), id, punch() );
}

std::string Puanson::fontForParamHtml( const std::string& name ) const
{
  QFont f = fontForParam( QString::fromStdString(name) );
  return f.family().toStdString();
}

std::string Puanson::colorForParamHtml( const std::string& name ) const
{
  QColor c = colorForParam( QString::fromStdString(name) );
  return c.name().toStdString();
}

QString Puanson::descriptorName( int64_t descr, QString* description, int index, int* d_index )
{
  QString name = "unK";
  meteodescr::TMeteoDescriptor* md = TMeteoDescriptor::instance();
  if ( nullptr == md ) {
    warning_log << QObject::tr("Не удалось загрузить библиотеку дескрипторов");
    return name;
  }
  if ( false == md->isExist(descr) ) {
    *description = QObject::tr("Выдумнная величина");
    return name;
  }
  QString n = md->name(descr);
  if ( false == md->isAdditional(n) ) {
    name = md->name(descr);
    *description = md->description(name);
    if ( nullptr != d_index ) {
      *d_index = 0;
    }
    return name;
  }
  QList<QString> alladd = md->addNames();
  int  addfound = -1;
  meteodescr::Additional defadd;
  QString defaddname;
  meteodescr::Additional add;
  QString addname;
  for ( int i = 0, sz = alladd.size(); i < sz; ++i ) {
    addname = alladd[i];
    add = md->additional(addname);
    if ( descr == add.descr ) {
      if ( -1 == addfound ) {
        addfound = 0;
        defaddname = addname;
        defadd = add;
        *description = defadd.description;
        if ( nullptr != d_index ) {
          *d_index = defadd.index;
        }
      }
      if ( index == add.index ) {
        addfound = 1;
        *description = add.description;
        if ( nullptr != d_index ) {
          *d_index = add.index;
        }
        break;
      }
    }
  }
  if ( -1 == addfound ) {
    error_log << QObject::tr("Неизвестная ошибка. Не найдено описание дополнительной величины %1. Но все проерки были выполнены")
                 .arg(descr);
    *description = QObject::tr("Неизвестная ошибка");
    return name;
  }
  else if ( 0 == addfound ) {
    return defaddname;
  }
  else {
    return addname;
  }
}
//
//puanson::proto::Id Puanson::descr2Id( int64_t descr, int index, int* d_index )
//{
//  QString description;
//  int indx = index;
//  QString name = descriptorName( descr, &description, index, &indx );
//  puanson::proto::Id id;
//  if ( 0 != d_index ) {
//    *d_index = indx;
//  }
//  id.set_descr(descr);
////  id.set_index(indx);
//  id.set_name( name.toStdString() );
//  id.set_description( description.toStdString() );
//  return id;
//}

bool Puanson::fakeDescriptor( const QString& name )
{
  meteodescr::TMeteoDescriptor* md = TMeteoDescriptor::instance();
  if ( nullptr == md ) {
    error_log << QObject::tr("Не удалось загрузить библиотеку метеодескрипторов");
    return false;
  }
  if ( false == md->contains(name) && false == md->isAdditional(name) ) {
    return true;
  }
  return false;
}

void Puanson::setMeteoParam( const QString& name, const TMeteoParam& param, TMeteoData* data )
{
  if ( nullptr == data ) {
    error_log << QObject::tr("Нулевой указатель TMeteoData");
    return;
  }
  meteodescr::TMeteoDescriptor* md = TMeteoDescriptor::instance();
  if ( nullptr == md ) {
    error_log << QObject::tr("Не удалось загрузить библиотеку метеодескрипторов");
    return;
  }
  if ( true == md->isAdditional(name) ) {
    meteodescr::Additional add = md->additional(name);
    descr_t descr = add.descr;
    //    int indx = add.index;
    data->add( descr, param );
  }
  else if ( true == md->contains(name) ) {
    descr_t descr = md->descriptor(name);
    data->remove(name);
    data->add( descr, param );
  }
  else {
    error_log << QObject::tr("Неизвестное имя дескриптора %1").arg(name);
  }
}

GeoVector Puanson::windSkelet( bool* ok ) const
{
  if ( nullptr != ok ) {
    *ok = false;
  }
  GeoVector gv;
  if ( 0 == skelet().size() ) {
    return gv;
  }
  gv.append( skelet()[0] );
  if ( nullptr == wind_ ) {
    return gv;
  }
  if ( false == meteodata_.hasParam("dd") ) {
    return gv;
  }
  if ( nullptr == projection() ) {
    return gv;
  }
  if ( nullptr == document() ) {
    return gv;
  }

  QPoint pnt = document()->coord2screen( gv[0] );
  QLineF line( pnt, QPoint(0,0) );
  line.setLength(200);
  float angle = 0.0;
  if ( true == bindedToSkelet() ) {
    bool locok;
    angle = bindedAngle( pnt, &locok )*RAD2DEG;
    if ( nullptr != ok ) {
      *ok = locok;
    }
    if ( false == locok ) {
      return gv;
    }
  }
  else if ( 0 != document() ) {
    angle = document()->deviationFromNorth(pnt)*RAD2DEG;
  }
  //↓отрицательное dd для поворота по часовой; 90, чтоб начать от севера; -180 для противоположной точки, как это для sxf надо
  line.setAngle(-( dd().value() + angle ) + 90 - 180 );

  GeoPoint gp = document()->screen2coord(line.p2());
  //  if ( false == projection()->X2F_one( line.p2().toPoint(), &gp ) ) {
  //    return gv;
  //  }
  gv.append(gp);
  if ( nullptr != ok ) {
    *ok = true;
  }
  return gv;
}

GeoVector Puanson::bindedSkelet( bool* ok ) const
{
  if ( nullptr != ok ) {
    *ok = false;
  }
  GeoVector gv;
  if ( 0 == skelet().size() ) {
    return gv;
  }
  gv.append( skelet()[0] );
  if ( nullptr == projection() ) {
    return gv;
  }
  if ( nullptr == document() ) {
    return gv;
  }

  QPoint pnt = document()->coord2screen(gv[0]);
  QLineF line( pnt, QPoint(0,0) );
  line.setLength(200);
  float angle = 0.0;
  if ( true == bindedToSkelet() ) {
    bool locok = false;
    angle = bindedAngle( pnt, &locok )*RAD2DEG;
    if ( nullptr != ok ) {
      *ok = locok;
    }
    if ( false == locok ) {
      return gv;
    }
  }
  else if ( nullptr != document() ) {
    angle = document()->deviationFromNorth(pnt)*RAD2DEG;
  }
  //↓отрицательное dd для поворота по часовой; 90, чтоб начать от севера; -180 для противоположной точки, как это для sxf надо
  line.setAngle(-( + angle ) /*+ 90 - 180*/ );

  GeoPoint gp = document()->screen2coord( line.p2() );
  gv.append(gp);
  if ( nullptr != ok ) {
    *ok = true;
  }
  return gv;
}

const std::string& Puanson::punchString() const
{
  LayerPunch* l = maplayer_cast<LayerPunch*>( layer() );
  if ( nullptr == l ) {
    if ( 0 == cached_punch_string_.size() ) {
      punch().SerializeToString(&cached_punch_string_);
    }
  }
  return cached_punch_string_;
}

int32_t Puanson::dataSize() const
{
  int32_t objsz = Object::dataSize();
  const std::string& str = punchString();
  int32_t sz = str.size();
  objsz += sizeof(sz);
  objsz += sz;
  QByteArray loc;
  QDataStream stream( &loc, QIODevice::WriteOnly );
  stream << meteodata_;
  sz = loc.size();
  objsz += sizeof(sz);
  objsz += sz;

  if ( nullptr != screenpos_ ) {
    objsz += sizeof(bool); //есть экранная координата
    objsz += 2*sizeof(int32_t); //размер x и y
  }
  else {
    objsz += sizeof(bool);
  }
  objsz += sizeof(bool); //bindedtoskelet_
  objsz += sizeof(float); //lenft_

  return objsz;
}

int32_t Puanson::serializeToArray( char* arr ) const
{
  int32_t pos = Object::data(arr);
  const std::string& str = punchString();
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
  ::memcpy( arr + pos, &bindedtoskelet_, sizeof(bindedtoskelet_) );
  pos += sizeof(bindedtoskelet_);
  ::memcpy( arr + pos, &lenft_, sizeof(lenft_) );
  pos += sizeof(lenft_);

  return pos;
}

int32_t Puanson::parseFromArray( const char* arr )
{
  int32_t pos = Object::setData(arr);
  if ( -1 == pos ) {
    return pos;
  }
  cached_punch_string_.clear();
  delete pix_; pix_ = nullptr;
  cached_rect_ = QRect();
  int32_t sz;
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  if ( 0 != sz ) {
    puanson::proto::Puanson p;
    p.ParseFromArray( arr + pos, sz );
    setPunch(p);
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
  global::fromByteArray( arr + pos, &bindedtoskelet_);
  pos += sizeof(bindedtoskelet_);
  global::fromByteArray( arr + pos, &lenft_);
  pos += sizeof(lenft_);
  setLenft(lenft_);
  return pos;
}

bool Puanson::isEmpty() const
{
  if ( false == hasPunch() || true == meteodata_.isEmpty() ) {
    return true;
  }
  return false;
}

QList< QPair< QPoint, float > > Puanson::screenPoints( const QTransform& transform ) const
{
  QList< QPair< QPoint, float > > points;
  if ( false == hasPunch() ) {
    return points;
  }
  if ( nullptr != screenpos_ ) {
    points.append( qMakePair( *screenpos_, float(0) ) );
    return points;
  }
  for ( int i = 0, isz = cartesian_points_.size(); i < isz; ++i ) {
    for ( int j = 0, jsz = cartesian_points_[i].size(); j < jsz; ++j ) {
      QPoint pnt  = transform.map(cartesian_points_[i][j]);
      float locangle = 0.0;
      if ( true == bindedtoskelet_ ) {
        bool res = false;
        locangle = bindedAngle(pnt, &res );
        if ( false == res ) {
          continue;
        }
      }
      else if ( nullptr != document() && proto::kNoOrient != document()->puansonOrientation() ) {
        locangle = (-document()->deviationFromNorth(pnt));
      }
      points.append( qMakePair( pnt, locangle ) );
    }
  }
  return points;
}

QSize Puanson::size() const
{
  if ( false == hasPunch() ) {
    return QSize(0,0);
  }
  return boundingRect().size();
}

bool Puanson::preparePixmap()
{
  if ( nullptr != pix_ ) {
    delete pix_; pix_ = nullptr;
  }
  meteo::internal::WeatherFont* wf = WeatherFont::instance();
  if ( nullptr == wf ) {
    error_log << QObject::tr("Не удалось загрузить библиотеку специальных шрифтов");
  }
  QRect r = boundingRect();
  if ( false == r.isValid() ) {
    return false;
  }
  cached_rect_ = r;
  QPoint rtopleft = r.topLeft();
  QSize sz = r.size();
  if ( true == sz.isEmpty() ) {
    return false;
  }
  pix_ = new QImage( sz, QImage::Format_ARGB32 );
  pix_->fill( QColor(0,0,0,0) );
  if ( true == punch().drawrect() ) {
    //    pix_->fill( QColor::fromRgba( punch().rectbrushcolor() ) );
    QPainter pntr(pix_);
    pntr.setBrush( QBrush( QColor::fromRgba( punch().rectbrushcolor() ) ) );
    QPen pn( QColor::fromRgba( punch().rectpencolor() ) );
    pn.setWidth(1);
    pntr.setPen(pn);
    pntr.drawRect( QRect( 0, 0, pix_->width() - 1, pix_->height() - 1) );
  }
  QPainter pntr(pix_);
  if ( false == MnMath::isEqual( 1.0, punch().scale() ) ) {
    pntr.translate( -rtopleft );
    pntr.scale( punch().scale(), punch().scale() );
  }
  else {
    pntr.translate( -rtopleft );
  }
  pntr.setRenderHint( QPainter::Antialiasing, true );
  pntr.setRenderHint( QPainter::TextAntialiasing, true );
  pntr.setRenderHint( QPainter::HighQualityAntialiasing, true );
  //  pntr.save();
  //  pntr.setBrush( QBrush(Qt::white) );
  //  pntr.drawEllipse( -3, -3, 6, 6 );
  //  pntr.restore();
  bool hasval = false;
  for ( int i = 0, sz = punch().rule_size(); i < sz; ++i ) {
    const puanson::proto::CellRule& rule = punch().rule(i);
    if ( false == rule.visible() ) {
      continue;
    }
    const puanson::proto::Id id = rule.id();
    QString paramname = QString::fromStdString( id.name() );
    if ( false == meteodata_.hasParam(paramname) && false == rule.has_absent_rule() ) {
      continue;
    }
    pntr.save();
    TMeteoParam param = meteodata_.meteoParam(paramname);
    float val = param.value();
    QString code = param.code();
    QRect rr = boundingRect(rule);
    if ( false == meteodata_.hasParam(paramname) ) {
      val = param.value();
      code = "";
      param.setQuality( control::RIGHT, control::NOCONTROL, true );
    }

    bool drawrect = rule.drawrect();
    if ( nullptr != document() ) {
      if ( true == ( control::SPECIAL_VALUE < param.quality() ) ) {
        if ( kBadWithRect == document()->property().ruleview() ) {
          drawrect = true;
        }
        else if ( kBadInvisible == document()->property().ruleview() ) {
          continue;
        }
      }
    }
    else {
      if ( true == ( control::SPECIAL_VALUE < param.quality() ) ) {
        if ( kBadWithRect == punch().ruleview() ) {
          drawrect = true;
        }
        else if ( kBadInvisible == punch().ruleview() ) {
          continue;
        }
      }
    }

    if ( true == drawrect ) {
      pntr.save();
      QPen ppp(Qt::black);
      ppp.setWidth(0);
      pntr.setPen(ppp);
      pntr.drawRect(rr);
      pntr.restore();
    }

    QString str = stringFromRuleValue( val, rule, code );
    if ( true == str.isEmpty() ) {
      pntr.restore();
      continue;
      warning_log << QObject::tr("Пустая строка значения %1 величины %2")
                     .arg(val)
                     .arg( QString::fromStdString(rule.id().Utf8DebugString() ) );
    }
    hasval = true;
    QFont font = fontFromRuleValue( val, rule );
    QColor clr;
    if ( false == punch().has_color() ) {
      clr = colorFromRuleValue( val, rule );
    }
    else {
      clr = QColor::fromRgba( punch().color() );
    }

    pntr.setFont(font);
    pntr.setPen(clr);
    pntr.drawText( rr, Qt::AlignHCenter | Qt::AlignVCenter, str );
    pntr.restore();
  }
  if ( false == hasval ) {
    return false;
  }
  pntr.save();
  QPen pn = pntr.pen();
  if ( true == punch().has_color() ) {
    pn.setColor( QColor::fromRgba( punch().color() ) );
  }
  else {
    pn.setColor( Qt::black );
  }
  pntr.setPen(pn);
  switch ( punch().center() ) {
    case puanson::proto::kNoCenter:
      break;
  case puanson::proto::kCircle:
    pntr.drawEllipse(QRect( -punch().radius(), -punch().radius(), punch().radius()*2, punch().radius()*2 ));
    break;
  case puanson::proto::kSquare:
    pntr.drawRect( QRect( -punch().radius(), -punch().radius(), punch().radius()*2, punch().radius()*2 ) );
    break;
  }
  if ( true == punch().crisscross() ) {
    pntr.drawLine( QPoint(-punch().radius(), 0), QPoint(+punch().radius(), 0) );
    pntr.drawLine( QPoint( 0, -punch().radius() ), QPoint( 0, +punch().radius() ) );
  }
  pntr.restore();
  return true;
}

}
}
