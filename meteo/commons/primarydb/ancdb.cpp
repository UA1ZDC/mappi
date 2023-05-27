#include "ancdb.h"

#include <qfile.h>
#include <qdir.h>
#include <qdatetime.h>
#include <qdatastream.h>
#include <quuid.h>

#include <sql/dbi/gridfs.h>

#include <sql/dbi/dbi.h>
#include <sql/dbi/dbiquery.h>
#include <sql/nosql/nosqlquery.h>


#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/meteo_data/complexmeteo.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <cross-commons/debug/tmap.h>

#include <meteo/commons/global/global.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/proto/state.pb.h>
#include <meteo/commons/proto/surface.pb.h>
#include <meteo/commons/proto/sigwx.pb.h>

#include <meteo/commons/proto/field.pb.h>

#define CONF_FILE MnCommon::etcPath() + "ancmongo.conf"

namespace meteo {
AncDb* ancdb()
{
  static AncDb* dbase = new AncDb;
  return dbase;
}
}

using namespace meteo;

AncDb::AncDb():
  _iiTlg(0),
  _idPtkpp(""),
  _source(""),
  _sourceType(kANCSource)
{
  _stat.set(kANCSource, kSaveOk, meteo::app::OperationState_WARN,
            QObject::tr("Сохранено сводок"));
  _stat.set(kANCSource, kSaveErr, meteo::app::OperationState_NORM,
            QObject::tr("Отброшено сводок из-за ошибок при сохранении"));

  _stat.set(kBufrSource, kSaveOk, meteo::app::OperationState_WARN,
            QObject::tr("Сохранено сводок"));
  _stat.set(kBufrSource, kSaveErr, meteo::app::OperationState_NORM,
            QObject::tr("Отброшено сводок из-за ошибок при сохранении"));
  _stat.set(kBufrSource, kRepeated, meteo::app::OperationState_NORM,
            QObject::tr("Повторных сводок radar и sigwx"));

  _stat.set(kGphSource, kSaveOk, meteo::app::OperationState_WARN,
            QObject::tr("Сохранено карт"));
  _stat.set(kGphSource, kSaveErr, meteo::app::OperationState_NORM,
            QObject::tr("Отброшено карт из-за ошибок при сохранении"));

  const ConnectProp conf = meteo::Global::instance()->mongodbConfMeteo();
  init(conf);
}

AncDb::AncDb( const ConnectProp& params ):
  _iiTlg(0),
  _idPtkpp(""),
  _source(""),
  _sourceType(kANCSource)
{
  _stat.set(kANCSource, kSaveOk, meteo::app::OperationState_WARN,
            QObject::tr("Сохранено сводок"));
  _stat.set(kANCSource, kSaveErr, meteo::app::OperationState_NORM,
            QObject::tr("Отброшено сводок из-за ошибок при сохранении"));

  _stat.set(kBufrSource, kSaveOk, meteo::app::OperationState_WARN,
            QObject::tr("Сохранено сводок"));
  _stat.set(kBufrSource, kSaveErr, meteo::app::OperationState_NORM,
            QObject::tr("Отброшено сводок из-за ошибок при сохранении"));

  _stat.set(kGphSource, kSaveOk, meteo::app::OperationState_WARN,
            QObject::tr("Сохранено карт"));
  _stat.set(kGphSource, kSaveErr, meteo::app::OperationState_NORM,
            QObject::tr("Отброшено карт из-за ошибок при сохранении"));

  _defaultDb = false;
  params_ = params;
  init(params);
}


AncDb::~AncDb()
{

  delete gridfs_;
  gridfs_ = nullptr;
  delete meta_;
  meta_ = nullptr;
}

void AncDb::init( const ConnectProp& params )
{
  params_ = params;

  gridfs_ = new GridFs();
  gridfs_->connect(params_);

  meta_ = new anc::DbMeta();
  // meta_->fillTypes();
  meta_->checkStations();
  //meta_->fillStations();


  QFile file(CONF_FILE);
  if ( !file.open(QIODevice::ReadOnly) ) {
    error_log << QObject::tr("Ошибка при загрузке конфигурации '%1'").arg(CONF_FILE);
  } else {
    QString text = QString::fromUtf8(file.readAll());
    file.close();

    if ( !TProtoText::fillProto(text, &_prop) ) {
      error_log << QObject::tr("Ошибка в структуре файла конфигурации");
    }
  }

  //_nosql = new NoSql(host, port, name, login, pass);
  db_ = meteo::global::dbMeteo();

  if ( nullptr == db_ ) {
    debug_log << QObject::tr("Не удалось подключиться к dbMeteo");
  }
}

//проверка инициализации БД и подключения
bool AncDb::checkDb()
{
  if (nullptr == gridfs_) return false;

  //если БД так и не была инициализирована (напр., ошибки настроек)
  if (nullptr == db_ && _defaultDb) {
    db_ = meteo::global::dbMeteo();

    const ConnectProp conf = meteo::Global::instance()->mongodbConfMeteo();
    params_ = conf;
    gridfs_->connect(params_);
  }

  if ( nullptr == db_ ) {
    error_log << QObject::tr("Не удалось подключиться к dbMeteo");
    return false;
  }

  //если пропало соединение
  if (!db_->connected()) {
    db_->connect();
    gridfs_->connect(params_);
  }

  return true;
}

void AncDb::setConnectionStatus(meteo::app::OperationStatus* status)
{
  if (nullptr == status) { return; }

  meteo::app::OperationParam* param = status->add_param();
  param->set_title( QObject::tr("Подключение к БД").toUtf8().constData() );
  if (nullptr != db_ && nullptr != gridfs_ && db_->connected()) {
    param->set_value(QObject::tr("Есть").toStdString());
    param->set_state(meteo::app::OperationState_NORM);
  } else {
    param->set_value(QObject::tr("Нет").toStdString());
    param->set_state(meteo::app::OperationState_ERROR);
    status->set_state(meteo::app::OperationState_ERROR);
  }
  param->set_dt(QDateTime::currentDateTime().toString("dd.MM.yy hh:mm").toStdString());
}

bool AncDb::execQuery(DbiQuery* query)
{
  if (nullptr == db_ || nullptr == gridfs_) return false;

  if (!db_->connected()) {
    db_->connect();
    gridfs_->connect(params_);
  }

  return db_->execQuery(query);
}

