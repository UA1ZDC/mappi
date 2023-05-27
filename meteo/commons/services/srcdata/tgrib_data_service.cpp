#include "tgrib_data_service.h"

#include <float.h>
#include <qmap.h>
#include <qprocess.h>
#include <qfile.h>
#include <qstringlist.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>

#include <cross-commons/debug/tlog.h>


#include <commons/meteo_data/meteo_data.h>
#include <commons/obanal/obanal_struct.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/tgribformat.pb.h>
#include <meteo/commons/grib/parser/tgridgrib.h>
#include <meteo/commons/primarydb/gribmongo.h>
#include <sql/nosql/nosqlquery.h>
#include <sql/nosql/document.h>


static const QString kDbName         = "meteodb";
static const QString kGribColl       = "grib";
static const QString kParametersColl = "parameters";

namespace meteo {
namespace surf {

TGribDataService::TGribDataService(google::protobuf::Closure* done)
  : TBaseDataService(done),
    iface_(TGribIface(0))
{
}

TGribDataService::~TGribDataService()
{
}


void TGribDataService::run()
{
  //QTime ttt; ttt.start();

  bool ret_val = false;

  switch(req_type_)
  {
  case rGetNotAnalysedGribData:
    if(nullptr != grib_data_reply_) {
      ret_val= getNotAnalysedGribData(grib_data_reply_);
    }
  break;
  case rRemoveGribDataAnalysed:
    if(nullptr != simple_data_request_ && nullptr != simple_data_reply_) {
      ret_val= markGribDataAnalysed(simple_data_request_,simple_data_reply_);
    }
  break;
  case rGetGribData:
    if ( nullptr != grib_data_request_ && nullptr != grib_data_reply_ ) {
      ret_val = getGribData(grib_data_request_, grib_data_reply_);
    }

  case rGetGribMeteoData:
    if ( nullptr != grib_data_request_ && nullptr != data_reply_ ) {
      ret_val = getGribMeteoData(grib_data_request_, data_reply_);
    }

  case rGetGribAvailable:
    if ( nullptr != data_request_ && nullptr != data_avail_reply_ ) {
      ret_val = getAvailableGrib(data_request_, data_avail_reply_);
    }

  default:
  break;
  }
  if(false == ret_val) {
    //TODO
  }
  // int cur =  ttt.elapsed();
  // info_log << QObject::tr("Время обработки запроса TGribDataService: %1 мсек").arg(cur);

  emit finished(this);
}

bool TGribDataService::markGribDataAnalysed(const QStringList &idList ){
  if (idList.isEmpty()) {
    error_log << QObject::tr("Удаление проанализированный грибов: пустой список ID.");
    return false;
  }

  static const QString queryName = QObject::tr("update_grib_set_analysed_true");

  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() )  {
    error_log << msglog::kDbConnectFailed;
    return false;
  }

  auto query = db->queryptrByName("update_grib_set_analysed_true");
  if(nullptr == query) {
    error_log << msglog::kDbRequestNotFound.arg(queryName);
    return false;
  }

  query->argOid("id",idList);

  if ( false == query->exec() ) {
    error_log << msglog::kDbRequestFailed << query->query();
    return false;
  }
  return true;
}

bool TGribDataService::markBrokenGrib(const QString& gribId, const QString& comment)
{
  if ( true == gribId.isEmpty() || true == comment.isEmpty()){
    error_log << QObject::tr("Недопустымые аргументы функции");
    return false;
  }


  std::unique_ptr<Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Невозможно установить соединение с базой данных");
    return false;
  }
  auto query = db->queryptrByName("update_grib_set_error_true_with_comment");
  if(nullptr == query) {return false;}
  query->argOid("id",gribId);
  query->arg("comment",comment);

  if ( false == query->exec() ) {
    error_log << QObject::tr("Не удается выполнить запрос в базу данных");
    return false;
  }
//  const DbiEntry& doc = query->result();
//  if ( 1 != static_cast<int>(doc.valueDouble("ok")) ){
//    error_log << QObject::tr("Запрос в базу данных выполнен с ошибкой");
//    return false;
//  }
  return true;
}

