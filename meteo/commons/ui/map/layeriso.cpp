#include "layeriso.h"

#include <qdebug.h>
#include <quuid.h>
#include <qpainter.h>
#include <qelapsedtimer.h>

#include <sql/psql/psqlquery.h>
#include <meteo/commons/global/dateformat.h>
#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/projection.h>
#include <commons/obanal/tfield.h>
#include <commons/obanal/tisolinedata.h>
#include <meteo/commons/global/global.h>

#include "map.h"
#include "document.h"
#include "object.h"
#include "isoline.h"
#include "geogradient.h"
#include "geotext.h"
#include "event.h"
#include <meteo/commons/global/weatherloader.h>
#include "puanson.h"
#include <meteo/commons/global/gradientparams.h>
#include "layermenu.h"

namespace meteo {
namespace map {

namespace {
  Layer* createLayer( Document* d )
  {
    return new LayerIso(d);
  }
  static const bool res = singleton::PtkppFormat::instance()->registerLayerHandler( LayerIso::Type, createLayer );
}
#define start_time QElapsedTimer _timer__name; _timer__name.start()
#define reset_time(text) debug_log << text << _timer__name.restart() << "msec"

LayerIso::LayerIso ( Document* d, const QString& n )
  : Layer ( d, n )
{
  info_.set_type(kLayerIso);
}

LayerIso::LayerIso( Document* map, const proto::FieldColor& clr, obanal::TField* f )
  : Layer(map),
  fieldcolor_(clr)
{
  info_.set_type(kLayerIso);
  setField(f);
}

LayerIso::~LayerIso()
{
}

//проверяет, содержит ли "a" хотя бы один из объектов из "lb"
bool LayerIso::containGeo( meteo::map::Object * a){
  QVector <QPolygon > scrA;
  if(nullptr == projection() || !projection()->F2X(a->skelet(),&scrA,true)) return false;
  if(scrA.size() < 1) return false;
  foreach(Object* o, objects_){

 // for ( int i = 0; i <kol_obj; ++i ){
    if(  a == o ) {
      continue;
    }
    const GeoVector& gv = o->skelet();
    if ( 0 == gv.size() ) {
      continue;
    }
    if( false == projection()->isInMap(gv.first())) continue;
    QPoint scrB;
    if(nullptr == projection() || !projection()->F2X_one(o->skelet().first(),&scrB)) return false;

    if(true == scrA.at(0).containsPoint(scrB,Qt::OddEvenFill)){
      return true;
    }
  }
  return false;
}

LayerMenu* LayerIso::layerMenu()
{
  if ( nullptr == menu_ ) {
    menu_ = new IsoMenu(this);
  }
  return menu_;
}

void LayerIso::incGradientCount()
{
  ++gradientcount_;
}

void LayerIso::decGradientCount()
{
  if ( 0 == gradientcount_ ) {
    warning_log << QObject::tr("Количество градиентов не может быть меньше нуля");
    return;
  }
  --gradientcount_;
}

void LayerIso::gruppingIsolines(){
  QList< IsoLine* > list_poly;
  foreach(Object* o, objects_){
    IsoLine* gp = mapobject_cast<IsoLine*>(o);
    if(nullptr == gp ) continue;

    if(!containGeo(gp)){
      list_poly.append(gp);
    }
  }
  QList< IsoLine* >::iterator it = list_poly.begin();
  while (it != list_poly.end()) {
    GeoGroup *gr = new GeoGroup(this);
    gr->add(*it);
    gr_list.append(gr);
    ++it;
  }
}

void LayerIso::render( QPainter* painter, const QRect& target, Document* document )
{
  if ( false == visible() ) {
    return;
  }
  Document* olddoc = document_;
  document_ = document;
  drawedlabels_.clear();
  std::unordered_set<GeoGradient*> grad = objectsByType<GeoGradient*>(true);
  foreach(Object* o, grad){
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
  std::unordered_set<IsoLine*> iso = objectsByType<IsoLine*>(true);

  foreach(Object* o, iso){
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
  foreach(Object* o, objects_){
    if ( nullptr != mapobject_cast<GeoGradient*>(o) || 0 != mapobject_cast<IsoLine*>(o) ) {
      continue;
    }
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

int LayerIso::addIsoLines( bool* ok )
{
  if ( nullptr != ok ) {
    *ok = false;
  }
  if ( nullptr == field_ ) {
    return 0;
  }
  if ( false == fieldcolor_.has_descr() ) {
    GradientParams params( meteo::global::kIsoParamPath() );
    const proto::FieldColor& clr = params.protoParams( field_->getDescr() );
    return addIsoLines( clr, ok );
  }
  return addIsoLines( fieldcolor_, ok );
}

int LayerIso::addIsoLines( const proto::FieldColor& color, bool* ok )
{
  if ( nullptr != ok ) {
    *ok = false;
  }
  if ( nullptr == field_ ) {
    return 0;
  }
  if ( &color != &fieldcolor_ ) {
    fieldcolor_.CopyFrom(color);
  }
  proto::LevelColor lvlclr = GradientParams::levelProto( field_->getLevel(), field_->getLevelType(), fieldcolor_ );
  if ( nullptr != ok ) {
    *ok = true;
  }
  return addIsoLines(lvlclr);
}

int LayerIso::addIsoLines( const proto::LevelColor& color )
{
  float step = color.step_iso();
  const proto::ColorGrad& grad = color.color();
  float min = grad.min_value();
  float max = grad.max_value();
  for ( int i = 0, sz = fieldcolor_.level_size(); i < sz; ++i ) {
    if ( color.level() == fieldcolor_.level(i).level() && color.type_level() == fieldcolor_.level(i).type_level() ) {
      fieldcolor_.mutable_level(i)->CopyFrom(color);
      return addIsoLines( min, max, step );
    }
  }
  proto::LevelColor* newlvl = fieldcolor_.add_level();
  newlvl->CopyFrom(color);
  return addIsoLines( min, max, step );
}

int LayerIso::addIsoLines ( float min, float max, float step ) {
  bool autostep = levelColor().autostep();
  QVector<meteo::GeoVector> isolines;
  int iso_count = 0;
  start_time;
  TIsoLineData iso_iso(field_);
  bool checkauto = false;
  for ( float i = min; i <= max; i+=step ) {
    isolines.clear();
    iso_iso.makeOne(i,&isolines);
    for ( int j = 0; j < isolines.count(); j++ ) {
      IsoLine* iso = new IsoLine( this, fieldcolor_ );
      iso->setSplineFactor(10.);
      iso->setPen(fieldcolor_.pen());
      iso->setBrush(Qt::NoBrush);
      iso->setSkelet( isolines.at(j) );
      iso->setValue( i, QString::fromStdString( fieldcolor_.format() ) );
 //     if(isolines.at(j).first().compareLatLon(isolines.at(j).last())){
 //         iso->setClosed(true);
 //       }

      if ( true == autostep ) {
        if ( true == checkauto ) {
          Generalization g = iso->general();
          g.setHighLimit(14);
          iso->setGeneral(g);
        }
      }
    }
    checkauto = !checkauto;
    iso_count += isolines.count();
  }
  return iso_count;
}

bool LayerIso::addGradient()
{
  if ( nullptr == field_ ) {
    return false;
  }
  GeoGradient* gr = new GeoGradient(this);
  Q_UNUSED(gr);
  return true;
}

bool LayerIso::addGradient( const proto::FieldColor& clr )
{
  if ( nullptr == field_ ) {
    return false;
  }
  GeoGradient* gr = new GeoGradient(this, clr);
  Q_UNUSED(gr);
  return true;
}

void LayerIso::removeGradient()
{
  std::unordered_set<GeoGradient*> list = Layer::objectsByType<GeoGradient*>(true);
  foreach(Object* o, list){
    delete o;
  }
}

void LayerIso::removeIsolines()
{
  std::unordered_set<Puanson*> list = Layer::objectsByType<Puanson*>(true);
  foreach(Object* o, list){
    delete o;
  }
  std::unordered_set<IsoLine*> list2 = Layer::objectsByType<IsoLine*>(true);
  foreach(Object* o, list2){
    delete o;
  }
}

void LayerIso::setFieldColor( const proto::FieldColor& fc )
{
  fieldcolor_ = fc;
  setInfo( info(this) );
  updateFieldColor();
}

void LayerIso::setPen( const QPen& pen )
{
  fieldcolor_.mutable_pen()->CopyFrom( qpen2pen(pen) );
  updateFieldColor();
}

bool LayerIso::hasGradient() const
{
  if ( 0 < gradientcount_ ) {
    return true;
  }
  return false;
}

bool LayerIso::hasIsoline() const
{
  foreach(Object* o, objects_){
    if ( nullptr != mapobject_cast<IsoLine*>(o) ) {
      return true;
    }
    else if ( nullptr != mapobject_cast<GeoGroup*>(o) ) {
      GeoGroup* gr = mapobject_cast<GeoGroup*>(o);
      foreach(Object* childs, gr->childs()){
        if ( nullptr != mapobject_cast<IsoLine*>(childs) ) {
          return true;
        }
      }
    }
  }
  return false;
}

bool LayerIso::gradientVisible() const
{
  std::unordered_set<GeoGradient*> list = objectsByType<GeoGradient*>();
  for ( auto g: list ) {
    if ( true == g->visible() ) {
      return true;
    }
  }
  return false;
}

void LayerIso::showGradient()
{
  std::unordered_set<GeoGradient*> list = objectsByType<GeoGradient*>();
  for ( auto g: list ) {
    if ( false == g->visible() ) {
      g->setVisible(true);
      incGradientCount();
    }
  }
}

void LayerIso::hideGradient()
{
  std::unordered_set<GeoGradient*> list = objectsByType<GeoGradient*>();
  for ( auto g: list ) {
    if ( true == g->visible() ) {
      g->setVisible(false);
      decGradientCount();
    }
  }
}

int LayerIso::level() const
{
  if ( nullptr == field_ ) {
    return -1;
  }
  return field_->getLevel();
}

int LayerIso::typeLevel() const
{
  if ( nullptr == field_ ) {
    return -1;
  }
  return field_->getLevelType();
}

int LayerIso::descriptor() const
{
  if ( nullptr == field_ ) {
    return -1;
  }
  return field_->getDescr();
}

proto::LevelColor LayerIso::levelColor() const
{
  proto::LevelColor lvlclr;
  if ( nullptr == field_ ) {
    return lvlclr;
  }
  return GradientParams::levelProto( field_->getLevel(), field_->getLevelType(), fieldcolor_ );
}

QPen LayerIso::pen() const
{
  return pen2qpen( fieldcolor_.pen() );
}

float LayerIso::minValue() const
{
  float min = 0.0;
  if ( nullptr == field_ ) {
    return min;
  }
  return GradientParams::isoMin( field_->getLevel(), field_->getLevelType(), fieldcolor_ );
}

float LayerIso::maxValue() const
{
  float max = 0.0;
  if ( nullptr == field_ ) {
    return max;
  }
  return GradientParams::isoMax( field_->getLevel(), field_->getLevelType(), fieldcolor_ );
}

QColor LayerIso::minColor() const
{
  QColor clr;
  if ( nullptr == field_ ) {
    return clr;
  }
  return GradientParams::isoColorMin( field_->getLevel(), field_->getLevelType(), fieldcolor_ );
}

QColor LayerIso::maxColor() const
{
  QColor clr;
  if ( nullptr == field_ ) {
    return clr;
  }
  return GradientParams::isoColorMax( field_->getLevel(), field_->getLevelType(), fieldcolor_ );
}

TColorGradList LayerIso::gradient() const
{
  if ( nullptr == field_ ) {
    return TColorGradList();
  }
  return GradientParams::gradParams( field_->getLevel(), field_->getLevelType(), fieldcolor_ );
}

float LayerIso::stepIso() const
{
  return GradientParams::isoStep( field_->getLevel(), field_->getLevelType(), fieldcolor_ );
}

void LayerIso::setColor( const TColorGrad& grad )
{
  if ( nullptr == field_ ) {
    return;
  }
  GradientParams::setIsoColor( field_->getLevel(), field_->getLevelType(), grad, &fieldcolor_ );
  updateFieldColor();
}

void LayerIso::setColor( const TColorGradList& gradlist )
{
  if ( nullptr == field_ ) {
    return;
  }
  GradientParams::setGradColor( field_->getLevel(), field_->getLevelType(), gradlist, &fieldcolor_ );
  updateFieldColor();
}

void LayerIso::setStepIso( float step )
{
  if ( nullptr == field_ ) {
    return;
  }
  GradientParams::setIsoStep( field_->getLevel(), field_->getLevelType(), step, &fieldcolor_ );
  updateFieldColor();
}

void LayerIso::setMin( float min )
{
  if ( nullptr == field_ ) {
    return;
  }
  GradientParams::setIsoMin( field_->getLevel(), field_->getLevelType(), min, &fieldcolor_ );
  updateFieldColor();
}

void LayerIso::setMax( float max )
{
  if ( nullptr == field_ ) {
    return;
  }
  GradientParams::setIsoMax( field_->getLevel(), field_->getLevelType(), max, &fieldcolor_ );
  updateFieldColor();
}

void LayerIso::setColorMin( const QColor& clr )
{
  if ( nullptr == field_ ) {
    return;
  }
  GradientParams::setIsoColorMin( field_->getLevel(), field_->getLevelType(), clr, &fieldcolor_ );
  updateFieldColor();
}

void LayerIso::setColorMax( const QColor& clr )
{
  if ( nullptr == field_ ) {
    return;
  }
  GradientParams::setIsoColorMax( field_->getLevel(), field_->getLevelType(), clr, &fieldcolor_ );
  updateFieldColor();
}

void LayerIso::updateFieldColor()
{
  foreach(Object* o, objects_){
    IsoLine* iso = mapobject_cast<IsoLine*>(o);
    if ( nullptr != iso ) {
      iso->setFieldColor(fieldcolor_);
      continue;
    }
    GeoGroup* gr = mapobject_cast<GeoGroup*>(o);
    if ( nullptr != gr ) {
      foreach(Object* childs, gr->childs()){
        iso = mapobject_cast<IsoLine*>( childs );
        if ( nullptr == iso ) {
          continue;
        }
        iso->setFieldColor(fieldcolor_);
      }
    }
    GeoGradient* grad = mapobject_cast<GeoGradient*>(o);
    if (nullptr != grad) {
      grad->setFieldColor(fieldcolor_);
      continue;
    }
    Puanson* puan = mapobject_cast<Puanson*>(o);
    if ( nullptr != puan ) {
      const TMeteoData& md = puan->meteodata();
      QStringList allnames = md.allNames();
      if ( 0 == allnames.size() ) {
        continue;
      }
      TColorGrad grad = GradientParams::isoParams( level(), typeLevel(), fieldcolor_ );
      const TMeteoParam& param = md.meteoParam(allnames.at(0));
      float val = param.value();
      puanson::proto::Puanson punch = puan->punch();
      bool ok = false;
      QColor clr = grad.color( val, &ok );
      if ( ( grad.begcolor() != Qt::black || grad.endcolor() != Qt::black ) && true == ok ) {
        punch.set_color( clr.rgba() );
        puan->setPunch(punch);
      }
      else {
        punch.clear_color();
        puan->setPunch(punch);
      }
      continue;
    }
  }
}



int LayerIso::addExtremums()
{
 // QTime ttt; ttt.start();

  int descr = descriptor();
  if ( 10009 != descr
    && 10051 != descr
    && 10061 != descr ) {
    return 0;
  }
  TColorGrad grad  = GradientParams::isoParams( level(), typeLevel(), fieldcolor_ );
  gruppingIsolines();
 // return 1;
  extremums_.clear();
  QVector<fieldExtremum> extremums = field_->calcExtremum();

 // debug_log<<"extremums1 calc " << ttt.elapsed()<<extremums.size();ttt.restart();

  int kol_extr = extremums.size();
  meteo::GeoVector gv ( 1 );
  int kol_centers = 0;
  QList< GeoGroup * >::iterator git = gr_list.begin();
  QList< GeoGroup * >::iterator gitn = gr_list.end();
  for(; git != gitn; ++git) {
    QVector <QPolygon > scrA;
    if(nullptr == projection() ||
       !projection()->F2X((*(*git)->childs().begin())->skelet(),&scrA,true)) continue;

    for ( int i = 0; i< kol_extr; ++i ) {
      meteo::GeoPoint egp = extremums.at ( i ).koord;
      if(!projection()->isInMap(egp)) continue;
      QPoint ep;

      if(nullptr == projection() || !projection()->F2X_one(egp,&ep)){
        continue;
      }
     for(int m=0;m<scrA.size();++m){
      if(true == scrA.at(m).containsPoint(ep,Qt::OddEvenFill) ){
          int tip = extremums.at ( i ).tip;
          float value = extremums.at ( i ).znach;
          std::unordered_set<Object*> objl =  (*git)->objectsByType(kPuanson);
          if(0  < objl.size()  ){
            float vs_val = (*objl.begin())->property("extrem").toFloat();
            int  vs_tip = (*objl.begin())->property("zn").toInt();
           if(vs_tip != tip) break;
            if(tip == PLUS_LAPLA && vs_val > value){
              continue;
            }
            if(tip == MINUS_LAPLA && vs_val < value){
              continue;
            }
          qDeleteAll(objl);
          }
          extremums_.append(extremums.at ( i ));
          QMap< QString, meteo::puanson::proto::Puanson > map = WeatherLoader::instance()->punchlibraryspecial();
          gv[0] = egp;
          bool has_catch_10061 = false;
          if ( 10061 == field()->getDescr() ) {
            if ( 0.1f > ::fabsf(value)  ) {
              continue;
            }
            else if ( 0 > value && MINUS_LAPLA == tip ) {
              tip = PLUS_LAPLA;
              has_catch_10061 = true;
            }
            else if ( 0 < value && PLUS_LAPLA == tip ) {
              tip = MINUS_LAPLA;
              has_catch_10061 = true;
            }
          }
          Puanson* puan = new Puanson(this);
          puanson::proto::Puanson punch = map["P_ex"];

          /*bool ok = false;
          QColor clr = grad.color( value, &ok );
          if ( ( grad.begcolor() != Qt::black || grad.endcolor() != Qt::black ) && true == ok ) {
            punch.set_color( clr.rgba() );
          }
          else {
            punch.clear_color();
          }*/
          puan->setPunch(punch);
          TMeteoParam param("", value, control::RIGHT);
          if ( 10061 == field()->getDescr() ) {
              puan->setParamValue("WW1", param);
            }

          puan->setSkelet(gv);
          setEtrLabel(tip,&param);

          puan->setParamValue("zn", param);
          if ( true == has_catch_10061 ) {
            if ( MINUS_LAPLA == tip ) {
              tip = PLUS_LAPLA;
            }
            else if ( PLUS_LAPLA == tip ) {
              tip = MINUS_LAPLA;
            }
          }
          puan->setProperty("zn", QString::number(tip));
          puan->setProperty("extrem",QString::number(value));
  //        puan->setDrawAlways(true);
          (*git)->add(puan);
          ++kol_centers;
          //break;

        }
     }

    }

  }

  for ( int i = 0, sz = gr_list.size(); i < sz; ++i ) {
    GeoGroup* o = gr_list.at(i);
    o->ungroup();
    delete o;
  }
  gr_list.clear();

  return kol_extr;
}

void LayerIso::setField( obanal::TField* f )
{
  Layer::setField(f);
  setInfo( info(this) );
  if(nullptr != f && 0 < f->kolData()) {
      layer_data_type_ = fieldData;
    }
}

valLabel LayerIso::valueLabel() const
{
  valLabel sss;
  //FIXME switch(info_.type_level()){
  switch(typeLevel()){

    case 1:
      if ( 22021 != descriptor() && 22043 != descriptor() ) {
        sss.level = QObject::tr("у земли");
      }
      else {
        sss.level = "";
      }
      break;
    case 100:
      sss.level = QObject::tr("%1 мбар").arg( info_.level() );
      break;
    case 102:
      sss.level = QObject::tr("уровень моря");
      break;
    case 160:
      sss.level = QObject::tr("дно");
      break;
    default:
      break;
  }

 /* if(fieldcolor_.has_name()){
   sss = QString::fromStdString(fieldcolor_.name()) + lvlstr;
  } else {*/
  sss.name = TMeteoDescriptor::instance()->property(descriptor()).description;
  sss.unit = TMeteoDescriptor::instance()->property(descriptor()).unitsRu;
  if(nullptr != field()){
   sss.date = meteo::dateToHumanTimeShort(field()->getDate());
  }
  if(info_.has_center_name()){

    sss.center  =  QString::fromStdString(info_.center_name());
  }

  if(info_.has_hour() && 0 != info_.hour() ){
    sss.hour.setNum(info_.hour());
  }

  return sss;
}

proto::WeatherLayer LayerIso::info( const LayerIso* l )
{
  proto::WeatherLayer i;
  if ( nullptr == l ) {
    return i;
  }
  if ( nullptr == l->field_ ) {
    return i;
  }
  i.set_type( kLayerIso );
  i.set_source( proto::kField );
  if ( true == l->hasIsoline() && true == l->hasGradient() ) {
    i.set_mode( proto::kIsoGrad );
  }
  else if ( true == l->hasIsoline() ) {
    i.set_mode( proto::kIsoline );
  }
  else if ( true == l->hasIsoline() ) {
    i.set_mode( proto::kGradient );
  }
  else {
    i.set_mode( proto::kIsoline );
  }
  i.set_center( l->field_->getCenter() );
  i.set_model( l->field_->getModel() );
  i.set_level( l->field_->getLevel() );
  i.set_type_level( l->field_->getLevelType() );
  i.set_template_name( QString::number( l->fieldcolor_.descr() ).toStdString() );
  i.set_datetime( l->field_->getDate().toString(Qt::ISODate).toStdString() );
  i.set_hour( l->field_->getHour()/3600 );
  i.set_center_name( l->field_->getCenterName().toStdString() );
  i.set_data_size( l->field_->getSrcPointCount() );
  i.set_net_type(l->field_->typeNet());
  i.add_meteo_descr(l->field_->getDescr());
  return i;
}

void LayerIso::setEtrLabel(int tip,TMeteoParam* param ){

  int param_val = field()->getDescr()+field()->getLevelType()*100000;
  if(15000==field()->getLevelType()){
    param_val = field()->getDescr()+field()->getLevelType()*1000;
  }
  if (MINUS_LAPLA == tip){
      param_val *=-1;
  }
  param->setValue(param_val);
//debug_log<< param_val;
  return;

}

int32_t LayerIso::dataSize() const
{
  int32_t pos = Layer::dataSize();
  std::string str;
  if ( true == fieldcolor_.IsInitialized() ) {
    fieldcolor_.SerializeToString( &str );
  }
  int32_t sz = str.size();
  pos += sizeof(sz);
  pos += sz;
  return pos;
}

int32_t LayerIso::serializeToArray( char* arr ) const
{
  int32_t pos = Layer::serializeToArray(arr);
  std::string str;
  if ( true == fieldcolor_.IsInitialized() ) {
    fieldcolor_.SerializeToString( &str );
  }
  int32_t sz = str.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, str.data(), sz );
  pos += sz;
  return pos;
}

int32_t LayerIso::parseFromArray( const char* arr )
{
  int32_t pos = Layer::parseFromArray(arr);
  int32_t sz;
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  if ( 0 != sz ) {
    fieldcolor_.ParseFromArray( arr + pos, sz );
  }
  pos += sz;
  return pos;
}

}
}