//! Сохранение радиолокационных данных
bool AncDb::saveRadarMap(const TMeteoData* data, int category,
                         const QDateTime& dt, const QString& collection)
{
  if (!checkDb()) {
    _stat.add(_sourceType, kSaveErr, meteo::app::OperationState_WARN);
    return false;
  }

  if (false == meteo::global::kMongoQueriesNew.contains("insert_radar")) {
    return false;
  }

  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();

  meteo::GeoPoint gp;
  if(false == d->getCoord(*data, &gp)) {
    debug_log << QObject::tr("Не удалось получить координаты из TMeteoData");
    _stat.add(_sourceType, kSaveErr, meteo::app::OperationState_WARN);
    return false;
  }

  if (!dt.isValid()) {
    debug_log << QObject::tr("Ошибка даты/времени в данных dt='%1'\n").arg(dt.toString("yyyy-MM-dd hh:mm:ss"));
    _stat.add(_sourceType, kSaveErr, meteo::app::OperationState_WARN);
    return false;
  }

  QByteArray msg;
  (*data) >> msg;

  //--
  float h1 = 0, h2 = 0;
  control::QualityControl q1, q2;

  if (data->hasParam(d->descriptor("ha"))) {
    data->getValue(d->descriptor("ha"), 0, &h1, &q1);
    data->getValue(d->descriptor("ha"), 100, &h2, &q2);
  } else if (data->hasParam(d->descriptor("h0"))) {
    data->getValue(d->descriptor("h0"), 0, &h1, &q1);
    data->getValue(d->descriptor("h0"), 100, &h2, &q2);
  }

  descr_t productDescr = data->getValue(d->descriptor("product"), BAD_METEO_ELEMENT_VAL);
  int count = 0;
  if (productDescr != BAD_METEO_ELEMENT_VAL) {
    count = data->countParam(productDescr, true);
  }
  int product = data->getValue(d->descriptor("product"), BAD_METEO_ELEMENT_VAL);

  QString station = d->stationIdentificator(*data);
  //--

  QString filename = dt.toString("yyyyMMdd_hhmmss") + station + "_" +
      QString::number(h1, 'f', 0) + "_" + QString::number(h2, 'f', 0) + "_" +
      QString::number(count) + "_" + QString::number(product);

  GridFile gridfile;
  bool ok = saveToDb(collection, filename, msg, &gridfile);
  if (!ok) {
    _stat.add(_sourceType, kSaveErr, meteo::app::OperationState_WARN);
    return false;
  }

  if (gridfile.id().isEmpty()) {
    _stat.add(_sourceType, kRepeated, meteo::app::OperationState_NORM);
    return true;
  }

  auto query = db_->queryptrByName("insert_radar");
  if(nullptr == query) {
    return false;
  }
  query->arg("collection",collection);
  query->arg("dt_write",QDateTime::currentDateTimeUtc());
  query->arg("dt",dt);
  query->arg("station",station);
  query->arg("h1",h1);
  query->arg("h2",h2);

  if (BAD_METEO_ELEMENT_VAL != product) {
    query->arg("product",product);
  }
  query->arg("count",count);
  query->arg("fileid", (int64_t)( gridfile.id().toLongLong() ) );
  query->arg("tlgid",_idPtkpp);
  query->arg("data_type",category);
  query->arg("source",_source);
  query->arg("ii",_iiTlg);

  ok = query->exec();

  if (ok) {
    _stat.add(_sourceType, kSaveOk, meteo::app::OperationState_NORM);
  } else {
    _stat.add(_sourceType, kSaveErr, meteo::app::OperationState_NORM);
    gridfile.remove();
  }

  return ok;
}

bool AncDb::saveSigWx(const sigwx::Header& header, const QByteArray& ba, int category,
                      const QDateTime& dt, const QString& collection)
{
  if (!checkDb()) {
    _stat.add(_sourceType, kSaveErr, meteo::app::OperationState_WARN);
    return false;
  }

  QDateTime beg = DbiQuery::datetimeFromString(header.dt_beg());
  QDateTime end = DbiQuery::datetimeFromString(header.dt_end());

  QString filename = beg.toString("yyyyMMddhhmm") + end.toString("yyyyMMddhhmm") + "_" +
      QString::number(header.center()) + "_" +
      QString::number(header.h_lo(), 'f', 0) + "_" + QString::number(header.h_hi(), 'f', 0) + "_" +
      QString::number(header.descr()) + "_" + QString::number(header.value());

  //uint id = qHash(QTime::currentTime().toString("hhmmsszzz") + _idPtkpp + QString::number(_cnt));
  GridFile gridfile;
  bool ok = saveToDb(collection, filename, ba, &gridfile);
  if (!ok) {
    _stat.add(_sourceType, kSaveErr, meteo::app::OperationState_WARN);
    return false;
  }

  if ( true == gridfile.id().isEmpty() ) {
    //debug_log << "exist";
    _stat.add(_sourceType, kRepeated, meteo::app::OperationState_NORM);
    return true;
  }

  auto query = db_->queryptrByName("insert_sigwx");
  if ( nullptr == query ) {
    return false;
  }

  query->arg("collection", collection);
  query->arg("dt_write", QDateTime::currentDateTimeUtc());
  query->arg("dt", dt);
  query->arg("center", (int16_t)header.center() );
  query->arg("dt_beg",beg);
  query->arg("dt_end",end);
  query->arg("level_type",(int16_t)header.lev_type());
  query->arg("level_lo",(int32_t)header.h_lo());
  query->arg("level_hi",(int32_t)header.h_hi());
  query->arg("descriptor",(int32_t)header.descr());
  query->arg("value",(int32_t)header.value());
  query->argOid("fileid",gridfile.id());
  query->arg("tlgid", (int64_t)_idPtkpp.toLongLong() );
  query->arg("data_type",(int16_t)category);
  query->arg("source",_source);
  query->arg("ii",(int16_t)_iiTlg);

  ok = query->exec();
  if (ok) {
    _stat.add(_sourceType, kSaveOk, meteo::app::OperationState_NORM);
  }
  else {
    _stat.add(_sourceType, kSaveErr, meteo::app::OperationState_NORM);
    gridfile.remove();
  }

  return ok;
}



bool AncDb::fillStationInfo(TMeteoData* data, int category, StationInfo* info)
{
  if(nullptr == meta_) return false;
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();

  const StationInfo* meta = nullptr;
  QString station_id;

  if (category == meteo::surf::kAerodrome) {
    station_id = TMeteoDescriptor::instance()->stationName(*data);
    meta = meta_->aerodromeInfo(station_id);
  }
  else {
    station_id = d->stationIdentificator(*data);
    if (station_id.isEmpty() && //в гелиофизике может идти название станции
        category >= meteo::surf::kGeophysicsType && category < meteo::surf::kEndGeophysicsType) {
      station_id = TMeteoDescriptor::instance()->stationName(*data);
    }
    meta = meta_->stationInfo(category, station_id);
  }

  info->index = d->station(*data);
  info->icao = data->getCode(d->descriptor("CCCC"), false);
  info->buoy =  d->buoy(*data);
  info->call_sign = data->getCode(d->descriptor("D____D"), false);
  info->name =  data->getCode(d->descriptor("station"), false);
  info->satellite =  data->getCode(d->descriptor("sat"), false);
  info->instrument = data->getCode(d->descriptor("instr"), false);
  GeoPoint coord;
  bool okcoord = d->getCoord(*data, &coord);

  if (!okcoord && category >= meteo::surf::kGeophysicsType && category <= meteo::surf::kEndGeophysicsType &&
      (meta == nullptr || !meta->coord.isValid())) {
    data->setCoord(0, 0, 0);
    okcoord = true;
    // info->type = 9;
  }
  QDateTime dt = TMeteoDescriptor::instance()->dateTime(*data);

  if (!okcoord && (nullptr == meta || !meta->coord.isValid())) {
    error_log << QObject::tr("Не найдена станция '%1' для типа данных '%2' (id='%3')")
                 .arg(station_id)
                 .arg(category)
                 .arg(_idPtkpp);
    if ( nullptr == meta ) {
      //debug_log << QObject::tr("Не найдена информация!");
    }
    else {
      //debug_log << QObject::tr("Инфо =") << meta->toString();
    }
    //station, int station_type,int data_type, const QDateTime & dt
    updateStationHistory(station_id, -1, category, dt);
    return false;
  }

  if (nullptr == meta) {
    info->type = meta_->stationType(category);
    //var(info->type);
    return true;
  }


  //  if (!okcoord) {
  if (meta->coord.isValid()) {  //чтоб из станциий координаты, а не из bufr, чтоб не отличались от anc
    data->setCoord(meta->coord.latDeg(), meta->coord.lonDeg(), meta->coord.alt());
  }

  info->type = meta->type;
  info->country = meta->country;
  info->country_en = meta->country_en;
  info->country_ru = meta->country_ru;

  if (!meta->icao.isEmpty()) {
    info->icao = meta->icao;
  }
  if (meta->index != BAD_METEO_ELEMENT_VAL) {
    info->index = meta->index;
  }
  if (!meta->name.isEmpty()) {
    info->name = meta->name;
  }
  if (!meta->name_ru.isEmpty()) {
    info->name_ru = meta->name_ru;
  }
  // var(gp);

  if (!info->icao.isEmpty()) {
    data->set(TMeteoDescriptor::instance()->descriptor("CCCC"),
              TMeteoParam(info->icao, BAD_METEO_ELEMENT_VAL, control::RIGHT));
  }
  if (BAD_METEO_ELEMENT_VAL != info->index && info->index > 1000) {
    data->setStation(info->index);
  }

  if (-1 == info->type) {
    debug_log << QObject::tr("Не найден тип станции для типа данных %1").arg(category);
    if(false == station_id.isEmpty())
    {
      updateStationHistory(station_id, -1, category, dt);
    }
    return false;
  }

  return true;
}