bool TGribDataService::markGribDataAnalysed(const SimpleDataRequest* req, SimpleDataReply* resp)
{
 // QTime ttt; ttt.start();
  QStringList toRemove;
  for (const std::string& id :req->id()){
    toRemove.append(QString::fromStdString(id));
  }
  int removeCount = this->markGribDataAnalysed(toRemove);
  resp->set_id(0);
  resp->set_result(removeCount);
  //info_log << QObject::tr("markGribDataAnalysed завершен за %1").arg(ttt.elapsed());
  return true;
}

bool TGribDataService::getNotAnalysedGribData(GribDataReply* res)
{
  QTime ttt; ttt.start();

  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() ) {
    res->clear_info();
    res->set_comment( QObject::tr("Не удалось подключиться к базе данных").toStdString() );
    res->set_result(false);
    return false;
  }

  GribMongoDb gribdb;
  ::grib::TGribData grib;
  QList< QPair<QString,QString> > grib_list;

  if ( false == gribdb.getNotAnalysedGribs( &grib_list, res->mutable_info() ) ) {
    res->set_comment( msglog::kDataServiceNoData.toStdString() );
    res->set_finished(true);
    return false;
  }

  if (grib_list.isEmpty()) {
    res->set_finished(true);
    return true;
  }

  for ( const QPair<QString,QString>& gribs : grib_list) {
    QString oid = gribs.first;
    QString fileId = gribs.second;
    if ( false == gribdb.readFromDb( fileId, &grib ) ) {
      markBrokenGrib(oid, QObject::tr("Отсутствует файл в GRIDfs"));
      error_log << msglog::kDbRequestFailed;
      res->Clear();
      res->set_comment( msglog::kDbRequestFailed.toStdString() );
      res->set_result(false);
      return false;
    }
    iface_.setGrib(&grib);
    GribData* gdata = res->add_data();
    const TGrid::LatLon* gridLatLon = static_cast<const TGrid::LatLon*>( iface_.grid().definition );
    if ( 0 != iface_.grid().type ) {
      markBrokenGrib(oid, "iface_.grib().type() != 0");
      continue;
    }
    if ( nullptr == gridLatLon ) {
      markBrokenGrib(oid, "nullptr == gridLatLon");
      continue;
    }

    gdata->mutable_grid_def()->CopyFrom(*gridLatLon);
    if(false == iface_.fillData(gdata->mutable_value())) {
      markBrokenGrib(oid, "false == iface_.fillData(gdata->mutable_value())");
      continue;
    }
    if(false == iface_.fillPoints(gdata->mutable_points())) {
      //      continue;
    }
    if (true == iface_.isBitmap()) {
      gdata->set_bitmap(grib.bitmap().bitmap());
    }

    if (res->info().param_size() != 0) {
      gdata->set_descr(res->info().param(0));
    }
    res->add_ids(oid.toStdString());
  }

  int cur =  ttt.elapsed();
  if(1000 < cur){
    debug_log << msglog::kServiceRequestTime.arg("getOneGribData").arg(cur);
  }
  res->set_comment(msglog::kServerAnswerOK.arg("getOneGribData").toStdString());
  res->set_result(true);
  //debug_log << "GetNotAnalysedGribData finished in " << cur;
  return true;
}

