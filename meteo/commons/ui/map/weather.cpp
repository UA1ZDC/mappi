#include "weather.h"
#include "vbuilder.h"

#include <qdatetime.h>
#include <qobject.h>
#include <qdir.h>
#include <qfile.h>
#include <qstringlist.h>
#include <qbytearray.h>
#include <qprocess.h>

#include <sql/nosql/nosqlquery.h>

#include <cross-commons/debug/tlog.h>

#include <commons/textproto/tprototext.h>
#include <commons/textproto/pbtools.h>
#include <commons/funcs/tcolorgrad.h>
#include <commons/obanal/tfield.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/document_service.pb.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/sigwx.pb.h>
#include <meteo/commons/proto/services.pb.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/geogradient.h>


#include "document.h"
#include "layeriso.h"
#include "layerpunch.h"
#include "layermrl.h"
#include "layersigwx.h"
#include "puanson.h"
#include "map.h"
#include "aerodocument.h"
#include "vprofiledoc.h"
#include "formaldoc.h"
#include <meteo/commons/global/weatherloader.h>
#include "loader.h"

static const QString kObanaldb    = "meteodb";
static const QString kObanalcol   = "obanal";

static const QString kDocumentdb  = "documentdb";
static const QString kWeatherMaps = "weather_maps";

namespace meteo {
namespace map {

Weather::Weather()
  : isoparams_  (WeatherLoader::instance()->isolibrary())
  , punchparams_(WeatherLoader::instance()->punchlibrary())
{
}

Weather::~Weather() { }

bool Weather::createAd( Document* doc, const proto::Map& info )const {
  QList<Layer*> layers;
  AeroDocument* aero = document_cast<AeroDocument*>(doc);
  bool fl = buildAD( aero, info );
  if ( false == fl ) {
    error_log << QObject::tr("Аэрологическая диаграмма не построена!");
    return false;
  }
  for ( auto l : aero->layers() ) {
    if ( false == l->isBase() ) {
      layers.append(l);
    }
  }
  if ( 0 == layers.size() ) {
    debug_log << QObject::tr("Количество слоев в АД == 0");
    return false;
  }
  return true;
}


bool Weather::createVC( Document* doc, const proto::Map& info )const {
  VProfileDoc* vert = document_cast<VProfileDoc*>(doc);
  QList<Layer*> layers;
  bool fl = buildVProfile( vert, info );
  if ( false == fl ) {
    return false;
  }
  for ( auto l : vert->layers() ) {
    if ( false == l->isBase() ) {
      layers.append(l);
    }
  }
  if ( 0 == layers.size() ) {
    return false;
  }
  return true;
}

bool Weather::createMap( Document* doc, const proto::Map& cinfo )
{
  proto::Map info;
  info.CopyFrom(cinfo);
  QList<Layer*> layers;
  switch (info.document().doctype()) {
    case proto::kAeroDiagram:
      return createAd(doc,info);
    case proto::kVerticalCut:
      return createVC(doc,info);
    case proto::kFormalDocument: {
      if ( false == info.has_formal() ) { return false; }
      meteo::map::proto::StartArguments args;
      args.ParsePartialFromString(info.formal().proto_args());
      return  buildFormalDocument( document_cast<FormalDoc*>(doc), info );
    }
    case proto::kGeoMap:
    case proto::kOceanDiagram:
    case proto::kMeteogram:
      break;
  }

  for ( int i = 0, sz = info.data_size(); i < sz; ++i ) {
    proto::WeatherLayer* l = info.mutable_data(i);
    if ( true == info.has_center() && false == l->has_center() ) {
      l->set_center( info.center() );
    }
    if ( true == info.has_model() && false == l->has_model() ) {
      l->set_model( info.model() );
    }
  }

  for ( int i = 0, sz = info.data_size(); i < sz; ++i ) {
    proto::WeatherLayer layer = info.data(i);
    setLayerFromMap( info, &layer );
    if ( true == doc->hasLayer(layer) ) {
      continue;
    }
    Layer* l = nullptr;
    switch ( layer.mode() ) {
      case proto::kPuanson:
        l = buildPuansonLayer( doc, layer );
      break;
      case proto::kIsoline:
      case proto::kGradient:
      case proto::kIsoGrad:
        l = buildIsoLayer( doc, layer, info.spline_koef() );
      break;
      case proto::kRadar:
        l = buildMrlLayer( doc, layer );
      break;
      case proto::kEmptyLayer:
        l = buildEmptyLayer( doc, layer );
      break;
      case proto::kSigwx:
        l = buildSigwxLayer( doc, layer );
      break;
    }
    if ( nullptr != l ) {
      layers.append(l);
    }
  }
  //QString center = QString::number(info.center());
  QMap<int, QPair<QString, int>> centers = global::kMeteoCenters();
  if(centers.contains(info.center())) {
    info.set_center_title(  centers[info.center()].first.toStdString());
  }

  doc->setMap(info);
  if ( 0 == layers.size() ) {
    if ( true == info.allow_empty() ) {
      doc->addLabelToIncut(0, QObject::tr("emptymapdata"), QObject::tr("Данные отсутствуют"));
    }
    return false;
  }

  return true;
}

Layer* Weather::buildLayer( Document* doc, const proto::WeatherLayer& info, int spline_koef )
{
  Layer* l = nullptr;
  switch ( info.mode() ) {
    case proto::kPuanson:
      l = buildPuansonLayer( doc, info );
    break;
    case proto::kIsoline:
    case proto::kGradient:
    case proto::kIsoGrad:
      l = buildIsoLayer( doc, info, spline_koef );
    break;
    case proto::kRadar:
      l = buildMrlLayer( doc, info );
    break;
    case proto::kEmptyLayer:
      l = buildEmptyLayer( doc, info );
    break;
    case proto::kSigwx:
      l = buildSigwxLayer( doc, info );
    break;
  }
  return l;
}

Layer* Weather::buildPuansonLayer( Document* doc, const proto::WeatherLayer& info )
{
  QString template_name = QString::fromStdString( info.template_name() );
  meteo::puanson::proto::Puanson punch;
  if ( false == WeatherLoader::instance()->punchlibrary().contains(template_name) ) {
    error_log << QObject::tr("Не найден шаблон пуансона %1").arg(template_name);
    return nullptr;
  }

  proto::DataSource source = proto::kAuto;
  if ( true == info.has_source() && proto::kAuto != info.source() ) {
    source = info.source();
  }
  else {
    if ( global::kCenterWeather != info.center() || 0 != info.hour() ) {
      source = proto::kField;
    }
  }
  punch.CopyFrom(WeatherLoader::instance()->punchlibrary()[template_name]);
  QString srvname;
  meteo::surf::DataReply* reply = nullptr;
  switch (source) {
    case proto::kGribSource:
      reply = sendGribDataRequest(info);
      srvname = meteo::global::serviceTitle(meteo::settings::proto::kGrib);
    break;
    case proto::kField:
      reply = sendFieldDataRequest(info);
      srvname = meteo::global::serviceTitle(meteo::settings::proto::kField);
    break;
    default:
      reply = sendSurfaceDataRequest(info);
      srvname = meteo::global::serviceTitle(meteo::settings::proto::kSrcData);
    break;
  }

  if ( nullptr == reply ) {
    error_log << QObject::tr("Ответ от сервиса %1 не получен.").arg(srvname);
    return nullptr;
  }

  if ( 0 == reply->meteodata_size() ) {
    error_log << QObject::tr("Не удалось создать слой %1. Нет данных за выбранный период %2, срок %3.")
                 .arg(QString::fromStdString(info.template_name()))
                 .arg(QString::fromStdString(info.datetime()))
                 .arg(info.hour());
    delete reply;
    return nullptr;
  }
  LayerPunch* l = new LayerPunch(doc, punch);
  l->setInfo(info);
  l->setData(*reply);

  delete reply;
  return l;
}

Layer* Weather::buildIsoLayer( Document* doc, const proto::WeatherLayer& info, int spline_koef )
{
  QString template_name = QString::fromStdString( info.template_name() );
  if ( false == WeatherLoader::instance()->isolibrary().contains(template_name) ) {
    error_log << QObject::tr("Не найден шаблон изолинии %1").arg(template_name);
    return nullptr;
  }
  proto::FieldColor isoparam = WeatherLoader::instance()->isolibrary()[template_name];
  return buildIsoLayer( doc, info, spline_koef, isoparam );
}

Layer* Weather::buildIsoLayer( Document* doc, const proto::WeatherLayer& info,
                               int spline_koef, const proto::FieldColor& isoparam )
{
  rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kField );
  if ( nullptr == ch ) {
    error_log << QObject::tr("Не удалось установить связь с сервисом полей");
    return nullptr;
  }
  meteo::field::DataRequest request;
  QDateTime dt = QDateTime::fromString(QString::fromStdString(info.datetime()), Qt::ISODate);
  //QDateTime dt = TSqlQuery::datetimeFromString( info.datetime() );

