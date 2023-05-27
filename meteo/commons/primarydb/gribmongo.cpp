#include "gribmongo.h"

#include <qfile.h>
#include <qdir.h>
#include <qdatetime.h>
#include <qdatastream.h>
#include <qjsondocument.h>
#include <qjsonobject.h>

#include <sql/dbi/gridfs.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/obanal/obanal_struct.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/state.pb.h>

#include <sql/dbi/dbi.h>
#include <sql/dbi/dbiquery.h>

//using namespace meteo;

namespace meteo {

namespace grib {
GribMongoDb* dbmongo() {
  static GribMongoDb* dbase = new GribMongoDb;
  return dbase;
}
}

static const QString kDbName         = "meteodb";
static const QString kGribColl       = "grib";
static const QString kParametersColl = "parameters";

GribMongoDb::GribMongoDb()
{
  init();
  _stat.set(::grib::kGrib1, ::grib::kSaveOk, meteo::app::OperationState_WARN,
            QObject::tr("Сохранено сводок GRIB"));
  _stat.set(::grib::kGrib1, ::grib::kSaveErr, meteo::app::OperationState_NORM,
            QObject::tr("Отброшено сводок GRIB из-за ошибок при сохранении"));
  _stat.set(::grib::kGrib1, ::grib::kRepeated, meteo::app::OperationState_NORM,
            QObject::tr("Получено повторных сводок GRIB"));


  _stat.set(::grib::kGrib2, ::grib::kSaveOk, meteo::app::OperationState_WARN,
            QObject::tr("Сохранено сводок GRIB2"));
  _stat.set(::grib::kGrib2, ::grib::kSaveErr, meteo::app::OperationState_NORM,
            QObject::tr("Отброшено сводок GRIB2 из-за ошибок при сохранении"));
  _stat.set(::grib::kGrib2, ::grib::kRepeated, meteo::app::OperationState_NORM,
            QObject::tr("Получено повторных сводок GRIB2"));

}


GribMongoDb::~GribMongoDb()
{
  delete db_;
  db_ = nullptr;
  delete gridfs_;
  gridfs_ = nullptr;
}

/*
void GribMongoDb::setDb(Dbi *client )
{
  delete db_;
}
db_ = client;

_mongoDbName = kDbName;
}*/

bool GribMongoDb::init()
{
  if ( nullptr != db_ ) {
    delete db_; db_ = nullptr;
  }
  if ( nullptr != gridfs_ ) {
    delete gridfs_; gridfs_ = nullptr;
  }
  const ConnectProp conf = meteo::Global::instance()->mongodbConfMeteo();
  db_ = meteo::global::dbMeteo();
  if ( nullptr == db_ ) {
    debug_log << QObject::tr("Не удалось подключиться к dbMeteo");
    return false;
  }
  gridfs_ = new GridFs();
  gridfs_->connect(conf);
  loadParameters();
  return true;
}

bool GribMongoDb::checkDb() const
{
  if ( nullptr == db_ || false == db_->connected()
       || nullptr == gridfs_ || false == gridfs_->isConnected() ) {
    return false;
  }
  return true;
}

void GribMongoDb::setConnectionStatus(meteo::app::OperationStatus* status)
{
  if (nullptr == status) { return; }

  meteo::app::OperationParam* param = status->add_param();
  param->set_title( QObject::tr("Подключение к БД").toUtf8().constData() );
  if (checkDb()) {
    param->set_value(QObject::tr("Есть").toStdString());
    param->set_state(meteo::app::OperationState_NORM);
  } else {
    param->set_value(QObject::tr("Нет").toStdString());
    param->set_state(meteo::app::OperationState_ERROR);
    status->set_state(meteo::app::OperationState_ERROR);
  }
  param->set_dt(QDateTime::currentDateTime().toString("dd.MM.yy hh:mm").toStdString());
}

bool GribMongoDb::loadParameters()
{
  if ( false == checkDb() && false == init() ) {
    debug_log << QObject::tr("Нет подключения к БД");
    return false;
  }
  auto query = db_->queryptrByName("load_grib_params");
  if(nullptr == query) return false;

  if ( false == query->exec()) {
    debug_log << QObject::tr("Не удалось выполнит запрос = %1")
                 .arg( query->query() );
    return false;
  }
  if ( false == query->initIterator() ) {
    return false;
  }

  gribparams_.Clear();
  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    ::grib::Parameter* p = gribparams_.add_param();
    if ( true == doc.hasField("grib1_parameter") ) {
      p->set_paramv1( doc.valueInt32("grib1_parameter") );
      paramv1_.insert( p->paramv1(), p );
    }
    if ( true == doc.hasField("parameter") ) {
      p->set_paramv2( doc.valueInt32("parameter") );
      p->set_discipline( doc.valueInt32("discipline") );
      p->set_category( doc.valueInt32("category") );
      paramv2_.insert( qMakePair( p->discipline(), qMakePair( p->category(), p->paramv2() ) ),  p );
    }
    if ( true == doc.hasField("bufr_descr") ) {
      p->set_descr( doc.valueInt32("bufr_descr") );
    }
    p->set_name( doc.valueString("name").toStdString() );
    p->set_unit( doc.valueString("unit").toStdString() );
    p->set_unit_en( doc.valueString("unit_en").toStdString() );
  }
  if(0==paramv1_.size() || 0==paramv2_.size()){
    error_log << QObject::tr("Ошибка получения параметров раскодирования GRIB");
    return false;
  }

