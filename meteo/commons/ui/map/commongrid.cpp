#include "commongrid.h"

#include <fstream>

#include <sql/nosql/nosqlquery.h>

#include <meteo/commons/global/common.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/ui/map/layergrid.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/loadcity.h>

#include "loader.h"

#include "geopixmap.h"
#include "geotext.h"

namespace {
  static bool res = meteo::map::Loader::instance()->registerGridHandler(meteo::global::kGridLoaderCommon, &meteo::map::loadCoordLayer );
}

namespace meteo {
namespace map {

  QString cityMarkIconByIndex(meteo::map::proto::CityMark cityMark)
  {    
    if ( cityMark == meteo::map::proto::kNoCityMark ) {
      return "";
    }
    
    if ( cityMark == meteo::map::proto::kCircleCityMark ) {
      return ":/meteo/icons/geopixmap/citymark/circle_1.png";
    }
    
    if ( cityMark == meteo::map::proto::kCircleFilledCityMark ) {
      return ":/meteo/icons/geopixmap/citymark/circle_2.png";
    }
    
    if ( cityMark == meteo::map::proto::kSquareCityMark ) {
      return ":/meteo/icons/geopixmap/citymark/circle_5.png";
    }
    
    if ( cityMark == meteo::map::proto::kSquareFilledCityMark ) {
      return ":/meteo/icons/geopixmap/citymark/circle_6.png";
    }
    
    if ( cityMark == meteo::map::proto::kStarCityMark ) {
      return ":/meteo/icons/geopixmap/citymark/star_5.png";
    }
    
    if ( cityMark == meteo::map::proto::kStarFilledCityMark ) {
      return ":/meteo/icons/geopixmap/citymark/star_9.png";
    }
    if ( cityMark == meteo::map::proto::kAirCityMark ) {
      return ":/meteo/icons/geopixmap/citymark/air.png";
    }
    if ( cityMark == meteo::map::proto::kFlagCityMark ) {
      return ":/meteo/icons/geopixmap/citymark/flag.png";
    }
    
    if ( cityMark == meteo::map::proto::kFlagRedCityMark ) {
      return ":/meteo/icons/geopixmap/citymark/flag_red.png";
    }
    if ( cityMark == meteo::map::proto::kFlagRectRedCityMark ) {
      return ":/meteo/icons/geopixmap/citymark/flagrect_red.png";
    }
    if ( cityMark == meteo::map::proto::kFlagBlueCityMark ) {
      return ":/meteo/icons/geopixmap/citymark/flag_blue.png";
    }
    if ( cityMark == meteo::map::proto::kFlagRectBlueCityMark ) {
      return ":/meteo/icons/geopixmap/citymark/flagrect_blue.png";
    }
    
    return "";
}