  // request.set_date_start(dt.addSecs(info.hour()*3600).toString( Qt::ISODate ).toStdString() );
  request.set_date_start(dt.toString( Qt::ISODate ).toStdString() );
  //  request.set_date_end( info.datetime() );
  request.add_meteo_descr( isoparam.descr() );
  request.add_hour( info.hour()*3600 );
  if ( true == info.has_model() ) {
    request.set_model( info.model() );
  }
  request.add_level( info.level() );
  request.add_center( info.center() );
  request.add_type_level( info.type_level() );
  if ( true == info.has_net_type() ) {
    request.set_net_type( info.net_type() );
  }
  meteo::field::DataReply* reply = ch->remoteCall( &meteo::field::FieldService::GetFieldData, request, 50000);
  delete ch;
  if (nullptr == reply ) {
    error_log << QObject::tr("Ответ от севиса полей не получен");
    return nullptr;
  }
  if ( false == reply->result() ) {
    /*debug_log << QObject::tr("Нет данных по запросу")
    .arg( QString::fromStdString( reply->error() ) );*/
    delete reply;
    return nullptr;
  }
  if ( false == reply->has_fielddata()) {
    delete reply;
    return nullptr;
  }
  obanal::TField* field = new obanal::TField;
  QByteArray arr(reply->fielddata().data(), reply->fielddata().size());
  field->setDbId(QString::fromStdString(reply->descr().id()));
  delete reply;

  if ( false == field->fromBuffer(&arr) ) {
    error_log << QObject::tr("Ошибка извлечения поля из массива байтов");
    delete field;
    return nullptr;
  }

  if (0 != spline_koef && 0 != field->smootchField( spline_koef*10.0 ) ) {

    warning_log << QObject::tr("Ошибка сглаживания изолиний с коэффициентом %1").arg(spline_koef);
  }
  LayerIso* l = new LayerIso( doc, isoparam, field );
  proto::WeatherLayer copyinfo;
  copyinfo.CopyFrom(info);
  copyinfo.set_center_name( global::kMeteoCenters()[copyinfo.center()].first.toStdString() );
  l->setInfo(copyinfo);
  bool ok = false;
  if ( proto::kIsoline == copyinfo.mode()
       || proto::kIsoGrad == copyinfo.mode() ) {
    int iso_count = l->addIsoLines(&ok);
    if ( false == ok ) {
      warning_log << QObject::tr("Неизвестная ошибка при попытке построить изолинии");
    }
    if ( 0 == iso_count ) {
      warning_log << QObject::tr("Количество созданных изолиний = 0. Проверьте параметры изолиний %1")
                     .arg( QString::fromStdString( info.template_name() ) );
    }
    if ( 0 != iso_count && true == ok ) {
      l->addExtremums();
    }
  }
  if ( proto::kGradient == copyinfo.mode()
       || proto::kIsoGrad == copyinfo.mode() ) {
    l->addGradient();

    std::unordered_set<GeoGradient*> grad = l->objectsByType<GeoGradient*>();
    for ( auto o: grad ) {
      o->setAlphaPercent(info.transparency());
    }
  }
  return l;
}

Layer* Weather::buildMrlLayer( Document* doc, const proto::WeatherLayer& info )
{
  LayerMrl* l = nullptr;
  QMap< QString, proto::RadarColor > radarlib = WeatherLoader::instance()->radarlibrary();
  QString radarname = QString::fromStdString( info.template_name() );
  if ( false == radarlib.contains( radarname ) ) {
    error_log << QObject::tr("Не найден шаблон для наноски радиолокационных данных = %1")
                 .arg(radarname);
    return l;
  }
  if ( true == info.has_center() &&  info.center() != global::kCenterWeather ) {
    return l;
  }
  if ( true == info.has_model() &&  info.center() != global::kModelSurface ) {
    return l;
  }
  const proto::RadarColor& rclr = radarlib[radarname];
  surf::DataRequest request;
  request.set_date_start( info.datetime() );
  request.add_meteo_descr( rclr.descr() );
  request.set_level_h( info.h1() );
  request.set_level_h2( info.h2() );
  request.add_type(surf::kRadarMapType);
  meteo::rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kSrcData );
  if ( nullptr == ch ) {
    error_log << QObject::tr("Не удалось подключиться к сервису данных");
    return l;
  }
  surf::ManyMrlValueReply* response = ch->remoteCall(
                                        &surf::SurfaceService::GetMrlDataOnAllStation,
                                        request,
                                        50000
                                        );
  delete ch; ch = nullptr;
  if ( nullptr != response ) {
    if ( 0 != response->values_size() ) {
      l = new LayerMrl( doc, rclr );
      l->setInfo(info);
      l->setProtoRadar(*response);
    }
    else {
      debug_log << QObject::tr("Пустой ответ с радарами от сервиса данных");
    }
  }
  delete response;
  return l;
}