  return true;
}


bool GribMongoDb::save( const ::grib::TGribData* grib)
{
  if ( false == checkDb() && false == init() ) {
    debug_log << QObject::tr("Нет подключения к БД");
    return false;
  }

  if (false == grib->has_grid()) {
    debug_log << QObject::tr("В данных нет параметров сетки");
    return false;
  }

  if ( true == grib->has_product2() ) {
    _edition = ::grib::kGrib2;
  }
  else {
    _edition = ::grib::kGrib1;
  }

  if ( true == paramv1_.isEmpty() || true == paramv2_.isEmpty() ) {
    loadParameters();
  }

  std::string data;
  if ( false == grib->SerializeToString(&data) ) {
    debug_log << QObject::tr("Некорректные данные");
    _stat.add(_edition, ::grib::kSaveErr, meteo::app::OperationState_ERROR);
    return false;
  }

  //  uint16_t crc = MnMath::makeCrc(data.data(), data.length());
  uint16_t crc = qChecksum(data.data(), data.length());

  QDateTime dt = QDateTime::fromString(QString::fromStdString(grib->dt()), Qt::ISODate);
  // uint id = qHash(QTime::currentTime().toString("hhmmsszzz") + _ptkppId + QString::number(_cnt));
  //  _cnt++;

  QString filename = dt.toString("yyyyMMdd_hhmmss_v") + QString::number(_edition) + "_" +
                     QString::number(data.length()) + "_" + QString::number(crc);

  GridFile gridfile;
  QString collection = "grib";
  bool ok = saveToDb(collection, filename, data, &gridfile);
  if (!ok) {
    return false;
  }
  if (gridfile.id().isEmpty()) {
    _stat.add(_edition, ::grib::kRepeated, meteo::app::OperationState_NORM);
    return true;
  }

  QString queryname;
  if (_edition == ::grib::kGrib2) {
    queryname = "insert_grib2";
  }
  else {
    queryname = "insert_grib1";
  }

  auto query = db_->queryptrByName(queryname);
  if ( nullptr == query ) {
    return false;
  }

  if (_edition == ::grib::kGrib2) {
    int64_t descr = descrForParam(grib->discipline(), grib->product2().category(), grib->product2().number());

    // query->arg("collection",collection);

    query->arg("edition",_edition);
    query->arg("dt_write",QDateTime::currentDateTimeUtc());
    query->arg("center",(int32_t)grib->center());
    query->arg("subcenter",(int32_t)grib->subcenter());
    query->argDt("dt",grib->dt());
    query->arg("sign_dt",(int32_t)grib->signdt());
    query->arg("status",(int64_t)grib->status());
    query->arg("data_type",(int32_t)grib->datatype());
    query->arg("discipline",(int32_t)grib->discipline());
    query->arg("category",(int32_t)grib->product2().category());
    query->arg("product",(int32_t)grib->product2().number());
    query->arg("descr",descr);
    query->arg("defType",(int32_t)grib->product2().deftype());
    query->arg("levelType",(int32_t)grib->product2().surf1_type());
    query->arg("levelValue",grib->product2().surf1());
    query->arg("levelType2",(int32_t)grib->product2().surf2_type());
    query->arg("levelValue2",grib->product2().surf2());
    query->arg("procType",(int32_t)grib->product2().processtype());
    query->arg("timeRange",(int32_t)grib->product2().timerange());
    query->argDt("dt1",grib->product2().dt1());
    query->argDt("dt2",grib->product2().dt2());
    query->arg("hour",(int64_t)grib->product2().forecasttime());
    if (grib->grid().has_di() && grib->grid().has_dj()) {
      query->arg("stepLat",(int32_t)grib->grid().di());
      query->arg("stepLon",(int32_t)grib->grid().dj());
    }
    query->argOid("fileid",gridfile.id());
    query->arg("tlgid",ptkppId());
    query->arg("crc",crc);
  }
  else {
    int64_t descr = descrForParam(grib->product1().number());
    query->arg("collection",collection);
    query->arg("edition",_edition);
    query->arg("dt_write",QDateTime::currentDateTimeUtc());
    query->arg("center",(int32_t)grib->center());
    query->arg("subcenter",(int32_t)grib->subcenter());
    query->argDt("dt",grib->dt());
    query->arg("product",(int32_t)grib->product1().number());
    query->arg("descr",descr);
    query->arg("levelType",(int32_t)grib->product1().leveltype());
    query->arg("levelValue",(float)grib->product1().levelvalue());
    query->arg("procType",(int32_t)grib->product1().processtype());
    query->arg("timeRange",grib->product1().timerange());
    query->argDt("dt1",grib->product1().dt1());
    query->argDt("dt2",grib->product1().dt2());
    query->arg("hour",(int64_t)grib->product1().forecasttime());
    if (grib->grid().has_di() && grib->grid().has_dj()) {
      query->arg("stepLat",(int32_t)grib->grid().di());
      query->arg("stepLon",(int32_t)grib->grid().dj());
    }
    query->argOid("fileid",gridfile.id());
    query->arg("tlgid",ptkppId());
    query->arg("crc",crc);
  }

  ok = query->exec();

  if (ok) {
    _stat.add(_edition, ::grib::kSaveOk, meteo::app::OperationState_NORM);
  }
  else {
    gridfile.remove();
    debug_log << QObject::tr("Ошибка записи в БД");
    _stat.add(_edition, ::grib::kSaveErr, meteo::app::OperationState_ERROR);
  }

  return ok;
}


