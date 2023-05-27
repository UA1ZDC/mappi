#include "vprofiledoc.h"

#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/generalproj.h>
#include <commons/geobasis/geopoint.h>
#include <commons/obanal/tfield.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/global/dateformat.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/zond/diagn_func.h>
#include <meteo/commons/ui/map/event.h>
#include <meteo/commons/ui/map/geoaxis.h>
#include <meteo/commons/ui/map/geogroup.h>
#include <meteo/commons/ui/map/geopixmap.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/global/gradientparams.h>
#include <meteo/commons/ui/map/profile/layerprofile.h>
#include <meteo/commons/ui/map/isoline.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/layeriso.h>
#include <meteo/commons/ui/map/layervprofile.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/ui/map/ramka.h>
#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/ui/map/view/actions/infotextaction.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/ui/map/axis/ramkaaxis.h>
#include <meteo/commons/ui/map/axis/layeraxis.h>
#include <meteo/commons/ui/map/axis/axissimple.h>


// минимальное расстояние между станциями, при котором будет добавляться новый профиль
const float MIN_DIST_STATION = 10.;
static const float kZoom = 0.5;

static const float kBegFi = 0.0;
static const float kEndFi = 799.999; // делаем немного меньше, чем kBlankWidth, чтобы правая ось с пуансонами не пропадала на границе документа
static const float kBegLa = 1000.0;
static const float kEndLa = 100.0;
static float maxH = 31000.0;

static QVector<float> mkSequence(float beg, float end, float step)
{
  QVector<float> vec;
  for ( float v = beg; v <= end; v += step ) {
    vec << v;
  }
  return vec;
}