Layer* Weather::buildSigwxLayer( Document* doc, const proto::WeatherLayer& info )
{
  surf::SigwxDesc sgwx = layer2sigwx( info );
  sigwx::SigWx* reply = nullptr;

  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel(meteo::settings::proto::kSrcData );
  if ( nullptr == ctrl ) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных");
    return nullptr;
  }
  reply = ctrl->remoteCall( &surf::SurfaceService::GetSigwx, sgwx, 100000);
  delete ctrl; ctrl = nullptr;
  if ( nullptr == reply ) {
    error_log << QObject::tr("Ответ от сервиса данных не получен.");
    return nullptr;
  }

  if ( false == reply->result() ) {
    debug_log << QObject::tr("Не удалось создать слой %1. Нет данных за выбранный период.")
                 .arg(QString::fromStdString(info.template_name()));
    delete reply;
    return nullptr;
  }
  LayerSigwx* l = new LayerSigwx(doc);
  l->setInfo(info);
  l->setData(*reply);

  delete reply;
  return l;
}

Layer* Weather::buildEmptyLayer( Document* doc, const proto::WeatherLayer& info )
{
  Layer* l = new Layer( doc, QString::fromStdString( info.layer_name_pattern() ) );
  return l;
}

bool Weather::buildADPoSurf(zond::PlaceData *place, const proto::Map& map , QString *error) const
{
  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
  if ( nullptr == ctrl ) {
    if(nullptr != error) {
      *error = QObject::tr("Не удалось установить соденинение с сервисом данных.");
    }
    error_log <<  QObject::tr("Не удалось установить соденинение с сервисом данных.");
    return false;
  }

  surf::DataRequest request;
  request.add_type(surf::kAeroType);
  request.set_date_start( map.datetime() );
  request.set_date_end(map.datetime());
  request.clear_station();
  request.add_station( map.station(0).station() );
  //debug_log<<request.DebugString();
  //    request.set_only_last(1);
  auto reply = ctrl->remoteCall( &surf::SurfaceService::GetTZondOnStation, request, 20000 );
  delete ctrl;
  if ( nullptr == reply ) {
    if(nullptr != error) {
      *error = QObject::tr("Не удалось получить ответ от сервиса данных на запрос GetTZondOnStation = %1")
               .arg( QString::fromStdString( request.Utf8DebugString() ) );
    }
    error_log << QObject::tr("Не удалось получить ответ от сервиса данных на запрос GetTZondOnStation = %1")
                 .arg( QString::fromStdString( request.Utf8DebugString() ) );
    return false;
  }
  if ( 0 == reply->data_size() ) {
    if(nullptr != error) {
      *error = QObject::tr("Пустой ответ на запрос зонда = %1")
               .arg( QString::fromStdString( request.Utf8DebugString() ) );
    }
    debug_log << QObject::tr("Пустой ответ на запрос зонда = %1")
                 .arg( QString::fromStdString( request.Utf8DebugString() ) );
    delete reply;
    return false;
  }
  if ( 1 < reply->data_size() ) {
    warning_log << QObject::tr("В ответе зондов > 1. Был запрошен один.");
    delete reply;
    return false;
  }
  place->setIndex( QString::fromStdString( map.station(0).station() ).toInt() );
  place->setEnName(QString::fromStdString( map.station(0).name().international() ) );
  place->setRuName(QString::fromStdString( map.station(0).name().rus() ) );
  place->setName(QString::fromStdString( map.station(0).name().short_() ));

  place->setCoord(GeoPoint(map.station(0).position().lat_radian(),
                           map.station(0).position().lon_radian(),
                           map.station(0).position().height_meters() ) );
  for ( int i = 0, sz = reply->data_size(); i < sz; ++i ) {
    QByteArray buf( reply->data(i).meteodata().data(), reply->data(i).meteodata().size() );
    place->setZond(buf);
    place->zond().preobr(true);
    place->setDt(place->zond().dateTime());
  }
  delete reply;
  return true;
}