bool GribMongoDb::parceGrib( const QString& path, ::grib::TGribData* grib )
{
  if ( true == path.isEmpty() ) {
    debug_log << QObject::tr("Не указан путь к файлу");
    return false;
  }

  QFile file(path);
  if ( false == file.open(QIODevice::ReadOnly) ) {
    debug_log << meteo::msglog::kFileOpenError.arg(path).arg(file.errorString());
    return false;
  }
  QByteArray arr = file.readAll();
  file.close();

  std::string data( arr.data(), arr.size() );
  return grib->ParsePartialFromString(data);
}

bool GribMongoDb::readFromDb( const QString& id, ::grib::TGribData* grib )
{
  if ( false == checkDb() && false == init() ) {
    debug_log << QObject::tr("Нет подключения к БД");
    return false;
  }

  gridfs_->use(kDbName,kGribColl);

  GridFile file = gridfs_->findOneById(id);
  if ( !file.isValid() ) {
    debug_log << QObject::tr("Неизвестная ошибка при поиска файла по id = %1. Ошибка =").arg(id) << gridfs_->lastError();
    return false;
  }

  if ( !file.hasFile() ) {
    debug_log << QObject::tr("Файл с идентификатором '%1' не найден").arg(id);
    return false;
  }

  bool ok = false;
  QByteArray ba = file.readAll(&ok);
  if ( !ok ) {
    debug_log << file.lastError();
    return false;
  }

  return grib->ParsePartialFromArray(ba.data(),ba.size());
}

bool GribMongoDb::getNotAnalysedGribs( QList< QPair<QString,QString> > *gribs, ::grib::GribInfo* info )
{
  if ( false == checkDb() && false == init() ) {
    debug_log << QObject::tr("Нет подключения к БД");
    return false;
  }
  QTime ttt;
  if( nullptr == gribs || nullptr == info  ) {
    return false;
  }

  ttt.start();
  auto query = db_->queryptrByName("find_grib_not_analysed");
  if ( nullptr == query ) {
    return false;
  }
  if ( false == query->exec() ) {
    error_log << msglog::kDbRequestFailedArg.arg(query->query());
    return false;
  }
  if ( false == query->initIterator() ) {
    error_log << msglog::kDbCursorCreationFailed;
    return false;
  }

  int center;
  QDateTime dt;
  int procType;
  int levelType;
  int subcenter;
  //FIXME double levelValue;
  float levelValue;
  int descr;
  QDateTime dt1, dt2;
  int timeRange;
  int hour;
  int stepLat;
  int stepLon;
  // debug_log << query->query();
  if ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    center = doc.valueInt32("center");
    subcenter = doc.valueInt32("subcenter");
    dt = doc.valueDt("dt");
    procType = doc.valueInt32("procType");
    levelType = doc.valueInt32("levelType");
    levelValue = doc.valueDouble("levelValue");
    descr = doc.valueInt32("descr");
    dt1 = doc.valueDt("dt1");
    dt2 = doc.valueDt("dt2");
    timeRange = doc.valueInt32("timeRange");
    hour = doc.valueInt32("hour");
    stepLat = doc.valueInt32("stepLat");
    stepLon = doc.valueInt32("stepLon");
  }
  else {
    return false;
  }
  if ( true == query->next()) {
    debug_log << QObject::tr("Почему-то пришло 2 результата вместо одного.");
    return false;
  }

  query = db_->queryptrByName("grib_find");
  if(nullptr == query) {return false;}
  ttt.restart();
  query->arg("descr",descr);
  query->arg("center",center);
  query->arg("subcenter",subcenter);
  query->arg("dt",dt);
  query->arg("levelType",levelType);
  query->arg("levelValue",levelValue);
  query->arg("procType",procType);
  query->arg("dt1",dt1);
  query->arg("dt2",dt2);
  query->arg("hour",hour);
  query->arg("timeRange",timeRange);
  query->arg("stepLat",stepLat);
  query->arg("stepLon",stepLon);

  if ( false == query->exec( ) ) {
    error_log << msglog::kDbRequestFailedArg.arg(query->query());
    return false;
  }
  if ( false == query->initIterator() ) {
    error_log << msglog::kDbCursorCreationFailed;
    return false;
  }
  info->set_center(center);
  info->set_date( dt.toString(Qt::ISODate).toStdString() );
  info->set_model( procType );
  info->set_level_type( levelType);
  info->set_level( qRound(levelValue) );
  info->clear_param();
  info->add_param(descr);
  info->mutable_hour()->set_dt1( dt1.toString(Qt::ISODate).toStdString() );
  info->mutable_hour()->set_dt2( dt2.toString(Qt::ISODate).toStdString() );
  info->mutable_hour()->set_time_range( timeRange );
  if (stepLat == std::numeric_limits<int32_t>::max()) {
    info->set_net_type(obanal::netTypeFromStep(stepLon));
  }
  else {
    info->set_net_type(obanal::netTypeFromStep(stepLat));
  }
  int cnt = 0;
  while ( true == query->next() ) {
    ++cnt;
    const DbiEntry& adoc = query->entry();
    QString s_id = adoc.valueOid("_id");
    QString s_path = adoc.valueOid("fileid");
    gribs->append( qMakePair(s_id,s_path));
  }
  if ( 0 == cnt ){
    debug_log << QObject::tr("Ошибка: количество GRIB полученных во втором запросе равно 0, хотя ожидается хотя бы 1 GRIB");
  }
  return true;
}