bool TGribDataService::getAvailableGrib(const DataRequest* req, GribDataAvailableReply* res)
{
  auto return_func = [res](QString err)
  {
    error_log << err;
    res->set_result(false);
    res->set_comment(err.toStdString());
    return false;
  };

  QTime ttt; ttt.start();

  if (!req->has_date_start() || !req->has_date_start()) {
    return return_func(QObject::tr("Не указан интервал времени для выборки данных"));
  }

  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() ) {
    return return_func(QObject::tr("Не удалось подключиться к базе данных"));
  }

  auto query = db->queryptrByName("get_available_grib");
  if ( nullptr == db.get() ) {
    return return_func(QObject::tr("Не найден запрос get_available_grib"));
  }

  if (req->has_type_level()) {
    query->arg("level_type", req->type_level());
  }
  if (req->has_level_p()) {
    query->arg("level",req->level_p());
  }

  if (req->has_hour()) {
    query->arg("hour",req->hour());
  }
  if (req->has_center()) {
    query->arg("center",req->center());
  }

  if (true == req->has_date_start() ) {
    query->argDt("start_dt",req->date_start());
  }
  if( true == req->has_date_end() ) {
    query->argDt("end_dt",req->date_end());
  }
  if (req->meteo_descr_size() != 0) {
    query->arg("descr",req->meteo_descr());
  } else {
    if (req->meteo_descrname_size() != 0) {
      QList<descr_t> descr;
      for (auto descrname : req->meteo_descrname()) {
        descr.append(TMeteoDescriptor::instance()->descriptor(descrname));
      }
      query->arg("descr", descr);
    }
  }
  if(true == req->has_model()){
    query->arg("model",req->model());
  }
  if(true == req->has_net_type()){
    QList<int> ntypes;
    ntypes<<req->net_type();
    query->arg("net_type",ntypes);
  }


  if(true == req->has_step_lat()){
    query->arg("step_lat",req->step_lat());
  }
  if(true == req->has_step_lon()){
    query->arg("step_lon",req->step_lon());
  }

  if (true == req->has_forecast_beg() ) {
    query->argDt("forecast_beg",req->forecast_beg());
  }
  if( true == req->has_forecast_end() ) {
    query->argDt("forecast_end",req->forecast_end());
  }


  QString error;
  if (false == query->execInit( &error)) {
    return return_func(error);
  }
  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    ::grib::GribInfo* info = res->add_info();

    info->set_center(doc.valueInt32("center"));
    info->set_level(doc.valueDouble("level"));
    info->set_level_type(doc.valueInt32("level_type"));
    info->set_model(doc.valueInt32("model"));

    QDateTime dt = doc.valueDt("dt");
    info->set_date(dt.toString(Qt::ISODate).toStdString());

    int hour = doc.valueInt32("hour") / 3600; //в часы
    ::grib::HourType* ht = info->mutable_hour();
    ht->set_hour(hour);
    dt = doc.valueDt("dt1");
    ht->set_dt1( dt.toString(Qt::ISODate).toStdString() );
    dt = doc.valueDt("dt2");
    ht->set_dt2( dt.toString(Qt::ISODate).toStdString() );
    ht->set_time_range( doc.valueInt32("time_range") );
    int stepLat = doc.valueInt32("step_lat");
    info->set_step_lat(stepLat);
    int stepLon = doc.valueInt32("step_lon");
    info->set_step_lon(stepLon);
    if (stepLat != std::numeric_limits<int32_t>::max() &&
       (stepLat != std::numeric_limits<int32_t>::max())) {
        info->set_net_type(obanal::netTypeFromStep(stepLon));
    }

/*
   if (stepLat == std::numeric_limits<int32_t>::max()) {
      info->set_net_type(obanal::netTypeFromStep(stepLon));
    }
    else {
      info->set_net_type(obanal::netTypeFromStep(stepLat));
    }
*/

    Document descrs = doc.valueDocument("descr");
    Array arr = descrs.valueArray("d");
    while (arr.next()) {

      info->add_param(arr.valueInt32());
    }

    QString centerName = doc.valueString("centers.name");
    QString levelTypesNames = doc.valueString("level_types.full_name");
    QString centerShortName = doc.valueString("centers.short_name");
    if ( true == centerShortName.isEmpty() ) {
      info->set_center_name(centerName.toStdString());
    }
    else {
      info->set_center_name(centerShortName.toStdString());
    }
    info->set_level_type_name(levelTypesNames.toStdString());


  }

  res->set_result(true);

  int cur =  ttt.elapsed();
  if(1000 < cur) { debug_log << msglog::kServiceRequestTime.arg(query->query()).arg(cur); }

  return true;
}

//! Формирование набора TMeteoData из полей GRIB
bool TGribDataService::getGribMeteoData(const GribDataRequest* req, DataReply* res)
{
  GribDataReply gres;
  bool ret_val = getGribForMeteo(req, &gres);

  if (false == ret_val) {
    res->set_result(false);
    res->set_comment(gres.comment());
    return false;
  }

  if (gres.data_size() == 0) {
    res->set_result(false);
    res->set_comment(QObject::tr("Нет данных").toStdString());
    return false;
  }

  //создание TMeteoData из данных
  QVector<TMeteoData> md;
  fillMeteoData(gres, &md);

  if (md.isEmpty()) {
    res->set_result(false);
    res->set_comment(QObject::tr("Нет данных").toStdString());
    return false;
  }

  for (auto one : md) {
    QByteArray buf;
    one >> buf;
    res->add_meteodata( buf.data(), buf.size() );
  }

  res->set_date_start(req->date_start());
  res->set_date_end(req->date_start());
  res->set_result(true);

  return true;
}

