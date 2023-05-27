#include "tobanaldb.h"

#include <qdom.h>
#include <qfile.h>
#include <qdir.h>
#include <qdatetime.h>


#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/textproto/pbtools.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/log.h>
#include <commons/mathtools/mnmath.h>

#include <sql/dbi/gridfs.h>


const QString kAeroDir = "aero_";
const QString kSurfDir = "surf_";
const QString kGridDir = "grib";
const QString kMrlDir  = "mrl/";

TObanalDb::TObanalDb()
  : dbname_("meteodb")
{
}

TObanalDb::~TObanalDb()
{
}

bool TObanalDb::loadForecastPunkts( meteo::field::DataRequest& arp ) //TODO доделать. ЗАПРЕЩЕНО ПЕРЕДАВАТЬ ПАРАМЕТРЫ КАК НЕКОНСТАНТНЫЕ ССЫЛКИ
{

  std::unique_ptr<meteo::Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) { return false; }
  auto query = db->queryptrByName("find_collection");
  if(nullptr == query) {return false;}
  query->arg("collection","punkts");

  QString error;
  if(false == query->execInit( &error)){
      //error_log << error;
      return false;
    }

  while ( true == query->next() ) {
      const meteo::DbiEntry& doc = query->entry();
      QString name    = doc.valueString("name");
      double la       = meteo::DEG2RAD*doc.valueDouble("la");
      double fi       = meteo::DEG2RAD*doc.valueDouble("fi");
      double height   = doc.valueDouble("height");
      QString id      = doc.valueOid("_id");
      QString station = doc.valueString("station_id");

      meteo::surf::Point* pnt = arp.add_coords();
      pnt->set_name(name.toStdString());
      pnt->set_la(la);
      pnt->set_fi(fi);
      pnt->set_height( height );
      pnt->set_id( id.toStdString() );
      pnt->set_index( station.toStdString() );
    }
  return true;
}

bool TObanalDb::loadRegions(QMap<int, RegionParam>& arp) //TODO доделать. ЗАПРЕЩЕНО ПЕРЕДАВАТЬ ПАРАМЕТРЫ КАК НЕКОНСТАНТНЫЕ ССЫЛКИ
{
  not_impl;
  Q_UNUSED(arp);
  return false;
  //  const QString collection = "forecast_regions";
  //
  //  auto client = meteo::NoSqlManager::instance().getClient();
  //  if(nullptr == client) {
  //    error_log << client->getLastError();
  //    return false;
  //  }
  //
  //  auto query = meteo::Find(collection, meteo::util::field("is_active", true));
  //
  //  auto cursor = client->find(dbname_, query);
  //  if(nullptr == cursor)
  //  {
  //    error_log << client->getLastError();
  //    return false;
  //  }
  //
  //  QString doc;
  //  while (cursor->next()) {
  //    doc = cursor->getDocument();
  //    if(false == doc.isEmpty()) {
  //      QJsonObject obj = QJsonDocument::fromJson(doc.toUtf8()).object();
  //      RegionParam rp;
  //      rp.start.setFiDeg(obj["fi_min"].toDouble());
  //      rp.start.setLaDeg(obj["la_min"].toDouble());
  //      rp.end.setFiDeg  (obj["fi_max"].toDouble());
  //      rp.end.setLaDeg  (obj["la_max"].toDouble());
  //      arp.insert       (obj["_id"].toInt(), rp);
  //    }
  //  }
  //  return true;
}