namespace meteo {
namespace map {

const int VProfileDoc::kBlankWidth = 800;

const QVector<float> VProfileDoc::kIsoLevels = QVector<float>()
      << 200 << 250 << 300 << 400 << 500 << 600 << 700 << 850 << 925 << 950 << 1000;


VProfileDoc::VProfileDoc( const GeoPoint& mapcenter,
          const GeoPoint& doc_center,
          proto::DocumentType type,
          ProjectionType proj_type )
  : Document( mapcenter, doc_center, type, proj_type )
{
}

VProfileDoc::VProfileDoc( const proto::Document& doc )
  : Document( doc )
{
}

VProfileDoc::VProfileDoc()
  : Document()
{
}

VProfileDoc::~VProfileDoc()
{
}

bool VProfileDoc::init()
{
  clearDocument();
  layerProfiles_ = nullptr;

  ProjType proj_type = GENERAL;
  GeoPoint mc( 500, 400, 0.0, LA_GENERAL );
  projection_ = Projection::createProjection( proj_type, mc );
  if ( nullptr == projection_ ) {
    error_log << QObject::tr("Не удалось создать проекцию с типом = %1")
      .arg(proj_type);
    return false;
  }

  backgroundColor_ = Qt::white;

  GeoPoint sc = mc;
  if ( true == property_.has_doc_center() ) {
    sc = pbgeopoint2geopoint( property_.doc_center() );
  }
  setScreenCenter(sc);
  if ( false == property_.has_mapsize() ) {
    property_.mutable_mapsize()->CopyFrom( qsize2size( QSize( 3000, 3000) ) );
  }
  if ( false == property_.has_docsize() ) {
    property_.mutable_docsize()->CopyFrom( property_.mapsize() );
  }
  float Yfactor = 0.4;
  GeoPoint dc = GeoPoint( 500, 400, 0.0, LA_GENERAL );
  GeneralProj* proj = projection_cast<meteo::GeneralProj*>( Document::projection() );
  if ( nullptr == proj ) {
    return false;
  }
  property_.set_geoloader("");
  proj->setMapCenter(dc);
  GeoPoint start = GeoPoint( kBegFi, kBegLa, 0, LA_GENERAL );
  GeoPoint end = GeoPoint( kEndFi, kEndLa, 0, LA_GENERAL );
//  GeoPoint center = GeoPoint( ( start.lat() + end.lat() )/2.0, ( start.lon() + end.lon() )/2.0*(Yfactor), 0, LA_GENERAL );
  float h;
  float t;
  float p = end.la();
  zond::SA81_H_ext( p, &t, &h );
  maxH = h;
  proj->setStart(start);
  proj->setEnd(end);
  proj->setYfactor(Yfactor);

  proj->setFuncTransform( cutLinearF2X_one, cutLinearX2F_one);
  proj->setFuncTransform( cutLinearF2X_onef, cutLinearX2F_onef);
  proj->setRamka( start.fi(), start.la(), end.fi(), end.la() );

  QString name = QObject::tr("Вертикальный разрез");
  Document::setName(name);
  Document::setScale(18.75);

  RamkaAxis* ramka = new RamkaAxis(this);
  Document::setRamka(ramka);

//  if ( true == WeatherLoader::instance()->punchlibraryspecial().contains("ddff") ) {
//    ddff_.CopyFrom(meteo::map::WeatherLoader::instance()->punchlibraryspecial()["ddff"]);
//  }
//  else {
//    warning_log << QObject::tr("Не найден знак ветра. Ветер не будет отображен в диаграмме");
//  }
  return true;
}

bool VProfileDoc::create(const QList<zond::PlaceData>& aprofilesData, int type)
{
  int count_profile = aprofilesData.size();
  if (count_profile == 0 ) {
    showNoData();
    return false;
  }

   profilesData_ = aprofilesData;

  if (type == kSpaceType ) {
    calcDistanceByCoord(kBlankWidth);
  }
  else {
    calcDistanceByDt(kBlankWidth);
    createAxis(type);
  }

  vcut_.setData(&profilesData_,kIsoLevels);

  addTemperature();
//  addNanoska();
  addTropo();
  addMaxWind();
  addH();
  addTemperatureR();

  addWind();
 // addWinddd();
//  addWindff();

  addObled();
  addOblak();
  addBoltan();
  addTrace();
  addOblakSloi();

  if ( nullptr == layerProfiles_ ) {
    QMap< QString, meteo::puanson::proto::Puanson > map = WeatherLoader::instance()->punchlibraryspecial();

    layerProfiles_ = new map::LayerProfile(this, QObject::tr("Профили"));
    layerProfiles_->setBase(true);
    layerProfiles_->setPuanson(map["vcutl_levels"]);

    map::AxisProfile* y1 = new map::AxisProfile(layerProfiles_, layerProfiles_->mapRect().size());
    y1->setAxisType(map::AxisProfile::kLeft);
    y1->setPuanson(map["vcutl_levels"]);
    y1->setRange(100,1000);
    y1->setTickVector(mkSequence(100,1000,100), 20);
    y1->setTickVector(mkSequence(100,1000, 50), 18);
    y1->setTickVector(mkSequence(100,1000, 10), 15);
    y1->setTickVector(mkSequence(100,1000,  5), 12);
    map::AxisProfile* y2 = new map::AxisProfile(layerProfiles_, layerProfiles_->mapRect().size());
    y2->setAxisType(map::AxisProfile::kRight);
    y2->setPuanson(map["vcutl_levels"]);
    y2->setRange(100,1000);
    y2->setTickVector(mkSequence(100,1000,100), 20);
    y2->setTickVector(mkSequence(100,1000, 50), 18);
    y2->setTickVector(mkSequence(100,1000, 10), 15);
    y2->setTickVector(mkSequence(100,1000,  5), 12);

    layerProfiles_->setAxisY(y1);
    layerProfiles_->setAxisY2(y2);

    layerProfiles_->xAxis()->setVisible(false);
    layerProfiles_->xAxis2()->setVisible(false);
  }

  // добавляем на слой профили для промежуточных пунктов
  for ( int i = 1, isz = profilesData_.size() - 1; i < isz; ++i ) {
    layerProfiles_->addProfile(profilesData_.at(i).xPos(), kIsoLevels);
  }

  LayerAxis* l = new LayerAxis(this, QObject::tr("Пункты"));
  l->setBase(true);
  l->yAxis()->setVisible(false);

  map::AxisSimple* x = l->xAxis();
  if (type == kSpaceType ) {
    x->setRange(0,800);

    QVector<QString> labels;
    QVector<float> ticks;
    for ( const zond::PlaceData& pd : profilesData_ ) {
      ticks << pd.xPos();
      labels << pd.name();

    }
    x->setSubTickCount(1);
    x->setTickVector(ticks);
    x->setTickLabelVector(labels);
  }
  else {
    quint32 beg = profilesData_.first().dt().toTime_t();
    quint32 end = profilesData_.last().dt().toTime_t();

    x->setSubTickCount(1);
    x->setRange(beg,end);
    x->setTickLabelType(map::AxisSimple::kltDateTime);
    x->setTickLabelFormat("hh:mm", 20);
    x->setTickLabelFormat("hh:mm", 17);
    x->setTickLabelFormat("hh:mm", 16);
    x->setTickLabelFormat("hh:mm", 14);

    x->setTickVector(mkSequence(beg, end, 10800), 20);
    x->setTickVector(mkSequence(beg, end,  3600), 17);
    x->setTickVector(mkSequence(beg, end,  1800), 16);
    x->setTickVector(mkSequence(beg, end,   900), 14);
  }

  return true;
}

void VProfileDoc::clear()
{
  vcut_.clear();
  showNoData();
}

bool VProfileDoc::createForBulletin(const QList<zond::PlaceData>& profiles, const proto::Map &mapProto)
{
  profilesData_ = profiles;
  const int padding = 30;
  if ( mapProto.profile() == meteo::map::proto::kBulletinSpace ) {
    calcDistanceByCoord(kBlankWidth);
  }
  else {
    calcDistanceByDt(kBlankWidth);
    createAxis(kTimeType);
  }
  vcut_.setData(&profilesData_, kIsoLevels);
  addObled();
  addOblak();
  addBoltan();
  prepLayer(CUT_LAYER_H, QString("Геопотенциальная высота"));
  addWind();
  isoLayers_[CUT_LAYER_dd]->setVisisble(false);
  isoLayers_[CUT_LAYER_ff]->setVisisble(false);
  isoLayers_[CUT_LAYER_uu]->setVisisble(false);
  isoLayers_[CUT_LAYER_vv]->setVisisble(false);
//  addH();
  addTropo();
  addTemperature();
  if ( profilesData_.size() > 0 &&
      MnMath::isEqual(kBlankWidth,profilesData_.last().xPos()) ) {
    profilesData_[profilesData_.size() - 1].setXPos(profilesData_[profilesData_.size() - 1].xPos() - 1);
  }

  // удаляем вспомогательные профили
  for ( int i=0; i<profilesData_.size(); ++i ) {
    if ( profilesData_[i].helpPoint() ) {
      profilesData_.removeAt(i);
      --i;
    }
  }

  //
  // шкала высот
  //

  prepLayerEasy(CUT_LAYER_H, QString("Высота полёта"));
//  prepLayer(CUT_LAYER_T, QString("Температура"));
//  fillLayer(isoLayers_[CUT_LAYER_T], -150, 80, 10, Qt::red);

  int maxH = 12000;
  for ( int i=0,isz=profilesData_.size(); i<isz; ++i ) {
    maxH = qMax(maxH, profilesData_[i].echelon());
  }
  maxH += ( maxH > 12000 ) ? 1000 : 0;

  float first = -padding;
  float last  = kBlankWidth + padding;

  for ( int h=1000; h<=maxH; h+=1000 ) {
    float y1 = getPByH(first, h);
    float y2 = getPByH(last, h);

    GeoVector line;
    line << GeoPoint(first, y1, 0, LA_GENERAL);
    line << GeoPoint(last, y2, 0, LA_GENERAL);

    map::GeoPolygon* heightObj = new map::GeoPolygon(dataLayerRef_[CUT_LAYER_H]);
    heightObj->setPenWidth(1);
    heightObj->setPenColor(QColor(150,150,150));
    heightObj->setSkelet(line);
  }

  //setRamka(paddings(), -1, maxH);

  obanal::TField* fieldH = nullptr;
  if ( nullptr != isoLayers_[CUT_LAYER_H] ) {
    fieldH = isoLayers_[CUT_LAYER_H]->field();
  }

  ScaleOptions lScale;
  lScale.tickSide = ScaleOptions::kLeftSide;
  lScale.showLabel = ScaleOptions::kLeftSide;
  fillScaleOptions(&lScale, fieldH, maxH, 0);
  if ( lScale.tickValues.size() != 0 ) {
    lScale.point = GeoPoint(-padding, lScale.tickValues.last(), 0, LA_GENERAL);
    lScale.length = lScale.tickValues.first() - lScale.tickValues.last();
  }

  ScaleOptions rScale;
  rScale.point.setLat(kBlankWidth);
  rScale.tickSide = ScaleOptions::kRightSide;
  rScale.showLabel = ScaleOptions::kRightSide;
  fillScaleOptions(&rScale, fieldH, maxH, kBlankWidth - 1);
  if ( rScale.tickValues.size() != 0 ) {
    rScale.point = GeoPoint(kBlankWidth + padding, rScale.tickValues.last(), 0, LA_GENERAL);
    rScale.length = rScale.tickValues.first() - rScale.tickValues.last();
  }

  Scale scale;

  scale.setOptions(lScale);
  scale.createObject(dataLayerRef_[CUT_LAYER_H]);

  scale.setOptions(rScale);
  scale.createObject(dataLayerRef_[CUT_LAYER_H]);

  if (mapProto.profile() != meteo::map::proto::kBulletinTime) {
    addEchelon();
  }
  // Профили
  if ( nullptr == layerProfiles_ ) {
    QMap< QString, meteo::puanson::proto::Puanson > map = WeatherLoader::instance()->punchlibraryspecial();

    layerProfiles_ = new map::LayerProfile(this, QObject::tr("Профили"));
    layerProfiles_->setBase(true);
    layerProfiles_->setPuanson(map["vcutl_levels"]);
    float angle = 0;
    int sz = profilesData_.size();
    map::AxisProfile* y1 = new map::AxisProfile(layerProfiles_, layerProfiles_->mapRect().size());
    if (mapProto.profile() != meteo::map::proto::kBulletinTime) {
      if ( sz >= 2 ) {
        angle = profilesData_.at(0).coord().radianAzimuth(profilesData_.at(1).coord()) * RAD2DEG + 90.f;
      }
      y1->setAngleWindCorrection(angle);
    }
    y1->setAxisType(map::AxisProfile::kLeft);
    y1->setPuanson(map["vcutl_levels"]);
    y1->setRange(100,1000);
    y1->setTickVector(mkSequence(100,1000,100), 20);
    y1->setTickVector(mkSequence(100,1000, 50), 18);
    y1->setTickVector(mkSequence(100,1000, 10), 15);
    y1->setTickVector(mkSequence(100,1000,  5), 12);
    map::AxisProfile* y2 = new map::AxisProfile(layerProfiles_, layerProfiles_->mapRect().size());
    if (mapProto.profile() != meteo::map::proto::kBulletinTime) {
      if ( sz >= 2 ) {
        angle = profilesData_.at(sz-2).coord().radianAzimuth(profilesData_.at(sz-1).coord()) * RAD2DEG + 90.f;
      }
      y2->setAngleWindCorrection(angle);
    }
    y2->setAxisType(map::AxisProfile::kRight);
    y2->setPuanson(map["vcutl_levels"]);
    y2->setRange(100,1000);
    y2->setTickVector(mkSequence(100,1000,100), 20);
    y2->setTickVector(mkSequence(100,1000, 50), 18);
    y2->setTickVector(mkSequence(100,1000, 10), 15);
    y2->setTickVector(mkSequence(100,1000,  5), 12);

    layerProfiles_->setAxisY(y1);
    layerProfiles_->setAxisY2(y2);

    layerProfiles_->xAxis()->setVisible(false);
    layerProfiles_->xAxis2()->setVisible(false);
  }
  // добавляем на слой профили для промежуточных пунктов

  for ( int i = 1, isz = profilesData_.size() - 1; i < isz; ++i ) {
    float angle = 0;
    if (mapProto.profile() != meteo::map::proto::kBulletinTime) {
      angle = profilesData_.at(i-1).coord().radianAzimuth(profilesData_.at(i).coord()) * RAD2DEG + 90.f;
    }
    layerProfiles_->addProfile(profilesData_.at(i).xPos(), kIsoLevels, angle);
  }
  // Подписи к профилям
  QVector<QString> labels;
  QVector<float> ticks;
  for ( const zond::PlaceData& pd : profilesData_ ) {
    ticks << pd.xPos();
    QString label;
    label += pd.name();
    label += pd.dt().addSecs(mapProto.offsetutc()).toString("\nhh:mm");
    labels << label;
  }
  //Дополнительные профили при временном разрезе
  if (mapProto.profile() == meteo::map::proto::kBulletinTime) {
    auto findDt = [=] (const QDateTime& dt) {
      bool find = false;
      for ( auto zond : profilesData_ ) {
        if ( zond.dt() == dt) {
          return true;
        }
      }
      return find;
    };
    if (profilesData_.size() > 1) {
      zond::PlaceData profile = profilesData_.at(0);
      float coordDiff = profilesData_.at(1).xPos() - profilesData_.at(0).xPos();
      float timeDiff = profilesData_.at(0).dt().secsTo(profilesData_.at(1).dt());
      float coeff = coordDiff/timeDiff;
      for ( int i = 0, isz = mapProto.point_size() ; i < isz; ++i ) {
        QDateTime dt{QDateTime::fromString( QString::fromStdString( mapProto.point(i).date_time() ), "yyyy-MM-ddThh:mm:ss")};
        if (!findDt(dt)) {
          profile.setDt( dt );
          profile.setCoord( profile.zond().coord() );
          float xpos = profilesData_.at(0).dt().secsTo(profile.dt());
          profile.setXPos(xpos*coeff);
          layerProfiles_->addProfile(profile.xPos(), kIsoLevels);
          QString label;
          label += profile.name();
          label += profile.dt().addSecs(mapProto.offsetutc()).toString("\nhh:mm");
          labels << label;
          ticks << profile.xPos();
        }
      }
    }
  }

  LayerAxis* l = new LayerAxis(this, QObject::tr("Пункты"));
  l->setBase(true);
  l->yAxis()->setVisible(false);
  map::AxisSimple* x = l->xAxis();
  x->setRange(0,800);
  x->setSubTickCount(1);
  x->setTickVector(ticks);
  x->setTickLabelVector(labels);

  return true;
}

void VProfileDoc::setRamka(const QMargins &p, float maxP, float maxH)
{
  if (MnMath::isEqual(-1,maxP) &&
      MnMath::isEqual(-1 ,maxH) ) { return; }

  GeneralProj* proj = static_cast<GeneralProj*>(this->projection()); //TODO что это и зачем?
  float yOffset = MnMath::isEqual( -1, maxP ) ? getPByH(kBlankWidth/2, maxH) : maxP;

  float l = 0 - p.left();
  float r = p.right() + kBlankWidth;
  float t = yOffset - p.top();
  float b = p.bottom() + 75 + 1000;

  proj->setRamka(l, t, r, b);
  proj->setFuncTransform(cutLinearF2X_one, cutLinearX2F_one);
  proj->setFuncTransform(cutLinearF2X_onef, cutLinearX2F_onef);

  this->setProjScaleFactorToSize(QSize(r - l, b - t));
  //size_ = QSize(r - l, b - t);
}

void VProfileDoc::showNoData()
{
  QMutableMapIterator<int, map::Layer*> it(dataLayerRef_);
  while (it.hasNext()) {
    it.next();
 //   var(it.value());
    if (it.value() != nullptr) {
      delete it.value();
      it.value() = nullptr;
    }
    it.remove();
  }

 QMutableMapIterator<int, map::LayerIso*> it1(isoLayers_);
  while (it1.hasNext()) {
    it1.next();
 //   var(it1.value());
    if (it1.value() != nullptr) {
      delete it1.value();
      it1.value() = nullptr;
    }
    it1.remove();
  }

  if ( nullptr != nanoskaLayer_ ) {
    delete nanoskaLayer_;
    nanoskaLayer_ = nullptr;
  }

  if ( nullptr != layerProfiles_ ) {
    delete layerProfiles_;
    layerProfiles_ = nullptr;
  }
}

bool VProfileDoc::fillLayer(map::LayerIso* layer, float min, float max, float step, const QColor& color)
{
  if ( nullptr == layer || nullptr == layer->field() ) { return false; }

  map::proto::FieldColor fieldcolor;
  fieldcolor.set_descr( layer->field()->getDescr() );
  fieldcolor.mutable_pen()->set_width(1);
  fieldcolor.mutable_pen()->set_color( color.rgba() );
  map::proto::LevelColor* lvlclr = fieldcolor.add_level();
  lvlclr->set_step_iso(step);
  lvlclr->set_level( layer->field()->getLevel() );
  lvlclr->set_type_level( layer->field()->getLevelType() );
  map::GradientParams::setIsoMin( lvlclr->level(), lvlclr->type_level(), min, &fieldcolor );
  map::GradientParams::setIsoMax( lvlclr->level(), lvlclr->type_level(), max, &fieldcolor );
  map::GradientParams::setIsoColorMax( lvlclr->level(), lvlclr->type_level(), color.rgba(), &fieldcolor );
  map::GradientParams::setIsoColorMin( lvlclr->level(), lvlclr->type_level(), color.rgba(), &fieldcolor );
  int iso_count = layer->addIsoLines(fieldcolor);

  for ( map::Object* o : layer->objects() ) {
    o->setDrawOnOnEnds(false);
  }

  if ( 0 < iso_count ) {
    return true;
  }

  return false;
}

void VProfileDoc::fillScaleOptions(ScaleOptions* opt, obanal::TField* f, float maxH, float x) const
{
  float T = 0;
  float P = 0;
  zond::SA81_P(0, &T, &P);
  opt->tickValues += P;
  opt->tickLabels += QString::number(0);
//  debug_log << "h:" << 0 << "P:" << P << "(SA-81)";

  for ( int h=1000; h<=maxH; h+=1000 ) {
    QVector<float> y = yByValue(f, x, h);
    if ( y.size() > 0 ) {
      opt->tickValues += y.first();
//      debug_log << "h:" << h << "P:" << y.first() << "(field)";
    }
    else {
      float T = 0;
      float P = 0;
      zond::SA81_P(h, &T, &P);
      opt->tickValues += P;
//      debug_log << "h:" << h << "P:" << P << "(SA-81)";
    }
    opt->tickLabels += QString::number(h);
  }
}

float VProfileDoc::getPByH(float x, float h) const
{
  obanal::TField* fieldH = nullptr;
  if ( nullptr != isoLayers_[CUT_LAYER_H] ) {
    fieldH = isoLayers_[CUT_LAYER_H]->field();
  }

  QVector<float> v = yByValue(fieldH, x, h);
  if ( !v.isEmpty() ) {
    return v.first();
  }

  float T = 0;
  float P = 0;
  zond::SA81_P(h, &T, &P);
  return P;
}

bool VProfileDoc::cutLinearF2X_one( const Projection& proj, const GeoPoint& geoCoord, QPoint* meterCoord )
{
  if ( nullptr == meterCoord ) { return false; }

  float lat = geoCoord.lat() - proj.getMapCenter().lat();
  float lon = geoCoord.lon() - proj.getMapCenter().lon();

  float x = kRVSG*lat*proj.xfactor()*kZoom;
  float y = kRVSG*lon*proj.yfactor()*kZoom;
  meterCoord->setX(x);
  meterCoord->setY(y);
  if ( 0 > x &&
      MnMath::isEqual(::abs(x), x) ) {
    return false;
  }
  if ( 0 > y && MnMath::isEqual(::abs(y), y) ) {
    return false;
  }

  return true;
}

bool VProfileDoc::cutLinearX2F_one( const Projection& proj, const QPoint& meterCoord, GeoPoint* geoCoord )
{
  if ( nullptr == geoCoord ) { return false; }

  *geoCoord = GeoPoint( 0,0,0, LA_GENERAL );

  float x = meterCoord.x();
  float y = meterCoord.y();
  float lat = proj.getMapCenter().lat() + x/(kRVSG*proj.xfactor()*kZoom);
  float lon = proj.getMapCenter().lon() + y/(kRVSG*proj.yfactor()*kZoom);
  geoCoord->setLat(lat);
  geoCoord->setLon(lon);

  return true;
}

bool VProfileDoc::cutLinearF2X_onef( const Projection& proj, const GeoPoint& geoCoord, QPointF* meterCoord )
{
  if ( nullptr == meterCoord ) { return false; }

  float lon = geoCoord.lon() - proj.getMapCenter().lon();
  float lat = (geoCoord.lat() - proj.getMapCenter().lat());

  float x = kRVSG*lat*proj.xfactor()*kZoom;
  float y = kRVSG*lon*proj.yfactor()*kZoom;
  meterCoord->setX(x );
  meterCoord->setY(y);

  return true;
}

bool VProfileDoc::cutLinearX2F_onef( const Projection& proj, const QPointF& meterCoord, GeoPoint* geoCoord )
{
  if ( nullptr == geoCoord ) { return false; }

  *geoCoord = GeoPoint( 0,0,0, LA_GENERAL );

  float x = meterCoord.x();
  float y = meterCoord.y();
  float lat = proj.getMapCenter().lat() + x/(kRVSG*proj.xfactor()*kZoom);
  float lon = proj.getMapCenter().lon() + y/(kRVSG*proj.yfactor()*kZoom);
  geoCoord->setLat(lat);
  geoCoord->setLon(lon);

  return true;
}

QVector<float> VProfileDoc::yByValue(const obanal::TField* field, float x, float value, bool* ok)
{
  if ( nullptr != ok ) { *ok = false; }

  if ( nullptr == field ) { return QVector<float>(); }

  float step = 0;
  if ( !field->stepLa(&step) ) {
    return QVector<float>();
  }

  QVector<float> values;

  float startLa = field->netLa(0);
  for ( int j=1,jsz=field->kolLa(); j<jsz; ++j ) {
    GeoPoint gp1(x, startLa + step*(j - 1), 0, LA_GENERAL);
    GeoPoint gp2(x, startLa + step*j, 0, LA_GENERAL);

    bool hasValue = false;

    float v1 = field->pointValue(gp1, &hasValue);
    if ( !hasValue ) { continue; }

    float v2 = field->pointValue(gp2, &hasValue);
    if ( !hasValue ) { continue; }

    float y1 = field->getLa(j - 1);
    float y2 = field->getLa(j);

    if ( isInRange(value,v1,v2) ) {
      values += y1 + (value - v1) / (v2 - v1) * (y2 - y1);
    }
  }

  if ( nullptr != ok ) { *ok = true; }

  return values;
}

bool VProfileDoc::isInRange(float value, float begin, float end, bool properBegin, bool properEnd)
{
  if ( !properBegin && qFuzzyCompare(1. + value, 1. + begin) ) { return true; }
  if ( !properEnd   && qFuzzyCompare(1. + value, 1. + end)   ) { return true; }

  if ( end < begin ) { qSwap(begin, end); }

  return ( value > begin && value < end );
}


void VProfileDoc::createAxis( int type)
{
  if ( 1 > kIsoLevels.size() || 1 > profilesData_.size() ) {
    error_log << "Нет информации об уровнях kIsoLevels"<<kIsoLevels;
    return;
  }

  if (dataLayerRef_.contains(CUT_LAYER_AXIS) && nullptr != dataLayerRef_[CUT_LAYER_AXIS] ) {
    delete dataLayerRef_[CUT_LAYER_AXIS];
    dataLayerRef_[CUT_LAYER_AXIS] = nullptr;
  }
  if (!dataLayerRef_.contains(CUT_LAYER_AXIS) || nullptr == dataLayerRef_[CUT_LAYER_AXIS] ) {
    dataLayerRef_[CUT_LAYER_AXIS] = new map::Layer(this, QString("Оси по станциям"));
    dataLayerRef_[CUT_LAYER_AXIS]->setBase(true);
  }

  for ( int i = 0, isz = profilesData_.size(); i < isz; ++i )
  {
    float minP = kIsoLevels.first();
    float maxP = kIsoLevels.last();
    GeoPoint p1(profilesData_.at(i).xPos(), minP, 0, LA_GENERAL);
    GeoPoint p2(profilesData_.at(i).xPos(), maxP, 0, LA_GENERAL);
    GeoVector axisSkelet;
    axisSkelet << p1 << p2;
    map::GeoAxis* axis = new map::GeoAxis(dataLayerRef_[CUT_LAYER_AXIS]);

    QPen pen = axis->qpen();
    pen.setWidth(1);
    pen.setColor(Qt::black);
    axis->setPen(pen);
    axis->setSkelet(axisSkelet);
    axis->setTickValues(kIsoLevels);
    axis->setRange(minP, maxP);
    map::GeoText txtPlace(this->projection());
    txtPlace.setPos(kBottomCenter);
    if(type == kSpaceType){
      txtPlace.setText(profilesData_.at(i).name() + "\n"+profilesData_.at(i).coord().toString(false,"%1\n%2"));
    }
    else {
      const QDateTime dt = profilesData_.at(i).dt();
      txtPlace.setText(meteo::dateToHumanTimeShort(dt));
    }
    txtPlace.setPos(kBottomCenter);
    axis->addAxisLabel(txtPlace, QPoint(15,0), map::GeoAxis::kEnd);
  }
}

bool VProfileDoc::addObled()
{
  int psize = profilesData_.size();

  if( 1 > psize ) {
    return false;
  }
  if ( !prepLayerEasy(CUT_LAYER_OBLED, QString("Обледенение") )) {return false;}

  for ( int i=0; i<psize; ++i ) {
    QList<float> p_obled;
    profilesData_.at(i).zond().oprGranObled(&p_obled);
    //  debug_log<<"Обледенение" << p_obled;
    if( p_obled.size() <2) continue;

    for ( int j=0; j<p_obled.size(); ++j ) {
      float p = p_obled.at(j);
      if(p < 200. ) continue;
      GeoVector lineSkelet;
      lineSkelet.append(GeoPoint(profilesData_.at(i).xPos(), p, 0, LA_GENERAL));
      map::GeoPixmap* pix = new map::GeoPixmap(dataLayerRef_[CUT_LAYER_OBLED]);
      pix->setImage(QImage(":/meteo/icons/geopixmap/obledenenie.png"));
      pix->setPenColor(Qt::red);
      pix->setPos(kCenter);
      pix->setScaleXy(QPointF(0.15,0.15));
      pix->setDrawAlways(true);
      pix->setSkelet(lineSkelet);
    }
  }

  return true;
}


bool VProfileDoc::addOblakSloi()
{
  int psize = profilesData_.size();

  if( 1 > psize ) {
    return false;
  }
  if (!prepLayerEasy(CUT_LAYER_OBLAK, QString("Облачность")) ) { return false; }
  for ( int i=0; i<psize; ++i ) {
    QList<float> p_obled;
    profilesData_.at(i).zond().oprGranOblak(&p_obled);
    // debug_log<<"Облачность" << p_obled;
    if( p_obled.size() <2) continue;

    for( int j=0; j<p_obled.size(); ++j ) {
      float p = p_obled.at(j);
      if(p < 200. ) continue;
      GeoVector lineSkelet;
      lineSkelet.append(GeoPoint(profilesData_.at(i).xPos(), p, 0, LA_GENERAL));
      map::GeoText* label = new map::GeoText(dataLayerRef_[CUT_LAYER_OBLAK]);
      QPen pen = label->qpen();
      QFont font = label->qfont();
      QColor clr(Qt::blue);
      clr.setAlpha(128);

      pen.setColor(clr);
      font.setPointSize(10);
      label->setPen(pen);
      label->setFont(font);
      label->setPos(kCenter);
      label->setDrawAlways(false);
      label->setText(QObject::tr("Облачность"));
      label->setSkelet(lineSkelet);
    }
  }
  return true;
}

bool VProfileDoc::addOblak()
{
  int psize = profilesData_.size();

  if( 1 > psize ) {
    return false;
  }

  if (!prepLayer(CUT_LAYER_D, QObject::tr("Облачность") ) ) { return false; }
  //return fillLayer( isoLayerRef_[CUT_LAYER_H], -1000, 15000, 1000, Qt::black);
  if( nullptr == isoLayers_[CUT_LAYER_D] ||
      false == isoLayers_[CUT_LAYER_D]->hasField() ){
    return false;
  }
  obanal::TField* field_D = isoLayers_[CUT_LAYER_D]->field();
  zond::CloudDkr cl;

  for(int i = 0; i < field_D->kolFi(); ++i){
    for(int j = 0; j < field_D->kolLa(); ++j){
      float d = 0.;
     // int x = field_D->getFi(i);
      int p = field_D->getLa(j);
        //debug_log<< "p" << p<< "x"<<x;

      if(field_D->getData(i, j, &d)){
        //debug_log<< "p" << p<< "x"<<x<<d;
        if(!cl.isCloud(p,d)){
          field_D->setData(i, j,0.,false);
        }
        else {
         //     debug_log << "have cloud" << x << p << d;
        }
      }
    }
  }
  QColor clr(Qt::blue);
  clr.setAlpha(128);
  map::proto::FieldColor fc;
  map::proto::ColorGrad *grad = fc.add_def_grad();;
  grad->set_min_value(0);
  grad->set_max_value(4);
  grad->set_min_color(clr.rgba());
  grad->set_max_color(clr.rgba());

  isoLayers_[CUT_LAYER_D]->addGradient(fc);
  return true;
}

bool VProfileDoc::addBoltan()
{
  int psize = profilesData_.size();

  if( 1 > psize ) {
    return false;
  }

  if (!prepLayerEasy(CUT_LAYER_BOLT, QString("Болтанка")) ) { return false; }

  for ( int i=0; i<psize; ++i ) {
    QList<float> p_obled;
    profilesData_.at(i).zond().oprGranBoltan_Td(&p_obled,profilesData_.at(i).coord().fi() );
    //debug_log<<"Болтанка" << p_obled;
    if( p_obled.size() <2) continue;

    for ( int j=0; j<p_obled.size(); ++j ) {
      float p = p_obled.at(j);
      if(p < 200. ) continue;
      GeoVector lineSkelet;
      lineSkelet.append(GeoPoint(profilesData_.at(i).xPos(), p, 0, LA_GENERAL));
      map::GeoPixmap* pix = new map::GeoPixmap(dataLayerRef_[CUT_LAYER_BOLT]);
      pix->setImage(QImage(":/meteo/icons/geopixmap/boltanka.png"));
      pix->setPenColor(Qt::red);
      pix->setPos(kCenter);
      pix->setScaleXy(QPointF(0.15,0.15));
      pix->setDrawAlways(true);
      pix->setSkelet(lineSkelet);
    }
  }
  return true;
}

bool VProfileDoc::addTrace()
{
  int psize = profilesData_.size();

  if( 1 > psize ) {
    return false;
  }

  if (!prepLayerEasy(CUT_LAYER_TRACE, QString("Конденсационные следы")) ) { return false; }

  for ( int i=0; i<psize; ++i ) {
    QList<float> p_obled;
    profilesData_.at(i).zond().oprGranTrace(&p_obled);
    //  debug_log<<"Конденсационные следы" << p_obled;
    if( p_obled.size() < 2 ) continue;

    for ( int j=0; j<p_obled.size(); ++j ) {
      float p = p_obled.at(j);
      if(p < 200. ) continue;
      GeoVector lineSkelet;
      lineSkelet.append(GeoPoint(profilesData_.at(i).xPos(), p, 0, LA_GENERAL));
      map::GeoPixmap* pix = new map::GeoPixmap(dataLayerRef_[CUT_LAYER_TRACE]);
      pix->setImage(QImage(":/meteo/icons/geopixmap/kond.sledi.png"));
      pix->setPenColor(Qt::red);
      pix->setPos(kCenter);
      pix->setScaleXy(QPointF(0.15,0.15));
      pix->setDrawAlways(true);
      pix->setSkelet(lineSkelet);
    }
  }
  return true;
}

bool VProfileDoc::addMaxWind(){

  QVector<DecartData> ddMax;
  QVector<DecartData> ffMax;
  if (! vcut_.getMaxWind(&ddMax, &ffMax) ) {
    return false;
  }
  if (!prepLayerEasy(CUT_LAYER_M_WIND, QString("Максимальный ветер")) ) { return false; }
  GeoVector lineSkelet;
  for ( int i=0; i<ffMax.size(); ++i ) {
    lineSkelet.append(GeoPoint(ffMax.at(i).x, ffMax.at(i).y, 0, LA_GENERAL));
  }
  map::GeoPolygon* line = new map::GeoPolygon( dataLayerRef_[CUT_LAYER_M_WIND]);
  QPen pen = line->qpen();
  pen.setWidth(1);
  pen.setColor(Qt::red);
  line->setPen(pen);
  line->setValue(ffMax.at(0).fun);
  line->setSplineFactor(4);
  line->setSkelet(lineSkelet);
  return true;

}

bool VProfileDoc::addH()
{
  if ( !prepLayer(CUT_LAYER_H, QString("Геопотенциальная высота")) ) { return false; }

  return fillLayer( isoLayers_[CUT_LAYER_H], -1000, 15000, 1000, Qt::black);
}

bool VProfileDoc::addWindff()
{
  if (!prepLayer(CUT_LAYER_ff, QString("Скорость ветра") ) ) { return false; }
  // изотахи начиная со 100 км/ч, через 40 км/ч
  float ffMin = 100.0 * 1000.0/3600;
  float ffStep = 40.0 * 1000.0/3600;

  return fillLayer(isoLayers_[CUT_LAYER_ff], ffMin, 400, ffStep, Qt::green);
}



bool VProfileDoc::addWinddd()
{
  if (!prepLayer(CUT_LAYER_dd, QString("Направление ветра") ) ) { return false; }

  return fillLayer(isoLayers_[CUT_LAYER_dd], 0, 360, 30, Qt::blue);
}

bool VProfileDoc::addTemperatureR()
{
  if (!prepLayer(CUT_LAYER_Td, QString("Температура точки росы")) ) { return false; }

  return fillLayer(isoLayers_[CUT_LAYER_Td], -80, 50, 10, Qt::magenta);
}

bool VProfileDoc::addTemperature()
{
  if (!prepLayer(CUT_LAYER_T, QString("Температура")) ) { return false; }

  fillLayer(isoLayers_[CUT_LAYER_T], -80, -50, 5, Qt::red);

  return fillLayer(isoLayers_[CUT_LAYER_T], -50, 50, 10, Qt::red);
}

bool VProfileDoc::addNanoska()
{
  if ( nullptr != nanoskaLayer_ ) {
    delete nanoskaLayer_;
  }
  nanoskaLayer_ = new map::LayerVProfile(this, QString("Наноска"));
  nanoskaLayer_->setPlaceData(profilesData_,kIsoLevels);
  return true;
}

bool VProfileDoc::addTropo()
{
  QVector<DecartData> tropo;
  if (! vcut_.getTropo(&tropo) ) {
    return false;
  }
  if (!prepLayerEasy(CUT_LAYER_TROPO, QString("Тропопауза")) ) {
    return false;
  }

  GeoVector lineSkelet;
  for ( int i=0; i<tropo.size(); ++i ) {
    lineSkelet.append(GeoPoint(tropo.at(i).x,tropo.at(i).y,0,LA_GENERAL));
  }
  GeoPolygon* line = new GeoPolygon(dataLayerRef_[CUT_LAYER_TROPO]);
  QPen pen = line->qpen();
  pen.setWidth(8);
  pen.setColor(QColor(65,45,20));
  line->setPen(pen);
  line->setValue(tropo.at(0).fun);
  line->setSplineFactor(4);
  line->setSkelet(lineSkelet);

  GeoPoint labelP(0,kIsoLevels.last(),0,LA_GENERAL);
  for ( int i=0; i<tropo.size(); ++i ) {
    if ( labelP.lon() > tropo.at(i).y ) {
      labelP.setLat(kBlankWidth / 2);
      labelP.setLon(tropo.at(i).y - 30);
    }
  }
  GeoVector labelSkelet;
  labelSkelet.append(labelP);

  GeoText* label = new GeoText(dataLayerRef_[CUT_LAYER_TROPO]);
  pen = label->qpen();
  QFont font = label->qfont();
  pen.setColor(QColor(65,45,20));
  font.setPointSize(20);
  label->setPen(pen);
  label->setFont(font);
  label->setPos(kCenter);
  label->setDrawAlways(true);
  label->setText(QObject::tr("Тропопауза"));
  label->setSkelet(labelSkelet);

  return true;
}

bool VProfileDoc::prepLayerEasy(int type, const QString& nm)
{
  if ( dataLayerRef_.contains(type) && nullptr != dataLayerRef_[type] ) {
    delete dataLayerRef_[type];
    dataLayerRef_[type] =nullptr;
  }
  dataLayerRef_[type]  = new Layer(this);
  dataLayerRef_[type]->setName(nm);
  //dataLayerRef_[type]->setShotName(nm);
  return true;
}

bool VProfileDoc::prepLayer(int type, const QString& nm)
{
  if(isoLayers_.contains(type) && nullptr != isoLayers_[type] ) {
    delete isoLayers_[type]->field();
    delete isoLayers_[type];
    isoLayers_[type] =nullptr;
  }
  obanal::TField* field = new obanal::TField();
  if ( !vcut_.calc(field, type) ) {
    delete field;
    field = nullptr;
    return false;
  }
  field->setDescr(zond::descrPoUrType( static_cast< zond::ValueType>(type)));
  field->setLevelType(-1);
  isoLayers_[type] = new map::LayerIso(this);
  isoLayers_[type]->setName(nm);
  //isoLayers_[type]->setShotName(nm);
  isoLayers_[type]->setField(field);
  return true;
}

bool VProfileDoc::addWind()
{

  if(!prepWindLayers()) { return false; }
  fillLayer(isoLayers_[CUT_LAYER_dd], 0, 360, 30, Qt::blue);
  float ffMin = 100.0 * 1000.0/3600;
  float ffStep = 40.0 * 1000.0/3600;
  return fillLayer(isoLayers_[CUT_LAYER_ff], ffMin, 400, ffStep, Qt::green);
}

void VProfileDoc::addEchelon()
{
  //
  // эшелон
  //
  GeoVector echelon;

  if ( profilesData_.size() > 0 ) {
    zond::PlaceData place = profilesData_.first();
    echelon << GeoPoint(place.xPos(), getPByH(place.xPos(),0.), 0., LA_GENERAL);
  }
  for ( int i=0,isz=profilesData_.size()-1; i<isz; ++i ) {
    zond::PlaceData place1 = profilesData_.at(i);
    zond::PlaceData place2 = profilesData_.at(i + 1);

    float x1 = place1.xPos();
    float x2 = place2.xPos();
    float e = place1.echelon();

    float y1 = getPByH(x1, e);
    float y2 = getPByH(x2, e);

    if ( 0 == i ) {
      x1 += 20.;
    }
    if ( i == isz - 1 ) {
      x2 -= 20.;
    }

    echelon << GeoPoint(x1, y1, 0., LA_GENERAL);
    echelon << GeoPoint(x2, y2, 0., LA_GENERAL);
  }
  if ( profilesData_.size() > 0 ) {
    zond::PlaceData place = profilesData_.last();
    echelon << GeoPoint(place.xPos(), getPByH(place.xPos(),0.), 0., LA_GENERAL);
  }

  map::GeoPolygon* echelonObj = new map::GeoPolygon(dataLayerRef_[CUT_LAYER_H]);
  echelonObj->setPenWidth(2);
  echelonObj->setSkelet(echelon);

//
// ветер (по маршруту)
//
  // берём только координаты на уровне эшелона
    map::LayerVProfile *windlayeresh = new map::LayerVProfile(this, QString("Ветер на эшелоне"));

    for ( int i=1,isz=profilesData_.size(); i<isz; ++i ) {
     float xStep = kBlankWidth / 10.;
     GeoPoint gp1 = echelon.at(i-1);
     float azimuth = profilesData_.at(i-1).coord().radianAzimuth(profilesData_.at(i).coord()) * RAD2DEG + 90.f;

     float x = profilesData_.at(i-1).xPos();
     float x2 = profilesData_.at(i).xPos();

     bool firstIter = true;
     while ( true ) {
  //      пропускаем первый пуансон на эшелоне, т.к. он конфликтует с ветром по высотам в пункте вылета
       if ( 1 == i && firstIter ) {
         x += xStep;
         firstIter = false;
         continue;
       }

       if ( x > x2 ) { x = x2; }
       gp1.setLat(x);
       gp1.setLon(getPByH(x, profilesData_.at(i-1).echelon()));
       windlayeresh->addWind(isoLayers_[CUT_LAYER_uu], isoLayers_[CUT_LAYER_vv], gp1, azimuth);
       if ( x >= x2 ) { break; }
       x += xStep;

       // пропускаем последний пуансон на эшелоне, т.к. он конфликтует с ветром по высотам в пункте посадки
       if ( isz - 1 == i && x >= x2 ) { break; }
     }
   }
}

bool VProfileDoc::prepWindLayers()
{

 if(false == prepLayer(CUT_LAYER_uu, QString("U составляющая ветра"))) { return false; }
 if(false == prepLayer(CUT_LAYER_vv, QString("V составляющая ветра"))) { return false; }
 if(nullptr == isoLayers_[CUT_LAYER_uu]) { return false; }
 if(nullptr == isoLayers_[CUT_LAYER_vv]) { return false; }
 obanal::TField* f_u = isoLayers_[CUT_LAYER_uu]->field();
 if(nullptr == f_u) { return false; }
 obanal::TField* f_v = isoLayers_[CUT_LAYER_vv]->field();
 if(nullptr == f_v) { return false; }

 obanal::TField* fd_dd = new obanal::TField();// f_u->getCopy();
 obanal::TField* fd_ff = new obanal::TField();// f_u->getCopy();
 if(false == vcut_.initField(fd_dd, CUT_LAYER_dd)) { return false; }
 if(false == vcut_.initField(fd_ff, CUT_LAYER_ff)) { return false; }

 int dsize = f_u->kolData();
 int j=0;
 for(int i=0; i < dsize; ++i) {
   if(f_u->getMask(i) && f_v->getMask(i)) {
     float dd = 0.0;
     float ff = 0.0;
     float u = 0.0;
     float v = 0.0;

     u = f_u->getData(i);
     v = f_v->getData(i);

     float rtog=180./3.1415926;

     ff = sqrt(u*u+v*v);
     if((ff) == 0.) {dd = 0.;}
     else {
         dd = atan2(float(v),float(u))*rtog;
        dd = MnMath::M0To360(270.- dd);
     }
     // MnMath::preobrUVtoDF(u, v, &dd, &ff);
   //  debug_log<<"uu "<<u<<" vv "<<v<<" dd "<<dd<<" ff "<<ff;
     fd_dd->setData(i, dd, true);
     fd_ff->setData(i, ff, true);
     ++j;
   }
   else  {
     fd_dd->setMasks(i, false);
     fd_ff->setMasks(i, false);
   }
 }
 isoLayers_[CUT_LAYER_vv]->setVisisble(false);
 isoLayers_[CUT_LAYER_uu]->setVisisble(false);

 fd_dd->setSrcPointCount(j);
 fd_dd->setDescr(zond::descrPoUrType( static_cast< zond::ValueType>(CUT_LAYER_dd)));
 fd_dd->setLevelType(-1);
 fd_ff->setSrcPointCount(j);
 fd_ff->setDescr(zond::descrPoUrType( static_cast< zond::ValueType>(CUT_LAYER_ff)));
 fd_ff->setLevelType(-1);

 isoLayers_[CUT_LAYER_dd] = new map::LayerIso(this);
 isoLayers_[CUT_LAYER_dd]->setName(QString("Направление ветра"));
 //isoLayers_[CUT_LAYER_dd]->setShotName(QString("Направление ветра"));
 isoLayers_[CUT_LAYER_dd]->setField(fd_dd);

 isoLayers_[CUT_LAYER_ff] = new map::LayerIso(this);
 isoLayers_[CUT_LAYER_ff]->setName(QString("Скорость ветра"));
 //isoLayers_[CUT_LAYER_ff]->setShotName(QString("Скорость ветра"));
 isoLayers_[CUT_LAYER_ff]->setField(fd_ff);

  return true;
}



void VProfileDoc::calcDistanceByCoord(int length)
{
  int psize = profilesData_.size();

  if ( psize < 1 ) { return; }

  float total = 0;// сумма расстояний
  // список расстояний между i-й и первой координатой
  QVector<float> distanceList = QVector<float>(psize, 0.);
  for ( int i=1,isz=psize; i<isz; ++i ) {
    distanceList[i] = profilesData_.at(i-1).coord().calcDistance(profilesData_.at(i).coord());
   // var(distanceList[i]);
    total += distanceList[i];
  }

  if ( total <= 0   ) {
   // debug_log << QObject::tr("Невозможно расчитать положение оси для пункта, длина маршрута %1 м.").arg(total);
    return;
  }
  float k = length/total;

  float dop = 0.;
  for ( int i=0,isz = psize; i<isz; ++i ) {
    dop += k * distanceList[i];
    profilesData_[i].setXPos(MnMath::ftoi_norm(dop));
  }
}

void VProfileDoc::calcDistanceByDt( int length)
{
  int psize = profilesData_.size();

  if ( psize < 1 ) { return; }

  float total = 0;// сумма расстояний
  // список расстояний между i-й и первой датой/временем
  QVector<int> distanceList = QVector<int>(psize, 0);
  for ( int i=0,isz=psize-1; i<isz; ++i ) {
    uint startOffset = profilesData_.at(i).dt().toTime_t();
    uint t = profilesData_.at(i+1).dt().toTime_t();
    distanceList[i+1] = t - startOffset;
    total+=distanceList[i+1];
  }

  if (MnMath::isZero(total) ) {
   // debug_log << QObject::tr("Невозможно расчитать положение оси для пункта.");
    return;
  }
  float k = length/total;

  float dop = 0.;
  for ( int i=0,isz=psize; i<isz; ++i ) {
    //float k = (distanceList[i] );
    dop += k * distanceList[i];
    profilesData_[i].setXPos(MnMath::ftoi_norm(dop));
  }

}

}
} // meteo