//! сохранение набора параметров с одним заголовком
bool AncDb::saveReport(const TMeteoData &data, int category, const QDateTime& dt,
                       const StationInfo& info, const QString &collection)
{
  QTime ttt;
  ttt.start();

  ComplexMeteo cm;
  cm.fill(data);
  if (0 == cm.count() && 0 == cm.childsCount()) {
    return true;
  }
  bool withName = info.type == meteo::sprinf::kStationGeophysics;

  meteo::GeoPoint gp = cm.coord();
  QString station = TMeteoDescriptor::instance()->stationIdentificator(data, gp, withName);
  // if (!gp.isValid()) { NOTE координаты могут быть у child
  //   debug_log << QObject::tr("Не удалось получить координаты из TMeteoData, станция '%1'").arg(station);
  //   _stat.add(_sourceType, kSaveErr, meteo::app::OperationState_WARN);
  //   return false;
  // }

  QMap<QString, QString> header = TMap<QString, QString>()
      << qMakePair(QString("dt"), dt.toString(Qt::ISODate))
      << qMakePair(QString("station"), station)
      << qMakePair(QString("station_type"), QString::number(info.type))
      << qMakePair(QString("data_type"), QString::number(category))
      << qMakePair(QString("tlgid"), _idPtkpp)
      << qMakePair(QString("dt_write"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

  QString jinfo = createStationInfo(info);
  bool notEmptyFlag = false;//true - если были данные
  bool ok = false;
  updateStationHistory(station, info.type, category, dt);

  if (info.type == meteo::sprinf::kStationGeophysics) {
    ok = saveGeophysData(gp, header, jinfo, cm, collection, &notEmptyFlag);
  } else {
    ok = saveReportData(gp, header, jinfo, cm, collection, &notEmptyFlag);
  }

  if (notEmptyFlag) {
    if (ok) {
      _stat.add(_sourceType, kSaveOk, meteo::app::OperationState_NORM);
    } else {
      _stat.add(_sourceType, kSaveErr, meteo::app::OperationState_NORM);
    }
  }

  //debug_log << "new time save=" << ttt.elapsed() << _idPtkpp;

  return ok;
}



bool AncDb::saveReportData(const GeoPoint& point, const QMap<QString, QString>& header, const QString& jinfo,
                           const ComplexMeteo& cm, const QString& collection,  bool* notEmptyFlag)
{
  bool ok = true;

  if (!checkDb()) {
    _stat.add(_sourceType, kSaveErr, meteo::app::OperationState_WARN);
    return false;
  }

  if (0 == cm.count() && 0 == cm.childs().count() ) {
    //debug_log << "empty";
    //Пустая метеодата. Ничего не делаем
    return ok;
  }
  //вставка/поиск данных с таким же заголовков
  QString querys = createMeteoReportQuery(cm, header, point, jinfo, collection);
  if (!querys.isEmpty()) {

    DbiQuery* query = db_->query(querys);
    bool fl = execQuery(query);
    if ( false == fl ) {
      debug_log << QObject::tr("Не удалось выполнить запрос = %1").arg(querys);
      delete query;
      return false;
    }
    QMap<QString, Document> paramPrev;
    QString oid;
    bool upserted = false;
    //обновление данных, если уже есть в БД
    if (fillPrevParams(query->result(), &paramPrev, &oid, &upserted) && !upserted) {
      QString updparam = createUpdateParamsJson(cm, header, paramPrev, oid, collection);
      //var(updparam);
      if (!updparam.isEmpty()) {
        query->setQuery(updparam);
        if (!execQuery(query)) {
          debug_log << QObject::tr("Не удалось выполнить запрос = %1")
                       .arg(updparam);
          ok = false;
        }
      }
    }

    *notEmptyFlag = true;
    delete query;

  }

  for(const TMeteoData* ch : cm.childs()) {
    const ComplexMeteo* complch = static_cast<const ComplexMeteo*>(ch);
    if (0 == complch->count() && 0 == complch->childsCount() ) {
      continue;
    }
    if ( complch->levelType() == cm.levelType() && complch->levelLo() == cm.levelLo() ) {
      continue;
    }
    ok |= saveReportData(point, header, jinfo, *complch, collection, notEmptyFlag);
  }

  return ok;
}

//! параметры, которые добавляются при вставке нового документа
QString AncDb::createUpsertedParams(const ComplexMeteo& cm, const QMap<QString, QString>& header, const QString& uuid)
{
  //параметры в родительской
  QString addparam = createParentUpsertedParams(cm, header, uuid);

  //параметры в childs
  for ( auto ch : cm.childs() ) {
    QString chuuid;
    const ComplexMeteo* complch = static_cast<const ComplexMeteo*>(ch);
    if ( complch->levelType() != cm.levelType() || complch->levelLo() != cm.levelLo() ) {
      continue;
    }
    if ( complch->descriptor() != BAD_DESCRIPTOR_NUM) {
      chuuid = QUuid::createUuid().toString();
    }
    auto chadd = createUpsertedParams( *complch, header, chuuid );
    if ( false == chadd.isEmpty() ) {
      chadd.remove(0,1);
      chadd.remove(-1,1);
      chadd += ",";
      addparam += chadd;
    }
  }

  if (!addparam.isEmpty()) {
    addparam.remove(-1, 1);
    addparam = QString("[") + addparam + "]";
  } else {
    //debug_log << "empty" << header.value("tlgid");
  }

  return addparam;
}


//! параметры, которые добавляются при вставке нового документа
QString AncDb::createParentUpsertedParams(const ComplexMeteo& cm, const QMap<QString, QString>& header, const QString& uuid)
{
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
  QString addparam;

  for (auto it = cm.data().cbegin(); it != cm.data().cend(); ++it) {

    //debug_log << it.key() << d->isIdentDescr(it.key());

    if (d->isIdentDescr(it.key()))
    { continue; }
    //for (const TMeteoParam& param : it.value()) {
    QMapIterator<int, TMeteoParam> itp(it.value());
    while (itp.hasNext()) {
      itp.next();
      const TMeteoParam& param = itp.value();
      if ( false == param.isInvalid()) {
        QString name = d->name(it.key());
        if (d->isAddParent(name)) {
          QString addname = d->findAdditional(it.key(), itp.key(), param.value());
          if (!addname.isEmpty()) {
            name = addname;
          }
        }
        addparam += createParamJson(cm, header, name, it.key(), param, uuid );
      } else {
        //debug_log << "invalid";
      }
    }
  }

  return addparam;
}

//! формирование json для вставки параметров
QString AncDb::createParamJson(const ComplexMeteo& cm, const QMap<QString, QString>& header,
                               const QString& name, descr_t descr,
                               const TMeteoParam& param, const QString& uuid )
{
  if (name.isEmpty() && false == _prop.noname() ) {
    return QString();
  }
  NosqlQuery query;
  if ( false == meteo::global::kMongoQueriesNew.contains("param") ) {
    error_log << QObject::tr("Не найден запрос = 'param'");
    return QString();
  }
  query.setQuery( meteo::global::kMongoQueriesNew["param"] );
  query.arg("descrname",name);
  query.arg("descr",(int64_t)descr);
  query.arg("value",param.value());
  query.arg("code",param.code());
  query.arg("quality",param.quality());
  query.arg("control",param.controlFlag());
  query.arg("tlgid",(int64_t)(header.value("tlgid").toLongLong()));

  if (_prop.dtwrite()) {
    query.argDt("dt_write",header.value("dt_write"));
  }
  query.arg("dt_type",cm.dtType());
  if (cm.dtBeg() != cm.dtEnd() ||
      header.value("dt") != cm.dtEnd().toString(Qt::ISODate)) {
    query.arg("dt_beg",cm.dtBeg());
    query.arg("dt_end",cm.dtEnd());

  }
  if (!uuid.isEmpty()) {
    query.arg("uuid",uuid);
  }
  query.arg("cor_number",_cor_number);
  query.removeDogs();
  return query.query() + ",";
}


//! заполнение параметров уже сохраненных в БД с тем же заголовком при их наличии
bool AncDb::fillPrevParams(const DbiEntry &doc, QMap<QString, Document>* paramPrev,
                           QString* oid, bool* upserted)
{
  if (0 == paramPrev || 0 == oid || 0 == upserted || !doc.hasField("value")) {
    return false;
  }

  *upserted = false;
  *oid = QString();
  paramPrev->clear();

  if (doc.typeField("value") == BSON_TYPE_DOCUMENT) {
    Document valDoc = doc.valueDocument("value");
    *oid = valDoc.valueOid("_id");

    if ( valDoc.hasField("param")) {
      Array arr = valDoc.valueArray("param");
      while(arr.next()) {
        Document param;
        arr.valueDocument(&param);
        //var(param.jsonString());
        QString name = param.valueString("descrname");
        if (!name.isEmpty()) {
          paramPrev->insert(name, param);
        }
      }
    }
  } else if (doc.hasField("lastErrorObject") &&
             doc.valueDocument("lastErrorObject").hasField("upserted")) {
    *oid = doc.valueDocument("lastErrorObject").valueOid("upserted");
    *upserted = true;
    //var(upserted);
    //ok = true;
  }

  if (oid->isEmpty()) {
    debug_log << "oid empty";
    return false;
  }

  return true;
}

bool AncDb::saveGeophysData(const GeoPoint& point, const QMap<QString, QString>& header, const QString& jinfo,
                            const ComplexMeteo& cm, const QString& collection, bool* notEmptyFlag)
{
  bool ok = true;

  if (!checkDb()) {
    _stat.add(_sourceType, kSaveErr, meteo::app::OperationState_WARN);
    return false;
  }

  if (0 == cm.count() && 0 == cm.childs().count() ) {
    //Пустая метеодата. Ничего не делаем
    return ok;
  }
  //вставка/поиск данных с таким же заголовков
  QString querys = createGeophysReportQuery(cm, header, point, jinfo, collection);
  if (querys.isEmpty()) {
    return false;
  }
  //  var(querys);
  DbiQuery* query = db_->query(querys);
  bool fl = execQuery(query);
  if ( false == fl ) {
    debug_log << QObject::tr("Не удалось выполнить запрос = %1").arg(querys);
    delete query;
    return false;
  }
  QMap<QString, Document> paramPrev;
  QString oid;
  bool upserted = false;
  //обновление данных, если уже есть в БД
  if (fillPrevParams(query->result(), &paramPrev, &oid, &upserted) && !upserted) {
    QString updparam = createUpdateParamsJson(cm, header, paramPrev, oid, collection);
    //var(updparam);
    if (!updparam.isEmpty()) {
      query->setQuery(updparam);
      if (!execQuery(query)) {
        debug_log << QObject::tr("Не удалось выполнить запрос = %1")
                     .arg(updparam);
        ok = false;
      }
    }
  }

  *notEmptyFlag = true;

  delete query;

  return ok;
}


//! формирование запроса для вставки/получения предыдущих данных
QString AncDb::createGeophysReportQuery(const ComplexMeteo& cm,
                                        const QMap<QString, QString>& header,
                                        const GeoPoint& point, const QString& jinfo,
                                        const QString& collection)
{
  QString station = header.value("station");

  QString addparam;

  if (header.value("data_type").toShort() == surf::kGeophGeoa ||
      header.value("data_type").toShort() == surf::kGeophFlare) {
    addparam = createGeophysUpsertedParams(cm, header, "");
  } else {
    addparam = createGeophysSimpleUpsertedParams(cm, header, "");
  }

  if (addparam.isEmpty()) {
    return QString();
  }

  if ( nullptr == db_ ) { return QString(); }
  auto query = db_->queryptrByName("find_meteoparam");
  if(nullptr == query) { return QString(); }
  //добавляем документ
  query->arg("collection", collection);
  query->argDt("dt",    header.value("dt"));
  query->arg("station", station);
  query->arg("station_type", header.value("station_type").toShort());
  query->arg("data_type",    header.value("data_type").toShort());
  query->arg("level_type",   (int16_t)cm.levelType());
  query->arg("level",        cm.levelLo());
  query->arg("location",     point);

  if (-1.0f != cm.levelHi()) {
    query->arg("level_hi", cm.levelHi());
  }
  query->argJson("station_info", jinfo);
  query->argDt("dt_write", header.value("dt_write"));
  query->argJson("param", QString("[") + addparam + "]");

  return query->query();
}

//! параметры, которые добавляются при вставке нового документа
QString AncDb::createGeophysSimpleUpsertedParams(const ComplexMeteo& cm, const QMap<QString, QString>& header, const QString& uuid)
{
  QString addparam = createParentUpsertedParams(cm, header, uuid);

  QString chadd;

  for ( auto ch : cm.childs() ) {

    QString chuuid;
    const ComplexMeteo* complch = static_cast<const ComplexMeteo*>(ch);

    if ( complch->descriptor() != BAD_DESCRIPTOR_NUM) {
      chuuid = QUuid::createUuid().toString();
    }
    chadd += createGeophysSimpleUpsertedParams(*complch, header, chuuid) + ",";
  }
  if ( false == chadd.isEmpty() ) {
    //  chadd.remove(0, 1);
    addparam += chadd;
  }

  // var(addparam);
  // var(chadd);

  if (!addparam.isEmpty()) {
    addparam.remove(-1, 1);
  }

  return addparam;
}


//! параметры, которые добавляются при вставке нового документа
QString AncDb::createGeophysChildUpsertedParams(const ComplexMeteo& cm, const QMap<QString, QString>& header, const QString& uuid)
{
  QString addparam = createParentUpsertedParams(cm, header, uuid);


  QString chadd;

  for ( auto ch : cm.childs() ) {

    QString chuuid;
    const ComplexMeteo* complch = static_cast<const ComplexMeteo*>(ch);

    if ( complch->descriptor() != BAD_DESCRIPTOR_NUM) {
      chuuid = QUuid::createUuid().toString();
    }
    chadd += createGeophysUpsertedParams(*complch, header, chuuid) + ",";
  }
  if ( false == chadd.isEmpty() ) {
    //  chadd.remove(0, 1);
    addparam += chadd;
  }

  // var(addparam);
  // var(chadd);

  if (!addparam.isEmpty()) {
    addparam.remove(-1, 1);
  }

  return addparam;
}


//! параметры, которые добавляются при вставке нового документа
QString AncDb::createGeophysUpsertedParams(const ComplexMeteo& cm, const QMap<QString, QString>& header, const QString& uuid)
{
  QString addparam = createParentUpsertedParams(cm, header, uuid);

  QString chadd;

  for ( auto ch : cm.childs() ) {
    QString chuuid;
    const ComplexMeteo* complch = static_cast<const ComplexMeteo*>(ch);

    if ( complch->descriptor() != BAD_DESCRIPTOR_NUM) {
      chuuid = QUuid::createUuid().toString();
    }

    QString res = QString("{ \"param\" : [") + createGeophysChildUpsertedParams(*complch, header, chuuid) + " ]}" + ",";
    chadd += res;
  }

  addparam += chadd;

  if (!addparam.isEmpty()) {
    addparam.remove(-1, 1);
  }

  // var(addparam);
  // var(chadd);

  QString result;
  if (!addparam.isEmpty()) {
    result =  addparam;
  }

  //var(result);

  return result;
}


void AncDb::updateStationHistory(const QString& station, int station_type,int data_type, const QDateTime &dt)
{
  if ( nullptr == db_ ) {  return; }
  auto query = db_->queryptrByName("update_station_history");
  if(nullptr == query) {  return; }
  query->arg("dt",dt);
  query->arg("station",station);
  query->arg("station_type",station_type);
  query->arg("data_type",data_type);
 // debug_log<<"update_station_history"<<query->query();
  if(true != query->exec()){
    error_log<<"EROR update_station_history"<<query->query();
  }
}


//! сохранение набора параметров с одним заголовком, без дублей, с обновлением инфы
bool AncDb::updateMeteoData(const TMeteoData &data, int category, const QDateTime& dt,
                            const StationInfo& info, const QString &collection)
{
  QTime ttt;
  ttt.start();

  if (!checkDb()) {
    _stat.add(_sourceType, kSaveErr, meteo::app::OperationState_WARN);
    return false;
  }

  //data.printData();

  ComplexMeteo cm;
  cm.fill(data);
  //cm.printData();

  if (0 == cm.count() && 0 == cm.childsCount()) {
    return true;
  }

  bool withName = info.type == meteo::sprinf::kStationGeophysics;

  meteo::GeoPoint gp = cm.coord();
  QString station = TMeteoDescriptor::instance()->stationIdentificator(data, gp, withName);
  // if (!gp.isValid()) { NOTE координаты могут быть у child
  //   debug_log << QObject::tr("Не удалось получить координаты из TMeteoData, станция '%1'").arg(station);
  //   _stat.add(_sourceType, kSaveErr, meteo::app::OperationState_WARN);
  //   return false;
  // }

  QMap<QString, QString> header = TMap<QString, QString>()
      << qMakePair(QString("dt"), dt.toString(Qt::ISODate))
      << qMakePair(QString("station"), station)
      << qMakePair(QString("station_type"), QString::number(info.type))
      << qMakePair(QString("data_type"), QString::number(category))
      << qMakePair(QString("tlgid"), _idPtkpp)
      << qMakePair(QString("dt_write"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
  QString jinfo = createStationInfo(info);
  bool ok = updateMeteoData(gp, header, jinfo, cm, collection);

  if (ok) {
    _stat.add(_sourceType, kSaveOk, meteo::app::OperationState_NORM);
  } else {
    _stat.add(_sourceType, kSaveErr, meteo::app::OperationState_NORM);
  }
  //debug_log << "new time save=" << ttt.elapsed() << _idPtkpp;
  return ok;
}

bool AncDb::updateMeteoData(const GeoPoint& point,
                            const QMap<QString, QString>& header,
                            const QString& jinfo,
                            const ComplexMeteo& cm,
                            const QString& collection)
{
  bool ok = true;

  QString uuid;
  if (cm.descriptor() != BAD_DESCRIPTOR_NUM) {
    uuid = QUuid::createUuid().toString();
  }

  if (0 == cm.count() ) {
    return false;
  }


  //вставка/поиск данных с таким же заголовков
  QString querys = createUpdFindAndModifyJson(cm, header, point, uuid, jinfo, collection);
  //var(query);
  if (!querys.isEmpty()) {
    DbiQuery* query = db_->query(querys);
    bool fl = execQuery(query);
    if ( false == fl ) {
      debug_log << QObject::tr("Не удалось выполнить запрос = %1").arg(querys);
      delete query;
      return false;
    }

    QMap<QString, Document> paramPrev;
    QString oid;
    bool upserted = false;

    //обновление данных, если уже есть в БД
    if (fillPrevParams(query->result() , &paramPrev, &oid, &upserted)&&!upserted) {
      QString updparam = createParamsJson(cm, header, uuid, paramPrev, oid, collection);
      //var(updparam);
      if (!updparam.isEmpty()) {
        query->setQuery(updparam);
        if (!execQuery(query)) {
          debug_log << QObject::tr("Не удалось выполнить запрос = %1")
                       .arg(updparam);
          ok = false;
        }
      }
    }

    delete query;
  }

  for(const TMeteoData* ch : cm.childs()) {
    const ComplexMeteo* complch = static_cast<const ComplexMeteo*>(ch);
    if (0 != complch->count() || 0 != complch->childsCount()) {
      ok |= updateMeteoData(point, header, jinfo, *complch, collection);
    }
  }

  return ok;
}


//! формирование json для дополнения, замены параметров, если в БД с таким заголовком уже есть
QString AncDb::createParamsJson(const ComplexMeteo& cm,
                                const QMap<QString, QString>& header,
                                const QString& uuid, QMap<QString, Document>& paramPrev,
                                const QString& oid, const QString& collection)
{
  if (false == meteo::global::kMongoQueriesNew.contains("update_meteoparam")) {
    return QString();
  }

  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
  QString updparam, addparam;

  for (auto it = cm.data().cbegin(); it != cm.data().cend(); ++it) {
    if (d->isIdentDescr(it.key())) {
      continue;
    }

    QMapIterator<int, TMeteoParam> itp(it.value());
    while (itp.hasNext()) {
      itp.next();
      const TMeteoParam& param = itp.value();

      if ( false == param.isInvalid()) {
        QString name = d->name(it.key());
        if (d->isAddParent(name)) {
          QString n = d->findAdditional(it.key(), itp.key(), param.value());
          if (!n.isEmpty()) {
            name = n;
          }
        }
        if (!name.isEmpty() && paramPrev.contains(name)) {
          //обновляем, если уже есть и значения новые
          if (!paramsEqual(paramPrev[name], param, cm)) {
            updparam += setParamJson(cm, header, name, param, uuid, oid) + ",";
          }
        } else { //добавляем, если нет
          addparam += createParamJson(cm, header, name, it.key(), param, uuid );
        }
      }
    }
  }

  if (!updparam.isEmpty()) {
    updparam.remove(-1, 1);
  }

  if ( nullptr == db_ ) {  return QString(); }
  auto query = db_->queryptrByName("push_meteoparam");
  if(nullptr == query) {  return QString(); }

  QString addjson;
  if (!addparam.isEmpty()) {
    addparam.remove(-1, 1);
    query->argOid("id",oid);
    query->argJson("param",addparam);
    addjson = query->query();
    //var(addjson);

    if (!updparam.isEmpty()) {
      updparam += ", ";
    }
    updparam += addjson;
  }

  if (updparam.isEmpty()) {
    return QString();
  }

  return updateParamsJson(updparam,collection);
}

//-----------------

bool AncDb::paramsEqual(const Document& prev, const TMeteoParam& param, const ComplexMeteo& cm)
{
  QDateTime dtbeg = cm.dtBeg();
  if (prev.hasField("dt_beg")) {
    dtbeg = prev.valueDt("dt_beg");
    dtbeg.setTimeSpec(Qt::UTC);
  }
  QDateTime dtend = cm.dtEnd();
  if (prev.hasField("dt_end")) {
    dtend = prev.valueDt("dt_end");
    dtend.setTimeSpec(Qt::UTC);
  }

  return (prev.valueInt32("quality") == param.quality() &&
          qFuzzyCompare((float)prev.valueDouble("value"), param.value()) &&
          prev.valueString("code")   == param.code() &&
          prev.valueInt32("dt_type") == cm.dtType() &&
          dtbeg == cm.dtBeg() &&
          dtend == cm.dtEnd());
}


//! формирование запроса для вставки/получения предыдущих данных
QString AncDb::createUpdFindAndModifyJson(const ComplexMeteo& cm, const QMap<QString, QString>& header,
                                          const GeoPoint& point, const QString& uuid, const QString& jinfo,
                                          const QString& collection)
{
  meteo::GeoPoint gp(point);
  if (!gp.isValid() || cm.levelLo() < 0 || meteodescr::kUnknownLevel == cm.levelType()) {
    return QString();
  }

  QString addparam = createUpsertedParams(cm, header, uuid);
  if (addparam.isEmpty()) {
    return QString();
  }

  if ( nullptr == db_ ) {  return QString(); }
  auto query = db_->queryptrByName("findupd_meteoparam");
  if(nullptr == query) {  return QString(); }
  query->arg("collection",collection);
  query->argDt("dt",header.value("dt"));

  //добавляем документ

  query->arg("station",header.value("station"));
  query->arg("station_type",header.value("station_type").toInt());
  query->arg("data_type",header.value("data_type").toInt());
  query->arg("level_type",cm.levelType());
  query->arg("level",cm.levelLo());
  query->arg("location",gp);

  if (-1.0f != cm.levelHi()) {
    query->arg("level_hi",cm.levelHi());
  }

  query->argJson("param",addparam);
  query->argJson("station_info",jinfo);
  query->argDt("dt_write",header.value("dt_write"));

  return query->query();
}

//! сохранение бинарных данных в бд
bool AncDb::saveToDb(const QString& collection, const QString& fileName,
                     const QByteArray& msg, GridFile* gridfile)
{
  if (0 == msg.length() || nullptr == gridfs_ || nullptr == gridfile) {
    return false;
  }

  gridfs_->use( params_.name(), collection);

  GridFile old = gridfs_->findOne(QString("{ \"filename\": \"%1\" }").arg(fileName));
  if ( old.isValid() && ! old.id().isEmpty()) { return true; }


  gridfs_->put(fileName, msg, gridfile, false);

  if (!gridfile->isValid() || gridfile->id().isEmpty()) {
    return false;
  }

  return true;
}


//! формирование запроса для вставки/получения предыдущих данных
QString AncDb::createMeteoReportQuery(const ComplexMeteo& cm,
                                      const QMap<QString, QString>& header,
                                      const GeoPoint& point, const QString& jinfo,
                                      const QString& collection)
{

  //NOTE поиск по координатам нужен для добавления нескольких точек от одной станции (FM62)
  //для фиксированных станций может создавать дубликаты при малых отличиях в координатах
  QString station = header.value("station");

  meteo::GeoPoint gp(point);
  if (!gp.isValid()) {
    gp = cm.coord();
    if (gp.isValid()) {
      station = QObject::tr("%1, %2").arg(gp.strLat()).arg(gp.strLon());
    }
  }

  //уровней может не быть с пустыми данными
  if (!gp.isValid() || cm.levelLo() < 0 || meteodescr::kUnknownLevel == cm.levelType()) {
    //debug_log<< "err level:" << gp.isValid() <<  cm.levelLo() << cm.levelType();
    return QString();
  }

  QString addparam = createUpsertedParams(cm, header, "");
  if (addparam.isEmpty()) {
    return QString();
  }

  if ( nullptr == db_ ) {  return QString(); }
  auto query = db_->queryptrByName("find_meteoparam");
  if(nullptr == query) {  return QString(); }
  //добавляем документ
  query->arg("collection",collection);
  query->argDt("dt",header.value("dt"));
  query->arg("station",station);
  query->arg("station_type",header.value("station_type").toShort());
  query->arg("data_type",header.value("data_type").toShort());
  query->arg("level_type",(int16_t)cm.levelType());
  query->arg("level",cm.levelLo());
  query->arg("location",gp);

  if (-1.0f != cm.levelHi()) {
    query->arg("level_hi",cm.levelHi());
  }
  query->argJson("station_info",jinfo);
  query->argDt("dt_write",header.value("dt_write"));
  query->argJson("param",addparam);


  return query->query();
}


//! формирование json для дополнения, замены параметров,
//!  если в БД с таким заголовком уже есть
QString AncDb::createUpdateParamsJson(const ComplexMeteo& cm,
                                      const QMap<QString, QString>& header,
                                      QMap<QString, Document>& paramPrev,
                                      const QString& oid,
                                      const QString& collection)
{
  if (false == meteo::global::kMongoQueriesNew.contains("push_meteoparam") ||
      false == meteo::global::kMongoQueriesNew.contains("pull_meteoparam")) {
    return QString();
  }

  QString addparam;
  QStringList pullparam;

  fillCreateUpdateParamsJson( cm, header, paramPrev, "", &addparam, &pullparam );

  //удаляем последнюю запятую
  if (!addparam.isEmpty()) {
    addparam.remove(-1, 1);
  }
  QString pulljson;
  if ( nullptr == db_ ) { return QString(); }
  if ( false == pullparam.isEmpty() ) {
    auto query = db_->queryptrByName("pull_meteoparam");
    if(nullptr == query) {  return QString(); }
    query->argOid("id", oid);
    query->arg("descrname", pullparam);
    pulljson = query->query();
  }

  QString addjson;
  if (!addparam.isEmpty()) {
    auto query = db_->queryptrByName("push_meteoparam");
    if(nullptr == query) {  return QString(); }
    query->argOid("id", oid);
    query->argJson("param", addparam);

    addjson = query->query();
    //var(addjson);

    if (!pulljson.isEmpty()) {
      pulljson += ", ";
    }
    pulljson += addjson;
  }
  if ( true == pulljson.simplified().isEmpty() ) {
    return "";
  }
  auto query = db_->queryptrByName("update_meteoparam");
  if(nullptr == query) {  return QString(); }
  query->argOid("collection", collection);
  query->argJson("param", pulljson);
  return query->query();
}

//! json для дубликатов
QString AncDb::createDublParamJson(const ComplexMeteo& /*cm*/, const QMap<QString, QString>& header,
                                   const QString& name, descr_t descr,
                                   const TMeteoParam& param, const QString& uuid, const QString& oid)
{
  if (!_prop.dubl()) {
    return QString();
  }

  if ( nullptr == db_ ) { return QString(); }
  auto query = db_->queryptrByName("dublparam");
  if(nullptr == query) {  return QString(); }

  query->argOid("id",oid);
  query->arg("descrname",name);
  query->arg("descr",(int64_t)descr);
  query->arg("value",param.value());
  query->arg("code",param.code());
  query->arg("quality",param.quality());
  query->arg("control",param.controlFlag());
  query->arg("tlgid",(int64_t)(header.value("tlgid").toLongLong()));
  query->argDt("dt_write",header.value("dt_write"));

  if (!uuid.isEmpty()) {
    query->arg("uuid",uuid);
  }
  return query->query() + ",";
}


//! json для параметров, пок качества которых выше, чем уже сохраненных
QString AncDb::updateParamJson(const ComplexMeteo& /*cm*/, const QMap<QString, QString>& header,
                               const QString& name, descr_t descr, const TMeteoParam& param,
                               const QString& uuid, Document& paramPrev, const QString& oid)
{

  if ( nullptr == db_ ) { return QString(); }
  auto query = db_->queryptrByName("updateparam");
  if(nullptr == query) {  return QString(); }

  query->argOid("id",oid);
  query->arg("descrname",name);
  query->arg("descr",(int64_t)descr);

  query->arg("value",param.value());
  query->arg("code",param.code());
  query->arg("quality",param.quality());
  query->arg("control",param.controlFlag());
  query->arg("tlgid",(int64_t)(header.value("tlgid").toLongLong()));

  if (_prop.dtwrite()) {
    query->argDt("dt_write",header.value("dt_write"));
  }
  if (!uuid.isEmpty()) {
    query->arg("uuid",uuid);
  }

  if (_prop.dubl()) {
    query->arg("d_value",paramPrev.valueDouble("value"));
    query->arg("d_code",paramPrev.valueString("code"));
    query->arg("d_quality",paramPrev.valueInt32("quality"));
    query->arg("d_control",paramPrev.valueInt32("control"));
    query->arg("d_tlgid",paramPrev.valueInt64("tlgid"));
    if (_prop.dtwrite()) {
      query->arg("d_dt_write",paramPrev.valueDt("dt_write"));
    }
    if (!uuid.isEmpty()) {
      query->arg("d_uuid",paramPrev.valueString("uuid"));
    }
  }

  return query->query() + ",";
}

//! формирование json для дополнения, замены параметров, если в БД с таким заголовком уже есть
QString AncDb::updateParamsJson(const QString& updparam, const QString& collection)
{

  if (updparam.isEmpty()) {
    return QString();
  }
  if ( nullptr == db_ ) { return QString(); }
  auto query = db_->queryptrByName("update_meteoparam");
  if(nullptr == query) {  return QString(); }
  query->arg("collection",collection);
  query->argJson("param",updparam);

  return query->query();
}


//! json для параметров, пок качества которых выше, чем уже сохраненных
QString AncDb::setParamJson(const ComplexMeteo& cm, const QMap<QString, QString>& header,
                            const QString& name, const TMeteoParam& param,
                            const QString& uuid, const QString& oid)
{
  if ( nullptr == db_ ) { return QString(); }
  auto query = db_->queryptrByName("setparam");
  if(nullptr == query) {  return QString(); }
  query->argOid("id",oid);
  query->arg("descrname",name);
  query->arg("value",param.value());
  query->arg("code",param.code());
  query->arg("quality",param.quality());
  query->arg("control",param.controlFlag());
  query->arg("tlgid",(int64_t)(header.value("tlgid").toLongLong()));

  query->argDt("dt_write",header.value("dt_write"));
  query->arg("dt_type",cm.dtType());
  query->arg("dt_beg",cm.dtBeg());
  query->arg("dt_end",cm.dtEnd());
  query->arg("uuid",uuid);

  return query->query();
}


QString AncDb::createStationInfo(const StationInfo& info)
{
  //if ( nullptr == db_ ) { return QString(); }
  //auto query = db_->queryptrByName("station_info");
  //if(nullptr == query) {  return QString(); }
  NosqlQuery query;
  if ( false == meteo::global::kMongoQueriesNew.contains("station_info") ) {
    error_log << QObject::tr("Не найден запрос = 'station_info'");
    return QString();
  }
  query.setQuery( meteo::global::kMongoQueriesNew.value("station_info") );


  if (BAD_METEO_ELEMENT_VAL != info.index) {
    query.arg("index",info.index);
  }
  if (!info.icao.isEmpty()) {
    query.arg("icao",info.icao);
  }
  if (!info.call_sign.isEmpty()) {
    query.arg("call_sign",info.call_sign);
  }
  if (BAD_METEO_ELEMENT_VAL != info.buoy) {
    query.arg("buoy",info.buoy);
  }
  if (!info.name.isEmpty()) {
    query.arg("name",info.name);
  }
  if (!info.name_ru.isEmpty()) {
    query.arg("name_ru",info.name_ru);
  }
  if (BAD_METEO_ELEMENT_VAL != info.country) {
    query.arg("country",info.country);
  }

  if (!info.country_en.isEmpty() && BAD_METEO_ELEMENT_VAL != info.country) {
    query.arg("country_en",info.country_en);
  }
  if (!info.country_ru.isEmpty() && BAD_METEO_ELEMENT_VAL != info.country) {
    query.arg("country_ru",info.country_ru);
  }
  if (!info.satellite.isEmpty()) {
    query.arg("satellite",info.satellite);
  }
  if (!info.instrument.isEmpty()) {
    query.arg("instrument",info.instrument);
  }
  query.removeDogs();
  return query.query();
}



bool AncDb::saveClimatIsd(const QStringList& reports, int first, int size, const QString& collection)
{
  QTime ttt;
  ttt.start();

  if (!checkDb()) {
    return false;
  }


  QString repstr;
  int last = first + size;
  for (int idx = first; idx < last; idx++) {
    if (idx >= reports.size()) break;
    repstr += reports.at(idx) + ",";
  }
  if (repstr.isEmpty()) {
    return false;
  }

  repstr.remove(-1, 1);

  //добавляем документ
  if ( nullptr == db_ ) { return false; }
  auto query = db_->queryptrByName("isd_insert_observ");
  if(nullptr == query) {  return false; }


  query->arg("collection",collection);
  query->argJson("documents",repstr);

  bool fl = query->exec();
  if ( false == fl ) {
    return false;
  }

  debug_log << "isd time save=" << ttt.elapsed();

  return true;
}

QString AncDb::createIsdReports(const QMap<QString, QString>& header,
                                const ComplexMeteo& cm)
{
  QString strq = meteo::global::kMongoQueriesNew["isd_observe"];
  if (strq.isEmpty()) {
    return QString();
  }

  if (0 == cm.count() && 0 == cm.childsCount()) {
    return QString();
  }

  QString addparam = createReportParams(header, cm);
  if (!addparam.isEmpty()) {
    addparam.remove(-1, 1);
  }

  meteo::GeoPoint gp = cm.coord();

  QDateTime dt = QDateTime::fromString(header.value("dt"), Qt::ISODate);
  if (!dt.isValid()) {
    debug_log << QObject::tr("Ошибка даты/времени") << dt << dt.isValid();
    return QString();
  }
  //добавляем документ
  if ( nullptr == db_ ) { return QString(); }
  auto query = db_->queryptrByName("isd_observe");
  if(nullptr == query) {  return QString(); }


  query->arg("dt",dt);
  query->arg("year",dt.date().year());
  query->arg("month",dt.date().month());
  query->arg("station",header.value("station"));
  query->arg("station_type",header.value("station_type").toInt());
  query->arg("data_type",header.value("data_type").toInt());
  query->arg("level_type",cm.levelType());
  query->arg("level",cm.levelLo());
  query->arg("location",gp);

  if (-1.0f != cm.levelHi()) {
    query->arg("level_hi",cm.levelHi());
  }
  query->arg("usaf",header.value("usaf"));
  query->arg("wban",header.value("wban"));
  query->arg("name",header.value("name"));
  if (!header.value("call").isEmpty()) {
    query->arg("call",header.value("call"));
  }
  query->argJson("param",addparam);

  return query->query();
}

//! формирование json для вставки параметров
QString AncDb::createParamIsd(const QMap<QString, QString>& header, const ComplexMeteo& cm, const QString& name,
                              descr_t descr, const TMeteoParam& param, const QString& uuid)
{

  if ( nullptr == db_ ) { return QString(); }
  auto query = db_->queryptrByName("isd_param");
  if(nullptr == query) {  return QString(); }
  query->arg("descrname",name);
  query->arg("descr",(int64_t)descr);
  query->arg("value",param.value());
  query->arg("code",param.code());
  query->arg("quality",param.quality());
  query->arg("source",header.value("source"));
  query->argDt("dt_write",header.value("dt_write"));
  query->arg("dt_type",cm.dtType());
  query->arg("dt_beg",cm.dtBeg());
  query->arg("dt_end",cm.dtEnd());
  query->arg("uuid",uuid);

  return query->query();
}


void AncDb::fillCreateUpdateParamsJson( const ComplexMeteo& cmeteo, const QMap<QString, QString>& header,
                                        const QMap<QString, Document>& paramPrev, const QString& uuid, QString* addparam, QStringList* pullparam )
{
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
  for ( auto it = cmeteo.data().begin(); it != cmeteo.data().end(); ++it ) {
    if ( d->isIdentDescr( it.key() ) ) {
      continue;
    }
    QMapIterator<int, TMeteoParam> itp(it.value());
    while (itp.hasNext()) {
      itp.next();
      const TMeteoParam& param = itp.value();

      if ( true == param.isInvalid()) {
        continue;
      }
      QString name = d->name(it.key());
      if (d->isAddParent(name)) {
        QString n = d->findAdditional(it.key(), itp.key(), param.value());
        if (!n.isEmpty()) {
          name = n;
        }
      }

      if ( false == name.isEmpty() && true == paramPrev.contains(name) ) { //FIXME TODO реализовать полную замену комплексного дочернего параметра, а не его части
        if ( _cor_number > paramPrev[name].valueInt32("cor_number") ) {
          if ( false == pullparam->contains(name) ) {
            pullparam->append(name);
          }
          addparam->append( createParamJson( cmeteo, header, name, it.key(), param, uuid ) );
        }
      }
      else {
        addparam->append( createParamJson( cmeteo, header, name, it.key(), param, uuid ) );
      }
    }
  }
  for ( auto ch : cmeteo.childs() ) {
    QString chuuid;
    const ComplexMeteo* complch = static_cast<const ComplexMeteo*>(ch);
    if ( complch->levelType() != cmeteo.levelType() || complch->levelLo() != cmeteo.levelLo() ) {
      continue;
    }
    if ( complch->descriptor() != BAD_DESCRIPTOR_NUM) {
      chuuid = QUuid::createUuid().toString();
    }
    fillCreateUpdateParamsJson( *complch, header, paramPrev, chuuid, addparam, pullparam );
  }
}

bool AncDb::saveClimatIsd(const QStringList& reports, const QString& collection, int* maxlines)
{
  if (0 == reports.size()) {
    return true;
  }

  int curmax = *maxlines;
  bool ok = false;
  int cnt = 0;
  while (cnt < reports.size()) {
    debug_log << reports.size() << *maxlines << cnt << curmax;
    //    bson_error_t err;
    ok = saveClimatIsd(reports, cnt, curmax, collection);
    if (!ok) {
      //Ошибка = BSONObj size: 24579150 (0x1770C4E) is invalid. Size must be between 0 and 16793600(16MB) First element: insert: "test_isd". Код ошибки = 10334.
      //Ошибка = Write batch sizes must be between 1 and 1000. Got 4900 operations.. Код ошибки = 16.
      ///      if (err.code != 10334 && err.code != 16) { // && err.code != 16
      ///    return false;
      ///      }
      curmax = 0.7 * curmax;
      if (curmax == 0) {
        return false;
      }
    } else {
      cnt += curmax;
    }
  }

  if (ok) {
    *maxlines = curmax;
  }
  return ok;
}



QString AncDb::createReportParams(const QMap<QString, QString>& header, const ComplexMeteo& cm)
{
  QString addparam;
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();

  QString uuid = QUuid::createUuid().toString();

  for (auto it = cm.data().cbegin(); it != cm.data().cend(); ++it) {
    QMapIterator<int, TMeteoParam> itp(it.value());
    while (itp.hasNext()) {
      itp.next();
      const TMeteoParam& param = itp.value();

      if ( false == param.isInvalid()) {

        QString name = d->name(it.key());
        if (d->isAddParent(name)) {
          QString addname = d->findAdditional(it.key(), itp.key(), param.value());
          if (!addname.isEmpty()) {
            name = addname;
          }
        }

        if (name.isEmpty()) {
          continue;
        }
        addparam += createParamIsd(header, cm, name, it.key(), param, uuid) + ",";
      }
    }
  }
  for(const TMeteoData* ch : cm.childs()) {
    const ComplexMeteo* complch = static_cast<const ComplexMeteo*>(ch);
    if (0 != complch->count() || 0 != complch->childsCount()) {
      addparam += createReportParams(header, *complch);
    }
  }
  return addparam;
}




//! формирование запроса для вставки/получения предыдущих данных
/*QString AncPsqlDb::createMeteoReportQuery(const ComplexMeteo& cm,
  const QMap<QString, QString>& header,
  const GeoPoint& point, const QString& jinfo,
  const QString& collection)
  {
  Q_UNUSED(cm);
  Q_UNUSED(header);
  Q_UNUSED(point);
  Q_UNUSED(jinfo);
  Q_UNUSED(collection);

  //NOTE поиск по координатам нужен для добавления нескольких точек от одной станции (FM62)
  //для фиксированных станций может создавать дубликаты при малых отличиях в координатах
  bool findCoord = false;
  QString station = header.value("station");
  meteo::GeoPoint gp(point);
  if (!gp.isValid()) {
  gp = cm.coord();
  if (gp.isValid()) {
  station = QObject::tr("%1, %2").arg(gp.strLat()).arg(gp.strLon());
  }
  findCoord = true;
  }

  //уровней может не быть с пустыми данными
  if (!gp.isValid() || cm.levelLo() < 0 || meteodescr::kUnknownLevel == cm.levelType()) {
  return QString();
  }

  QString addparam = createUpsertedParams(cm, header, "");
  if (addparam.isEmpty()) {
  return QString();
  }

  QString qname("add_anc_data");
  if ( false == meteo::global::kSqlQueries.contains( qname ) ) {
  error_log << QObject::tr("Не найден запрос = %1")
  .arg(qname);
  return QString();
  }

  QString sqlq = meteo::global::kSqlQueries[qname];
  QDateTime dt = QDateTime::fromString(header.value("dt"),Qt::ISODate);
  meteo::PsqlQuery query( sqlq, sql_ );
  Array code_values(addparam);
  query.arg( "data_type", header.value("data_type").toShort())
  .arg( "dt", dt)
  .arg( "level", cm.levelLo())
  .arg( "lev_type", (int16_t)cm.levelType())
  .arg( "station", station)
  .arg( "stat_type", header.value("station_type").toShort())
  .arg( "location", gp)
  .arg( "param", code_values )
  .arg( "to_replace", true);
  return query.query();
  }

*/