bool GribMongoDb::getGribData(QList<QPair<QString, QString> >* gribs,
                              const surf::GribDataRequest* req, surf::GribDataReply* res)
{
  if ( false == checkDb() && false == init() ) {
    debug_log << QObject::tr("Нет подключения к БД");
    return false;
  }
  QTime ttt;
  ttt.start();
  ::grib::GribInfo* info = res->mutable_info();
  if( nullptr == gribs || nullptr == info  ) {
    return false;
  }

  auto query = db_->queryptrByName("get_grib_data");
  if ( nullptr == query ) {
    return false;
  }
  if ( true == req->has_date_start() && true == req->has_date_end() ) {
    query->argDt("start_dt",req->date_start());
    query->argDt("end_dt",req->date_end());
  }
  if ( 0 < req->info().param_size() ) {
    query->arg("descr",req->info().param());
  }
  if ( true == req->info().has_level_type() ) {
    query->arg("levelType",req->info().level_type());
  }
  if ( true == req->info().has_level() ) {
    query->arg("levelValue",static_cast<double>(req->info().level()));
  }
  if ( true == req->info().has_center() ) {
    query->arg("center",req->info().center());
  }
  if ( true == req->info().hour().has_hour() ) {
    query->arg("hour",req->info().hour().hour());
  }
  if ( true == req->info().has_model() ) {
    query->arg("procType",req->info().model());
  }

  bool ok = query->exec();
  if ( false == ok ) {
    debug_log << QObject::tr("Запрос get_grib_data выполнился с ошибкой");
    return false;
  }
  if ( false == query->initIterator() ) {
    debug_log << QObject::tr("Невозможно получить курсор в запросе get_grib_data");
    return false;
  }
  int center;
  QDateTime dt;
  int procType;
  int levelType;
  int subcenter;
  //FIXME double levelValue;
  float levelValue;
  int descr;
  QDateTime dt1, dt2;
  int timeRange;
  int hour;

  if ( true == query->next()) {
    const DbiEntry& doc = query->entry();
    center = doc.valueInt32("center");
    subcenter = doc.valueInt32("subcenter");
    dt = doc.valueDt("dt");
    procType = doc.valueInt32("procType");
    levelType = doc.valueInt32("levelType");
    levelValue = doc.valueDouble("levelValue");
    descr = doc.valueInt32("descr");
    dt1 = doc.valueDt("dt1");
    dt2 = doc.valueDt("dt2");
    timeRange = doc.valueInt32("timeRange");
    hour = doc.valueInt32("hour");
  }
  else {
    debug_log << QObject::tr("В запросе get_grib_data нет документов");
    return true;
  }
  if ( true == query->next() ) {
    debug_log << QObject::tr("Почему-то пришло 2 результата вместо одного.");
    return false;
  }

  query = db_->queryptrByName("grib_find");
  if(nullptr == query) {return false;}
  ttt.restart();

  info_log << QObject::tr("grib_find завершено за %1 мсек").arg(ttt.elapsed());
  query->arg("descr",descr);
  query->arg("center",center);
  query->arg("subcenter",subcenter);
  query->arg("dt",dt);
  query->arg("levelType",levelType);
  query->arg("levelValue",levelValue);
  query->arg("procType",procType);
  query->arg("dt1",dt1);
  query->arg("dt2",dt2);
  query->arg("hour",hour);
  query->arg("timeRange",timeRange);

  ok = query->exec();
  if ( false == ok ) {
    return false;
  }
  if ( false == query->initIterator() ) {
    return false;
  }
  info->set_center(center);
  info->set_date( dt.toString(Qt::ISODate).toStdString() );
  info->set_model( procType );
  info->set_level_type( levelType);
  info->set_level( qRound(levelValue) );
  info->clear_param();
  info->add_param( descr );
  info->mutable_hour()->set_dt1( dt1.toString(Qt::ISODate).toStdString() );
  info->mutable_hour()->set_dt2( dt2.toString(Qt::ISODate).toStdString() );
  info->mutable_hour()->set_time_range( timeRange );
  int cnt = 0;
  while ( true == query->next()) {
    ++cnt;
    const DbiEntry& adoc = query->entry();
    QString s_id = adoc.valueOid("_id");
    QString s_path = adoc.valueOid("fileid");
    gribs->append( qMakePair(s_id,s_path));
  }
  if ( 0 == cnt ){
    debug_log << QObject::tr("Ошибка: количество полученных грибов во втором запросе равно 0, хотя ожидается хотя бы один GRIB");
  }
  return true;
}

