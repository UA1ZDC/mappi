#include "layer.h"

#include "geogroup.h"
#include "geopixmap.h"
#include "geopolygon.h"
#include "geotext.h"
#include "puanson.h"
#include "geogradient.h"
#include "isoline.h"
#include "geomrl.h"
#include "wind.h"
#include "flow.h"
#include "layeriso.h"
#include "label.h"

#include <qwidgetaction.h>
#include <qdebug.h>
#include <quuid.h>
#include <qpainter.h>
#include <qdatastream.h>
#include <qbuffer.h>

#include <cross-commons/debug/tlog.h>
#include <sql/psql/psqlquery.h>
#include <commons/geobasis/projection.h>
#include <commons/obanal/tfield.h>
#include <meteo/commons/global/global.h>

#include "map.h"
#include "document.h"
#include "object.h"
#include "geopolygon.h"
#include "geotext.h"
#include <meteo/commons/global/weatherloader.h>
#include "legend.h"
#include "layermenu.h"

namespace meteo {
namespace map {

namespace {
Layer* createLayer( Document* d )
{
  return new Layer(d);
}
static const bool res = singleton::PtkppFormat::instance()->registerLayerHandler( Layer::Type, createLayer );
}
//
//int Layer::LAYER_COUNTER = 0;
//static QMutex gMutex(QMutex::Recursive);

Layer::Layer( Document* d, const QString& n, int ct )
  : document_(d),
    uuid_( QUuid::createUuid().toString() ),
    name_(n),
    visible_(true),
    field_(nullptr),
    isbase_(false),
    alphapercent_(0),
    menu_(nullptr),
    customtype_(ct),
    layer_data_type_(noData)
{
  info_.set_type(kLayerCommon);
  document_->addLayer(this);
  //  QMutexLocker loc(&gMutex);
  //  LAYER_COUNTER += 1;
}

Layer::~Layer()
{
  //  QMutexLocker loc(&gMutex);
  //  LAYER_COUNTER -= 1;
  if ( nullptr != menu_ ) {
    menu_->deleteLater(); menu_ = nullptr;
  }
  foreach(Object* o, objects_){
    o->layer_ = nullptr;
  }


  if ( nullptr != document_ ) {
    document_->rmLayer(this);
  }
  document_ = nullptr;
  foreach(Object* o, objects_){
    delete o;
  }
  objects_.clear();
  if ( nullptr != field_ ) {
    delete field_;
    field_ = nullptr;
  }
}

void Layer::removeAllObjects()
{
  foreach(Object* o, objects_){
    delete o;
  }
  objects_.clear();
}

bool Layer::copyFrom( Layer* l )
{
  if ( nullptr == l ) {
    error_log << QObject::tr("Нулевой указатель на копируемый слой");
    return false;
  }
  foreach(Object* o, l->objects_){
    o->copy(this);
  }

  /*  for (;itr != l->objects_.end(); ++itr)
  {
    Object* o = *itr;
    o->copy(this);
  }
*/
  name_ = l->name_;
  visible_ = l->visible_;
  field_ = l->field_;
  shortName_ = l->shortName_;
  isbase_ = l->isbase_;
  alphapercent_ = l->alphapercent_;
  return true;
}

void Layer::updateProjection()
{
  foreach(Object* o, objects_){
    o->updateProjection();
  }
}

void Layer::render( QPainter* painter, const QRect& target, Document* document )
{
  if ( false == visible() ) {
    return;
  }
  Document* olddoc = document_;
  document_ = document;
  drawedlabels_.clear();

  foreach(Object* o, objects_){
    painter->save();
    int ap = o->alphapercent();
    if ( 0 > ap ) {
      ap = 0;
    }
    if ( 100 < ap ) {
      ap = 100;
    }
    painter->setOpacity(1.0 - ap/100.0);
    o->render( painter, target, document );
    painter->restore();
  }
  document_ = olddoc;
}

bool Layer::isBase() const
{
  return isbase_;
}

Object* Layer::objectByUuid( const QString& id ) const
{
  foreach(Object* o, objects_){
    if ( o->uuid() == id ) {
      return o;
    }
  }
  return nullptr;
}

QList<Label*> Layer::labels( bool childs ) const
{
  QList<Label*> list;
  for ( auto o : objects_ ) {
    if ( true == o->isLabel() ) {
      list.append( static_cast<Label*>( o ) );
    }
    if ( true == childs ) {
      list.append( o->labels(childs) );
    }
  }
  return list;
}

Projection* Layer::projection() const
{
  if ( nullptr != document_ ) {
    return document_->projection();
  }
  return nullptr;
}

float Layer::fieldValue( const GeoPoint& gp, bool* ok ) const
{
  if ( nullptr != ok ) {
    *ok = false;
  }
  if ( nullptr == field_ ) {
    return 0.0;
  }
  return field_->pointValue( gp, ok );
}

QString Layer::valuePodpis( float val, bool* ok ) const
{
  if ( nullptr != ok ) {
    *ok = false;
  }
  if ( nullptr == field_ ) {
    return QString();
  }
  QString podpis = QString("%1: %2")
                   .arg( name_ )
                   .arg( val, 10, 'f', 2 );
  if ( nullptr != ok ) {
    *ok = true;
  }
  return podpis;
}

QStringList Layer::uuidsUnderGeoPoint( const GeoPoint& gp ) const
{
  QStringList uuids;
  foreach(Object* o, objects_){
    if ( true == o->underGeoPoint(gp) ) {
      if ( false == uuids.contains( o->uuid() ) ) {
        uuids.append( o->uuid() );
      }
    }
  }
  return uuids;
}

QList<Object*> Layer::objectsUnderGeoPoint( const GeoPoint& gp ) const
{
  QList<Object*> olist;
  if ( nullptr == document() ) {
    return olist;
  }
  QPoint pnt = document()->coord2screen(gp);
  foreach(Object* o, objects_){
    int dist = o->minimumScreenDistance(pnt);
    if ( 0 >= dist ) {
      olist.append(o);
    }
  }
  return olist;
}

QList<Object*> Layer::objectsByValue( float val ) const
{
  QList<Object*> olist;
  foreach(Object* o, objects_){
    if ( o->hasValue() && true == MnMath::isEqual( val, o->value() ) ) {
      olist.append(o);
    }
  }
  return olist;
}

QList<GeoPolygon*> Layer::polygonsByValue( float val ) const
{
  QList<GeoPolygon*> olist;
  foreach(Object* o, objects_){
    if ( false == o->hasValue() || kPolygon != o->type() ) {
      continue;
    }
    GeoPolygon* gp = mapobject_cast<GeoPolygon*>(o);
    if ( nullptr == gp ) {
      continue;
    }
    if ( true == MnMath::isEqual( val, gp->value() ) ) {
      olist.append(gp);
    }
  }
  return olist;
}

QList<GeoText*> Layer::textsByValue( float val ) const
{
  QList<GeoText*> olist;
  foreach(Object* o, objects_){
    if ( false == o->hasValue() || kText != o->type() ) {
      continue;
    }
    GeoText* gt = reinterpret_cast<GeoText*>(o);
    if ( true == MnMath::isEqual( val, gt->value() ) ) {
      olist.append(gt);
    }
  }
  return olist;
}

QString Layer::name() const {
  if(true== name_.isEmpty()){
    return layernameFromInfo(info_);
  }
  return name_;
}

void Layer::setName( const QString& n )
{
  name_ = n;
  if ( nullptr != document() && nullptr != document()->eventHandler() ) {
    document()->eventHandler()->notifyLayerChanges( document()->activeLayer(), LayerEvent::Activity );
  }
}

void Layer::setVisisble( bool fl )
{
  visible_ = fl;
  if ( nullptr != document() && nullptr != document()->eventHandler() ) {
    document()->eventHandler()->notifyLayerChanges( this, LayerEvent::Visibility );
  }
}

void Layer::setActive()
{
  if ( nullptr != document_ ) {
    document_->setActiveLayer(uuid_);
  }
}

void Layer::setField( obanal::TField* f )
{
  delete field_;
  field_ = f;
}

bool Layer::isObjectInDrawedList( const Object* o ) const
{
  if ( nullptr != document() && proto::kCrossDocument == document()->property().cross_type() ) {
    return document()->isObjectInDrawedList(o);
  }
  if ( 1 == drawedlabels_.count(o) ) {
    return true;
  }
  return false;
}

void Layer::addToDrawedLabels( Object* o )
{
  if ( nullptr != document() && proto::kCrossDocument == document()->property().cross_type() ) {
    document()->addToDrawedLabels(o);
  }
  else {
    if ( 0 == drawedlabels_.count(o) ) {
      drawedlabels_.insert(o);
    }
  }
}

//
void Layer::setProperty(const QString &name, const QVariant &value)
{
  property_.insert(name, value);
}

QVariant Layer::property(const QString &name)
{
  return property_.value(name);
}

void Layer::setInfo( const proto::WeatherLayer& info )
{
  info_.CopyFrom(info);
  /*  if ( true == name_.isEmpty() ) {
    setName( layernameFromInfo(info) );
  }*/
}

void Layer::setAlphaPercent( int a )
{
  if ( 100 < a ) {
    a = 100;
  }
  if ( 0 > a ) {
    a = 0;
  }
  alphapercent_ = a;
  foreach(Object* o, objects_){
    o->setAlphaPercent(a);
  }
}

void Layer::repaint()
{
  if ( nullptr != document() && nullptr != document()->eventHandler() ) {
    document()->eventHandler()->notifyLayerChanges( this, LayerEvent::ObjectChanged );
  }
}

bool Layer::isEqual( Layer* l ) const
{
  if ( nullptr == l ) {
    QObject::tr("Нулевой указатель");
    return false;
  }
  return Layer::isEqual( l->info_ );
}

bool Layer::isEqual( const proto::WeatherLayer& info ) const
{
  proto::WeatherLayer i1; i1.CopyFrom(info_);
  proto::WeatherLayer i2; i2.CopyFrom(info);
  i1.clear_data_size();
  i1.clear_center_name();
  i2.clear_data_size();
  i2.clear_center_name();
  if ( false == i1.has_level() ) {
    i1.set_level(0);
  }
  if ( false == i1.has_type_level() ) {
    i1.set_type_level(0);
  }
  if ( false == i2.has_level() ) {
    i2.set_level(0);
  }
  if ( false == i2.has_type_level() ) {
    i2.set_type_level(0);
  }
  if ( false == i2.has_net_type() ) {
    i2.set_net_type(0);
  }
  if ( false == i1.has_net_type() ) {
    i1.set_net_type(0);
  }
  i1.clear_meteo_descr();
  i2.clear_meteo_descr();

  return global::isEqual( i1, i2 );
}

bool Layer::isEqual( const field::DataDesc& info ) const
{
  if ( nullptr == field_ ) {
    return false;
  }
  QString dt1 = QString::fromStdString( info.date() );
  QString dt2 = field_->getDate().toString(Qt::ISODate);
  dt1.replace('Z',"");
  dt2.replace('Z',"");
  //  debug_log <<
  //   dt1                                      << dt2                       << "\n"
  //    << info.meteodescr()                    << field_->getDescr() << "\n"
  //    << info.hour()                          << field_->getHour() << "\n"
  //    << info.model()                         << field_->getModel() << "\n"
  //    << info.center()                        << field_->getCenter() << "\n"
  //    << info.level()                         << field_->getLevel() << "\n"
  //    << info.level_type()                    << field_->getLevelType() << "\n"
  //    << info.net_type()                      << field_->typeNet();
  if ( dt1                                  != dt2
       || info.meteodescr()                    != field_->getDescr()
       || info.hour()                          != field_->getHour()
       || info.model()                         != field_->getModel()
       || info.center()                        != field_->getCenter()
       || info.level()                         != field_->getLevel()
       || info.level_type()                    != field_->getLevelType()
       || info.net_type()                      != field_->typeNet()
       ) {
    return false;
  }
  return true;
}

void Layer::addObject( Object* o )
{
  if ( nullptr == o ) {
    error_log << QObject::tr("Нулевой указатель");
    return;
  }

 /* if ( 0 != objects_.count(o)){
    return;
  }*/
  objects_.insert(o);
}

void Layer::rmObject( Object* o )
{
  if ( nullptr == o ) {
    error_log << QObject::tr("Нулевой указатель");
    return;
  }

 /*
  if ( false == objects_.contains(o) ) {
    return;
  }*/
  objects_.erase(o);
//  objects_.removeAll(o);
}

int32_t Layer::dataSize() const
{
  int32_t layersz = 0;
  layersz += sizeof(int32_t);           //размер типа слоя
  QByteArray loc = uuid_.toUtf8();
  int32_t sz = loc.size();
  layersz += sizeof(sz);
  layersz += sz;
  loc = name_.toUtf8();
  sz = loc.size();
  layersz += sizeof(sz);
  layersz += sz;
  layersz += sizeof(visible_);

  layersz += sizeof(int32_t);           //размер размера field
  if ( nullptr != field_ ) {
    layersz += field_->sizeofBuffer();
  }
  loc = shortName_.toUtf8();
  sz = loc.size();
  layersz += sizeof(sz);
  layersz += sz;
  layersz += sizeof(isbase_);
  loc.clear();
  QDataStream stream( &loc, QIODevice::WriteOnly );
  stream << property_;
  layersz += sizeof(sz);
  layersz += loc.size();
  layersz += sizeof(alphapercent_);
  std::string str;
  info_.SerializeToString(&str);
  sz = str.size();
  layersz += sizeof(sz);
  layersz += sz;

  layersz += sizeof(int32_t); //размер кастом-типа

  return layersz;
}

int32_t Layer::serializeToArray( char* arr ) const
{
  int32_t t = type();
  int32_t pos = 0;
  ::memcpy( arr + pos, &t, sizeof(t) );
  pos += sizeof(t);

  QByteArray loc = uuid_.toUtf8();
  int32_t sz = loc.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, loc.data(), sz );
  pos += sz;