bool Weather::buildADPoField(zond::PlaceData *place, const proto::Map& map , QString *error) const
{
  field::DataRequest request;

  //request.set_only_last(1);
  rpc::Channel* ctrl_field = global::serviceChannel(settings::proto::kField);
  if(nullptr == ctrl_field ) {
    error_log << msglog::kNoConnect.arg(global::serviceTitle(settings::proto::kField));
    if(nullptr != error) {
      *error = msglog::kNoConnect.arg(global::serviceTitle(settings::proto::kField));
    }
    return false;
  }
  QDateTime forecast_start = pbtools::toQDateTime(map.datetime());
  forecast_start=forecast_start.addSecs(map.hour()*3600);
  request.set_forecast_start(pbtools::toString(forecast_start));
  request.set_forecast_end(pbtools::toString(forecast_start));
  // request.add_hour(map.hour()*3600);
  request.set_only_best(true);
  request.set_only_last(true);
  surf::Point* p = request.add_coords();
  p->set_fi(map.document().map_center().lat_radian());
  p->set_la(map.document().map_center().lon_radian());
  p->set_height(map.document().map_center().height_meters());
  if(true == map.has_center()){
    request.add_center(map.center());
  }

  // debug_log << request.Utf8DebugString();
  surf::TZondValueReply* reply = ctrl_field->remoteCall(&field::FieldService::GetADZond, request, 240000);
  delete ctrl_field;ctrl_field=nullptr;
  if ( nullptr == reply ) {
    error_log << QObject::tr("Не удалось получить ответ от сервиса доступа к результатам ОА на запрос GetADZond = %1")
                 .arg( QString::fromStdString( request.Utf8DebugString() ) );
    if(nullptr != error) {
      *error = QObject::tr("Не удалось получить ответ от сервиса доступа к результатам ОА на запрос GetADZond = %1")
               .arg( QString::fromStdString( request.Utf8DebugString() ) );
    }
    return false;
  }
  if ( 0 == reply->data_size() ) {
    if(nullptr != error) {
      *error = QObject::tr("Пустой ответ на запрос зонда ");// .arg( QString::fromStdString( request.Utf8DebugString() ) );
    }
    error_log << QObject::tr("Пустой ответ на запрос зонда ");// .arg( QString::fromStdString( request.Utf8DebugString() ) );
    delete reply;
    return false;
  }
  if ( 1 < reply->data_size() ) {
    warning_log << QObject::tr("В ответе зондов > 1. Был запрошен один.");
    delete reply;
    return false;
  }
  place->setIndex( QString::fromStdString( map.station(0).station() ).toInt() );
  place->setEnName(QString::fromStdString( map.station(0).name().international() ) );
  place->setRuName(QString::fromStdString( map.station(0).name().rus() ) );
  place->setName(QString::fromStdString( map.station(0).name().short_() ));
  place->setCoord(GeoPoint(map.document().map_center().lat_radian(),
                           map.document().map_center().lon_radian(),map.document().map_center().height_meters() ) );

  //debug_log << reply->DebugString();
  for ( int i = 0, sz = reply->data_size(); i < sz; ++i ) {
    QByteArray buf( reply->data(i).meteodata().data(), reply->data(i).meteodata().size() );
    place->setZond(buf);
    place->zond().preobr();
    place->setDt(place->zond().dateTime());
  }
  delete reply;
  return true;
}

bool Weather::buildADPoSat(zond::PlaceData *place, const proto::Map& map , QString *error) const
{
  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
  if ( nullptr == ctrl ) {
    if(nullptr != error) {
      *error = QObject::tr("Не удалось установить соденинение с сервисом данных.");
    }
    error_log <<  QObject::tr("Не удалось установить соденинение с сервисом данных.");
    return false;
  }

  surf::DataRequest request;
  request.add_type(surf::kSatAero);
  request.set_date_start( map.datetime() );
  request.set_date_end(map.datetime());
  request.clear_station();
  request.add_station( map.station(0).station() );
  //debug_log<<request.DebugString();
  //    request.set_only_last(1);
  auto reply = ctrl->remoteCall( &surf::SurfaceService::GetTZondOnStation, request, 20000 );
  delete ctrl;
  if ( nullptr == reply ) {
    if(nullptr != error) {
      *error = QObject::tr("Не удалось получить ответ от сервиса данных на запрос GetTZondOnStation = %1")
               .arg( QString::fromStdString( request.Utf8DebugString() ) );
    }
    error_log << QObject::tr("Не удалось получить ответ от сервиса данных на запрос GetTZondOnStation = %1")
                 .arg( QString::fromStdString( request.Utf8DebugString() ) );
    return false;
  }
  if ( 0 == reply->data_size() ) {
    if(nullptr != error) {
      *error = QObject::tr("Пустой ответ на запрос зонда = %1")
               .arg( QString::fromStdString( request.Utf8DebugString() ) );
    }
    debug_log << QObject::tr("Пустой ответ на запрос зонда = %1")
                 .arg( QString::fromStdString( request.Utf8DebugString() ) );
    delete reply;
    return false;
  }
  if ( 1 < reply->data_size() ) {
    warning_log << QObject::tr("В ответе зондов > 1. Был запрошен один.");
    delete reply;
    return false;
  }
  place->setIndex( QString::fromStdString( map.station(0).station() ).toInt() );
  place->setEnName(QString::fromStdString( map.station(0).name().international() ) );
  place->setRuName(QString::fromStdString( map.station(0).name().rus() ) );
  place->setName(QString::fromStdString( map.station(0).name().short_() ));
  place->setCoord(GeoPoint(map.document().map_center().lat_radian(),
                           map.document().map_center().lon_radian() ) );
  for ( int i = 0, sz = reply->data_size(); i < sz; ++i ) {
    QByteArray buf( reply->data(i).meteodata().data(), reply->data(i).meteodata().size() );
    place->setZond(buf);
    place->zond().preobr(true);
    place->setDt(place->zond().dateTime());
  }
  delete reply;
  return true;
}

bool Weather::buildAD(AeroDocument* doc, const proto::Map& map , QString *error) const
{
  if ( nullptr == doc ) {
    if(nullptr != error) {*error = QObject::tr("Параметр doc == nullptr."); }
    error_log << QObject::tr("Параметр doc == nullptr.");;
    return false;
  }
  if ( 0 == map.station_size() ) {
    if(nullptr != error) {*error = QObject::tr("Не указана станция, по которой необходимо построить аэрологическую диаграмму"); }
    error_log <<QObject::tr("Не указана станция, по которой необходимо построить аэрологическую диаграмму");;
    return false;
  }
  doc->setMap(map);
  zond::PlaceData place;
  if (map.source() == meteo::map::proto::DataSource::kSurface) {
    if(false == buildADPoSurf(&place,map,error)){
      return false;
    }
  }
  else if (map.source() == meteo::map::proto::DataSource::kField){
    if(false == buildADPoField(&place,map,error)){
      return false;
    }
  }
  else if (map.source() == meteo::map::proto::DataSource::kSatSource){
    if(false == buildADPoSat(&place,map,error)){
      return false;
    }
  }
  if ( false == doc->create(place) ) {
    if(nullptr != error) {
      *error =  QObject::tr("Невозможно построить диаграмму по %1 за %2")
                .arg(place.ruName())
                .arg(QString::fromStdString(map.datetime()));
    }
    return false;
  }
  if(nullptr != error) {
    *error = "";
  }
  return true;
}