bool TObanalDb::saveAccuracy(int reg_id, const QDateTime& an_dt, const QDateTime& acur_dt,
                             int level, int hour_0, int hour, int descr, int center, double kk)
{
  not_impl;
  Q_UNUSED(reg_id);
  Q_UNUSED(an_dt);
  Q_UNUSED(acur_dt);
  Q_UNUSED(level);
  Q_UNUSED(hour_0);
  Q_UNUSED(hour);
  Q_UNUSED(descr);
  Q_UNUSED(center);
  Q_UNUSED(kk);
  return false;
  //  QString dts  = acur_dt.toString(Qt::ISODate);   // TODO for debug
  //  QString dtas = an_dt.toString(Qt::ISODate);     // TODO for debug
  //
  //  const QString collection = "fields_acc";
  //
  //  QString doc;
  //  {
  //    QStringList doclist;
  //    doclist << meteo::util::field("analyse_dt"  , an_dt);
  //    doclist << meteo::util::field("dt"          , acur_dt);
  //    doclist << meteo::util::field("region_id"   , reg_id);
  //    doclist << meteo::util::field("center_id"   , center);
  //    doclist << meteo::util::field("descr"       , descr);
  //    doclist << meteo::util::field("level"       , level);
  //    doclist << meteo::util::field("srok"        , hour);
  //    doclist << meteo::util::field("accuracy"    , kk);
  //    doclist << meteo::util::field("pronoz_count", 1);
  //    doclist << meteo::util::field("srok0"       , hour_0);
  //    doc = meteo::util::document(doclist);
  //  }
  //  auto client = meteo::NoSqlManager::instance().getClient();
  //  if(nullptr == client) {
  //    error_log << client->getLastError();
  //    return false;
  //  }
  //
  //  meteo::Insert query = meteo::Insert(collection, doc);
  //
  //  const QString result = client->insert(dbname_, query);
  //  if(result.isEmpty()) {
  //    error_log << client->getLastError();
  //    return false;
  //  }
  //  return true;
}

bool TObanalDb::saveField(const obanal::TField& data, const meteo::field::DataDesc& fdescr,const QString collection)
{
  QDateTime dt = QDateTime::fromString(QString::fromStdString(fdescr.date()), Qt::ISODate);
  Q_UNUSED(data);

  QString fileName = QString("%1_%2_%3_%4_%5_%6_%7_%8_%9.obn")
    .arg( (0 == fdescr.level() ? kSurfDir : kAeroDir) + dt.toString("yyyy_M_d_hhmmss"))
    .arg(fdescr.hour())
    .arg(fdescr.meteodescr())
    .arg(fdescr.level())
    .arg(fdescr.net_type())
    .arg(fdescr.center())
    .arg(fdescr.model())
    .arg(fdescr.level_type())
    .arg(fdescr.hour());

  ::meteo::GridFile gridfile;  
  if(false == saveToFile(collection, fileName, data, &gridfile)) { return false; }

  QDateTime dt1 = fdescr.has_dt1() ? QDateTime::fromString(QString::fromStdString(fdescr.dt1()), Qt::ISODate) : dt;
  QDateTime dt2 = fdescr.has_dt2() ? QDateTime::fromString(QString::fromStdString(fdescr.dt2()), Qt::ISODate) : dt;
  int timeRange = fdescr.has_time_range() ? fdescr.time_range() : 255;
  QDateTime current = QDateTime::currentDateTimeUtc();

  std::unique_ptr<meteo::Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) { return false; }
  auto query = db->queryptrByName("update_obanal");
  if(nullptr == query) {return false;}

  query->arg("collection",collection);
  query->arg("dt",dt);
  query->arg("descr",fdescr.meteodescr());
  query->arg("net_type",fdescr.net_type());
  query->arg("hour",fdescr.hour());
  query->arg("model",fdescr.model());
  query->arg("level_type",fdescr.level_type());
  query->arg("level", fdescr.level());
  query->arg("center",fdescr.center());
  query->argOid("fs_id", gridfile.id());
  query->arg("count_points",fdescr.count_point());
  query->arg("forecast_start",dt1);
  query->arg("forecast_end",dt2);
  query->arg("time_range",timeRange);
  query->arg("dt_write",current);

  if(0 < fdescr.dates_size()) {
      query->argDt("dates",fdescr.dates());
    }


  if ( false == query->exec() ) {
    error_log << QObject::tr("Ошибка при выполнении запроса в базе данных");
    gridfile.remove();    
    return false;
  }
  
  const meteo::DbiEntry& result = query->result();
  if ( 1 != result.valueDouble("ok") ) {
    error_log << QObject::tr("Ошибка при выполнении запроса в базу данных.");
    gridfile.remove();
    return false;
  }
  
  return true;
}