  loc = name_.toUtf8();
  sz = loc.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, loc.data(), sz );
  pos += sz;

  ::memcpy( arr + pos, &visible_, sizeof(visible_) );
  pos += sizeof(visible_);

  if ( nullptr == field_ ) {
    sz = 0;
    ::memcpy( arr + pos, &sz, sizeof(sz) );
    pos += sizeof(sz);
  }
  else {
    //    sz = field_->sizeofBuffer();
    //    ::memcpy( arr + pos, &sz, sizeof(sz) );
    //    pos += sizeof(sz);
    //    loc = QByteArray( sz, '\0' );
    loc.clear();
    field_->getBuffer(&loc);
    sz = loc.size();
    ::memcpy( arr + pos, &sz, sizeof(sz) );
    pos += sizeof(sz);
    ::memcpy( arr + pos, loc.data(), sz );
    //    pos += sz;
    pos += field_->sizeofBuffer();
  }
  loc = shortName_.toUtf8();
  sz = loc.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, loc.data(), sz );
  pos += sz;

  ::memcpy( arr + pos, &isbase_, sizeof(isbase_) );
  pos += sizeof(isbase_);

  loc.clear();
  QDataStream stream( &loc, QIODevice::WriteOnly );
  stream << property_;
  sz = loc.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, loc.data(), sz );
  pos += sz;

  ::memcpy( arr + pos, &alphapercent_, sizeof(alphapercent_) );
  pos += sizeof(alphapercent_);

  std::string str;
  info_.SerializeToString(&str);
  sz = str.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, str.data(), sz );
  pos += sz;
  t = customtype_;
  ::memcpy( arr + pos, &t, sizeof(t) );
  pos += sizeof(t);
  return pos;
}