//! Запрос GRIB для формирования набора TMeteoData
bool TGribDataService::getGribForMeteo(const GribDataRequest *req, GribDataReply *res)
{
  QTime ttt; ttt.start();

  GribMongoDb gribdb;
  ::grib::TGribData grib;
  QList< QPair<int, QString> > grib_list; //дескриптор, id
  if ( false == gribdb.getGribFileId( &grib_list, req, res ) ) {
    res->clear_info();
    res->set_comment( msglog::kDataServiceNoData.toStdString() );
    res->set_result(false);
    return false;
  }
  if (grib_list.isEmpty()) {
    res->set_finished(true);
    return true;
  }

  for ( const QPair<int,QString>& gribs : grib_list) {
    QString fileId = gribs.second;
    if ( false == gribdb.readFromDb( fileId, &grib ) ) {
      error_log << msglog::kDbRequestFailed;
      res->Clear();
      res->set_comment( msglog::kDbRequestFailed.toStdString() );
      res->set_result(false);
      return false;
    }
    iface_.setGrib(&grib);
    GribData* gdata = res->add_data();
    const TGrid::LatLon* gridLatLon = static_cast<const TGrid::LatLon*>( iface_.grid().definition );
    if ( 0 != iface_.grid().type ) {
      continue;
    }
    if ( nullptr == gridLatLon ) {
      continue;
    }

    gdata->mutable_grid_def()->CopyFrom(*gridLatLon);
    if(false == iface_.fillData(gdata->mutable_value())) {
      continue;
    }
    if (true == iface_.isBitmap()) {
      gdata->set_bitmap((grib.bitmap().bitmap()));
    }

    gdata->set_descr(gribs.first);
  }

  int cur =  ttt.elapsed();
  if(1000 < cur) {
    debug_log << msglog::kServiceRequestTime.arg("getGribMeteo").arg(cur);
  }
  res->set_comment(msglog::kServerAnswerOK.arg("getGribMeteo").toStdString());
  res->set_result(true);
  return true;
}

//! Сохранение параметров из srclist в dstlist
void TGribDataService::unionMeteodata(const QVector<TMeteoData>& srclist, QVector<TMeteoData>* dstlist)
{
  if (srclist.size() != dstlist->size()) {
    return;
  }

  for (int idx = 0; idx < srclist.count(); idx++) {
    QList<QPair<descr_t, TMeteoParam>> src = srclist.at(idx).allParams();
    TMeteoData& dst = (*dstlist)[idx];
    for (auto param : src) {
      dst.set(param.first, param.second);
    }
  }
}

//Метеодата из геодата с созданием или дозаполнением (при условии одинаковых сеток)
void TGribDataService::fill(const meteo::GeoData& gdata, bool withGrid, int descr, const QString& unit, QVector<TMeteoData>* md)
{
  const int TRUE_DATA = 1;//48 //TODO это в geodata должно бы быть определено
  // const int FALSE_DATA = 0;//49

  if (withGrid) {
    md->resize(gdata.size());

    for (int idx = 0; idx < gdata.size(); idx++) {
      auto geo = gdata.at(idx);
      (*md)[idx].setCoord(geo.point.latDeg(), geo.point.lonDeg(), geo.point.alt());
      if (geo.mask == TRUE_DATA) {
        (*md)[idx].add(descr, "", geo.data, control::NO_CONTROL, unit);
      }
    }
  } else {
    for (int idx = 0; idx < gdata.size(); idx++) {
      if (gdata.at(idx).mask == TRUE_DATA) {

        (*md)[idx].add(descr, "", gdata.at(idx).data, control::NO_CONTROL, unit);
      }
    }
  }
}