bool TObanalDb::saveMLModel(const QByteArray& data, const meteo::field::DataDesc& fdescr,
                            const QString collection, const QString mlmodel,
                            const meteo::GeoPoint geopoint )
{
  QString fileName = QString("%1_%2_%3_%4_%5_%6_%7_%8_%9_%10.mdl")
      .arg( (0 == fdescr.level() ? kSurfDir : kAeroDir) )
      .arg(fdescr.meteodescr()) .arg(fdescr.level())      .arg(fdescr.net_type()) .arg(fdescr.center())
      .arg(fdescr.model())      .arg(fdescr.level_type()) .arg(fdescr.hour())
      .arg( geopoint.lat()) .arg(geopoint.lon());

  ::meteo::GridFile gridfile;
  if(false == saveBinaryToFile(collection, fileName, data, &gridfile)) { return false; }

  std::unique_ptr<meteo::Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) { return false; }
  auto query = db->queryptrByName("update_mlmodel");
  if(nullptr == query) {return false;}

  QDateTime current = QDateTime::currentDateTimeUtc();

  query->arg("collection",collection);
  query->arg("descr",fdescr.meteodescr());
  query->arg("net_type",fdescr.net_type());
  query->arg("hour",fdescr.hour());
  query->arg("model",fdescr.model());
  query->arg("level_type",fdescr.level_type());
  query->arg("level",fdescr.level());
  query->arg("center",fdescr.center());
  query->argOid("fs_id", gridfile.id());
  query->arg("mlmodel_type",mlmodel);//тип модели, которую сохраняем
  query->arg("dt_write",current);
  query->arg("location",geopoint);
  // query->argGeoWithin("location",geopoint);

  if ( false == query->exec() ) {
    error_log << QObject::tr("Ошибка при выполнении запроса в базе данных");
    return false;
  }

  return true;
}


/**
 * Удаляем данные по региону из базы
 * @param  gv    [description]
 * @param  descr [description]
 * @return       [description]
 */
bool TObanalDb::substractRegionFromSrc( const meteo::GeoVector& gv, const meteo::field::DataDesc descr )
{
  std::unique_ptr<meteo::Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) { return false; }
  auto query = db->queryptrByName("delete_regionfromsrc");
  if(nullptr == query) {return false;}

  query->arg("dt",QDateTime::fromString(QString::fromStdString(descr.date()), Qt::ISODate));
  query->argGeoWithin("location", gv);   // .arg(descr.level())
  
  debug_log << query->query();

  if ( false == query->exec()) {
      error_log << QObject::tr("Ошибка при выполнении запроса в базе данных");
      return false;
    }

  return true;
}



bool TObanalDb::saveToFile(const QString& collection, const QString& fileName, const obanal::TField& data, ::meteo::GridFile* gridfile)
{
  if(0 == data.kolFi()) {
      error_log << QObject::tr("Ошибка: data.kolFi == 0.");
      return false;
  }
  QByteArray ba;
  data.getBuffer(&ba);

  return saveBinaryToFile(collection,fileName,ba, gridfile);
}

//
// сохраняем бинарные данные
//
bool TObanalDb::saveBinaryToFile(const QString& collection, const QString& fileName, const QByteArray& ba, ::meteo::GridFile* gridfile /*=nullptr*/)
{
  meteo::GridFs gridfs;
  auto conf = meteo::global::mongodbConfMeteo();
  if ( false == gridfs.connect(conf) ) {
      error_log << QObject::tr("Невозможно сохранить данные в gridfs") << gridfs.lastError();
      return false;
  }
  gridfs.use( dbname_, collection);

  bool ok = gridfs.put( fileName, ba, gridfile, true);

  if( meteo::GridFs::kErrorDuplicate == gridfs.error()) {
    return true;
  }
  
  if (!ok) {
    error_log << QObject::tr("Ошибка при сохранении файла в gridfs") << gridfs.lastError();
  }

  return ok;
}


/**
 * Загружаем модели из базы
 * @return [description]
 */