int32_t Layer::parseFromArray( const char* arr )
{
  int32_t t;
  int32_t pos = 0;
  global::fromByteArray( arr + pos, &t  );
  if ( type() != t ) {
    error_log << QObject::tr("Не удалось открыть документ");
    return -1;
  }
  pos += sizeof(t);
  int32_t sz;
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  uuid_ = QString::fromUtf8( arr + pos, sz );
  pos += sz;
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  name_ = QString::fromUtf8( arr + pos, sz );
  pos += sz;
  global::fromByteArray( arr + pos, &visible_ );
  pos += sizeof(visible_);

  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);

  if ( 0 != sz ) {
    QByteArray loc( arr + pos, sz );
    //    sz = field_->sizeofBuffer();
    //    ::memcpy( &sz, arr + pos, sizeof(sz) );
    delete field_;
    field_ = new obanal::TField;
    field_->fromBuffer( &loc );
    //    pos += sz;
    pos += field_->sizeofBuffer();
  }
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  shortName_ = QString::fromUtf8( arr + pos, sz );
  pos += sz;

  global::fromByteArray( arr + pos, &isbase_ );
  pos += sizeof(isbase_);

  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  if ( 0 != sz ) {
    QByteArray loc( arr + pos, sz );
    QDataStream stream( &loc, QIODevice::ReadOnly );
    stream >> property_;
  }
  pos += sz;

  global::fromByteArray( arr + pos, &alphapercent_ );
  pos += sizeof(alphapercent_);

  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  info_.ParseFromArray( arr + pos, sz );
  pos += sz;
  global::fromByteArray( arr + pos, &t  );
  pos += sizeof(t);
  customtype_ = t;
  return pos;
}