  meteo::Position cityMarkPositionByIndex(meteo::map::proto::CityMark cityMark)
  {
    meteo::Position pos = kCenter;
    
    if ( cityMark == meteo::map::proto::kFlagRedCityMark     ||
	 cityMark == meteo::map::proto::kFlagRectRedCityMark ||
	 cityMark == meteo::map::proto::kFlagBlueCityMark    ||
	 cityMark == meteo::map::proto::kFlagRectBlueCityMark) {
      pos = kTopRight;
    }
    
    return pos;
  }

const QString kGrid01("Grid-01");
const QString kGrid02("Grid-02");
const QString kGrid05("Grid-05");
const QString kGrid10("Grid-10");
const QString kGrid30("Grid-30");

void createDefaultGridSettings( const QString& filename )
{
  meteo::Properties props;
  meteo::Property* prop = props.add_property();
  prop->mutable_pen()->set_color( kMAP_GRID_COLOR.rgba() );
  prop->mutable_pen()->set_width(0);
  prop->mutable_brush()->set_style( kNoBrush );
  prop->set_closed(false);
  Generalization g;
  g.setLimits(0,13);
  prop->mutable_general()->CopyFrom( g.proto() );
  prop->set_name( kGrid01.toStdString() );
  prop = props.add_property();
  prop->CopyFrom( props.property(0) );
  g.setLimits(0,14);
  prop->mutable_general()->CopyFrom( g.proto() );
  prop->set_name( kGrid02.toStdString() );
  prop = props.add_property();
  prop->CopyFrom( props.property(1) );
  g.setLimits( 0, 13 );
  g.setScaleVisibility( 15, true );
  prop->mutable_general()->CopyFrom( g.proto() );
  prop->set_name( kGrid05.toStdString() );
  prop = props.add_property();
  prop->CopyFrom( props.property(2) );
  g.setLimits(0,16);
  prop->mutable_general()->CopyFrom( g.proto() );
  prop->set_name( kGrid10.toStdString() );
  prop = props.add_property();
  prop->CopyFrom( props.property(3) );
  g.setLimits(0,20);
  prop->mutable_general()->CopyFrom( g.proto() );
  prop->set_name( kGrid30.toStdString() );
  TProtoText::toFile( props, filename );
}

bool loadCoordLayer( Document* doc )
{
  if ( false == global::createMapSettingsDir() ) {
    error_log << QObject::tr("Не удалось создать директорию настроек. Свойства картографической основы невозможно загрузить");
  }
  QFile file(global::kMapGridSettingsFilename);
  if ( false == file.exists() ) {
    createDefaultGridSettings( file.fileName() );
  }
  QMap< QString, meteo::Property > gridparams = gridSettings();
  LayerGrid* l = doc->gridLayer();
  delete l;
  l = new LayerGrid( doc, QObject::tr("Координатная сетка") );

  Projection* proj = doc->projection();

  int startFi = proj->startFi()*RAD2DEG;
  int endFi = proj->endFi()*RAD2DEG;
  int step = ( startFi < endFi ) ? 1 : -1;
  // longitude lines
  for ( int lon = -179; lon < 181; lon += 1 ) {
    GeoVector gv;
    for ( int lat = startFi; lat != endFi + step; lat += step ) {
      GeoPoint gp = GeoPoint::fromDegree( lat, lon );
      gv.append(gp);
    }
    GeoPolygon* geopol = new GeoPolygon(l);
    l->addLongitude(geopol);
    GeoText* geotxt = new GeoText(geopol);
    geotxt->setValue( lon, "1'0'", QObject::tr("\xc2\xb0") );
    geotxt->setDrawOnOnEnds(false);
    geotxt->setPos(kCenter);
    geotxt->setPosOnParent(kNoPosition);
    geopol->setSkelet(gv);
    if ( 0 == lon%30 ) {
      geopol->setProperty( gridparams[kGrid30] );
    }
    else if ( 0 == lon%10 ) {
      geopol->setProperty( gridparams[kGrid10] );
    }
    else if ( 0 == lon%5 ) {
      geopol->setProperty( gridparams[kGrid05] );
    }
    else if ( 0 == lon%2 ) {
      geopol->setProperty( gridparams[kGrid02] );
    }
    else {
      geopol->setProperty( gridparams[kGrid01] );
    }
    Font fnt = geopol->font();
    fnt.set_pointsize(9);
    geopol->setFont(fnt);
  }
  // latitude lines
  for ( int lat = startFi; lat != endFi+step; lat += step ) {
    GeoVector gv;
    for ( int lon = -180; lon <= 180; lon += 1  ) {
      GeoPoint gp = GeoPoint::fromDegree( lat, lon );
      gv.append(gp);
    }
    GeoPolygon* geopol = new GeoPolygon(l);
    l->addLatitude(geopol);
    GeoText* geotxt = new GeoText(geopol);
    geotxt->setValue( lat, "1'0'", QObject::tr("\xc2\xb0") );
    if ( MERCAT != l->projection()->type() ) {
      geotxt->setPosOnParent(kTopCenter);
    }
    else {
      geotxt->setPosOnParent(kNoPosition);
    }
    geotxt->setPos(kCenter);
    geotxt->setDrawOnOnEnds(false);
    geopol->setSkelet(gv);
    if ( 0 == lat%30 ) {
      geopol->setProperty( gridparams[kGrid30] );
    }
    else if ( 0 == lat%10 ) {
      geopol->setProperty( gridparams[kGrid10] );
    }
    else if ( 0 == lat%5 ) {
      geopol->setProperty( gridparams[kGrid05] );
    }
    else if ( 0 == lat%2 ) {
      geopol->setProperty( gridparams[kGrid02] );
    }
    else {
      geopol->setProperty( gridparams[kGrid01] );
    }
    Font fnt = geopol->font();
    fnt.set_pointsize(9);
    geopol->setFont(fnt);
  }

  return true;
}
/*
void saveCityToDb( const QString& name, const GeoPoint& gp, int priority, int minlevel, int maxlevel )
{
  return;
  auto db = global::dbMeteo();
  auto q = global::kMongoQueriesNew["insert_city"];
  NosqlQuery query(q);
  query
    .arg(name)
    .arg(gp)
    .arg(priority)
    .arg(minlevel)
    .arg(maxlevel);
  db->execQuery( query.query() );
}*/

bool loadCitiesLayer(meteo::map::Document *doc)
{
  DisplaySettingsOperator dsOperator;
  if (dsOperator.loadedOK() == false){
    return false;
  }

  Layer *layer = new Layer(doc, QObject::tr("Населенные пункты"));
  layer->setBase(true);

  meteo::settings::Location loc = meteo::global::punkt();
  Layer *self_layer = new Layer(doc, QObject::tr("Пункт"));
  self_layer->setBase(true);
  GeoText* geoText = new GeoText(self_layer);
  geoText->setPriority( 22 );
  geoText->setVisible(true);
  geoText->setPos(kTopRight);
  geoText->setDrawAlways(true);
  geoText->setText(QString::fromUtf8(loc.name().c_str()));
  QFont f;
  f.setFamily("Serif");
  geoText->setFont(f);
  geoText->setPen(QPen(QColor(112, 112, 105)));
  GeoPoint gp = pbgeopoint2geopoint( loc.coord() );

  float scaleFactor = 0.25;
  QImage pix(":/meteo/icons/geopixmap/citymark/flag.png");

  int iconHeight = pix.height();
  int iconWidth = pix.width();
  int offsetX = scaleFactor * iconWidth / 2;
  int offsetY = scaleFactor * iconHeight / 2;

  geoText->setTranslateXy(QPoint(offsetX, -offsetY ));
  geoText->setSkelet(gp);

  GeoPixmap* geoPixmap = new GeoPixmap(geoText);
  geoPixmap->setImage(pix);
  geoPixmap->setPos(kCenter);
  geoPixmap->setScaleXy(QPointF(scaleFactor, scaleFactor));
  geoPixmap->setTranslateXy( QPoint(0,0) );
  geoPixmap->setSkelet(gp);

  QMap <int, meteo::map::proto::CitySetting> * citiesMap = dsOperator.citiesMap();

  QMap<int, meteo::map::proto::CitySetting>::Iterator citiesMapRecord;
  for (citiesMapRecord = citiesMap->begin();
       citiesMapRecord != citiesMap->end(); citiesMapRecord++) {
    int cityIndex = citiesMapRecord.key();

    meteo::map::proto::DisplaySetting cityDisplaySetting = dsOperator.filledCityDisplaySetting(cityIndex);

    meteo::map::proto::CitySetting cs = citiesMapRecord.value();//TODO REMOVE

    QString cityTitle = QString::fromUtf8(cs.title().c_str());

    if (cs.has_geopoint() == false){//город без координат
      warning_log << QObject::tr("отсутствуют координаты города %1").arg(cityTitle);
      continue;
    }

    if (cityDisplaySetting.has_visible() &&
          cityDisplaySetting.visible() == false &&
          cityDisplaySetting.has_always_visible() == false){
      continue; // для скрытых городов даже геотекст не создается
    }

    GeoPoint geoPoint = meteo::pbgeopoint2geopoint(cs.geopoint());

    GeoText* geoText = new GeoText(layer);
    Generalization generalization;
    generalization.setLimits(0, cityDisplaySetting.generalization_level());
    geoText->setGeneral(generalization);
    geoText->setPriority( cityDisplaySetting.priority() );
    geoText->setVisible(cityDisplaySetting.visible());
    geoText->setPen(cityDisplaySetting.pen());
    geoText->setFont(cityDisplaySetting.font());
    geoText->setPos(cityDisplaySetting.position());
    if (cityDisplaySetting.has_always_visible() == true &&
          cityDisplaySetting.always_visible() == true ){
      geoText->setDrawAlways(true);
    }
    geoText->setText(cityTitle);

    int offsetY = 1; //3;
    int offsetX = 1; // 3;
    //float scaleFactor = 0.15;
    float scaleFactor = cityDisplaySetting.city_scale();
    meteo::Position markPos = cityMarkPositionByIndex(cityDisplaySetting.city_mark());
    QString cityMarkStr = cityMarkIconByIndex(cityDisplaySetting.city_mark());
    QImage citymarkPixmap(cityMarkStr);
    int iconHeight = citymarkPixmap.height();
    int iconWidth = citymarkPixmap.width();
    
    if (markPos == kTopRight) { //для флажков, чтоб не наползали
      offsetX = offsetX * scaleFactor * iconWidth;
      offsetY = offsetY * scaleFactor * iconHeight;      
    } else {
      offsetX = offsetX * scaleFactor * iconWidth / 2;
      offsetY = offsetY * scaleFactor * iconHeight / 2;
    }
    
    QPoint shift;
    switch ( cityDisplaySetting.position() ) {
      case kTopCenter:
        shift = QPoint( 0, -offsetY );
        break;
      case kTopLeft:
        shift = QPoint( -offsetX, -offsetY );
        break;
      case kTopRight:
      default:
        shift = QPoint( offsetX, -offsetY );
        break;
    }
   
    geoText->setTranslateXy(shift);
    geoText->setSkelet(geoPoint);
    if ( cityMarkStr.isEmpty() == false ) {
      GeoPixmap* geoPixmap = new GeoPixmap(geoText);
      geoPixmap->setImage(citymarkPixmap);
      geoPixmap->setPos(markPos);      
      geoPixmap->setScaleXy(QPointF(scaleFactor, scaleFactor));
      geoPixmap->setTranslateXy( QPoint(0,0) );
      geoPixmap->setSkelet(geoPoint);
    }
//    saveCityToDb( cityTitle, geoPoint, cityDisplaySetting.priority(), 0, cityDisplaySetting.generalization_level() );
  }

  return true;
}

meteo::map::DisplaySettingsOperator::DisplaySettingsOperator()
{
//    QString citySettingsFileName(MnCommon::sharePath() + "/geo/city.pbf");
//    QString displaySettingsFileName(MnCommon::sharePath() + "/geo/displaysettings.pbf");
    init(global::kCitySettingsFileName, global::kDisplaySettingsFileName);
    return;
}

meteo::map::DisplaySettingsOperator::DisplaySettingsOperator(QString citySettingsProtoFileName, QString displaySettingsProtoFileName)
{
    init(citySettingsProtoFileName, displaySettingsProtoFileName);
}

void meteo::map::DisplaySettingsOperator::init(QString citySettingsProtoFileName, QString displaySettingsProtoFileName)
{
  loadedOK_ = false;
  //сохраняет пути к файлам настроек, чтобы потом записать в них измененные данные без повторного указания путей
  citySettingsFileName_ = citySettingsProtoFileName;
  displaySettingsFileName_ = displaySettingsProtoFileName;

  if ( false == loadCitiesMap( &citiesMap_, citySettingsProtoFileName) ) {
    return;
  }
  if ( false == loadDisplaySettings( &cityDisplaySettingMap_,
                                      &cityModeDisplaySettingMap_,
                                      &defaultDisplaySetting_,
                                      displaySettingsProtoFileName ) ) {
    return;  //сообщения об ошибках выводятся в вызванных функциях, здесь только возврат
  }

  loadedOK_ = true;
  cityMapModified_ = false;
  displaySettingListModified_ = false;

  return;
}

meteo::map::proto::DisplaySetting meteo::map::DisplaySettingsOperator::filledCityDisplaySetting(int cityIndex)
{    //функция по названию города выдает настройки отображения, УЖЕ ЗАПОЛНЕННЫЕ групповыми и умолчальными
        //и даже записывает название города
        meteo::map::proto::CitySetting citySetting = citiesMap_.value(cityIndex);
        //meteo::map::proto::DisplaySetting cityDS, modeDS, defaultDS;

        meteo::map::proto::DisplaySetting cityDisplaySetting;
        if (cityDisplaySettingMap_.contains(cityIndex)) {
            cityDisplaySetting.CopyFrom(cityDisplaySettingMap_.value(cityIndex));
        }
        if ( cityDisplaySetting.has_city_index() == false ) //это если ничего не нашли для города
            cityDisplaySetting.set_city_index(citySetting.index());

        meteo::map::proto::DisplaySetting cityModeDisplaySetting;
        if (citySetting.has_city_mode() == true){
            cityModeDisplaySetting.CopyFrom(filledCityModeDisplaySetting(citySetting.city_mode()));
        }
        else {
            cityModeDisplaySetting.CopyFrom(defaultDisplaySetting_);
        }

        cityDisplaySetting.CopyFrom(fillDisplaySetting(cityDisplaySetting, cityModeDisplaySetting));

        return cityDisplaySetting;
}

meteo::map::proto::DisplaySetting meteo::map::DisplaySettingsOperator::differedCityDisplaySetting(
        const meteo::map::proto::DisplaySetting cityDisplaySetting)
{//здесь необходимо, чтобы в настройках отображения для населенного пункта (cityDisplaySetting) был его индекс!
    int cityIndex = cityDisplaySetting.city_index();

    meteo::map::proto::CitySetting citySetting = citiesMap_.value(cityIndex);

    meteo::map::proto::DisplaySetting cityModeDisplaySetting;
    if (citySetting.has_city_mode() == true){
        cityModeDisplaySetting.CopyFrom(filledCityModeDisplaySetting(citySetting.city_mode()));
    }
    else {
        cityModeDisplaySetting.CopyFrom(defaultDisplaySetting_);
    }

    meteo::map::proto::DisplaySetting returnCityDisplaySetting;
    returnCityDisplaySetting.CopyFrom(differDisplaySetting(cityDisplaySetting, cityModeDisplaySetting));

    return returnCityDisplaySetting;
}

meteo::map::proto::DisplaySetting meteo::map::DisplaySettingsOperator::filledCityModeDisplaySetting(meteo::map::proto::CityMode cityMode)
{
    meteo::map::proto::DisplaySetting cityModeDisplaySetting;
    if (cityModeDisplaySettingMap_.contains(cityMode) == true) {
        cityModeDisplaySetting.CopyFrom(fillDisplaySetting(cityModeDisplaySettingMap_.value(cityMode),
                                                           defaultDisplaySetting_));
    }
    else {
        cityModeDisplaySetting.CopyFrom(defaultDisplaySetting_);
        cityModeDisplaySetting.set_city_mode(cityMode);
    }
    // если настроек отображения для группы нет, то они полностью будут замещены умолчальными

    return cityModeDisplaySetting;
}

meteo::map::proto::DisplaySetting meteo::map::DisplaySettingsOperator::differedCityModeDisplaySetting(
        const meteo::map::proto::DisplaySetting cityModeDisplaySetting)
{
    meteo::map::proto::DisplaySetting returnCityModeDisplaySetting;
    returnCityModeDisplaySetting.CopyFrom(differDisplaySetting(cityModeDisplaySetting, defaultDisplaySetting_));

    return returnCityModeDisplaySetting;
}

QMap<int, meteo::map::proto::CitySetting> *meteo::map::DisplaySettingsOperator::citiesMap()
{
    return &citiesMap_;
}

QMap<int, meteo::map::proto::DisplaySetting> *meteo::map::DisplaySettingsOperator::cityDisplaySettingMap()
{
    return &cityDisplaySettingMap_;
}

QMap<meteo::map::proto::CityMode, meteo::map::proto::DisplaySetting> *meteo::map::DisplaySettingsOperator::cityModeDisplaySettingMap()
{
    return &cityModeDisplaySettingMap_;
}

bool meteo::map::DisplaySettingsOperator::loadedOK()
{
    return loadedOK_;
}

meteo::map::proto::DisplaySetting meteo::map::DisplaySettingsOperator::defaultDisplaySetting()
{
    if (defaultDisplaySetting_.has_is_default() == true &&
            defaultDisplaySetting_.is_default() == true){
        return defaultDisplaySetting_;
    }
    //TODO не самый хороший вариант
    //если не нашлись умолчальные настройки, они создаются здесь
    meteo::map::proto::DisplaySetting emergSetting;
    emergSetting.set_is_default(true);
    emergSetting.set_generalization_level(0);
    emergSetting.set_priority(0);
    emergSetting.set_city_scale(0.15);

    emergSetting.set_visible(false);

    QFont qFont("helvetica");
    Font * font = emergSetting.mutable_font();
    font->CopyFrom(qfont2font(qFont));

    QRgb qRgb(0);
    Pen * pen = emergSetting.mutable_pen();
    pen->set_color(qRgb);
    emergSetting.set_position(kNoPosition);

    return emergSetting;
}

void meteo::map::DisplaySettingsOperator::setDefaultDisplaySetting(meteo::map::proto::DisplaySetting defaultDS)
{
  defaultDisplaySetting_.CopyFrom(defaultDS);
}

void meteo::map::DisplaySettingsOperator::setCityMapModified(bool modified)
{
    cityMapModified_ = modified;
}

bool meteo::map::DisplaySettingsOperator::cityMapModified()
{
    return cityMapModified_;
}

void meteo::map::DisplaySettingsOperator::setDisplaySettingListModified(bool modified)
{
    displaySettingListModified_ = modified;
}

bool meteo::map::DisplaySettingsOperator::displaySettingListModified()
{
    return displaySettingListModified_;
}

bool meteo::map::DisplaySettingsOperator::flushSettingsToFiles()
{//сохраняет настройки в те файлы, из которых они считывались
    bool ok = flushSettingsToFiles(citySettingsFileName_, displaySettingsFileName_);
    return ok;
}

bool meteo::map::DisplaySettingsOperator::flushSettingsToFiles(QString citySettingsProtoFileName, QString displaySettingsProtoFileName)
{
    meteo::map::proto::DisplaySettings displaySettings;
    meteo::map::proto::Cities cities;

    if ( cityMapModified_ == true ){
        QMap<int, meteo::map::proto::CitySetting>::Iterator mapRecord;
        mapRecord = citiesMap_.begin();
        while (mapRecord != citiesMap_.end()) {
            meteo::map::proto::CitySetting * citySetting = cities.add_city_setting();
            citySetting->CopyFrom(mapRecord.value());
            mapRecord++;
        }

        std::fstream file(citySettingsProtoFileName.toStdString().c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
        if (cities.SerializeToOstream(&file) == false){
            error_log << QObject::tr("Ошибка сохранения списка городов");
            return false;
        }
    }

    cityMapModified_ = false;

    if ( displaySettingListModified_ == true ){
        meteo::map::proto::DisplaySetting * defaultDispSetting = displaySettings.add_display_setting();
        defaultDispSetting->CopyFrom(defaultDisplaySetting_);

        QMap <meteo::map::proto::CityMode, meteo::map::proto::DisplaySetting>::Iterator cityModeDSMapIterator;
        cityModeDSMapIterator = cityModeDisplaySettingMap_.begin();
        while (cityModeDSMapIterator != cityModeDisplaySettingMap_.end()) {
                meteo::map::proto::DisplaySetting * cityModeDispSetting = displaySettings.add_display_setting();
                cityModeDispSetting->CopyFrom(cityModeDSMapIterator.value());
                cityModeDSMapIterator++;
        }

        QMap <int, meteo::map::proto::DisplaySetting>::Iterator cityDSMapIterator;
        cityDSMapIterator = cityDisplaySettingMap_.begin();
        while (cityDSMapIterator != cityDisplaySettingMap_.end()) {
                meteo::map::proto::DisplaySetting * cityDispSetting = displaySettings.add_display_setting();
                cityDispSetting->CopyFrom(cityDSMapIterator.value());
                cityDSMapIterator++;
        }

        std::fstream file(displaySettingsProtoFileName.toStdString().c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
        if (displaySettings.SerializeToOstream(&file) == false){
            error_log << QObject::tr("Ошибка сохранения списка настроек отображения");
            return false;
        }
    }
    displaySettingListModified_ = false;
    return true;
}

meteo::map::proto::DisplaySetting fillDisplaySetting
(const meteo::map::proto::DisplaySetting fillDispSetting, const meteo::map::proto::DisplaySetting baseDispSetting)
{
    //функция заполняет не установленные поля левого DisplaySetting (т.е. fillDispSetting)
    //соответствующими полями из правого DisplaySetting (т.е. baseDispSetting)
    //предполагается, что правый DisplaySetting заполнен полностью, т.е. не установленных полей нет
    //в противном случае поведение функции не определено
    //сказанное не затрагивает поля is_default, city_title и city_mode,
    //поскольку они определяют принадлежность настроек отображения, а не сами настройки

    meteo::map::proto::DisplaySetting returnDispSetting;

    //debug_log << returnDispSetting.Utf8DebugString();
    //debug_log << fillDispSetting.Utf8DebugString();
    //debug_log << baseDispSetting.Utf8DebugString();

    if ( !(fillDispSetting.has_generalization_level()) )
        returnDispSetting.set_generalization_level(baseDispSetting.generalization_level());
    else
        returnDispSetting.set_generalization_level(fillDispSetting.generalization_level());

    if ( !(fillDispSetting.has_position()) )
        returnDispSetting.set_position(baseDispSetting.position());
    else
        returnDispSetting.set_position(fillDispSetting.position());

    if ( !(fillDispSetting.has_priority()) )
        returnDispSetting.set_priority(baseDispSetting.priority());
    else
        returnDispSetting.set_priority(fillDispSetting.priority());

    if ( !(fillDispSetting.has_city_scale()) )
        returnDispSetting.set_city_scale(baseDispSetting.city_scale());
    else
        returnDispSetting.set_city_scale(fillDispSetting.city_scale());

    
    if ( !(fillDispSetting.has_visible()) )
        returnDispSetting.set_visible(baseDispSetting.visible());
    else
        returnDispSetting.set_visible(fillDispSetting.visible());

    if ( fillDispSetting.has_city_mark() == false ) {
        returnDispSetting.set_city_mark(baseDispSetting.city_mark());
    }
    else {
        returnDispSetting.set_city_mark(fillDispSetting.city_mark());
    }

    if ( !(fillDispSetting.has_always_visible()) )
        {}//поскольку "видимый всегда" - атрибут отдельного города и группа или умолчальные настройки не могу его иметь, то и дополнение не производим - либо копируем, либо нет
    else
        returnDispSetting.set_always_visible(fillDispSetting.always_visible());

    //простые параметры закончились, а сложные параметры сравнивать придется поэлементно

    if ( !(fillDispSetting.has_font()) ){//если шрифта нет совсем
        Font* font = returnDispSetting.mutable_font();
        font->CopyFrom(baseDispSetting.font());
    }
    else {//если какие-то элементы шрифта есть
        Font* font = returnDispSetting.mutable_font();
        Font fill_Font, baseFont, returnFont;
        baseFont = baseDispSetting.font();
        fill_Font = fillDispSetting.font();
        returnFont = fillFont(fill_Font, baseFont);
        font->CopyFrom(returnFont);
    }

    if ( !(fillDispSetting.has_pen()) ){
        Pen * pen = returnDispSetting.mutable_pen();
        pen->CopyFrom(baseDispSetting.pen());
    }
    else {
        Pen * pen = returnDispSetting.mutable_pen();
        Pen fill_Pen, basePen, returnPen;
        basePen = baseDispSetting.pen();
        fill_Pen = fillDispSetting.pen();
        returnPen = fillPen(fill_Pen, basePen);
        pen->CopyFrom(returnPen);
    }

    //не забываем про is_default, city_index и city_mode, которые при их наличии копируются из исходного (левого) набора
    if ( fillDispSetting.has_is_default() )
        returnDispSetting.set_is_default(fillDispSetting.is_default());

    if ( fillDispSetting.has_city_mode() )
        returnDispSetting.set_city_mode(fillDispSetting.city_mode());

    if ( fillDispSetting.has_city_index() )
        returnDispSetting.set_city_index(fillDispSetting.city_index());

    //debug_log << returnDispSetting.Utf8DebugString();
    //debug_log << fillDispSetting.Utf8DebugString();
    //debug_log << baseDispSetting.Utf8DebugString();

    return returnDispSetting;
}

const int ZERO_THRESHOLD = 0.00000000001;

bool isEqual(const GeoPointPb gp1, const GeoPointPb gp2)
{
    //TODO отключил пока сравнения по высоте и типу, т.к. при создании переменной добавляются значения по умолчанию и при сравнении дают ложноотрицательный результат
    /*debug_log << gp1.Utf8DebugString();
    debug_log << gp2.Utf8DebugString();

    if ( gp1.has_height_meters() != gp2.has_height_meters() )
        return false;
    if ( gp1.has_height_meters() ){
        if ( gp1.height_meters() != gp2.height_meters() )
            return false;
    }

    if ( gp1.has_type() != gp2.has_type() )
        return false;
    if ( gp1.has_type() ){
        if ( gp1.type() != gp2.type() )
            return false;
    }*/
    //TODO сравнения переменных типа float - через ZERO_THRESHOLD!

    if ( gp1.has_lat_radian() != gp2.has_lat_radian() )
        return false;
    if ( gp1.has_lat_radian() ){
        if ( fabs(gp1.lat_radian() - gp2.lat_radian()) > ZERO_THRESHOLD )
            return false;
    }

    if ( gp1.has_lon_radian() != gp2.has_lon_radian() )
        return false;
    if ( gp1.has_lon_radian() ){
        if ( fabs(gp1.lon_radian() - gp2.lon_radian()) > ZERO_THRESHOLD )
            return false;
    }

    if ( gp1.has_lat_deg() != gp2.has_lat_deg() )
        return false;
    if ( gp1.has_lat_deg() ){
        if ( fabs(gp1.lat_deg() - gp2.lat_deg()) > ZERO_THRESHOLD )
            return false;
    }

    if ( gp1.has_lon_deg() != gp2.has_lon_deg() )
        return false;
    if ( gp1.has_lon_deg() ){
        if ( fabs(gp1.lon_deg() - gp2.lon_deg()) > ZERO_THRESHOLD )
            return false;
    }

    return true;

}



meteo::map::proto::DisplaySetting differDisplaySetting
(const meteo::map::proto::DisplaySetting tocutDispSetting, const meteo::map::proto::DisplaySetting fullDispSetting)
{
    //функция сравнивает левый (т.е. tocutDispSetting) и правый (т.е. fullDispSetting) DisplaySetting'и
    //одинаковые поля выкидываются из левого
    //предполагается, что правый DisplaySetting заполнен полностью, т.е. не установленных полей нет
    //в противном случае поведение функции не определено
    //сказанное не затрагивает поля is_default, city_title и city_mode,
    //поскольку они определяют принадлежность настроек отображения, а не сами настройки

    meteo::map::proto::DisplaySetting returnDispSetting;

    //debug_log << returnDispSetting.Utf8DebugString();
    //debug_log << tocutDispSetting.Utf8DebugString();
    //debug_log << fullDispSetting.Utf8DebugString();

    if ( tocutDispSetting.has_generalization_level() &&
         tocutDispSetting.generalization_level() != fullDispSetting.generalization_level() )
        returnDispSetting.set_generalization_level(tocutDispSetting.generalization_level());

    if ( tocutDispSetting.has_priority() &&
         tocutDispSetting.priority() != fullDispSetting.priority() )
        returnDispSetting.set_priority(tocutDispSetting.priority());

    if ( tocutDispSetting.has_visible() &&
         tocutDispSetting.visible() != fullDispSetting.visible() )
        returnDispSetting.set_visible(tocutDispSetting.visible());

    if ( tocutDispSetting.has_position() &&
         tocutDispSetting.position() != fullDispSetting.position() )
        returnDispSetting.set_position(tocutDispSetting.position());

    if ( tocutDispSetting.has_city_mark() == true &&
         tocutDispSetting.city_mark() != fullDispSetting.city_mark() ) {
        returnDispSetting.set_city_mark(tocutDispSetting.city_mark());
    }

    if ( tocutDispSetting.has_city_scale() == true &&
         tocutDispSetting.city_scale() != fullDispSetting.city_scale() ) {
        returnDispSetting.set_city_scale(tocutDispSetting.city_scale());
    }
    
    if ( tocutDispSetting.has_always_visible() )//особая обработка - этот параметр не дифференцируется, т.к. группа его иметь не может
        returnDispSetting.set_always_visible(tocutDispSetting.always_visible());

    if ( tocutDispSetting.has_font() ){
        Font returnFont,
                tocutFont = tocutDispSetting.font(),
                fullFont = fullDispSetting.font();
        returnFont = differFont(tocutFont, fullFont);
        if ( isEmpty(returnFont) )
            returnDispSetting.clear_font();
        else {
            Font * font = returnDispSetting.mutable_font();
            font->CopyFrom(returnFont);
        }
    }

    if ( tocutDispSetting.has_pen() ){
        Pen returnPen,
                tocutPen = tocutDispSetting.pen(),
                fullPen = fullDispSetting.pen();
        returnPen = differPen(tocutPen, fullPen);
        if ( isEmpty(returnPen) )
            returnDispSetting.clear_pen();
        else {
            Pen * pen = returnDispSetting.mutable_pen();
            pen->CopyFrom(returnPen);
        }
    }

    //не забываем про is_default, city_index и city_mode, которые при их наличии копируются из исходного (левого) набора
    if ( tocutDispSetting.has_is_default() )
        returnDispSetting.set_is_default(tocutDispSetting.is_default());

    if ( tocutDispSetting.has_city_mode() )
        returnDispSetting.set_city_mode(tocutDispSetting.city_mode());

    if ( tocutDispSetting.has_city_index() )
        returnDispSetting.set_city_index(tocutDispSetting.city_index());

    return returnDispSetting;
}

// следующие 4 функции работают аналогично fillDisplaySetting и differDisplaySetting, но fillPen и differPen
// имеют особенность - они работают ТОЛЬКО С ЦВЕТОМ КАРАНДАША, стиль и толщина пока не реализованы, т.к. нужны ли?
Font fillFont
(const Font fillFont, const Font baseFont)
{
    Font returnFont;

    //debug_log << returnFont.Utf8DebugString();
    //debug_log << fillFont.Utf8DebugString();
    //debug_log << baseFont.Utf8DebugString();

    if ( !(fillFont.has_family()) )
        returnFont.set_family(baseFont.family());
    else
        returnFont.set_family(fillFont.family());

    if ( !(fillFont.has_pointsize()) )
        returnFont.set_pointsize(baseFont.pointsize());
    else
        returnFont.set_pointsize(fillFont.pointsize());

    if ( !(fillFont.has_weight()) )
        returnFont.set_weight(baseFont.weight());
    else
        returnFont.set_weight(fillFont.weight());

    if ( !(fillFont.has_bold()) )
        returnFont.set_bold(baseFont.bold());
    else
        returnFont.set_bold(fillFont.bold());

    if ( !(fillFont.has_italic()) )
        returnFont.set_italic(baseFont.italic());
    else
        returnFont.set_italic(fillFont.italic());

    if ( !(fillFont.has_underline()) )
        returnFont.set_underline(baseFont.underline());
    else
        returnFont.set_underline(fillFont.underline());

    if ( !(fillFont.has_strikeout()) )
        returnFont.set_strikeout(baseFont.strikeout());
    else
        returnFont.set_strikeout(fillFont.strikeout());

    //debug_log << returnFont.Utf8DebugString();
    //debug_log << fillFont.Utf8DebugString();
    //debug_log << baseFont.Utf8DebugString();

    return returnFont;
}


Font differFont
(const Font tocutFont, const Font fullFont)
{
    Font returnFont;

    //debug_log << returnFont.Utf8DebugString();
    //debug_log << tocutFont.Utf8DebugString();
    //debug_log << fullFont.Utf8DebugString();

    if ( tocutFont.has_family() &&
        tocutFont.family() != fullFont.family() )
        returnFont.set_family(tocutFont.family());

    if ( tocutFont.has_pointsize() &&
         tocutFont.pointsize() != fullFont.pointsize() )
        returnFont.set_pointsize(tocutFont.pointsize());

    if ( tocutFont.has_weight() &&
         tocutFont.weight() != fullFont.weight() )
        returnFont.set_weight(tocutFont.weight());

    if ( tocutFont.has_bold() &&
         tocutFont.bold() != fullFont.bold() )
        returnFont.set_bold(tocutFont.bold());

    if ( tocutFont.has_italic() &&
         tocutFont.italic() != fullFont.italic() )
        returnFont.set_italic(tocutFont.italic());

    if ( tocutFont.has_underline() &&
         tocutFont.underline() != fullFont.underline() )
        returnFont.set_underline(tocutFont.underline());

    if ( tocutFont.has_strikeout() &&
         tocutFont.strikeout() != fullFont.strikeout() )
        returnFont.set_strikeout(tocutFont.strikeout());

    //debug_log << returnFont.Utf8DebugString();
    //debug_log << tocutFont.Utf8DebugString();
    //debug_log << fullFont.Utf8DebugString();

    return returnFont;
}


Pen fillPen
(const Pen fillPen, const Pen basePen)
{
    Pen returnPen;

    //debug_log << returnPen.Utf8DebugString();
    //debug_log << fillPen.Utf8DebugString();
    //debug_log << basePen.Utf8DebugString();

    if ( !(fillPen.has_color()) )
        returnPen.set_color(basePen.color());
    else
        returnPen.set_color(fillPen.color());

    //debug_log << returnPen.Utf8DebugString();
    //debug_log << fillPen.Utf8DebugString();
    //debug_log << basePen.Utf8DebugString();

    return returnPen;
}


Pen differPen
(const Pen tocutPen, const Pen fullPen)
{
    Pen returnPen;

//    debug_log << returnPen.Utf8DebugString();
//    debug_log << tocutPen.Utf8DebugString();
//    debug_log << fullPen.Utf8DebugString();

    if ( tocutPen.has_color() &&
         tocutPen.color() != fullPen.color() )
        returnPen.set_color(tocutPen.color());


//    debug_log << returnPen.Utf8DebugString();
//    debug_log << tocutPen.Utf8DebugString();
//    debug_log << fullPen.Utf8DebugString();

    return returnPen;
}

}
}