bool TObanalDb::loadlMLModel( QByteArray& data, const meteo::field::DataDesc& fdescr,
                              const QString collection, const QString mlmodel,
                              const meteo::GeoPoint geopoint )
{
  std::unique_ptr<meteo::Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) { return false; }
  // загружаем образец запроса
  auto query = db->queryptrByName("load_mlmodel");
  if(nullptr == query) {return false;}
  // заполняем его

  query->arg("descr", fdescr.meteodescr() );    //descr
  query->arg("level_type",fdescr.level_type() ) ;   //level_type
  query->arg("level", fdescr.level());    //level
  query->arg("hour", fdescr.hour());    //hour
  query->arg("center", fdescr.center());    //center
  query->arg("model", fdescr.model());    //model
  query->arg("mlmodel_type", mlmodel);    //mlmodel_type
  query->arg("location", geopoint);   //geopoint
  
  // debug_log<<query->query();

  QString er;
  if ( false == query->execInit(&er)) {
    //error_log<<er;
    return false;
  }

  // айди документа, который надо получить
  QString fs_id;
  if ( true == query->next() ) {
    const meteo::DbiEntry& doc = query->entry();
    fs_id = doc.valueOid("fs_id");
  }else {
    // error_log << QObject::tr("Нет данных в результате запроса");
    return false;
  }

  //
  // Подключаемся к системе gridfs
  //
  meteo::GridFs gridfs;
  if ( !gridfs.connect(meteo::global::mongodbConfMeteo()) ) {
      error_log << QObject::tr("Невозможно сохранить данные в gridfs") << gridfs.lastError();
      return false;
    }
  gridfs.use( dbname_, collection);


  //
  // получаем gridfile по id
  //  filter - фильтр отбора документов в формате json.
  //  Пример: { "_id": { "$oid": "5b926f7443f8ab054317a50f" } }
  //
  meteo::GridFile gridfile;
  gridfile = gridfs.findOneById( QString("%1").arg( fs_id ) );

  if ( !gridfile.hasFile() ){
    error_log << QObject::tr("Файл с идентификатором '%1' не найден").arg(fs_id);
    return false;
  }

  bool ok;

  data = gridfile.readAll(&ok);
  return ok;
}


/**
 * Загружаем модели из базы
 * @return [description]
 */
bool TObanalDb::loadAllMLModels( QList<meteo::GeoPoint>* data, QList<QString>* file_ids, const meteo::field::DataDesc& fdescr,
                                 const QString& mlmodel )
{
  std::unique_ptr<meteo::Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) { return false; }
  // загружаем образец запроса

  auto query = db->queryptrByName("load_mlmodels");
  if(nullptr == query) {return false;}

  query->arg("descr", fdescr.meteodescr() );    //descr
  query->arg("level_type",fdescr.level_type() ) ;   //level_type
  query->arg("level", fdescr.level());    //level
  query->arg("hour", fdescr.hour());    //hour
  query->arg("center", fdescr.center());    //center
  query->arg("model", fdescr.model());    //model
  query->arg("mlmodel_type", mlmodel);    //mlmodel_type

  QString er;
  if ( false == query->execInit(&er)) {
      //error_log<<er;
      return false;
    }

  while ( true == query->next() ) {
      const meteo::DbiEntry& doc = query->entry();
      (*data) << doc.valueGeo("location_json");
      (*file_ids) << doc.valueOid("fs_id");
    }
  return true;
}

/**
 * загружаем текущие процессы / задания
 * @return [description]
 */
bool TObanalDb::loadProcess( meteo::forecast::ProcessParams& processParam )
{
  std::unique_ptr<meteo::Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) { return false; }
  // загружаем образец запроса
  auto query = db->queryptrByName("load_processlist");
  if(nullptr == query) {return false;}
  // заполняем его
  query->arg("process_type", processParam.processtype());
  query->arg("is_finished",false ) ;   //is_finished

  QString er;
  if ( false == query->execInit(&er)) {
      //error_log<<er;
      return false;
    }

  while ( true == query->next() ) {
      const meteo::DbiEntry& adoc = query->entry();
      processParam.set_filename     (   adoc.valueString("file_name").toStdString()   );
      processParam.set_numberofexec (   adoc.valueInt32("number_of_exec")   );
      processParam.set_progress     (   adoc.valueInt32("progress")   );
      processParam.set_isfinished   (   adoc.valueBool("is_finished")   );
      processParam.set_error        (   adoc.valueString("error").toStdString()   );
      processParam.set_lastval      (   adoc.valueString("last_val").toStdString()  );
      processParam.set_processid    (   adoc.valueOid("_id").toStdString()  );
      processParam.set_hour_step    (   adoc.valueInt32("hour_step")  );
      processParam.set_step         (   adoc.valueInt32("step")  );
      processParam.set_step_percent (   adoc.valueInt32("step_percent")  );

      // добавляем станции
      if ( adoc.hasField("stantions") ){
          meteo::Array arr = adoc.valueArray("stantions");
          while(arr.next()) {
              processParam.add_stantions( arr.valueString().toStdString() );
            }
        }
      // добавляем станции
      if ( adoc.hasField("stationspdc") ){
          meteo::Array arr = adoc.valueArray("stationspdc");
          while(arr.next()) {
              processParam.add_stantionspdc( arr.valueString().toStdString() );
            }
        }

      // добавляем парамтеры
      if ( adoc.hasField("params") ){
          meteo::Array arr = adoc.valueArray("params");
          while(arr.next()) {
              processParam.add_params( arr.valueString().toStdString() );
            }
        }
      // добавляем парамтеры
      if ( adoc.hasField("paramspdc") ){
          meteo::Array arr = adoc.valueArray("paramspdc");
          while(arr.next()) {
              processParam.add_paramspdc( arr.valueString().toStdString() );
            }
        }
    }

  return true;
}


