#include <stdint.h>
#include <qmetatype.h>

#include "mapclient.h"

#include <qdir.h>
#include <qpixmap.h>
#include <qlabel.h>

#include <cross-commons/app/options.h>
#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geopoint.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/global/common.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/ui/map/weather.h>
#include <meteo/commons/global/weatherloader.h>

#include <meteo/commons/planner/timesheet.h>

#include <sql/psql/psqlquery.h>

namespace meteo {
namespace map {

namespace proto {
inline uint32_t qHash( const ::meteo::map::proto::Job& job )
{
  std::string str;
  job.SerializeToString(&str);
  QByteArray arr( str.data(), str.size() );
  return qHash(arr);
}

inline bool operator==( const ::meteo::map::proto::Job& job1, const ::meteo::map::proto::Job& job2 )
{
  std::string str1;
  job1.SerializeToString(&str1);
  std::string str2;
  job2.SerializeToString(&str2);
  return str1 == str2;
}

}

Client::Client()
{
}

Client::~Client()
{
}

proto::Job Client::jobDetali( const proto::Job& j ) const
{
  proto::Job job;
  job.CopyFrom(j);
  commons::ArgParser* options = commons::ArgParser::instance();
  proto::DocumentFormat format = proto::kJpeg;
  if ( true == options->installed(kFormat) ) {
    QString fstr = options->at(kFormat).value();
    if ( "bmp" == fstr ) {
      format = proto::kBmp;
    }
    else if ( "png" == fstr ) {
      format = proto::kPng;
    }
    else if ( "sxf" == fstr ) {
      format = proto::kSxf;
    }
  }
  else if ( true == job.has_format() ) {
    format = job.format();
  }
  job.set_format(format);
  job.mutable_map()->set_format(format);

  if ( true == options->installed(kPath) ) {
    QString path = options->at(kPath).value();
    job.set_clientsavepath( path.toStdString() );
    job.set_storetype(proto::kSaveClientFile);
  }
  else if ( false == job.has_storetype() || proto::kNoSave == job.storetype() ) {
    job.set_storetype(proto::kSaveServerFile);
  }
  if ( true == options->installed( kMapCnt ) ) {
    QString cntstr = options->at(kMapCnt).value();
    QStringList strlist = cntstr.split("x");
    if ( 2 != strlist.size() ) {
      error_log << QObject::tr("Неверно задан центр карты '%1'").arg(cntstr);
      exit(1);
    }
    double lat = strlist[0].toDouble();
    double lon = strlist[1].toDouble();
    GeoPoint gp = GeoPoint::fromDegree( lat, lon );
    job.mutable_document()->mutable_map_center()->CopyFrom( geopoint2pbgeopoint(gp) );
    job.mutable_document()->mutable_doc_center()->CopyFrom( geopoint2pbgeopoint(gp) );
  }
  else if ( false == job.mutable_document()->has_map_center() ) {
    GeoPoint gp = GeoPoint::fromDegree( 60, 30 );
    job.mutable_document()->mutable_map_center()->CopyFrom( geopoint2pbgeopoint(gp) );
    job.mutable_document()->mutable_doc_center()->CopyFrom( geopoint2pbgeopoint(gp) );
  }
  if ( true == options->installed(kScale) ) {
    job.mutable_document()->set_scale( options->at(kScale).value().toDouble()  );
  }
  else if ( false == job.mutable_document()->has_scale() ) {
    job.mutable_document()->set_scale(13);
  }
  if ( true == options->installed(kProj) ) {
    QString strproj = options->at(kProj).value();
    if ( "merkat" == strproj ) {
      job.mutable_document()->set_projection( kMercat );
    }
    else {
      job.mutable_document()->set_projection( kStereo );
    }
  }
  else if ( false == job.mutable_document()->has_projection() ) {
    job.mutable_document()->set_projection( kStereo );
  }
  if ( true == options->installed( kSize ) ) {
    QString sizestr = options->at(kSize).value();
    QStringList strlist = sizestr.split("x");
    if ( 2 != strlist.size() ) {
      error_log << QObject::tr("Неверно задан размер документа '%1'").arg(sizestr);
      exit(1);
    }
    int shir = strlist[0].toInt();
    int vys = strlist[1].toInt();
    QSize size( shir, vys );
    job.mutable_document()->mutable_docsize()->CopyFrom( qsize2size(size) );
  }
  else if ( false == job.mutable_document()->has_docsize() ) {
    job.mutable_document()->mutable_docsize()->CopyFrom( qsize2size( QSize(1920,1080) ) );
  }
  return job;
}

QHash< proto::Job, QList<proto::Map> > Client::getMapsFromCommandLine() const
{
  const QMap< QString, proto::Map >& knownmaps = WeatherLoader::instance()->weathermaps();
  const QMap< QString, proto::Job >& knownjobs = WeatherLoader::instance()->mapjobs();
  QHash< proto::Job, QList<proto::Map> > maps;

  commons::ArgParser* options = commons::ArgParser::instance();
  if ( true == options->installed(kJob) ) { //будет выполняться только одно задание
    proto::Job job;
    QString jobname = options->at(kJob).value();
    if ( false == knownjobs.contains(jobname) ) {
      error_log << QObject::tr("Задано неизвестное задание '%1'").arg(jobname);
      return maps;
    }
    job.CopyFrom( knownjobs[jobname] );
    if ( true == options->installed(kMap) ) {
      QString mapname = options->at(kMap).value();
      if ( false == knownmaps.contains(mapname) ) {
        error_log << QObject::tr("Задана неизвестная карта '%1'").arg(mapname);
        return maps;
      }
      job.set_map_id( mapname.toStdString() );
    }
    maps.insert( jobDetali(job), mapsForJob(job) );
  }
  else if ( true == options->installed(kMap) ) {
    proto::Job job;
    job.set_name("temporary");
    job.set_title("temporary");
    QString mapname = options->at(kMap).value();
    if ( false == knownmaps.contains(mapname) ) {
      error_log << QObject::tr("Задана неизвестная карта '%1'").arg(mapname);
      return maps;
    }
    job.set_map_id( mapname.toStdString() );
    maps.insert( jobDetali(job), mapsForJob(job) );
  }
  else {
    for ( const auto &a: knownjobs ) {
      maps.insert( jobDetali(a), mapsForJob(a) );
    }
  }
  return maps;
}

QList<proto::Map> Client::mapsForJob( const proto::Job& job ) const
{
  commons::ArgParser* options = commons::ArgParser::instance();
  QList<proto::Map> list;
//  QString mapname = QString::fromStdString( job.map_id() );
//  if ( false == WeatherLoader::instance()->weathermaps().contains(mapname) ) {
//    error_log << QObject::tr("Задана неизвестная карта '%1'").arg(mapname);
//    return list;
//  }
  proto::Job locjob;
  locjob.CopyFrom(job);
//  proto::Map origmap = WeatherLoader::instance()->weathermaps()[mapname];
  proto::Map origmap = job.map();
  QList<QDateTime> dates = termsForJob(job);
  for ( const auto &d : dates ) {
    origmap.set_datetime( d.toString( Qt::ISODate ).toStdString() );
    if ( true == options->installed(kCenterParam) ) {
      origmap.set_center( options->at(kCenterParam).value().toInt() );
    }
    else if ( true == job.has_center() ) {
      origmap.set_center( job.center() );
    }
    else {
      origmap.set_center(250);
    }
    if ( true == options->installed(kModel) ) {
      origmap.set_model( options->at(kModel).value().toInt() );
    }
    else if ( true == job.has_model() ) {
      origmap.set_model( job.model() );
    }
    else {
      origmap.set_model(250);
    }

    if ( 0 < job.map().station_size() ) {
      origmap.clear_station();
      for(int i = 0, sz=job.map().station_size();i<sz;++i){
        origmap.add_station()->CopyFrom(job.map().station(i));
      }
    }

    if ( true == job.document().has_doctype() ) {
      origmap.mutable_document()->set_doctype( job.document().doctype() );
    }
    if (true == job.has_profile()) {
      origmap.set_profile(job.profile());
    }
    if(true == origmap.has_source()){
    }
    if (true == job.document().has_scale()) {
      origmap.mutable_document()->set_scale(job.document().scale());
    }
    if ( true == options->installed(kHour) ) {
      origmap.set_hour( options->at(kHour).value().toInt()/**3600*/ );
      list.append(origmap);
    }
    else if ( 0 != locjob.hour_size() ) {
      for ( int i = 0, sz = locjob.hour_size(); i < sz; ++i ) {
        proto::Map hmap;
        hmap.CopyFrom(origmap);
        hmap.set_hour( locjob.hour(i)/**3600*/ );
        list.append(hmap);
      }
    }
  }
//  for ( auto m : list ) {
//    debug_log << m.Utf8DebugString();
//  }
  return list;
}

QList<QDateTime> Client::termsForJob( const proto::Job& job ) const
{
  proto::Job locjob;
  locjob.CopyFrom(job);
  commons::ArgParser* options = commons::ArgParser::instance();
  QList<QDateTime> list;
  if ( true == options->installed(kDate) ) {
    QString strdate = options->at(kDate).value();
    QDateTime dt = PsqlQuery::datetimeFromString(strdate);
    if ( false == dt.isValid() ) {
      error_log << QObject::tr("Неверный формат срока карты '%1'").arg(strdate);
      return list;
    }
    list.append(dt);
    return list;
  }
  if ( true == options->installed(kBegin) || true == options->installed(kBegin) ) {
    return dtListFromBegintoEnd(job);
  }
  QList<QDate> dates;
  QDate cdUtc = QDateTime::currentDateTimeUtc().date();
  dates.append( cdUtc );
  for ( int i = 0, sz = locjob.additional_date_size(); i < sz; ++i ) {
    QDate d = PsqlQuery::dateFromString(locjob.additional_date(i));
    if ( false == d.isValid() ) {
      warning_log << QObject::tr("Неверный формат даты '%1'")
        .arg( QString::fromStdString( locjob.additional_date(i) ) );
      continue;
    }
    dates.append(d);
  }
  if ( false == locjob.has_period() ) {
    warning_log << QObject::tr("В задании '%1' не указана периодичность. Устанавливается 60 мин.")
      .arg( QString::fromStdString( locjob.title() ) );
    locjob.set_period(60);
  }
  for ( const QDate& a : dates ) {
    QDateTime cdt;
    QDateTime jobdt;
    if ( a == QDateTime::currentDateTimeUtc().date() ) {
      cdt = QDateTime::currentDateTimeUtc();
      cdt = cdt.addSecs(job.await()*60*-1);
      int hourInSecs = (QTime(0,0,0).secsTo(cdt.time())/(60*locjob.period())) * (60*locjob.period());
      QDateTime nearest = QDateTime(cdt.date(), QTime(0,0,0).addSecs(hourInSecs));
      jobdt = nearest;
    }
    else {
      cdt = QDateTime( a, QTime(23,59,59) );
      jobdt = QDateTime( a, QTime(0,0,0) );
    }

    jobdt.setTimeSpec(Qt::UTC);
    while ( jobdt < cdt ) {
      list.append(jobdt);
      jobdt = jobdt.addSecs( 60*locjob.period() );
    }
  }

  return list;
}

bool Client::processJob( proto::Job& job )
{

  if(false == job.enabled()){
    warning_log << QObject::tr("Внимание! Задание помечено как неактивное");
    return false;
  }

  meteo::rpc::Channel* ch = meteo::global::serviceChannel( meteo::settings::proto::kMap );
  if ( nullptr == ch ) {
    //debug_log << QObject::tr("Не удалось установить соединение с картографическим сервисом");
    return false;
  }
  job.mutable_map()->set_format(job.format());
  proto::ExportResponse* resp = ch->remoteCall( &proto::DocumentService::ProcessJob, job, 100000 );
  if ( nullptr == resp ) {
    error_log << QObject::tr("От службы документов не получен ответ за 100 сек. Работа = %1")
      .arg( QString::fromStdString( job.Utf8DebugString() ) );
    delete ch;
    return false;
  }
  bool res = resp->result();
  if ( false == res ) {
//    debug_log << QObject::tr("Документ не сформирован. Ошибка: ") << resp->comment();
    delete resp;
    delete ch;
    return false;
  }
  QByteArray arr( resp->data().size(), '\0' );
  if ( proto::kSaveClientFile == ( job.storetype() & proto::kSaveClientFile ) ) {
    arr.setRawData( resp->data().data(), resp->data().size() );
    job.mutable_map()->set_format(job.format());
    res = saveFile( job.map(), arr, QString::fromStdString( job.clientsavepath()) );
  }
  delete resp;
  delete ch;
  return res;
}

bool Client::processJob(const proto::Job& job, const proto::Map& map, rpc::Channel* ctrl )
{
  if ( proto::kNoSave == ( job.storetype() ) ) {
    debug_log << QObject::tr("Работа по созданию карты не требует сохранения. Нечего делать.");
    return true;
  }
  proto::ExportRequest exprt;
  exprt.set_format( job.format() );

  proto::Response* resp = ctrl->remoteCall( &proto::DocumentService::CreateMap, map, 100000 );
  if ( nullptr == resp ) {
    error_log << QObject::tr("Ответ от сервиса при создании карты не получен. Шаблон = %1. Дата = %2")
      .arg( QString::fromStdString( map.name() ) )
      .arg( QString::fromStdString( map.datetime() ) );
    delete resp;
    return false;
  }
  if ( false == resp->result() ) {
    debug_log << QObject::tr("Документ не создан. Шаблон = %1. Дата = %2")
      .arg( QString::fromStdString( map.name() ) )
      .arg( QString::fromStdString( map.datetime() ) );
    delete resp;
    return false;
  } else {
      debug_log << QObject::tr("Документ успешно создан. Шаблон = %1. Дата = %2")
        .arg( QString::fromStdString( map.name() ) )
        .arg( QString::fromStdString( map.datetime() ) );

  }
  delete resp; resp = nullptr;

  QByteArray arr;

  proto::ExportResponse* expresp = nullptr;
  if ( proto::kSaveClientFile == ( job.storetype() & proto::kSaveClientFile ) ) {
    expresp = ctrl->remoteCall( &proto::DocumentService::GetDocument, exprt, 10000 );
    if ( nullptr == expresp ) {
      debug_log << QObject::tr("Документ не получен.");
      return false;
    }
    if ( false == expresp->result() ) {
      error_log << QObject::tr("Ошибка получения документа = %1")
        .arg( QString::fromStdString( expresp->comment() ) );
      delete expresp;
      return false;
    }
    arr.setRawData( expresp->data().data(), expresp->data().size() );
  }
  bool res = true;
  if ( proto::kSaveClientFile == ( job.storetype() & proto::kSaveClientFile ) ) {
    res = saveFile( map, arr, QString::fromStdString( job.clientsavepath()) );
  }
  delete expresp; expresp = nullptr;
  if ( false == res ) {
    return res;
  }
  if ( proto::kSaveServerFile == ( proto::kSaveServerFile & job.storetype() ) ) {
    res = saveServerFile( job, ctrl );
  }
  if ( false == res ) {
    return res;
  }
  return true;
}

bool Client::saveFile( const proto::Map& info, const QByteArray& data, const QString& path )
{
  QString ext;
  ext = meteo::global::getDocumentFormatExt( info.format() );

  QString locpath = path;

  QDateTime dt = PsqlQuery::datetimeFromString( info.datetime() );

  QDir dir(locpath);
  locpath = dir.absolutePath();
  if ( false == dir.exists() ) {
    bool res = dir.mkpath(locpath);
    if ( false == res ) {
      error_log << QObject::tr("Не удалось создать директорию %1")
                   .arg(locpath);
      return false;
    }
  }

  QFile file( locpath + '/' + dt.toString("yyyy_MM_dd_hh_mm_ss") + '.' + ext );
  if ( false == file.open(QIODevice::WriteOnly | QIODevice::Truncate) ) {
    error_log << QObject::tr("Не удалось открыть файл для записи %1")
      .arg( file.fileName() );
    return false;
  }
  file.write(data);
  file.flush();
  file.close();
  return true;
}

bool Client::saveServerFile(const proto::Job& job, rpc::Channel* ctrl )
{
  proto::ExportRequest request;
  request.set_format( job.format() );
  request.set_name( job.name() );
  request.set_title(job.title());
  proto::Response* response = ctrl->remoteCall( &proto::DocumentService::SaveDocument, request, 100000 );
  if ( 0 == response ) {
    error_log << QObject::tr("Во время запроса на сохранение ответ от картографического сервиса не получен");
    return false;
  }

  if ( false == response->result() ) {
    error_log << QObject::tr("Ошибка при сохранении карты погоды на сервере = %1")
      .arg( QString::fromStdString( response->comment() ) );
    delete response;
    return false;
  }
  delete response;
  return true;
}

void Client::getAvailableDocuments( rpc::Channel* ctrl )
{
  if ( nullptr == ctrl ) {
    error_log << QObject::tr("Нет соединения с картографическим сервисом");
    return;
  }
  commons::ArgParser* options = commons::ArgParser::instance();
  QDateTime dts = QDateTime::currentDateTime();
  if ( true == options->installed(kDate) ) {
    dts = QDateTime::fromString( options->at(kDate).value(), Qt::ISODate );
  }
  dts.setTime( QTime( 0, 0, 0 ) );
  QDateTime dte = dts;
  dte.setTime( QTime( 23, 59,59 ) );
  proto::MapRequest req;
  req.set_date_start( dts.toString(Qt::ISODate).toStdString() );
  req.set_date_end( dte.toString(Qt::ISODate).toStdString() );
  proto::MapList* resp = ctrl->remoteCall( &proto::DocumentService::GetAvailableDocuments, req, 100000 );
  if ( nullptr == resp ) {
    error_log << QObject::tr("Ответ от сервиса не получен");
    return;
  }
  for ( const auto &m : resp->map() ) {
    debug_log << "MAP =" << m.Utf8DebugString();
  }

  delete resp;
}

void Client::loadDocJobs(bool *ok)
{
  *ok = false;
  docjobs_.clear();
  QString path = MnCommon::sharePath("meteo") + "/jobs/formal";
  QDir dir(path);
  if ( false == dir.exists() ) {
    warning_log << QObject::tr("Директория %1 не существует, но будет создана")
      .arg(path);
    if ( false == dir.mkpath(path) ) {
      error_log << QObject::tr("Не удалось создать директорию %1")
        .arg(path);
      return;
    }
  }
  QMap< QString, meteo::map::proto::JobList > setts;
  bool res = TProtoText::loadProtosFromDirectory( path, &setts );
  if ( false == res ) {
    error_log << QObject::tr("Не удалось загрузить файлы из директории %1")
      .arg(path);
    return;
  }
  QMapIterator< QString, meteo::map::proto::JobList > it(setts);
  while ( true == it.hasNext() ) {
    it.next();
    const meteo::map::proto::JobList& jobs = it.value();
    for ( int i = 0, sz = jobs.job_size(); i < sz; ++i ) {
      const meteo::map::proto::Job& job = jobs.job(i);
      docjobs_.insert( QString::fromStdString( job.name() ), job );
    }
  }
  *ok = true;
}

void Client::load(bool *ok)
{
  loadDocJobs(ok);
}

QList<QDateTime> Client::jobDates( const proto::Job& job ) const
{
  QList<QDateTime> dates;
  for ( int i = 0, sz = job.additional_date_size(); i < sz; ++i ) {
    QDate date = QDate::fromString( QString::fromStdString( job.additional_date(i) ), "yyyy-MM-dd" );
    if ( false == date.isValid() ) {
      warning_log << QObject::tr("Дата в расписании формирования карт %1 указана не верно = %2")
        .arg( QString::fromStdString( job.name() ) )
        .arg( QString::fromStdString( job.additional_date(i) ) );
      continue;
    }
    QDateTime dt = QDateTime( date, QTime(0,0,0) );
    while ( date == dt.date() ) {
      dates.append(dt);
      dt = dt.addSecs( job.period()*60 );
    }
  }
  QDateTime curdt = QDateTime::currentDateTimeUtc();
  QDateTime dt = QDateTime( curdt.date(), QTime(0,0,0) );
  while ( curdt >= dt ) {
    dates.append(dt);
    dt = dt.addSecs( job.period()*60 );
  }
  return dates;
}

QList<QDateTime> Client::dtListFromBegintoEnd(const proto::Job& job) const
{
  commons::ArgParser* options = commons::ArgParser::instance();
  QDateTime beginDt;
  QDateTime endDt;
  if (true == options->installed(kBegin)) {
    beginDt  = QDateTime::fromString(options->at(kBegin).value(), Qt::ISODate);
  }
  else {
    beginDt = QDateTime::currentDateTimeUtc();
  }
  if (true == options->installed(kEnd)) {
    endDt  = QDateTime::fromString(options->at(kEnd).value(), Qt::ISODate);
  }
  else {
    endDt = QDateTime::currentDateTimeUtc();
  }
  QList<QDateTime> list;
  if (endDt < beginDt) {
    error_log << QObject::tr("Ошибка! Срок конца %1 раньше срока начала %2")
                 .arg(endDt.toString(Qt::ISODate))
                 .arg(beginDt.toString(Qt::ISODate));
    return list;
  }
  else if ( endDt == beginDt ) {
    list.append(beginDt);
    warning_log << QObject::tr("Внимание! Срок окончания совпадает со сроком начала");
    return list;
  }
  proto::Job locjob;
  locjob.CopyFrom(job);
  if ( false == locjob.has_period() ) {
    warning_log << QObject::tr("В задании '%1' не указана периодичность. Устанавливается 60 мин.")
      .arg( QString::fromStdString( locjob.title() ) );
    locjob.set_period(60);
  }
  QDateTime counter = beginDt;
  qint64 secs = int(QTime(0,0,0).secsTo(counter.time())/(locjob.period()*60)) * (locjob.period()*60);
  QTime startTime = QTime(0,0,0).addSecs(secs);
  counter.setTime(startTime);
  qint64 period = locjob.period() * 60; //Период в секундах
  for (; counter <= endDt  ; counter = counter.addSecs(period) ) {
    list.append(counter);
  }
  return list;
}

}
}