int32_t Layer::childDataSize() const
{
  int32_t sz = objects_.size();
  int32_t childsize = sizeof(sz);

  foreach(Object* o, objects_){
  //for ( int i = 0; i < sz; ++i ) {
 //   Object* o = objects_[i];
    childsize += o->dataSize();
    childsize += o->childDataSize();
  }
  return childsize;
}

int32_t Layer::serializeChildsToArray( char* arr ) const
{
  int32_t sz = objects_.size();
  int32_t pos = 0;
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  foreach(Object* o, objects_){
//  for ( int i = 0, osz = objects_.size(); i < osz; ++i ) {
//    Object* o = objects_[i];
    pos += o->serializeToArray( arr + pos );
    pos += o->serializeChildsToArray( arr + pos );
  }
  return pos;
}

int32_t Layer::parseChildsFromArray( const char* arr )
{
  int32_t sz;
  int32_t pos = 0;
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  for ( int i = 0; i < sz; ++i ) {
    int32_t t;
    global::fromByteArray( arr + pos, &t );
    Object* o = singleton::PtkppFormat::instance()->createObject( t, this );
    if ( nullptr == o ) {
      error_log << QObject::tr("Не удалось открыть документ. Не создан объект типа = %1").arg(t);
      return -1;
    }
    pos += o->parseFromArray( arr + pos );
    pos += o->parseChildsFromArray( arr + pos );
  }
  return pos;
}

LayerMenu* Layer::layerMenu()
{
  if ( nullptr == menu_ ) {
    menu_ = new LayerMenu(this);
  }
  return menu_;
}

void Layer::setCache( bool fl )
{
  if ( cache_ == fl ) {
    return;
  }
  cache_ = fl;
  if ( true == cache_ ) {
    document_->moveCachedLayer(this);
  }
}

bool Layer::hasData() const {
  return (noData != layer_data_type_);
}



}
}