/**
 * сохраняем рещультат выполнения процесса
 * @return [description]
 */
bool TObanalDb::saveProcess( meteo::forecast::ProcessParams& processParam )
{
  std::unique_ptr<meteo::Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) { return false; }
  // загружаем образец запроса
  auto query = db->queryptrByName("update_process");
  if(nullptr == query) {return false;}

  query->arg("file_name", processParam.filename());
  query->arg("number_of_exec", processParam.numberofexec());
  query->arg("progress", processParam.progress());
  query->arg("is_finished", processParam.isfinished());
  query->arg("error", processParam.error());
  query->arg("last_val", processParam.lastval());
  query->arg("process_type", processParam.processtype());
  query->arg("stations", processParam.stantions());
  query->arg("stationspdc", processParam.stantionspdc());
  query->arg("step", processParam.step());
  query->arg("step_percent", processParam.step_percent());
  query->arg("hour_step", processParam.hour_step());
  query->arg("params", processParam.params());
  query->arg("paramspdc", processParam.paramspdc());

  if ( false == query->exec()) {
      error_log << QObject::tr("Ошибка при выполнении запроса в базе данных");
      return false;
    }
  if (false == MnMath::isEqual(1,query->entry().valueDouble("ok")) ) {
      error_log << QObject::tr("Ошибка при выполнении запроса в базу данных.");
      return false;
    }


  return true;
}


/**
 * считываем файл из базы в переменную
 * @param  data   [description]
 * @param  fileId [description]
 * @return        [description]
 */
bool TObanalDb::readFileById( QByteArray& data, QString fileId, QString collection ){

  //
  // Подключаемся к системе gridfs
  //
  meteo::GridFs gridfs;
  if ( !gridfs.connect(meteo::global::mongodbConfDocument()) ) {
      error_log << QObject::tr("Невозможно сохранить данные в gridfs") << gridfs.lastError();
      return false;
    }
  gridfs.use( dbname_, collection);

  //
  // получаем gridfile по id
  //  filter - фильтр отбора документов в формате json.
  //  Пример: { "_id": { "$oid": "5b926f7443f8ab054317a50f" } }
  //
  meteo::GridFile gridfile;
  gridfile = gridfs.findOne( QString(" { \"_id\": \"%1\" } ").arg( fileId ) );

  if ( !gridfile.hasFile() ){
      error_log << QObject::tr("Нет файла в базе %1").arg(fileId);
      return false;
    }

  bool ok;

  data = gridfile.readAll(&ok);
  return ok;
}

/**
 * @brief Считываем файл из базы по айди
 * 
 * @param data 
 * @param fileId 
 * @return true 
 * @return false 
 */
bool TObanalDb::readFileByFileId( QByteArray& data, QString fileId ){

  // проверяем подключаение к базе, если его нет - то пробуем подключиться
  if (!gridfs_.isConnected()){
    error_log << QObject::tr("Нет подключения в файловой системе");
    return false;
  }

  meteo::GridFile gridfile;
  gridfile = gridfs_.findOneById( QString("%1").arg( fileId ) );

  if ( !gridfile.hasFile() ){
    error_log << QObject::tr("Файл с идентификатором '%1' не найден").arg(fileId);
    return false;
  }
  bool ok;
  data = gridfile.readAll(&ok);
  return true;
}


/**
 * считываем файл из базы в переменную
 * @param  data   [description]
 * @param  fileId [description]
 * @return        [description]
 */
bool TObanalDb::readFileByName( QByteArray& data, QString fileName, QString collection ){

  //
  // Подключаемся к системе gridfs
  //
  meteo::GridFs gridfs;
  if ( !gridfs.connect(meteo::global::mongodbConfDocument()) ) {
      error_log << QObject::tr("Невозможно сохранить данные в gridfs") << gridfs.lastError();
      return false;
    }
  gridfs.use( dbname_, collection);

  //
  // получаем gridfile по id
  //  filter - фильтр отбора документов в формате json.
  //  Пример: { "_id": { "$oid": "5b926f7443f8ab054317a50f" } }
  //
  meteo::GridFile gridfile;
  gridfile = gridfs.findOneByName( QString("%1").arg( fileName ) );

  if ( !gridfile.hasFile() ){
      error_log << QObject::tr("Нет файла в базе %1").arg(fileName);
      return false;
    }

  bool ok;

  data = gridfile.readAll(&ok);
  return ok;
}


