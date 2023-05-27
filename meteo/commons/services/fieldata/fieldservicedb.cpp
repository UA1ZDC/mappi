#include "fieldservicedb.h"

#include <memory>

#include <qmap.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qstringlist.h>
#include <qelapsedtimer.h>

#include <cross-commons/debug/tlog.h>

#include <sql/nosql/array.h>

#include <commons/meteo_data/meteo_data.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/log.h>

#include <sql/dbi/gridfs.h>

static constexpr bool kHashFile        = false;
static constexpr bool kHashField       = false;
static constexpr bool kHashAfield      = false;
static constexpr int  kReservedSize    = 1024;
static constexpr int  kMaxHashSize     = 256;
static constexpr int  kMaxForecastTime = 12 * 3600; //максимальное время действия прогноза



namespace meteo {
namespace field {

TFieldDataDb::TFieldDataDb()
{
  auto opt = Global::instance()->mongodbConfObanal();
  const_cast<QString&>(kObanaldb) = opt.name();

  if(kHashField)  { fieldHash.reserve(kReservedSize); fieldHash_old.reserve(kReservedSize); }
  if(kHashAfield) { aviableFieldHash.reserve(kReservedSize); }
  if(kHashFile)   { fileHash.reserve(kReservedSize);         }
  auto conf = meteo::Global::instance()->mongodbConfMeteo();
  dbname_ = conf.name();
}

TFieldDataDb::~TFieldDataDb() { }

bool TFieldDataDb::getFieldDates(const DataRequest *req, DateReply *resp)
{
  resp->clear_date();

  QString queryStr;
  auto db = meteo::global::dbObanal();
  if ( false == prepareQuery("get_field_dates", &db,&queryStr)){
      delete db;
      return false;
  }

  NosqlQuery query;
  if(0 == req->center_size()) {
      queryStr = query.removeArg(queryStr,"pipeline",0,"$match.center");
    }
  query.setQuery( queryStr)
      .arg( query.datetimeFromString(req->date_start()) )
      .arg( query.datetimeFromString(req->date_end()) );

  if(0 != req->center_size()) {
      query.arg( req->center() );
    }

  bool quer = db->execQuery(  query.query() );
  if ( false == quer ) {
      debug_log<<QObject::tr("Не удалось выполнить запрос %1").arg( query.query() );
      delete db;
      return false;
    }
  if ( false == db->toCursor() ) {
      debug_log<<QObject::tr("Нет курсора в результате выполнения запроса %1").arg( query.query() );
      delete db;
      return false;
    }
  for (int i = 0; true == db->next();++i ) {
      Document doc = db->document();
      resp->add_date();
      resp->set_date(i,doc.valueDt("_id").toString(Qt::ISODate).toStdString());
    }
  delete db;
  return true;
}

void TFieldDataDb::checkQueryMatch(const DataRequest* req, const QString& querys, NosqlQuery* query)
{
  QString queryStr = querys;
  if (false == req->has_date_start()) {
      queryStr = query->removeArg(queryStr, "pipeline", 0, "$match.dt");
    }
  if (0 == req->meteo_descr_size()) {
      queryStr = query->removeArg(queryStr, "pipeline", 0, "$match.descr");
    }
  if (0 == req->type_level_size()) {
      queryStr = query->removeArg(queryStr, "pipeline", 0, "$match.level_type");
    }
  if (0 == req->level_size()) {
      queryStr = query->removeArg(queryStr, "pipeline", 0, "$match.level");
    }
  if (0 == req->hour_size()) {
      queryStr = query->removeArg(queryStr, "pipeline", 0, "$match.hour");
    }
  if (0 == req->center_size()) {
      queryStr = query->removeArg(queryStr, "pipeline", 0, "$match.center");
    }
  if (false == req->has_model()) {
      queryStr = query->removeArg(queryStr, "pipeline", 0, "$match.model");
    }
  query->setQuery(queryStr);
}


bool TFieldDataDb::prepareQueryExec(const QString & templ_name, NoSql ** db, QString* errstr){
  QString queryStr;
  NosqlQuery query;

  if ( false == prepareQuery(templ_name, db, &queryStr,errstr)){
      return false;
    }

  query.setQuery(queryStr);

  bool queryRes = (*db)->execQuery( query.query());
  QString error_str;

  if (false == queryRes) {
      error_str = QObject::tr("Запрос выполнен с ошибкой") + templ_name;
      debug_log << error_str;
      if(nullptr != errstr){
          *errstr = error_str;
      }
      return false;
    }

  if (false == (*db)->toCursor()) {
      error_str = QObject::tr("Нет курсора в запросе") + templ_name;
      debug_log << error_str;
      if(nullptr != errstr){
          *errstr = error_str;
      }
      return false;
    }
 return true;
}



bool TFieldDataDb::prepareQuery(const QString& templ_name, NoSql** db,QString* queryStr, QString* errstr)
{
  QString error_str;
  if(nullptr == (*db)) {
    error_str = msglog::kDbObanalNotReady;
    debug_log << error_str;
    if(nullptr != errstr){
      *errstr = error_str;
    }
    return false;
  }

  if ( false == (*db)->connected() && false == (*db)->connect() ) {
    error_str = QObject::tr("Нет подключения к БД");
    debug_log << error_str;
    if(nullptr != errstr){
      *errstr = error_str;
    }
    return false;
  }

  if ( false == meteo::global::kMongoQueriesNew.contains(templ_name) ){
    error_str = QObject::tr("Не удается найти шаблон запроса в базу данных: ")+templ_name;
    debug_log << error_str;
    if(nullptr != errstr){
      *errstr = error_str;
    }
    return false;
  }

  (*queryStr) = meteo::global::kMongoQueriesNew[templ_name];
  if (queryStr->isEmpty()){
    error_str = QObject::tr("Отсутствует шаблон запроса в базу")+templ_name;
    debug_log << error_str;
    if(nullptr != errstr){
      *errstr = error_str;
    }
    return false;
  };

  return true;
}

bool TFieldDataDb::saveToFile(const QString& basePath,const QString& fileName, const std::string &data)
{

  QDir dir(basePath);
  if (!dir.exists()) {
      if (!dir.mkpath(basePath)) {
          debug_log <<msglog::kDirCreateFailed.arg(basePath);
          return false;
        }
    }

  const QString filePath = basePath + "/" + fileName;
  QFile file( filePath );
  if (!file.open(QIODevice::WriteOnly)) {
      debug_log << msglog::kFileOpenError.arg(filePath).arg(-1);
      return false;
    }

  QByteArray ba( data.data(), data.size() );
  file.write(ba);
  file.close();

  return true;
}

#ifdef __linux__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

const int  kBolckSize = 4096;
const int  kBufferSize = (2 * 4096);//(16 * 1024)

size_t f_read(int fd, char *buffer)
{
  if(fd == -1)
    return fd;
  else
    {
#ifdef __linux__
      posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);

      size_t bytes_read;
      size_t total_bytes = 0;

      while((bytes_read = read(fd, buffer + total_bytes, kBufferSize)))
        {
          if(!bytes_read)
            break;
          total_bytes += bytes_read;
        }
#endif
      return total_bytes;

    }

}
#endif

}
}