bool Weather::buildVProfile( VProfileDoc* doc, const proto::Map& map ) const
{
  VBuilder builder(doc);
  return builder.createVProfile(map);
}

bool Weather::buildFormalDocument(FormalDoc *doc, const proto::Map &map)
{
  if ( nullptr == doc ) {
    error_log << QObject::tr("Параметр doc == nullptr.");
    return false;
  }
  if ( false == map.formal().has_module_path() ) {
    error_log << QObject::tr("Не указан сценарий генерации формализованного документа.");
    return false;
  }

  doc->setMap(map);

  std::string str;
  if ( true == map.formal().has_proto_args() ) {
    str = map.formal().proto_args();
  }
  else {
    meteo::map::proto::StartArguments sa;
    
    if ( map.station_size()>0 ){
      auto station = sa.add_stations();
      station->set_index(map.station(0).station());
    }
    // если установлено, что данные берем по группе регионов, то выставляем его
    if ( map.has_regiongroup_id() ){
      sa.set_regiongroup_id(  map.regiongroup_id() );
    }
    // индикаци
    if ( map.indication_size()>0 ){
      for (int i = 0; i < map.indication_size(); i++) {
        auto indication = sa.add_indication();
        indication->CopyFrom(  map.indication(i) );
      }      
    }
    if ( map.region_id_size()>0 ){
      for (int i = 0; i < map.region_id_size(); i++) {
        sa.add_region_id( map.region_id(i) );
      }
    }
    if ( map.has_center() ){
      sa.set_center(  map.center() );
    }
    
    sa.mutable_map()->CopyFrom( map );
    
    sa.set_datetime(map.datetime());
    if ( map.formal().has_forecast_hour() ) {
      sa.set_forecast(map.formal().forecast_hour());
    }
    str = sa.SerializeAsString();
  }

  QProcess pyScript;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("PYTHONPATH", MnCommon::libPath() );
  pyScript.setProcessEnvironment(env);
  QString path = QString("#!/usr/bin/python3 ") + MnCommon::pythonPath() +
                 QString::fromStdString(map.formal().module_path()) +
                 QString(" -m proto");
  pyScript.start(path);
  int32_t sz = str.size();
  pyScript.write( (const char*)(&sz), sizeof(sz) );
  pyScript.write(str.data(), str.size());
  pyScript.waitForFinished(1000000);

  QTextStream errSt(pyScript.readAllStandardError());
  QStringList errOutput;
  int i = 0;
  while ( false == errSt.atEnd() ) {
    errOutput << errSt.readLine();
    debug_log << errOutput[i];
    ++i;
  }

  QByteArray st(pyScript.readAllStandardOutput());
  proto::ExportResponse result;

  auto retFunc = [doc](const QString& err, bool res)
  {
    if ( false == err.isEmpty() ) {
      error_log << err;
      auto prop = doc->property();
      prop.add_messages(err.toStdString());
      doc->setProperty(prop);
    }
    return res;
  };

  if ( false == st.isEmpty() ) {
    bool ok = result.ParseFromString(st.toStdString());
    if ( false == ok ) {
      return retFunc(QObject::tr("Ошибка при разборе прото-структуры из сценария формирования документа"), ok);
    }
    QString comment;
    if ( true == result.has_comment() ) {
      comment = QString::fromStdString(result.comment());
    }
    if ( false == result.has_result() ) {
      return retFunc(comment, false);
    }
    if ( true == result.result() ) {
      doc->setFilename(QString::fromStdString(result.filename()));
      doc->setFile(QByteArray::fromBase64(QByteArray(result.data().data(), result.data().size())));
      return retFunc(comment, true);
    }
    else {
      return retFunc(comment, false);
    }
  }
  else {
    return retFunc(QObject::tr("Python-модуль не вернул ответ."), false);
  }
}

QString Weather::keyFromParameters( const QDateTime& dt,
                                    const QString& datakey,
                                    int center,
                                    int model,
                                    int level,
                                    int leveltype,
                                    int hour )
{
  return QString("%1::%2::%3::%4::%5::%6::%7")
      .arg( dt.toString( Qt::ISODate ) )
      .arg(datakey)
      .arg(center)
      .arg(model)
      .arg(level)
      .arg(leveltype)
      .arg(hour);
}

bool Weather::parametersFromKey( const QString& key,
                                 QDateTime* dt,
                                 QString* datakey,
                                 int* center,
                                 int* model,
                                 int* level,
                                 int* leveltype,
                                 int* hour )
{
  if ( nullptr == dt
       || nullptr == datakey
       || nullptr == center
       || nullptr == model
       || nullptr == level
       || nullptr == leveltype
       || nullptr == hour
       ) {
    error_log << QObject::tr("Нулевые указатели на параметры");
    return false;
  }
  QStringList list = key.split( "::", QString::KeepEmptyParts );
  if ( 7 != list.size() ) {
    error_log << QObject::tr("Не удалось получить значения параметров из ключа %1").arg(key);
    return false;
  }

  *dt = QDateTime::fromString(list[0], Qt::ISODate);
  //*dt = TSqlQuery::datetimeFromString(list[0]);
  if ( false == dt->isValid() ) {
    error_log << QObject::tr("Не удалось получить дату из строки %1").arg(list[0]);
    return false;
  }

  *datakey = list[1];
  *center = list[2].toInt();
  *model = list[3].toInt();
  *level = list[4].toInt();
  *leveltype = list[5].toInt();
  *hour = list[6].toInt();

  return true;
}