/**
 * сохраняем приоритеты прогностических центров
 * @param  center              [номер центра]
 * @param  priority            [приоритет (сортировка)]
 * @param  priority_val        [отношение удавшихся прогнозов к неудавшимся]
 * @param  forecast_count      [количество проанализированных прогнозов]
 * @param  forecast_true_count [количество оправдавшихся прогнозов]
 * @return                     [description]
 */
bool TObanalDb::saveCentersPriority( const int center, const int priority, const double priority_val,
                                     const int forecast_count, const int forecast_true_count )
{

  std::unique_ptr<meteo::Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) { return false; }
  // загружаем образец запроса
  auto query = db->queryptrByName("update_meteo_centers");
  if(nullptr == query) {return false;}

  query->arg("center", center);
  query->arg("priority", priority);
  query->arg("priority_value", priority_val);

  if ( forecast_true_count >= 0 )  {
      query->arg("forecast_count", forecast_count);
    }
  if ( forecast_count >= 0 )  {
     query->arg("forecast_true_count", forecast_true_count);
    }

  if ( false == query->exec()) {
      error_log << QObject::tr("Ошибка при выполнении запроса в базе данных");
      return false;
    }
  if ( 1 != query->entry().valueDouble("ok") ) {
      error_log << QObject::tr("Ошибка при выполнении запроса в базу данных.");
      return false;
    }
  return true;
}


/**
 * Загружаем значения приоритетов (количества прогнозов) от центра
 * @param  center              [номер центра]
 * @param  priority            [приоритет (сортировка)]
 * @param  priority_val        [отношение удавшихся прогнозов к неудавшимся]
 * @param  forecast_count      [количество проанализированных прогнозов]
 * @param  forecast_true_count [количество оправдавшихся прогнозов]
 * @return        [description]
 */
bool TObanalDb::loadCenterPriority( const int center, int& priority, double& priority_val,
                                    int& forecast_count, int& forecast_true_count ){
  std::unique_ptr<meteo::Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) { return false; }
  // загружаем образец запроса
  auto query = db->queryptrByName("get_meteo_centers");
  if(nullptr == query) {return false;}

  query->arg("center", QList<int> ( {center} ));

   QString er;
  if ( false == query->execInit(&er)) {
      //error_log<<er;
      return false;
    }

  if ( true == query->next() ) {
      const meteo::DbiEntry& adoc = query->entry();
      priority             = adoc.valueInt32("priority");
      priority_val         = adoc.valueDouble("priority_value");
      forecast_count       = adoc.valueInt32("forecast_count");
      forecast_true_count  = adoc.valueInt32("forecast_true_count");
    }else {
      error_log << QObject::tr("Нет данных в результате запроса");
      return false;
    }

  return true;
}


//! Обновление показателя качества после пространственного контроля
bool TObanalDb::updateQuality(const QDateTime& dt, const QStringList& dataId, const QString& descrname, control::QualityControl qual)
{
  if ( true == dataId.isEmpty() ) {
      return true;
    }
  std::unique_ptr<meteo::Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() ) { return false; }
  // загружаем образец запроса
  auto query = db->queryptrByName("update_quality");
  if(nullptr == query) {return false;}

  query->argOid("id", dataId);
  query->arg("dt",dt);
  query->arg("descrname",descrname);
  query->arg("quality",qual);
  query->arg("control",control::SPACE_CTRL );

  bool ok = query->exec();
  if ( false == ok ) {
      error_log << QObject::tr("Не удалось выполнить запрос = %1")
                   .arg( query->query() );
    }
  return ok;
}


/**
 * @brief Подключаемся к ФС Gridfs
 * 
 * @param collection 
 */
void  TObanalDb::connectToGridFs(QString collection){
  
  if ( this->gridfs_.isConnected() ) { 
    return;
  }
  auto conf = meteo::global::mongodbConfMeteo();
  if ( false == this->gridfs_.connect(conf) ) {
      error_log << QObject::tr("Невозможно подключиться к gridfs") << this->gridfs_.lastError();
      return ;
  }
  this->gridfs_.use( dbname_, collection);
}