//! разные метеопараметры с общими параметрами сетки созданого поля
bool GribMongoDb::getGribFileId(QList<QPair<int, QString> >* gribs,
                                const surf::GribDataRequest* req, surf::GribDataReply* res)
{
  if ( false == checkDb() && false == init() ) {
    debug_log << QObject::tr("Нет подключения к БД");
    return false;
  }
  QTime ttt;
  ttt.start();
  ::grib::GribInfo* info = res->mutable_info();
  if( nullptr == gribs || nullptr == info  ) {
    return false;
  }

  if ( false == req->has_date_start() ||
       false == req->info().has_level_type() ||
       false == req->info().has_level() ||
       false == req->info().has_center() ||
       false == req->info().hour().has_hour() ||
       false == req->info().has_model() ||
       (false == req->info().has_net_type()&&
        false == req->info().has_step_lat()&&
        false == req->info().has_step_lon())) {
    debug_log << QObject::tr("Указаны не все параметры для запроса get_grib_fileid");
    return false;
  }

  auto query = db_->queryptrByName("get_grib_fileid");
  if ( nullptr == query ) {
    return false;
  }

  if ( 0 < req->info().param_size() ) {
    query->arg("descr",req->info().param());
  }

  query->argDt("dt",      req->date_start());
  query->arg("levelType", req->info().level_type());
  query->arg("levelValue",static_cast<double>(req->info().level()));
  query->arg("center",    req->info().center());
  query->arg("hour",      req->info().hour().hour());
  query->arg("procType",  req->info().model());
  if(req->info().has_step_lat()&&req->info().has_step_lon()){
    query->arg("steplat", req->info().step_lat()); //TODO
    query->arg("steplon", req->info().step_lon());
  }else{
    if(req->info().has_net_type()){
      meteo::GeoPoint steps;
      if (PropertyChooseData::getSteps(NetType(req->info().net_type()), &steps)) {
        query->arg("steplat", steps.latDeg()*1000); //TODO
        query->arg("steplon", steps.lonDeg()*1000);
      } else {
        debug_log << QObject::tr("Неверные параметры сетки для запроса get_grib_fileid");
        return false;
      }
    }
  }
  if (true == req->info().has_hour()){
    if (true == req->info().hour().has_dt1()){
      query->argDt("forecast_beg",req->info().hour().dt1());
    }
    if (true == req->info().hour().has_dt2()){
      query->argDt("forecast_end",req->info().hour().dt2());
    }
  }

    debug_log <<query->query();
    bool ok = query->exec();
    if ( false == ok ) {
      debug_log << QObject::tr("Запрос get_grib_fileid выполнился с ошибкой");
      return false;
    }

    info->set_center(req->info().center());
    info->set_date(  req->date_start() );
    info->set_model( req->info().model() );
    info->set_level_type( req->info().level_type() );
    info->set_level(req->info().level());

    int cnt = 0;
    while ( true == query->next()) {
      ++cnt;
      const DbiEntry& adoc = query->entry();
      int s_descr = adoc.valueInt32("descr");
      QString s_path = adoc.valueOid("fileid");
      gribs->append( qMakePair(s_descr, s_path));
    }
    if ( 0 == cnt ){
      debug_log << QObject::tr("Нет данных, соответствующих запросу");
    }
    return true;
  }

  int64_t GribMongoDb::descrForParam( int32_t paramv1 ) const
  {
    if ( false == paramv1_.contains(paramv1) ||
         false == paramv1_.value(paramv1)->has_descr()) {
      debug_log << QObject::tr("Не задан дескриптор для параметра grib 1-ой версии = %1")
                   .arg(paramv1);
      return -paramv1;
    }
    return paramv1_[paramv1]->descr();
  }

  int64_t GribMongoDb::descrForParam( int32_t discipline, int32_t category, int32_t paramv2 ) const
  {
    auto key = qMakePair( discipline, qMakePair( category, paramv2 )  );
    if ( false == paramv2_.contains(key) ||
         false == paramv2_.value(key)->has_descr()) {
      debug_log << QObject::tr("Не задан дескриптор для параметра grib 2-ой версии = %1, %2, %3")
                   .arg(discipline)
                   .arg(category)
                   .arg(paramv2);
      return -1*( (discipline*100000000) + (category*100000) + (paramv2));
    }
    return paramv2_[key]->descr();
  }

  QString GribMongoDb::unitByDescr(int64_t descr)
  {
    for (auto p : gribparams_.param()) {
      if (descr == p.descr()) {
        return QString::fromStdString(p.unit_en());
      }
    }
    return QString();
  }

  //! сохранение бинарных данных в бд
  //если return true, и gridfile пустой, значит такие данные уже есть, сохранять ничего не надо
  bool GribMongoDb::saveToDb(const QString& collection, const QString& fileName,
                             const std::string& msg, GridFile* gridfile)
  {
    if ( false == checkDb() && false == init() ) {
      return false;
    }
    if (0 == msg.length() || nullptr == gridfile) {
      return false;
    }

    gridfs_->use(db_->dbname(), collection);

    GridFile old = gridfs_->findOneByName(fileName);
    if ( old.isValid() && !old.id().isEmpty()) { return true; }

    QByteArray ba(msg.data(), msg.size());

    gridfs_->put(fileName, ba, gridfile);
    if (!gridfile->isValid() || gridfile->id().isEmpty()) {
      return false;
    }

    return true;
  }

  bool GribMongoDb::fillFromGrib(meteo::GeoData* data, /*NetType* nt,*/
                                 const meteo::surf::GribData& grib, bool convertUnit)
  {
    descr_t descr = BAD_DESCRIPTOR_NUM;
    QString unit;
    if (grib.has_descr()) {
      descr = grib.descr();
      unit  = unitByDescr(grib.descr());
    }

    // debug_log << "descr" << descr << unit;

    const TGrid::LatLon& msg = grib.grid_def();
    const google::protobuf::RepeatedField<google::protobuf::int32>& points = grib.points();
    const google::protobuf::RepeatedField<double>& val = grib.value();
    const int TRUE_DATA = 1;//48
    const int FALSE_DATA = 0;//49

    double subdivision = 1e-3;
    double subdiv_max = 1e6;
    //int subdivision1 = 1;
    if(msg.lo1() > subdiv_max ||
       msg.lo2() > subdiv_max ||
       msg.la1() > subdiv_max ||
       msg.la2() > subdiv_max) {
      subdivision = 1e-6;
      //subdivision1 = 1000;
    }

    bool  isLonAdd = (msg.scanmode() & 0x80) == 0;
    bool  isLatAdd = (msg.scanmode() & 0x40) != 0;
    float wlat     = fabs(msg.la2()*subdivision - msg.la1()*subdivision);
    float wlon;

    if(isLonAdd) {
      if(msg.lo2() > msg.lo1()) {
        wlon = msg.lo2() * subdivision - msg.lo1() * subdivision;
      }
      else {
        wlon = fabs(msg.lo2() * subdivision - msg.lo1() * subdivision + 360);
      }
    }
    else if(msg.lo2() > msg.lo1()) {
      wlon = msg.lo2() * subdivision - msg.lo1() * subdivision + 360;
    }
    else {
      wlon = msg.lo1() * subdivision - msg.lo2() * subdivision;
    }

    int   isRegular = 0; //0 - да, 1 - по широте нет, 2 - по долготе нет
    float start1, start2;
    float width = 0;
    bool  isAdd1, isAdd2;
    uint  cnt1, cnt2;
    float step1, step2;
    //int   di10 = 1;

    if (msg.nj() == 0xffff || msg.nj() == 0xffffffff) {
      if(0 == points.size()) { return false; }
      isRegular = 1;
      start2    = msg.la1() * subdivision;
      start1    = msg.lo1() * subdivision;
      isAdd2    = isLatAdd;
      isAdd1    = isLonAdd;
      cnt1      = msg.ni();
      step1     = msg.di() * subdivision;
      cnt2      = points.data()[0];
      width     = wlat;
      step2     = width / (points.data()[0] - 1);
    }
    else {
      start1    = msg.la1() * subdivision;
      start2    = msg.lo1() * subdivision;
      isAdd1    = isLatAdd;
      isAdd2    = isLonAdd;
      cnt1      = msg.nj();
      step1     = msg.dj() * subdivision;
      //di10      = msg.dj() / subdivision1;
    }

    if(msg.ni() == 0xffff || msg.ni() == 0xffffffff) {
      if(0 == points.size()) { return false; }
      isRegular = 2;
      cnt2      = points.data()[0];
      width     = wlon;
      step2     = width / (points.data()[0] - 1);
    }
    else {
      cnt2      = msg.ni();
      step2     = msg.di() * subdivision;
      //di10      = msg.di() / subdivision1;
    }
    // *nt = obanal::netTypeFromStep(di10);
    // if (*nt == NO_TYPE) {
    //   error_log << QObject::tr("Нет типа сетки для шага") << di10;
    //   return false;
    // }

    //  debug_log << "di"<<msg.di()*subdivision<<"dj"<< msg.dj()*subdivision;
    //  debug_log << "d10" <<di10 << *nt;

    float    coord1 = start1;
    float    coord2 = start2;
    uint64_t num     = 0;//!< позиция в данных
    uint64_t numgrid = 0;//!< позиция по координатам (данных в точке сетки может не быть, в соотв с bitmask)
    float    latCur, lonCur;
    for(int64_t cycle1 = 0; cycle1 < cnt1; ++cycle1) {
      if(0 != isRegular) {
        if(cycle1 >= points.size()) {
          debug_log << QObject::tr("Количество дополнительных точек не совпадает с параметрами сетки");
          return false;
        }

        cnt2 = points.data()[cycle1];
        step2 = width / (points.data()[cycle1] - 1);
        // var(cnt2);
        // var(step2);
      }

      for(uint64_t cycle2 = 0; cycle2 < cnt2; ++cycle2) {

        if(isRegular == 1) {
          latCur = coord2;
          lonCur = MnMath::M180To180(coord1);

          //FIXME костыль похоже:
          float coord1_vs =  coord1;
          if(isAdd1) {
            coord1_vs += step1;
          }
          else {
            coord1_vs -= step1;
          }
          if(MnMath::M180To180(coord1_vs)<0 && lonCur > 0){
            lonCur = -1*lonCur;
          }

          //  debug_log << "before" << latCur <<coord1<<lonCur;
        }
        else {
          latCur = coord1;
          lonCur = MnMath::M180To180(coord2);
          //FIXME костыль похоже:
          float coord2_vs =  coord2;
          if(isAdd2) {
            coord2_vs += step2;
          }
          else {
            coord2_vs -= step2;
          }
          if(MnMath::M180To180(coord2_vs)<0 && lonCur > 0){
            lonCur = -1*lonCur;
          }

        }
        //debug_log << cycle1 << cycle2 << latCur << lonCur<< val.data()[num]-273.15;
        //     if(latCur > 89.) latCur = 89.;
        //        if(latCur < -89.) latCur = -89.;
        // debug_log << "before" << latCur <<coord2<<lonCur;

        bool exists = true;
        if (grib.has_bitmap()) {
          uint64_t byte = numgrid / 8;
          uint8_t bit = numgrid % 8;
          if (byte > grib.bitmap().size()) {
            debug_log << "ERR" << byte << grib.bitmap().size();
          }
          exists = (grib.bitmap()[byte] & (0x1 << bit)) != 0;
        }


        if (exists) {
          if (num >= (uint64_t)val.size()) {
            debug_log << QObject::tr("Количество данных не совпадает с параметрами сетки");
            return false;
          }

          float cur = val.data()[num];
          if (convertUnit) {
            cur = TMeteoDescriptor::instance()->convertUnit(descr, cur, unit);
          }
          data->append(meteo::MData(MnMath::deg2rad(latCur), MnMath::deg2rad(lonCur), cur, TRUE_DATA));
          num++;
        } else {
          data->append(meteo::MData(MnMath::deg2rad(latCur), MnMath::deg2rad(lonCur), -9999, FALSE_DATA));
        }

        numgrid++;

        if(isAdd2) {
          coord2 += step2;
        }
        else {
          coord2 -= step2;
        }
      }
      if (isAdd1) {
        coord1 += step1;
      }
      else {
        coord1 -= step1;
      }
      coord2 = start2;
    }
    return true;
  }

  bool GribMongoDb::fillGridWithoutBorder(meteo::GeoData* data,
                                          meteo::GeoData* left,
                                          meteo::GeoData* top,
                                          meteo::GeoData* right,
                                          meteo::GeoData* bottom,
                                          const meteo::surf::GribData& grib)
  {
    const TGrid::LatLon& msg = grib.grid_def();
    const google::protobuf::RepeatedField<google::protobuf::int32>& points = grib.points();
    const google::protobuf::RepeatedField<double>& val = grib.value();
    const int TRUE_DATA  = 1;//48
    const int FALSE_DATA = 0;//49

    double subdivision = 1e-3;
    double subdiv_max = 1e6;

    if (msg.lo1() > subdiv_max ||
        msg.lo2() > subdiv_max ||
        msg.la1() > subdiv_max ||
        msg.la2() > subdiv_max) {
      subdivision = 1e-6;
    }

    bool  isLonAdd = (msg.scanmode() & 0x80) == 0;
    bool  isLatAdd = (msg.scanmode() & 0x40) != 0;
    float wlat     = fabs(msg.la2()*subdivision - msg.la1()*subdivision);
    float wlon;

    if(isLonAdd) {
      if(msg.lo2() > msg.lo1()) {
        wlon = msg.lo2() * subdivision - msg.lo1() * subdivision;
      }
      else {
        wlon = fabs(msg.lo2() * subdivision - msg.lo1() * subdivision + 360);
      }
    }
    else if(msg.lo2() > msg.lo1()) {
      wlon = msg.lo2() * subdivision - msg.lo1() * subdivision + 360;
    }
    else {
      wlon = msg.lo1() * subdivision - msg.lo2() * subdivision;
    }

    int   isRegular = 0; //0 - да, 1 - по широте нет, 2 - по долготе нет
    float start1, start2;
    float width = 0;
    bool  isAdd1, isAdd2;
    uint  cnt1, cnt2;
    float step1, step2;

    if (msg.nj() == 0xffff || msg.nj() == 0xffffffff) {
      if(0 == points.size()) { return false; }
      isRegular = 1;
      start2    = msg.la1() * subdivision;
      start1    = msg.lo1() * subdivision;
      isAdd2    = isLatAdd;
      isAdd1    = isLonAdd;
      cnt1      = msg.ni();
      step1     = msg.di() * subdivision;
      cnt2      = points.data()[0];
      width     = wlat;
      step2     = width / (points.data()[0] - 1);
    }
    else {
      start1    = msg.la1() * subdivision;
      start2    = msg.lo1() * subdivision;
      isAdd1    = isLatAdd;
      isAdd2    = isLonAdd;
      cnt1      = msg.nj();
      step1     = msg.dj() * subdivision;
    }

    if(msg.ni() == 0xffff || msg.ni() == 0xffffffff) {
      if(0 == points.size()) { return false; }
      isRegular = 2;
      cnt2      = points.data()[0];
      width     = wlon;
      step2     = width / (points.data()[0] - 1);
    }
    else {
      cnt2      = msg.ni();
      step2     = msg.di() * subdivision;
    }

    float    coord1 = start1;
    float    coord2 = start2;
    uint64_t num     = 0;//!< позиция в данных
    uint64_t numgrid = 0;//!< позиция по координатам (данных в точке сетки может не быть, в соотв с bitmask)
    float    latCur, lonCur;

    for (int64_t cycle1 = 0; cycle1 < cnt1; ++cycle1) {
      if (0 != isRegular) {
        if (cycle1 >= points.size()) {
          debug_log << QObject::tr("Количество дополнительных точек не совпадает с параметрами сетки");
          return false;
        }

        cnt2 = points.data()[cycle1];
        step2 = width / (points.data()[cycle1] - 1);
        // var(cnt2);
        // var(step2);
      }

      for (uint64_t cycle2 = 0; cycle2 < cnt2; ++cycle2) {

        if (isRegular == 1) {
          latCur = coord2;
          lonCur = MnMath::M180To180(coord1);

          //FIXME костыль похоже:
          float coord1_vs =  coord1;
          coord1_vs += isAdd1 ? step1 : -step1;
          if (MnMath::M180To180(coord1_vs)<0 && lonCur > 0){
            lonCur = -1*lonCur;
          }

          //  debug_log << "before" << latCur <<coord1<<lonCur;
        }
        else {
          latCur = coord1;
          lonCur = MnMath::M180To180(coord2);
          //FIXME костыль похоже:
          float coord2_vs =  coord2;
          coord2_vs += isAdd2 ? step2 : -step2;
          if(MnMath::M180To180(coord2_vs)<0 && lonCur > 0){
            lonCur = -1*lonCur;
          }
        }
        //debug_log << cycle1 << cycle2 << latCur << lonCur<< val.data()[num]-273.15;
        //     if(latCur > 89.) latCur = 89.;
        //        if(latCur < -89.) latCur = -89.;
        // debug_log << "before" << latCur <<coord2<<lonCur;

        bool exists = true;
        if (grib.has_bitmap()) {
          uint64_t byte = numgrid / 8;
          uint8_t bit = numgrid % 8;
          if (byte > grib.bitmap().size()) {
            debug_log << "ERR" << byte << grib.bitmap().size();
          }
          exists = (grib.bitmap()[byte] & (0x1 << bit)) != 0;
        }

        float value = -9999;
        int mask = FALSE_DATA;
        if (exists) {
          if (num >= (uint64_t)val.size()) {
            debug_log << QObject::tr("Количество данных не совпадает с параметрами сетки");
            return false;
          }
          value = val.data()[num];
          mask  = TRUE_DATA;
          num++;
        }

        //граничные точки левой части сетки
        if (cycle2 == 0 || (cycle2 == cnt2-1 && !isAdd2) ||
            (isRegular == 1 && (cycle1 == 0 || (cycle1 == cnt1-1 && !isAdd1) ))) {
          if (isAdd1 || (isRegular == 1 && isAdd2) ) {
            left->append(meteo::MData(MnMath::deg2rad(latCur), MnMath::deg2rad(lonCur), value, mask));
          } else {
            left->prepend(meteo::MData(MnMath::deg2rad(latCur), MnMath::deg2rad(lonCur), value, mask));
          }
        }

        //граничные точки правой части сетки
        if (cycle2 == cnt2-1 || (cycle2 == 0 && !isAdd2) ||
            (isRegular == 1 && (cycle1 == cnt1-1 || (cycle1 == 0 && !isAdd1) ))) {
          if (isAdd1 || (isRegular == 1 && isAdd2) ) {
            right->append(meteo::MData(MnMath::deg2rad(latCur), MnMath::deg2rad(lonCur), value, mask));
          } else {
            right->append(meteo::MData(MnMath::deg2rad(latCur), MnMath::deg2rad(lonCur), value, mask));
          }
        }

        //граничные точки нижней части сетки
        if (cycle1 == 0 || (cycle1 == cnt1-1 && !isAdd1) ||
            (isRegular == 1 && (cycle2 == 0 || (cycle2 == cnt2-1 && !isAdd2) ))) {
          if (isAdd2 || (isRegular == 1 && isAdd1)) {
            bottom->append(meteo::MData(MnMath::deg2rad(latCur), MnMath::deg2rad(lonCur), value, mask));
          } else {
            bottom->append(meteo::MData(MnMath::deg2rad(latCur), MnMath::deg2rad(lonCur), value, mask));
          }
        }

        //граничные точки верхней части сетки
        if (cycle1 == cnt1-1 || (cycle1 == 0 && !isAdd1) ||
            (isRegular == 1 && (cycle2 == cnt2-1 || (cycle2 == 0 && !isAdd2) ))) {
          if (isAdd2 || (isRegular == 1 && isAdd1)) {
            top->append(meteo::MData(MnMath::deg2rad(latCur), MnMath::deg2rad(lonCur), value, mask));
          } else {
            top->append(meteo::MData(MnMath::deg2rad(latCur), MnMath::deg2rad(lonCur), value, mask));
          }
        }

        //серединка
        if (cycle2 > 0 && cycle2 < cnt2-1 && cycle1 > 0 && cycle1 < cnt1-1) {
          data->append(meteo::MData(MnMath::deg2rad(latCur), MnMath::deg2rad(lonCur), value, mask));
        }

        numgrid++;

        coord2 += isAdd2 ? step2 : -step2;
      }

      coord1 += isAdd1 ? step1 : -step1;
      coord2 = start2;
    }
    return true;
  }

}