proto::MapList Weather::getAvailableMaps(const proto::Map& map, const QDateTime& dtbeg, const QDateTime& dtend)
{
  proto::MapList maplist;
  std::unique_ptr<Dbi> db(meteo::global::dbDocument());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Не удалось подключиться к БД");
    return maplist;
  }
  auto query = db->queryptrByName("get_available_maps");
  if(nullptr == query) {return maplist;}

  query->arg("start_dt",dtbeg);
  query->arg("end_dt",dtend);
  if ( true == map.has_name() ) {
    query->arg("map_template",map.name());

  }
  QString err;
  if ( false == query->execInit(&err) ) {
    //error_log << err;
    return maplist;
  }

  while ( true == query->next() ) {
    const ::meteo::DbiEntry&  doc = query->entry();
    proto::Map newmap = mapFromQuery( map, doc );
    maplist.add_map()->CopyFrom(newmap);
  }
  return maplist;
}

proto::MapList Weather::getAvailableDocuments( const proto::Map& map, const QDateTime& dtbeg, const QDateTime& dtend )
{
  proto::MapList maplist;

  std::unique_ptr<Dbi> db(meteo::global::dbDocument());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Не удалось подключиться к БД");
    return maplist;
  }
  auto query = db->queryptrByName("get_available_documents");
  if(nullptr == query) {return maplist;}

  query->arg("start_dt",dtbeg);
  query->arg("end_dt",dtend);
  if ( true == map.has_name() ) {
    query->arg("mapid",map.name());
  }
  if ( true == map.has_job_name() ) {
    query->arg("jobid",map.job_name());
  }
  if ( true == map.has_hour() ) {
    query->arg("hour",map.hour());
  }
  if ( true == map.has_format() ) {
    query->arg("format",map.format());
  }
  if ( true == map.has_center() ) {
    query->arg("center",map.center());
  }
  if ( true == map.has_model() ) {
    query->arg("model",map.model());
  }

  QStringList stlist;
  for( int i=0; i< map.station_size();++i ) {
    stlist.append(QString::fromStdString(map.station(i).station()));
  }

  if ( 0 != stlist.size() ) {
    query->arg("map_title",stlist);
  }


  QString error;
  if(false == query->execInit( &error)){
    //error_log <<error;
    return maplist;

  }

  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    proto::Map* map = maplist.add_map();
    map->set_name( doc.valueString("template").toStdString() );
    map->set_title( doc.valueString("title").toStdString() );
    map->set_datetime( doc.valueDt("dt").toString( Qt::ISODate ).toStdString() );
    map->set_hour( doc.valueInt32("hour") );
    map->set_center( doc.valueInt32("center") );
    map->set_center_title( doc.valueString("center_title").toStdString() );
    map->set_model( doc.valueInt32("model") );
    map->set_path( doc.valueString("path").toStdString() );
    map->set_format( static_cast<proto::DocumentFormat>( doc.valueInt32("format") ) );
    map->set_job_name( doc.valueString("job").toStdString() );
    map->set_job_title( doc.valueString("job_title").toStdString() );
    if ( true == doc.hasField("station") ) {
      map->add_station()->set_station(doc.valueString("station").toStdString() );
    }
    map->set_idfile( doc.valueString("idfile").toStdString() );
  }

  return maplist;
}

proto::Map Weather::mapFromQuery( const proto::Map& pattern, const ::meteo::DbiEntry& doc )
{
  proto::Map map;
  map.set_name( doc.valueString("_id.template").toStdString() );
  map.set_datetime( doc.valueDt("_id.dt").toString(Qt::ISODate).toStdString() );
  map.set_hour( doc.valueInt32("_id.hour")/3600 );
  map.set_center( doc.valueInt32("_id.center") );
  map.set_center_title( doc.valueString("_id.center_name").toStdString() );
  map.set_model( doc.valueInt32("_id.model") );
  Array layers = doc.valueArray("layer");
  while ( true == layers.next() ) {
    ::meteo::Document doc;
    if ( false == layers.valueDocument(&doc) ) {
      warning_log << QObject::tr("Не удалось получить документ из массива");
      continue;
    }
    std::string layername = doc.valueString("layer_name").toStdString();
    proto::WeatherLayer layer;
    bool fl = false;
    for ( int i = 0, sz = pattern.data_size(); i < sz; ++i ) {
      if ( pattern.data(i).template_name() == layername ) {
        layer.CopyFrom( pattern.data(i) );
        fl = true;
        break;
      }
    }
    if ( false == fl ) {
      error_log << QObject::tr("Не найден шаблон слоя с именем %1 в карте %2")
                   .arg( QString::fromStdString( layername ) )
                   .arg( QString::fromStdString( pattern.Utf8DebugString() ) );
      continue;
    }
    if(kLayerMrl == layer.type()) {
      layer.set_layer_name_pattern( LayerMrl::layerNamePattern().toStdString() );
    }
    layer.set_template_name( doc.valueString("layer_name").toStdString() );
    layer.set_level( doc.valueInt32("level") );
    layer.set_type_level( doc.valueInt32("type_level") );
    layer.set_source( static_cast<proto::DataSource>( doc.valueInt32("datasource") ) );
    layer.set_data_size( doc.valueInt32("point_size") );
    layer.set_center    (map.center());
    layer.set_model     (map.model());
    layer.set_datetime  (map.datetime());
    layer.set_hour      (map.hour());
    map.add_data()->CopyFrom(layer);
  }
  return map;
}