//! Формирование списка TMeteoData из полей grib
void TGribDataService::fillMeteoData(const GribDataReply& gres, QVector<TMeteoData>* mdres)
{
  struct Borders {
      QVector<TMeteoData> left;
      QVector<TMeteoData> top;
      QVector<TMeteoData> right;
      QVector<TMeteoData> bottom;
      Borders() {}
  };

  GribMongoDb grib_decoder;
  QList<const meteo::surf::GribData*> gridptr; //!список указателей на параметры с уникальными сетками
  QList<QVector<TMeteoData>> mdlist; //!< список метеодат для каждой уникальной сетки
  QList<Borders> mdBorders; //!< метеодаты граничных точек для каждой уникальной сетки

  //var(gres.data_size());

  for (int idx = 0; idx < gres.data_size(); idx++) {

    meteo::GeoData gdata;
    meteo::GeoData geoLeft, geoTop, geoRight, geoBottom;
    const meteo::surf::GribData& data = gres.data(idx);

    if (false == grib_decoder.fillGridWithoutBorder(&gdata,
                                                    &geoLeft, &geoTop, &geoRight, &geoBottom,
                                                    data)) {
      error_log << tr("Ошибка сетки") << gres.info().Utf8DebugString();
      continue;
    }

    // var(gdata.size());
    // var(data.descr());
    // var(gres.data(idx).grid_def().Utf8DebugString());

    bool isUniq = true;
    bool isEqual = false;
    int eqIdx = gridptr.size();
    for (int idx = 0; idx < gridptr.size(); idx++) {
      if (TGridGrib::isEqualGrid(*gridptr.at(idx), data)) {
        isEqual = true;
        isUniq = false;
        eqIdx = idx;
        break;
      }
      if (TGridGrib::isIntersectGrid(*gridptr.at(idx), data)) {
        isUniq = false;
        break;
      }
    }

    //debug_log << "isUniq" << isUniq << "isEqual" << isEqual;

    //заполняем метеодату с уникальной сеткой
    if (isUniq) {
      mdlist.append(QVector<TMeteoData>());
      QVector<TMeteoData>& md = mdlist.last();
      mdBorders.append(Borders());
      Borders& brd = mdBorders.last();

      gridptr.append(&data);
      if (data.has_descr()) {

        QString unit = grib_decoder.unitByDescr(data.descr());
        fill(gdata,     true, data.descr(), unit, &md);
        fill(geoLeft,   true, data.descr(), unit, &brd.left);
        fill(geoTop,    true, data.descr(), unit, &brd.top);
        fill(geoRight,  true, data.descr(), unit, &brd.right);
        fill(geoBottom, true, data.descr(), unit, &brd.bottom);
      }
    }

    //добавляем метеодату к совпадающей по сетке
    if (isEqual) {
      //debug_log << "fillData" << data.descr() << idx;
      QVector<TMeteoData>& md = mdlist[eqIdx];
      Borders& brd = mdBorders[eqIdx];
      if (data.has_descr()) {
        QString unit = grib_decoder.unitByDescr(data.descr());
        fill(gdata,     false, data.descr(), unit, &md);
        fill(geoLeft,   false, data.descr(), unit, &brd.left);
        fill(geoTop,    false, data.descr(), unit, &brd.top);
        fill(geoRight,  false, data.descr(), unit, &brd.right);
        fill(geoBottom, false, data.descr(), unit, &brd.bottom);
      }
    }

    if (!isUniq && !isEqual) {
      //TODO при указанных входных параметрах такого на практике не встречалось
      debug_log << QObject::tr("Пересечение сеток не реализовано");
    }
  }

  // for (auto md : mdlist) {
  //   var("md");
  //   var(md.count());
  //   for (int idx = 0; idx < 5; idx++) {
  //     var(idx);
  //     md.at(idx).printData();
  //   }
  // }


  for (int idx = 1; idx < gridptr.size(); idx++) {
    for (int odx = 0; odx < idx; odx++) {
      TGrid::GridEdge edge = TGridGrib::getCommonEdge(*gridptr.at(idx), *gridptr.at(odx));
      switch (edge) {
      case TGrid::NoEdge:
      continue;
      break;
      case TGrid::LeftEdge:
        unionMeteodata(mdBorders.at(idx).left, &mdBorders[odx].right);
        mdBorders[idx].left.clear();
      break;
      case TGrid::TopEdge:
        unionMeteodata(mdBorders.at(idx).top, &mdBorders[odx].bottom);
        mdBorders[idx].top.clear();
      break;
      case TGrid::RightEdge:
        unionMeteodata(mdBorders.at(idx).right, &mdBorders[odx].left);
        mdBorders[idx].right.clear();
      break;
      case TGrid::BottomEdge:
        unionMeteodata(mdBorders.at(idx).bottom, &mdBorders[odx].top);
        mdBorders[idx].bottom.clear();
      break;
      }
    }
  }

  // for (auto brd: mdBorders) {
  //   var("md");

  //   var(brd.left.count());
  //   for (int idx = 0; idx < 3 && idx < brd.left.count(); idx++) {
  //     brd.left.at(idx).printData();
  //   }

  //   var(brd.top.count());
  //   for (int idx = 0; idx < 3 && idx < brd.top.count(); idx++) {
  //     brd.top.at(idx).printData();
  //   }

  //   var(brd.right.count());
  //   for (int idx = 0; idx < 3 && idx < brd.right.count(); idx++) {
  //     brd.right.at(idx).printData();
  //   }

  //   var(brd.bottom.count());
  //   for (int idx = 0; idx < 3 && brd.bottom.count(); idx++) {
  //     brd.bottom.at(idx).printData();
  //   }
  // }

  for (auto md : mdlist) {

    *mdres += md;
  }

  for (auto brd: mdBorders) {
    *mdres += brd.left;
    *mdres += brd.top;
    *mdres += brd.right;
    *mdres += brd.bottom;
  }

  // var("mdres");
  //var(mdres->count());
  // for (auto md : *mdres) {
  //   md.printData();
  // }

}

