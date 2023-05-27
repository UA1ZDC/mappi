#include "methodbag.h"

#include <malloc.h>

#include <qpainter.h>
#include <qbuffer.h>
#include <qfileinfo.h>

#include <cross-commons/app/waitloop.h>
#include <cross-commons/app/paths.h>

#include <sql/nosql/nosqlquery.h>
#include <sql/dbi/gridfs.h>
#include <meteo/commons/proto/document_service.pb.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/loader.h>
#include <meteo/commons/ui/map/dataexchange/sxfexchange.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/ui/map/weather.h>
#include <meteo/commons/ui/map/aerodocument.h>
#include <meteo/commons/ui/map/vprofiledoc.h>
#include <meteo/commons/ui/map/formaldoc.h>
#include <meteo/commons/ui/map/dataexchange/passportbasis.h>
#include <meteo/commons/ui/map/dataexchange/sxfexchange.h>
#include <meteo/commons/ui/map/tileimage/basiscache.h>
#include <meteo/commons/global/dbnames.h>

#include "tfaxdb.h"
#include "mapservice.h"
#include "appstatus.h"

namespace meteo {
namespace map {

static const QString kGridFsSateliteImages = QObject::tr("image_satelite");

static const QString kRunjobName = QString( MnCommon::binPath() + "/meteo.map.runjob" );

//static const QString kSavePath = MnCommon::varPath("meteo") + "maps/";
//static const QString kDocSavePath = MnCommon::varPath("meteo") + "documents/";

//static const QString kWeatherDocuments = "weather_document";
//static const QString kDocPrefix = "formaldoc";
//static const QString kImgPrefix = "imgdoc";

static proto::DocumentFormat format( const QString& ext )
{
  proto::DocumentFormat frmt = proto::kPtkpp;
  if ( -1 != ext.indexOf("jpeg", 0, Qt::CaseInsensitive ) ) {
      frmt = proto::kJpeg;
    }
  else if ( -1 != ext.indexOf("bmp", 0, Qt::CaseInsensitive ) ) {
      frmt = proto::kBmp;
    }
  else if ( -1 != ext.indexOf("png", 0, Qt::CaseInsensitive ) ) {
      frmt = proto::kPng;
    }
  else if ( -1 != ext.indexOf("sxf", 0, Qt::CaseInsensitive)) {
      frmt = proto::kSxf;
    }
  else if ( -1 != ext.indexOf("odt", 0, Qt::CaseInsensitive)) {
      frmt = proto::kOdt;
    }
  else if ( -1 != ext.indexOf("ods", 0, Qt::CaseInsensitive)) {
      frmt = proto::kOds;
    }
  else {
      warning_log << QObject::tr("Не далось определить формат файла с расширением= %1").arg(ext);
    }
  return frmt;
}

MethodBag::MethodBag( Service* s )
  : QObject(),
    service_(s),
    sender_( new StatusSender(this) ),
    mutex_(QMutex::Recursive)
{
  global::Settings::instance()->load();
  param_ = global::Settings::instance()->docserviceparams();
  sender_->init();
}

MethodBag::~MethodBag()
{
  delete sender_; sender_ = nullptr;
}

void MethodBag::CreateDocument( QPair< rpc::ClientHandler*, const proto::Document* > param, proto::Response* resp )
{
  const proto::Document* req = param.second;
  rpc::ClientHandler* handler = param.first;
  rpc::Channel* client = handler->channel();
  QString error;

  Document* doc = Weather::documentFromProto(req);
  if ( nullptr == doc ) {
      error = QObject::tr("Не удалось создать документ по proto-структуре");
      error_log << error;
      resp->set_comment( error.toStdString() );
      resp->set_result(false);
      return;
    }
  sender_->incDoc( 1, 0 );

  if ( false == addDocument( client, doc ) ) {
      delete doc;
      sender_->incDoc( 0, 1 );
      error = QObject::tr("Не удалось добавить документ для последующей обработки");
      error_log << error;
      resp->set_comment( error.toStdString() );
      resp->set_result(false);
      return;
    }

  info_log << QObject::tr("Создан документ %1").arg(doc->uuid());

  resp->set_uuid(doc->uuid().toStdString());
  resp->set_comment( QObject::tr("Документ %1 успешно создан. uuid документа - %2")
                     .arg(doc->uuid()).toStdString() );
  resp->set_result(true);
}

void MethodBag::GetDocument( QPair< rpc::ClientHandler*, const proto::ExportRequest* > param, proto::ExportResponse* resp )
{
  rpc::ClientHandler* hndl = param.first;
  const proto::ExportRequest* req = param.second;
  if ( nullptr == req || nullptr == resp ) {
    QString error = QObject::tr("Нулевой указатель!");
    error_log << error;
    if(nullptr != resp){
      resp->set_comment( error.toStdString() );
      resp->set_result(false);
    }
    return;
  }

  if ( true == req->has_idfile() || true == req->has_path() ) {
    getDocumentFromGridfs( req, resp );
    return;
  }
  if ( true == req->has_map() ) {
    const proto::Document& doc = req->map().document();
    auto docparam = qMakePair( hndl, &doc );
    proto::Response docresp;
    CreateDocument( docparam, &docresp );
    if ( false == docresp.result() ) {
      resp->set_result(false);
      resp->set_comment( docresp.comment() );
      return;
    }
    auto mapparam = qMakePair( hndl, &(req->map()) );
    CreateMap( mapparam, &docresp );
    // debug_log << mapparam;
    if ( false == docresp.result() ) {
      resp->set_result(false);
      resp->set_comment( docresp.comment() );
      return;
    }
  }

  Document* doc = document( hndl->channel() );
  if ( nullptr == doc ) {
    QString error = QObject::tr("Не найден документ");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    return;
  }

  proto::Map map;
  map.set_format(req->format());
  QByteArray arr;
  bool res = doc->exportDocument( map, &arr );
  if ( true == res && 0 != arr.size()) {
    resp->set_result(true);
    resp->set_format( map.format() );
    resp->set_data( arr.data(), arr.size() );
  }
  else {
    QString error = QObject::tr("Не удалось получить данные из документа");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
  }
}

void MethodBag::RemoveDocument( QPair< rpc::ClientHandler* , const proto::Document* > param, proto::Response* resp )
{
  const proto::Document* req = param.second;
  rpc::ClientHandler* hndl = param.first;
  if ( nullptr == req || nullptr == resp ) {
      QString error = QObject::tr("Нулевой указатель. req или resp");
      error_log << error;
      if(nullptr != resp){
        resp->set_comment( error.toStdString() );
        resp->set_result(false);
      }
      return;
    }
  Document* doc = document( hndl->channel() );
  if ( nullptr == doc ) {
      QString error = QObject::tr("У клиента нет документов");
      warning_log << error;
      resp->set_comment( error.toStdString() );
      resp->set_result(false);
      return;
    }
  if ( req->uuid() != doc->uuid().toStdString() ) {
      QString error = QObject::tr("Попытка удалить чужой документ!");
      error_log << error;
      resp->set_comment( error.toStdString() );
      resp->set_result(false);
      return;
    }
  doc = removeDocument( hndl->channel() );
  if ( nullptr != doc ) {
      delete doc;
      sender_->incDoc( 0, 1 );
    }
  resp->set_result(true);
}

void MethodBag::CreateMap( QPair< rpc::ClientHandler*, const proto::Map* > param, proto::Response* resp )
{
  const proto::Map* req = param.second;
  rpc::ClientHandler* hndl = param.first;
  if ( nullptr == req || nullptr == resp ) {
      QString error = QObject::tr("Нулевой указатель. req или resp");
      error_log << error;
      if(nullptr != resp){
        resp->set_comment( error.toStdString() );
        resp->set_result(false);
      }
      return;
    }
  Document* doc = document( hndl->channel() );
  if ( nullptr == doc ) {
      QString error = QObject::tr("У клиента нет документов");
      warning_log << error;
      resp->set_comment( error.toStdString() );
      resp->set_result(false);
      return;
    }

  Weather weather;

  bool res = weather.createMap(doc, *req);
  QString title = QString::fromStdString( req->title() );
  if ( 0 == title.size() ) {
      title = QString::fromStdString( req->name() );
    }
  if ( 0 == title.size() ) {
      title = QObject::tr("Название не указано");
    }
  sender_->incMap( title, 1, 0 );
  if ( true == res ) {
      sender_->incMap( title, 0, 1 );
      //    sender_->incSuccessMap();
      auto prop = doc->property();
      if ( prop.messages_size() > 0 ) {
          resp->set_comment(prop.messages(prop.messages_size() - 1));
        }
      resp->set_result(true);
    }
  else if ( true == req->allow_empty() ) {
      auto prop = doc->property();
      if ( prop.messages_size() > 0 ) {
          resp->set_comment(prop.messages(prop.messages_size() - 1));
        }
      resp->set_result(true);
    }
  else {
      QString error = QObject::tr("Документ не создан (нет данных). Документ = %1. Дата = %2")
          .arg( QString::fromStdString(req->name()))
          .arg( QString::fromStdString( req->datetime() ) );
      error_log << error;
      resp->set_comment(error.toStdString());
      resp->set_result(false);
    }
}

void MethodBag::ProcessJob( const proto::Job* req, proto::ExportResponse* resp )
{
  QFileInfo fi(kRunjobName);
  if ( false == fi.exists() ) {
      QString err = QObject::tr("Файл программы не найден: %1").arg(kRunjobName);
      error_log << err;
      resp->set_result(false);
      resp->set_comment( err.toStdString() );
      return;
    }
  if ( false == fi.isExecutable() ) {
      QString err = QObject::tr("Нет прав на выполнение файла: %1").arg(kRunjobName);
      error_log << err;
      resp->set_result(false);
      resp->set_comment( err.toStdString() );
      return;
    }
  QProcess* proc = new QProcess();
  sender_->incJobSize( 0, 1 );
  QObject::connect( proc, SIGNAL( finished(int,QProcess::ExitStatus) ), this, SLOT( slotProcFinished(int,QProcess::ExitStatus) ) );
  proto::Job job;
  job.CopyFrom(*req);
  debug_log << job.Utf8DebugString();
  procpool_.insert( proc, job );
  startNextJob();
}

void MethodBag::SaveDocument( QPair< rpc::ClientHandler*, const proto::ExportRequest* > param, proto::Response* resp )
{
  const proto::ExportRequest* req = param.second;
  rpc::ClientHandler* hndl = param.first;
  if ( nullptr == req || nullptr == resp ) {
      QString error = QObject::tr("Нулевой указатель. req или resp");
      error_log << error;
      if(nullptr != resp){
        resp->set_comment( error.toStdString() );
        resp->set_result(false);
      }
      return;
    }
  Document* doc = document( hndl->channel() );
  if ( nullptr == doc ) {
      QString error = QObject::tr("У клиента нет документов");
      warning_log << error;
      resp->set_comment( error.toStdString() );
      resp->set_result(false);
      return;
    }
  proto::Map info = doc->info();

  QString name = doc->fileName( QString::fromStdString( req->name() ), req->format() );

  info.set_format(req->format());
  info.set_job_name(req->name());
  info.set_job_title(req->title());

  QByteArray arr;
  bool res = doc->exportDocument( info, &arr );
  if ( false == res ) {
      QString error = QObject::tr("Не удалось получить данные из документа");
      error_log << error;
      resp->set_comment(error.toStdString());
      resp->set_result(false);
      return;
    }

  resp->set_result(true);
  if ( false == doc->saveDocumentToMongo( arr, name, req->format() ) ) {
      QString error = QObject::tr("Не удалось сохранить документ в БД");
      error_log << error;
      resp->set_comment(error.toStdString());
      resp->set_result(false);
    }
}

void MethodBag::ImportDocument(QPair<rpc::ClientHandler *, const proto::ImportRequest *> param, proto::Response *resp)
{
  rpc::ClientHandler* hndl = param.first;
  const proto::ImportRequest* req = param.second;
  if ( nullptr == req || nullptr == resp ) {
      QString error = QObject::tr("Нулевой указатель!");
      error_log << error;
      if(nullptr != resp){
        resp->set_comment( error.toStdString() );
        resp->set_result(false);
      }
      return;
    }

  QByteArray bar = QByteArray::fromStdString( req->data() );
  QString id;

  if ( true == req->has_path() ) {
      auto res = Document::saveDocumentToGridFs(QString::fromStdString(req->path()), bar, &id);
      if ( false == res ) {
          QString error = QObject::tr("Ошибка при сохранении файла в БД");
          error_log << error;
          resp->set_comment(error.toStdString());
        }
      resp->set_result(res);
      return;
    }

  Document* doc = document( hndl->channel() );
  if ( nullptr == doc ) {
      QString error = QObject::tr("Не найден документ");
      error_log << error;
      resp->set_comment( error.toStdString() );
      resp->set_result(false);
      return;
    }

  resp->set_result(true);
  if ( false == doc->saveDocumentToMongo( bar, doc->fileName( doc->name(), doc->info().format() ), doc->info().format() ) ) {
      QString error = QObject::tr("Не удалось сохранить документ в БД");
      error_log << error;
      resp->set_comment(error.toStdString());
      resp->set_result(false);
    }
}

void MethodBag::GetAvailableDocuments( const proto::MapRequest* req, proto::MapList* resp )
{
  QDateTime dts;
  QDateTime dte;
  std::unique_ptr<Dbi> db(meteo::global::dbDocument());
  if ( nullptr == db.get() ) {
      resp->set_result(false);
      QString error = QObject::tr("Не удалось подключиться к БД");
      resp->set_comment( error.toStdString() );
      return;
    }
//TODO - это явно можно переписать более оптимально???
  if ( false == req->has_date_start() ) {
      if ( false == req->has_job_name() ) {
          QString error = QObject::tr("Не указана дата отбора документов или идентификатор работы");
          error_log << error;
          resp->set_result(false);
          resp->set_comment( error.toStdString() );
          return;
        }
      auto query = db->queryptrByName("get_last_doc_for_job");
      if(nullptr == query) {
          resp->set_result(false);
          QString error = QObject::tr("Не найдена функция для получения данных");
          resp->set_comment( error.toStdString() );
          return;
        }

      query->arg("jobid", req->job_name());
      QString error;
      if(false == query->execInit( &error)){
          //error_log << error;
          resp->set_result(false);
          resp->set_comment( error.toStdString() );
          return;
        }

      while ( true == query->next() ) {
          const ::meteo::DbiEntry& doc = query->entry();
          dts = doc.valueDt("dt");
          dte = dts;
          dts.setTime(QTime(0,0,0));
          dte.setTime(QTime(23,59,59));
        }
    }
  else {
      dts = NosqlQuery::datetimeFromString( req->date_start() );
      dte = dts;
      if ( true == req->has_date_end() ) {
          dte = NosqlQuery::datetimeFromString( req->date_end() );
        }
    }
  auto query = db->queryptrByName("get_available_documents");
  if(nullptr == query) {
      resp->set_result(false);
      QString error = QObject::tr("Не найдена функция для получения данных");
      resp->set_comment( error.toStdString() );
      return;
    }  
  query->arg("start_dt", dts);
  query->arg("end_dt", dte);

  if ( true == req->has_map_name() ) {
      query->arg("mapid",req->map_name() );
    }
  if ( true == req->has_job_name() ) {
      query->arg("jobid",req->job_name() );
    }
  if ( true == req->has_center() ) {
      query->arg("center", req->center() );
    }
  if ( true == req->has_model() ) {
      query->arg("model", req->model() );
    }
  if ( true == req->has_format() ) {
      query->arg("format", req->format() );
    }
  if ( true == req->has_hour() ) {
      query->arg("hour", req->hour() );
    }
  if ( true == req->has_map_title() ) {
      query->arg("map_title", req->map_title() );
    }
  if ( true == req->has_job_title() ) {
      query->arg("job_title", req->job_title() );
    }
    QString error;
  if(false == query->execInit( &error)){
      //error_log << error;
      resp->set_result(false);
      resp->set_comment( error.toStdString() );
      return;
    }
  while ( true == query->next() ) {
      const ::meteo::DbiEntry& doc = query->entry();
      auto map = resp->add_map();
      map->set_center( doc.valueInt32("center") );
      map->set_datetime( doc.valueDt("dt").toString(Qt::ISODate).toStdString() );
      QMap<QString, meteo::map::proto::DocumentFormat> formats;
      formats["ODT"] = proto::kOdt;
      formats["ODS"] = proto::kOds;
      formats["BMP"] = proto::kBmp;
      formats["PNG"] = proto::kPng;
      formats["SXF"] = proto::kSxf;
      formats["PTKPP"] = proto::kPtkpp;
      map->set_format(formats[doc.valueString("format")]);
      map->set_hour( doc.valueInt32("hour") );
      map->set_job_name( doc.valueString("jobid").toStdString() );
      map->set_job_title(doc.valueString("job_title").toStdString());
      map->set_name( doc.valueString("mapid").toStdString() );
      map->set_title( doc.valueString("map_title").toStdString() );
      map->set_model( doc.valueInt32("model") );
      map->set_idfile( doc.valueString("idfile").toStdString() );
      map->set_path( doc.valueString("path").toStdString() );
    }
  resp->set_result(true);
}

void MethodBag::GetAvailableClimat(const proto::ClimatRequest *req, proto::ClimatResponse *resp)
{
  Q_UNUSED(req);

  std::unique_ptr<Dbi> db(meteo::global::dbDocument());
  if ( nullptr == db.get() ) {
      resp->set_result(false);
      QString error = QObject::tr("Не удалось подключиться к БД");
      resp->set_comment( error.toStdString() );
      return;
    }
  auto query = db->queryptrByName("get_available_climat");
  if(nullptr == query) {
      resp->set_result(false);
      QString error = QObject::tr("Не найдена функция для получения данных");
      resp->set_comment( error.toStdString() );
      return;
    }
  QString error;
  if(false == query->execInit( &error)){
      //error_log << error;
      resp->set_result(false);
      resp->set_comment( error.toStdString() );
      return;
    }

  while ( true == query->next() ) {
      const ::meteo::DbiEntry& doc = query->entry();

      resp->add_path(doc.valueString("filename").toStdString());
    }

  resp->set_result(true);
}

void MethodBag::GetLastJobs(const proto::Dummy *req, proto::JobList *resp)
{
  Q_UNUSED(req);
  std::unique_ptr<Dbi> db(meteo::global::dbDocument());
  if ( nullptr == db.get() ) {
      resp->set_result(false);
      QString error = QObject::tr("Не удалось подключиться к БД");
      resp->set_comment( error.toStdString() );
      return;
    }
  auto query = db->queryptrByName("get_last_jobs");
  if(nullptr == query) {
      resp->set_result(false);
      QString error = QObject::tr("Не найдена функция для получения данных");
      resp->set_comment( error.toStdString() );
      return;
    }
  QString error;
  if(false == query->execInit( &error)){
      //error_log << error;
      resp->set_result(false);
      resp->set_comment( error.toStdString() );
      return;
    }

  while ( true == query->next() ) {
      const ::meteo::DbiEntry& doc = query->entry();
      auto j = resp->add_job();
      j->set_name(doc.valueString("jobid").toStdString());
      j->set_date(doc.valueDt("dt").toString(Qt::ISODate).toStdString());
    }
  resp->set_result(true);
}

//
//void Service::getWeatherTemplate( const QString& mapid, proto::Map* resp )
//{
//  QMap<QString, proto::Map> templates =  WeatherLoader::instance()->weathermaps();
//  if(false == templates.contains(mapid)) {
//    error_log << QObject::tr("Шаблон карты погоды %1 не найден").arg(mapid);
//    resp->Clear();
//    return;
//  }
//  resp->CopyFrom(templates[mapid]);
//}
//
//void Service::getWeatherTemplates( const QStringList& mapid, proto::MapList* resp )
//{
//  resp->Clear();
//
//  QStringList locmapid;
//  if(false == mapid.empty()) {
//    locmapid = mapid;
//  }
//
//  QMap<QString, proto::Map> templates = WeatherLoader::instance()->weathermaps();
//  for(auto it = templates.cbegin(); it != templates.cend(); ++it) {
//    if(mapid.empty() || locmapid.contains(it.key())) {
//      proto::Map* map = resp->add_map();
//      map->CopyFrom(it.value());
//      if(locmapid.contains(it.key())) {
//        locmapid.removeAll(it.key());
//      }
//    }
//  }
//
//  if(false == locmapid.empty()) {
//    error_log << QObject::tr("Шаблоны карты погоды %1 не найдены").arg(locmapid.join(", "));
//  }
//}
//

bool MethodBag::getDocumentFromGridfs( const proto::ExportRequest* req, proto::ExportResponse* resp )
{
  QString queryname;
  QString error;
  QString fileid;
  if ( true == req->has_idfile() ) {
      fileid = QString::fromStdString(req->idfile());
      queryname = "get_grid_file_by_id";
    }
  else if ( true == req->has_path() ) {
      fileid = QString::fromStdString(req->path());
      queryname  = "get_grid_file_by_filename";
    }
  else {
      error = QObject::tr("Не указан ни путь к файлу, ни id файла");
      error_log << error;
      resp->set_comment( error.toStdString() );
      resp->set_result(false);
      return false;
    }

  GridFs gridfs;
  auto conf = global::mongodbConfDocument();
  if ( false == gridfs.connect(conf )) {
      error = QObject::tr("Не удалось подключиться к БД документов");
      error_log << error;
      resp->set_comment( error.toStdString() );
      resp->set_result(false);
      return false;    
  }

  QString sourceDb;
  switch ( req->source() ){
    case proto::ExportRequest_ExportDb::ExportRequest_ExportDb_kClimatDb:{
      sourceDb = db::kDbDocuments;
      break;
    }
    case proto::ExportRequest_ExportDb::ExportRequest_ExportDb_kDocumentDb:
    default:{
      sourceDb = db::kDbDocuments;
      break;
    }
  }

  gridfs.use( sourceDb, "fs" );
  GridFile file = ( true == req->has_idfile() ) ? gridfs.findOneById(req->idfile())
                                                : gridfs.findOneByName(req->path());


  if ( false == file.isValid() ) {
      error = QObject::tr("Не удалось получить файл '%1' не получен")
          .arg(fileid);
      error_log << error;
      resp->set_comment( error.toStdString() );
      resp->set_result(false);
      return false;
    }

  bool result;
  QByteArray arr = file.readAll( &result );
  if ( false == result ) {
      error = QObject::tr("Не удалось прочитать файл = %1.")
          .arg( QString::fromStdString( req->path() ) );
      error_log << error;
      resp->set_comment( error.toStdString() );
      resp->set_result(false);
      return false;
    }
  resp->set_data( arr.data(), arr.size() );
  resp->set_format( format( QFileInfo( QString::fromStdString( req->path() ) ).suffix() ) );
  resp->set_filename(file.fileName().toStdString());
  resp->set_result(true);
  return true;
}

bool MethodBag::registerClient( rpc::Channel* client )
{
  if ( nullptr == client ) {
      error_log << QObject::tr("Попытка добавить клиента, который == nullptr");
      return false;
    }
  QMutexLocker lock(&mutex_);
  if ( true == connections_.contains(client) && nullptr != connections_[client] /*&& false*/ ) {
      error_log << QObject::tr("Попытка зарегистрировать уже зарегистрированного клиента");
      return false;
    }
  connections_.insert( client, nullptr );
  return true;
}

bool MethodBag::addDocument( rpc::Channel* client, Document* doc )
{
  QMutexLocker lock(&mutex_);
  if ( false == connections_.contains(client) ) {
      error_log << QObject::tr("Попытка неизвестного клиента добавить документ");
      return false;
    }
  if ( true == connections_.contains(client) && nullptr != connections_[client] ) {
      error_log << QObject::tr("Попытка клиента добавить второй документ");
      return false;
    }
  connections_.insert( client, doc );
  return true;
}

Document* MethodBag::document( rpc::Channel* client )
{
  QMutexLocker lock(&mutex_);
  if ( false == connections_.contains(client) ) {
      error_log << QObject::tr("Попытка получить документ для неизвестного клиента");
      return nullptr;
    }
  return connections_[client];
}

Document* MethodBag::removeDocument( rpc::Channel* client )
{
  QMutexLocker lock(&mutex_);
  if ( false == connections_.contains(client) ) {
      error_log << QObject::tr("Попытка неизвестного клиента удалить документ");
      return nullptr;
    }
  Document* doc = connections_[client];
  connections_.insert( client, nullptr );
  return doc;
}

void MethodBag::slotClientConnected( rpc::Channel* ch )
{
  if ( false == registerClient(ch) ) {
      error_log << QObject::tr("Непредвиденная ошибка. Невозможно зарегистрировать новго клиента");
      return;
    }
}

void MethodBag::slotClientDisconnected( rpc::Channel* ch )
{
  if ( false == connections_.contains(ch) ) {
      warning_log << QObject::tr("Сработал слот на отключение незарегистрированного клиента");
      return;
    }
  if ( nullptr != ch->handler() ) {
      if ( false == ch->handler()->finished() ) {
          QObject::connect( ch->handler(), SIGNAL( methodsFinished(meteo::rpc::ClientHandler*) ),
                            this, SLOT( slotMethodsFinished(meteo::rpc::ClientHandler*) ) );
        }
      else {
          if ( true == connections_.contains(ch) ) {
              Document* doc = connections_[ch];
              connections_[ch] = nullptr;
              if ( nullptr != doc ) {
                  delete doc;
                  sender_->incDoc( 0, 1 );
                }
            }
        }
    }
}

void MethodBag::slotMethodsFinished( rpc::ClientHandler* hndlr )
{
  if ( nullptr == hndlr->channel() ) {
      warning_log << QObject::tr("Нет соединения с клиентом!");
      return;
    }
  if ( false == connections_.contains( hndlr->channel() ) ) {
      error_log << QObject::tr("Неопознанное подкдчение!");
      return;
    }
  Document* doc = connections_[hndlr->channel()];
  connections_.remove( hndlr->channel() );
  if ( nullptr != doc ) {
      delete doc;
      sender_->incDoc( 0, 1 );
    }
}

void MethodBag::GetFaxes( QPair< rpc::ClientHandler*, const proto::FaxRequest*> param, proto::FaxReply* resp)
{
  auto req = param.second;

  auto dbConf = global::mongodbConfDocument();
  std::unique_ptr<Dbi> db(global::dbDocument());
  if ( nullptr == db ){
    auto err = msglog::kDbConnectFailed.arg(dbConf.name());
    error_log << err;
    resp->set_result(false);
    resp->set_comment(err.toStdString());
    return;
  }

  for ( const auto& faxInfo : req->faxes() )
  {
    static auto queryName = QObject::tr("find_fax");
    std::unique_ptr<DbiQuery> query(db->queryByName(queryName));
    if ( nullptr == query ){
      auto err = msglog::kDbRequestNotFound.arg(queryName);
      error_log << err;
      resp->set_result(false);
      resp->set_comment(err.toStdString());
      return;
    }

    if ( true == faxInfo.has_id() ){      
      query->argOid("id", faxInfo.id() );
    }
    if ( true == faxInfo.has_dt_start() ){
      auto dtStart = QString::fromStdString(faxInfo.dt_start());
      query->arg("start_dt", QDateTime::fromString(dtStart, Qt::ISODate));
    }
    if ( true == faxInfo.has_dt_end() ){
      auto dtEnd = QString::fromStdString(faxInfo.dt_end());
      query->arg("end_dt", QDateTime::fromString(dtEnd, Qt::ISODate));
    }
    if ( true == faxInfo.has_t1() ){
      query->arg("t1", faxInfo.t1());
    }
    if ( true == faxInfo.has_t2() ){
      query->arg("t2", faxInfo.t2());
    }
    if ( true == faxInfo.has_a1() ){
      query->arg("a1", faxInfo.a1());
    }
    if ( true == faxInfo.has_a2()){
      query->arg("a2", faxInfo.a2());
    }
    if ( true == faxInfo.has_ii() ){
      query->arg("ii", faxInfo.ii());
    }
    if ( true == faxInfo.has_cccc() ){
      query->arg("cccc", faxInfo.cccc());
    }
    if ( true == faxInfo.has_yygggg() ){
      query->arg("yygggg", faxInfo.yygggg());
    }

    QString error;    
    if( false == query->execInit( &error)){
      error_log << msglog::kDbRequestFailed.arg(dbConf.name())
                << error;
      continue;
    }

    while ( true == query->next() ){
      const ::meteo::DbiEntry& doc = query->entry();
      auto newFax = resp->add_faxes();
      QString id = doc.valueOid("_id");
      QString t1 = doc.valueString("t1");
      QString t2 = doc.valueString("t2");
      QString a1 = doc.valueString("a1");
      QString a2 = doc.valueString("a2");
      int ii = doc.valueInt32("ii");
      QString cccc = doc.valueString("cccc");
      QString yygggg = doc.valueString("yygggg");
      QDateTime dt = doc.valueDt("dt");
      QString fileName = doc.valueString("image_path");

      newFax->set_id(id.toStdString());
      newFax->set_a1(a1.toStdString());
      newFax->set_a2(a2.toStdString());
      newFax->set_t1(t1.toStdString());
      newFax->set_t2(t2.toStdString());
      newFax->set_ii(ii);
      newFax->set_cccc(cccc.toStdString());
      newFax->set_yygggg(yygggg.toStdString());
      newFax->set_dt_start(dt.toString(Qt::ISODate).toStdString());
      if ( true == req->header_only() ){
        continue;
      }
      GridFs gridfs;
      if ( false == gridfs.connect( dbConf ) ) {
        error_log << msglog::kDbConnectFailed.arg(dbConf.name())
                  << gridfs.lastError();
        continue;
      }
      static const QString prefixName = "fax";


      gridfs.use(dbConf.name(),prefixName);      
      meteo::GridFile file = gridfs.findOneByName(fileName);
      if ( false == file.isValid() ) {
        error_log << gridfs.lastError();
        continue;
      }
      if ( false == file.hasFile() ) {
        error_log << meteo::msglog::kFileNotFound.arg(fileName);
        continue;
      }
      bool ok = false;
      QByteArray fileData = file.readAll(&ok);
      if ( false == ok ) {
        error_log << msglog::kFileReadFailed.arg(fileName)
                  << file.lastError();
      }
      newFax->set_msg(fileData.toStdString());
      resp->set_comment("Запрос выполнен успешно");
      resp->set_result(ok);
    }
  }

  resp->set_result(true);
}

void MethodBag::GetWeatherJobs(const proto::JobList *req, proto::JobList *resp)
{
  internal::WeatherLoader* wl = WeatherLoader::instance();
  wl->reloadTypes();
  QStringList jobidlist;
  for ( const auto &j : req->job() ) {
      jobidlist << QString::fromStdString( j.name() );
    }
  auto jobs = wl->mapjobs();
  auto it = jobs.begin(),
      end = jobs.end();
  for ( ; it != end; ++it ) {
      if ( 0!= jobidlist.size() && false == jobidlist.contains( QString::fromStdString( it.value().name() ) ) ) {
          continue;
        }
      auto np = resp->add_job();
      np->CopyFrom( it.value() );
    }
}

void MethodBag::SetWeatherJobs(const proto::JobList *req, proto::Response *resp)
{
  auto ok = WeatherLoader::instance()->updateJobLibrary(*req);
  resp->set_result(ok);
}

void MethodBag::GetSateliteImageList( const proto::GetSateliteImageListRequest* req,
                                      proto::GetSateliteImageListReply* resp )
{
  QDateTime dtBegin = NosqlQuery::datetimeFromString( req->dt_begin() );
  QDateTime dtEnd = NosqlQuery::datetimeFromString( req->dt_end() );
  static const QString queryName = QObject::tr("find_image_satelite_by_dt");

  std::unique_ptr<Dbi> db(meteo::global::dbDocument());
  if ( nullptr == db.get() ) {
      resp->set_result(false);
      QString error = QObject::tr("Не удалось подключиться к БД");
      resp->set_comment( error.toStdString() );
      return;
    }
  auto query = db->queryptrByName(queryName);
  if(nullptr == query) {
      resp->set_result(false);
      QString error = QObject::tr("Не найдена функция для получения данных");
      resp->set_comment( error.toStdString() );
      return;
    }

  query->arg("start_dt",dtBegin);
  query->arg("end_dt", dtEnd);

  QString error;  
  if(false == query->execInit( &error)){
      //error_log << error;
      resp->set_result(false);
      resp->set_comment( error.toStdString() );
      return;
    }

  while ( true == query->next() ) {
      const ::meteo::DbiEntry& document = query->entry();
      auto t1 = document.valueString(QObject::tr("t1"));
      auto t2 = document.valueString(QObject::tr("t2"));
      auto a1 = document.valueString(QObject::tr("a1"));
      auto a2 = document.valueString(QObject::tr("a2"));
      int ii = document.valueInt32(QObject::tr("ii"));
      auto cccc = document.valueString(QObject::tr("cccc"));
      auto yygggg = document.valueString(QObject::tr("yygggg"));
      //auto imagePath = document.valueString(QObject::tr("image_path"));
      auto dt = document.valueDt(QObject::tr("dt"));
      QString gridfsid = QString::number( document.valueInt64(QObject::tr("fileid")) );

      auto header = resp->add_headers();
      header->set_t1(t1.toStdString());
      header->set_t2(t2.toStdString());
      header->set_a1(a1.toStdString());
      header->set_a2(a2.toStdString());
      header->set_ii(ii);
      header->set_cccc(cccc.toStdString());
      header->set_yygggg(yygggg.toStdString());
      header->set_gridfs_id(gridfsid.toStdString());
      header->set_dt(dt.toString(Qt::ISODate).toStdString());
    }

  resp->set_result(true);
}

void MethodBag::GetSateliteImageFile( const proto::GetSateliteImageRequest* req,
                                      proto::GetSateliteImageReply* resp )
{

  auto gridfsFileId = QString::fromStdString(req->gridfs_file_id());
  if ( (false == req->has_gridfs_file_id()) && (true == gridfsFileId.isEmpty()) ){
      auto emessage = QObject::tr("Ошибка: отсутствует часть обязательных агрументов");
      error_log << emessage;
      resp->set_comment(emessage.toStdString());
      resp->set_result(false);
      return;
    }

  auto conf = meteo::global::mongodbConfDocument();
  GridFs gridfs;
  if ( false == gridfs.connect( conf )) {
      QString emessage = meteo::msglog::kDbConnectHostPortFailed.arg(conf.host()).arg(conf.port());
      error_log << emessage;
      resp->set_comment(emessage.toStdString());
      resp->set_result(false);
      return;
    }
  gridfs.use( conf.name(), kGridFsSateliteImages);

  static const QString gridfsQueryFileById = QObject::tr("get_grid_file_by_id");

  //TODO
  ConnectProp prop;
  prop.setDriver(settings::kMongo);

  std::unique_ptr<DbiQuery> query(global::dbqueryByName(prop,gridfsQueryFileById));
  if ( nullptr == query ){
      auto emessage = meteo::msglog::kDbRequestNotFound.arg(gridfsQueryFileById);
      error_log << emessage;
      resp->set_comment(emessage.toStdString());
      resp->set_result(false);
      return;
    }

  query->arg("id",gridfsFileId);

  debug_log << query->query();

  auto file = gridfs.findOne(query->query());
  bool ok = false;
  auto data = file.readAll(&ok);
  if ( true == ok ) {
      resp->set_data(data.toStdString());
      resp->set_result(true);
    }
  else {
      auto emessage = meteo::msglog::kFileReadFailed
          .arg(gridfsFileId)
          .arg(gridfs.lastError());
      error_log << emessage;
      resp->set_comment(emessage.toStdString());
      resp->set_result(false);
    }
}

QByteArray MethodBag::getNetImage()
{
  QString filename( ":/meteo/icons/map/netpikchi.png" );
  if ( false == QFile::exists(filename) ) {
      QString error = QObject::tr("Файл '%1' не найден").arg(filename);
      error_log << error;
      return QByteArray();
    }
  QFile file(filename);
  if ( false == file.open(QIODevice::ReadOnly) ) {
      QString error = QObject::tr("Не могу открыть файл '%1'").arg(filename);
      error_log << error;
      return QByteArray();
    }
  QByteArray arr = file.readAll();
  file.close();
  return arr;
}

void MethodBag::GetTile( const proto::TileRequest* req, proto::TileResponse* resp )
{
  Q_UNUSED(req);
  BasisCache cache;
  proto::Document doc;
  doc.set_scale( req->scale() );
  doc.set_geoloader( req->loader() );
  doc.set_projection( req->proj() );
  doc.mutable_map_center()->set_lat_deg(0.0);
  doc.mutable_map_center()->set_lon_deg(0.0);
  QByteArray arr = cache.getTile( doc, req->address().x(), req->address().y() );
  if ( 0 == arr.size() ) {
      arr = getNetImage();
    }
  if ( 0 == arr.size() ) {
      resp->set_result(false);
      QString error = QObject::tr("Непредвиденная ощибка");
      error_log << error;
      resp->set_comment( error.toStdString() );
    }
  resp->set_result(true);
  resp->set_image( arr.data(), arr.size() );
}

void MethodBag::GetPunches( const proto::PunchRequest* req, proto::PunchResponse* resp )
{
  internal::WeatherLoader* wl = WeatherLoader::instance();
  auto punches = wl->punchlibrary();
  if ( true == req->specialpunches() ) {
      punches = wl->punchlibraryspecial();
    }
  auto it = punches.begin(),
      end = punches.end();;
  for ( ; it != end; ++it ) {
      if ( true == req->has_punchcode() && req->punchcode() != it.value().code() ) {
          continue;
        }
      auto np = resp->add_punch();
      np->CopyFrom( it.value() );
    }
}

void MethodBag::GetPunchMaps( const proto::Map* req, proto::MapList* resp )
{
  internal::WeatherLoader* wl = WeatherLoader::instance();
  auto punches = wl->punchmaps();
  auto it = punches.begin(),
       end = punches.end();
  bool onlywebpunches = false;
  if ( true == req->has_webpunchmap() && true == req->webpunchmap() ) {
    onlywebpunches = true;
  }
  for ( ; it != end; ++it ) {
    auto m = it.value();
    if ( true == req->has_name() && req->name() != m.name() ) {
      continue;
    }
    if ( true == onlywebpunches && false == m.webpunchmap() ) {
      continue;
    }
    // debug_log << "OK =" << m.Utf8DebugString();
    auto np = resp->add_map();
    np->CopyFrom( it.value() );
  }
}

void MethodBag::GetOrnament( const proto::Dummy* req, proto::Ornaments* resp )
{
  Q_UNUSED(req);
  internal::WeatherLoader* wl = WeatherLoader::instance();
  auto ornament = wl->ornamentlibrary();
  auto it = ornament.begin(),
       end = ornament.end();
  for ( ; it != end; ++it ) {
    auto o = it.value();
    // debug_log << "OK =" << m.Utf8DebugString();
    auto no = resp->add_ornament();
    no->CopyFrom( it.value() );
  }
}

void MethodBag::slotProcFinished( int exitcode, QProcess::ExitStatus status )
{
  Q_UNUSED(exitcode);
  Q_UNUSED(status);
  QProcess* proc = qobject_cast<QProcess*>( sender() );

  if ( nullptr != proc ) {
    const QByteArray &ba = proc->readAllStandardError();
    if(false == ba.simplified().isEmpty())
    {
      debug_log << QObject::tr("Предупреждения во время выполнения задания:\n")
                << ba;
    }
    }
  sender_->incJobSize( -1, 0 );
  if ( nullptr == proc ) {
      error_log << QObject::tr("Неизвестная ошибка. ПРоцесс завершен, но не удалось получить указатель на него");
    }
  else {
      auto req = runpool_[proc];
      sender_->incJob( QString::fromStdString( req.title() ), 0, 1 );
      runpool_.remove(proc);
    }
  delete proc;
  --procnum_;
  startNextJob();
}

void MethodBag::startNextJob()
{
  if ( 0 == procpool_.size() ) {
      return;
    }
  while ( procnum_ < param_.max_job_proc() && 0 != procpool_.size() ) {
      QMap< QProcess*, proto::Job >::iterator beg = procpool_.begin();
      QProcess* proc = beg.key();
      proto::Job req = beg.value();
      runpool_.insert( proc, req );
      sender_->incJobSize( 1, -1 );
      proc->start(kRunjobName);
      std::string str;
      req.SerializeToString(&str);
      int32_t sz = str.size();
      int32_t sizesz = sizeof(int32_t) ;
      proc->write( (char*)(&sz), sizesz );
      proc->write( str.data(), sz );
      ++procnum_;
      procpool_.erase(beg);
      sender_->incJob( QString::fromStdString( req.title() ), 1, 0 );
    }
}

}
}