surf::DataReply* Weather::sendGribDataRequest(const proto::WeatherLayer& info)
{
  QDateTime dt = NosqlQuery::datetimeFromString( info.datetime() );
  if ( false == dt.isValid() ) {
    error_log << QObject::tr("Не удалось получить дату из строки %1").arg( QString::fromStdString( info.datetime() ) );
    return nullptr;
  }
  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
  if ( nullptr == ctrl ) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных");
    return nullptr;
  }

  meteo::surf::GribDataRequest request;
  request.set_date_start( dt.toString(Qt::ISODate).toStdString() );
  request.set_date_end( request.date_start() );
  if ( surf::kTermValue != info.query_type() ) {
    request.set_date_end( QDateTime( dt.date(), QTime(23,59,59) ).toString(Qt::ISODate).toStdString() );
  }
  ::grib::GribInfo* ginfo = request.mutable_info();
  if ( true == info.has_level() ) {
    ginfo->set_level( info.level() );
  }
  if ( true == info.has_type_level() ) {
    ginfo->set_level_type( info.type_level() );
  }
  if ( true == info.has_center() ) {
    ginfo->set_center(info.center());
  }
  if ( true == info.has_hour() ) {
    ginfo->mutable_hour()->set_hour(info.hour()*3600);
  }
  if ( true == info.has_model() ) {
    ginfo->set_model(info.model() );
  }
  if ( true == info.has_net_type() ) {
    ginfo->set_net_type( info.net_type() );
  }
  if ( true == info.has_step_lat() ) {
    ginfo->set_step_lat( info.step_lat() );
  }
  if ( true == info.has_step_lon() ) {
    ginfo->set_step_lon( info.step_lon() );
  }

  if ( true == info.has_forecast_beg() ) {
    ::grib::HourType* hour = ginfo->mutable_hour();
    hour->set_dt1( info.forecast_beg() );
  }
  if ( true == info.has_forecast_end() ) {
    ::grib::HourType* hour = ginfo->mutable_hour();
    hour->set_dt2( info.forecast_end() );
  }

  QString punchname = QString::fromStdString( info.template_name() );
  if ( false == WeatherLoader::instance()->punchlibrary().contains( punchname ) ) {
    error_log << QObject::tr("Не найден шаблон наноски %1").arg(punchname);
    delete ctrl;
    return nullptr;
  }

  const puanson::proto::Puanson& punch = WeatherLoader::instance()->punchlibrary()[punchname];

  QStringList descrs;
  for ( int i = 0, sz = punch.rule_size(); i < sz; ++i ) {
    QString id = QString::fromStdString( punch.rule(i).id().name() );
    if ( false == descrs.contains(id) ) {
      descrs.append(id);
    }
  }

  for ( auto d : descrs ) {
    request.mutable_info()->add_param( TMeteoDescriptor::instance()->descriptor(d) );
  }
  meteo::surf::DataReply* reply = nullptr;
  if ( meteo::map::proto::kGribSource == info.source()) {
    reply = ctrl->remoteCall( &meteo::surf::SurfaceService::GetMeteoDataFromGrib, request, 100000 );
  }
  if ( nullptr == reply ) {
    error_log << QObject::tr("Ответ от сервиса данных не получен.");
  }
  delete ctrl; ctrl = nullptr;
  return reply;
}


surf::DataReply* Weather::sendSurfaceDataRequest(const proto::WeatherLayer& info)
{
  QDateTime dt = NosqlQuery::datetimeFromString( info.datetime() );
  if ( false == dt.isValid() ) {
    error_log << QObject::tr("Не удалось получить дату из строки %1").arg( QString::fromStdString( info.datetime() ) );
    return nullptr;
  }
  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
  if ( nullptr == ctrl ) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных");
    return nullptr;
  }

  meteo::surf::DataRequest request;
  request.set_date_start( dt.toString(Qt::ISODate).toStdString() );
  request.set_date_end( request.date_start() );
  if ( surf::kTermValue != info.query_type() ) {
    request.set_date_end( QDateTime( dt.date(), QTime(23,59,59) ).toString(Qt::ISODate).toStdString() );
  }
  if ( true == info.has_level() ) {
    request.set_level_p( info.level() );
  }
  if ( true == info.has_type_level() ) {
    request.set_type_level( info.type_level() );
  }
  for( int i = 0, sz = info.data_type_size(); i < sz; ++i ){
    request.add_type(info.data_type(i));
  }

  QString punchname = QString::fromStdString( info.template_name() );
  if ( false == WeatherLoader::instance()->punchlibrary().contains( punchname ) ) {
    error_log << QObject::tr("Не найден шаблон наноски %1").arg(punchname);
    delete ctrl;
    return nullptr;
  }

  const puanson::proto::Puanson& punch = WeatherLoader::instance()->punchlibrary()[punchname];

  QStringList descrs;
  for ( int i = 0, sz = punch.rule_size(); i < sz; ++i ) {
    QString id = QString::fromStdString( punch.rule(i).id().name() );
    if ( false == descrs.contains(id) ) {
      descrs.append(id);
    }
  }
  for ( auto d : descrs ) {
    request.add_meteo_descrname( d.toStdString() );
  }

  meteo::surf::DataReply* reply = nullptr;

  if ( false == info.has_query_type() || surf::kTermValue == info.query_type() ) {
    reply = ctrl->remoteCall( &meteo::surf::SurfaceService::GetMeteoData, request, 100000 );
  }
  else {
    request.set_query_type( info.query_type() );
    reply = ctrl->remoteCall( &meteo::surf::SurfaceService::GetMeteoDataByDay, request, 300000 );
  }

  if ( nullptr == reply ) {
    error_log << QObject::tr("Ответ от сервиса данных не получен.");
  }
  delete ctrl; ctrl = nullptr;
  return reply;
}