//для ОА
bool TGribDataService::getGribData(const GribDataRequest *req, GribDataReply *res)
{
  QTime ttt; ttt.start();

  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() ) {
    res->clear_info();
    res->set_comment( QObject::tr("Не удалось подключиться к базе данных").toStdString() );
    res->set_result(false);
    return false;
  }

  GribMongoDb gribdb;
  ::grib::TGribData grib;
  QList< QPair<QString,QString> > grib_list;
  if ( false == gribdb.getGribData( &grib_list, req, res ) ) {
    res->clear_info();
    res->set_comment( msglog::kDataServiceNoData.toStdString() );
    res->set_result(false);
    return false;
  }
  if (grib_list.isEmpty()) {
    res->set_finished(true);
    return true;
  }

  for ( const QPair<QString,QString>& gribs : grib_list) {
    QString oid = gribs.first;
    QString fileId = gribs.second;
    if ( false == gribdb.readFromDb( fileId, &grib ) ) {
      markBrokenGrib(oid, QObject::tr("Отсутствует файл в GRIDfs"));
      error_log << msglog::kDbRequestFailed;
      res->Clear();
      res->set_comment( msglog::kDbRequestFailed.toStdString() );
      res->set_result(false);
      return false;
    }
    iface_.setGrib(&grib);
    GribData* gdata = res->add_data();
    const TGrid::LatLon* gridLatLon = static_cast<const TGrid::LatLon*>( iface_.grid().definition );
    if ( 0 != iface_.grid().type ) {
      markBrokenGrib(oid, "iface_.grib().type() != 0");
      continue;
    }
    if ( nullptr == gridLatLon ) {
      markBrokenGrib(oid, "nullptr == gridLatLon");
      continue;
    }

    gdata->mutable_grid_def()->CopyFrom(*gridLatLon);
    if(false == iface_.fillData(gdata->mutable_value())) {
      markBrokenGrib(oid, "false == iface_.fillData(gdata->mutable_value())");
      continue;
    }
    if (true == iface_.isBitmap()) {
      gdata->set_bitmap((grib.bitmap().bitmap()));
    }

    res->add_ids(oid.toStdString());
  }

  int cur =  ttt.elapsed();
  if(1000 < cur) {
    debug_log << msglog::kServiceRequestTime.arg("getGribData").arg(cur);
  }
  res->set_comment(msglog::kServerAnswerOK.arg("getGribData").toStdString());
  res->set_result(true);
  return true;
}


}
}