surf::DataReply*Weather::sendFieldDataRequest(const proto::WeatherLayer& info)
{
  QDateTime dt = QDateTime::fromString(QString::fromStdString(info.datetime()), Qt::ISODate);
  //QDateTime dt = TSqlQuery::datetimeFromString( info.datetime() );
  if ( false == dt.isValid() ) {
    error_log << QObject::tr("Не удалось получить дату из строки %1").arg( QString::fromStdString( info.datetime() ) );
    return nullptr;
  }
  meteo::rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kField );
  if ( nullptr == ctrl ) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных");
    return nullptr;
  }
  meteo::field::DataRequest request;
  request.set_date_start( dt.toString( Qt::ISODate ).toStdString() );
  request.set_date_end( dt.toString(Qt::ISODate).toStdString() );
  // request.set_date_start( dt.addSecs(info.hour()*3600).toString( Qt::ISODate ).toStdString() );
  // request.set_date_end( dt.addSecs(info.hour()*3600).toString(Qt::ISODate).toStdString() );
  if ( true == info.has_level() ) {
    request.add_level( info.level() );
  }
  if ( true == info.has_type_level() ) {
    request.add_type_level( info.type_level() );
  }
  QString id = QString::fromStdString(info.template_name());
  QMap< QString, meteo::puanson::proto::Puanson > punchlib = WeatherLoader::instance()->punchlibrary();
  if ( false == punchlib.contains(id) ) {
    error_log << QObject::tr( "Не найден шаблон %1" ).arg(id);
    delete ctrl;
    return nullptr;
  }
  const meteo::puanson::proto::Puanson& punch = punchlib[id];
  for ( int i = 0, sz = punch.rule_size(); i < sz; ++i ) {
    request.add_meteo_descr( punch.rule(i).id().descr() );
  }
  request.add_hour( info.hour()*3600 );
  if ( true == info.has_model() ) {
    request.set_model( info.model() );
  }
  request.add_center( info.center() );
  if(info.has_net_type()&& 0 < info.net_type()){
    request.set_net_type( info.net_type() );
  }
  meteo::surf::DataReply* reply = ctrl->remoteCall( &meteo::field::FieldService::GetFieldsMData, request, 100000 );
  if ( nullptr == reply ) {
    error_log << QObject::tr("Ответ от сервиса полей не получен.");
  }
  delete ctrl;
  return reply;
}

proto::DataSource Weather::sourceFromViewModeAndSource( proto::DataSource source, proto::ViewMode mode )
{
  if ( proto::kAuto != source ) {
    return source;
  }
  if ( proto::kPuanson == mode ) {
    return proto::kSurface;
  }
  return proto::kField;
}

surf::SigwxDesc Weather::layer2sigwx( const proto::WeatherLayer& info )
{
  surf::SigwxDesc sigwx;
  sigwx.set_date( info.datetime() );
  if ( true == info.has_forecast_beg() ) {
    sigwx.set_forecast_beg( info.forecast_beg() );
  }
  else if ( true == info.has_datetime() ) {
    sigwx.set_forecast_beg( info.datetime() );
  }
  if ( true == info.has_forecast_end() ) {
    sigwx.set_forecast_end( info.forecast_end() );
  }
  else if ( true == info.has_hour() ) {
    QDateTime dt = QDateTime::fromString(QString::fromStdString(sigwx.forecast_beg()), Qt::ISODate);
    //QDateTime dt = TSqlQuery::datetimeFromString( sigwx.forecast_beg() );
    sigwx.set_forecast_end( pbtools::toString( dt.addSecs( info.hour()*3600 ).toString( Qt::ISODate ) ) );
  }
  sigwx.set_center( info.center() );
  sigwx.set_level_lo( info.h1() );
  sigwx.set_level_hi( info.h2() );
  sigwx.set_count( info.data_size() );
  return sigwx;
}

proto::WeatherLayer Weather::sigwx2layer( const surf::SigwxDesc& sigwx )
{
  proto::WeatherLayer info;
  info.set_mode( proto::kSigwx );
  info.set_source( proto::kSurface );
  info.set_center( sigwx.center() );
  info.set_center_name( global::kMeteoCenters()[info.center()].first.toStdString() );
  //set_model
  //set_level
  //set_type_level
  //set_template_name
  info.add_data_type( surf::kFlightSigWx );
  info.set_datetime( sigwx.date() );
  //set_hour
  info.set_data_size( sigwx.count() );
  info.set_type( kLayerSigwx );
  info.set_h1( sigwx.level_lo() );
  info.set_h2( sigwx.level_hi() );
  info.set_forecast_beg( sigwx.forecast_beg() );
  info.set_forecast_end( sigwx.forecast_end() );
  return info;
}

void Weather::setLayerFromMap( const proto::Map& info, proto::WeatherLayer* layer )
{
  if ( false == layer->has_center() ) {
    layer->set_center( info.center() );
  }
  if ( false == layer->has_center_name() ) {
    layer->set_center_name( info.center_title() );
  }

  if ( false == layer->has_model() && true == info.has_model() ) {
    layer->set_model( info.model() );
  }
  if ( false == layer->has_level() ) {
    layer->set_level( info.level() );
  }
  if ( false == layer->has_type_level() ) {
    layer->set_type_level( info.type_level() );
  }
  if ( false == layer->has_datetime() ) {
    layer->set_datetime( info.datetime() );
  }
  if ( false == layer->has_hour() ) {
    layer->set_hour( info.hour() );
  }
}

Document* Weather::documentFromProto( const proto::Document* req )
{

  QSize docsz(800, 600);
  QSize mapsz(800, 600);
  if ( true == req->has_docsize() ) {
    docsz = size2qsize(req->docsize());
    mapsz = docsz;
  }
  if ( true == req->has_mapsize() ) {
    mapsz = size2qsize(req->mapsize());
  }

  GeoPoint mc(0, 0);
  GeoPoint dc(0, 0);
  if ( true == req->has_map_center() ) {
    mc = pbgeopoint2geopoint(req->map_center());
    dc = mc;
  }
  if ( true == req->has_doc_center() ) {
    dc = pbgeopoint2geopoint(req->doc_center());
  }

  Document* doc = nullptr;
  if ( proto::kAeroDiagram == req->doctype() ) {
    doc = new AeroDocument(*req);
  }
  else if (proto::kVerticalCut == req->doctype() ) {
    doc = new VProfileDoc(*req);
  }
  else if (proto::kFormalDocument == req->doctype() ) {
    doc = new FormalDoc(*req);
  }
  else {
    doc = new Document(*req);
  }

  doc->init();
  if ( meteo::map::proto::kAeroDiagram != req->doctype()
       && meteo::map::proto::kVerticalCut != req->doctype() ) {
    if ( 0 != req->gridloader().size() ) {
      meteo::map::Loader::instance()->handleGrid(QString::fromStdString(req->gridloader()), doc);
    }
    meteo::map::Loader::instance()->handleStation(meteo::global::kStationLoaderCommon, doc);
    meteo::map::Loader::instance()->handleCities(meteo::global::kCitiesLoaderCommon, doc);
  }

  doc->setScale(req->scale());
  return doc;
}

}
}
