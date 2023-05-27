#include "stdinreader.h"

#include <fcntl.h>
#include <unistd.h>

#include <qcoreapplication.h>
#include "qdir.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/commongrid.h>
#include <meteo/commons/ui/map/loader.h>
#include <meteo/commons/ui/map/weather.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/ui/map/document.h>

namespace meteo {
namespace map {

static constexpr int32_t kSzint = sizeof(int32_t);

StdinReader::StdinReader()
  : QObject(),
  tm_( new QTimer(this) ),
  in_( new QFile(this) ),
  out_( new QFile(this) ),
  begdt_( QDateTime::currentDateTime() )
{
  Loader::instance()->registerCitiesHandler(meteo::global::kCitiesLoaderCommon, &meteo::map::loadCitiesLayer);
  QObject::connect( tm_, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
  tm_->setSingleShot(true);
}

StdinReader::~StdinReader()
{
  delete in_;
  in_ = nullptr;
  delete out_;
  out_ = nullptr;
  delete tm_;
  tm_ = nullptr;
}

bool StdinReader::connectStdinout()
{
  int flags = ::fcntl( STDIN_FILENO, F_GETFL, 0 );
  ::fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
  if ( false == in_->open( stdin, QIODevice::ReadOnly ) ) {
    error_log << QObject::tr("Не удалось открыть stdin (стандартный ввод)");
    return false;
  }
  if ( false == out_->open( stdout, QIODevice::WriteOnly ) ) {
    error_log << QObject::tr("Не удалось открыть stdout (стандартный ввод)");
    return false;
  }
  tm_->start(250);

  return true;
}

bool StdinReader::parseAndRunJob()
{
  proto::Job job;
  if ( false == parseJob(&job) ) {
    return false;
  }
  tm_->stop();
  meteo::map::proto::ExportResponse result;
  runJob( job, &result );
  std::string str;
  result.SerializePartialToString(&str);
  int32_t sz = str.size();
  char szstr[ sizeof(int32_t) ];
  ::memcpy( szstr, &sz, sizeof(int32_t) );
  out_->write( szstr, sizeof(int32_t) );
  out_->write( str.data(), str.size() );
  return true;
}

void StdinReader::runTestJob()
{
  auto wl = WeatherLoader::instance();
  auto jobs = wl->mapjobs();
  if ( 0 == jobs.size() ) {
    error_log << "empty job list!";
    return;
  }
  auto job = jobs["Yavlenia-Smolensk.job"];
  debug_log << "JOB =" << job.Utf8DebugString();
  Weather weather;
  Document* doc = Weather::documentFromProto( &( job.document() ) );
  if ( nullptr == doc ) {
    error_log << QObject::tr("Не удалось создать документ по proto-структуре");
    return;
  }
  QByteArray arr;
  bool res = weather.createMap( doc, job.map() );
//  if ( false == res ) {
//    error_log << QObject::tr("При созднии слоев документа произошла ошибка. Документ может содержать некорректные данные");
//    delete doc;
//    doc = nullptr;
//    return;
//  }


  res = doc->exportDocument( job.map(), &arr );
  if ( false == res ) {
    error_log << QObject::tr("Ошибка экспорта документа");
    delete doc;
    doc = nullptr;
    return;
  }
  QFile file("./test.jpeg");
  file.open(QIODevice::WriteOnly);
  file.write(arr);
  file.flush();
  file.close();
}

bool StdinReader::parseJob( meteo::map::proto::Job* job )
{
  Q_UNUSED(job);
  QByteArray arr(1024,'\0');
  char* data = arr.data();
  qint64 sz = in_->read( data, 1024 );
  if ( 0 >= sz ) {
    return false;
  }
  stdinarr_.append( data, sz );
  if ( kSzint > stdinarr_.size() ) {
    return false;
  }
  int32_t datasize;
  ::memcpy( &datasize, stdinarr_.data(), sizeof(int32_t) );
  if ( 0 >= datasize ) {
    error_log << QObject::tr("Ожидается задание нулевого размера. Сенарий завершается.");
    qApp->exit(EXIT_FAILURE);
    return false;
  }
  if ( datasize + kSzint <= stdinarr_.size() ) {
    if ( false == job->ParsePartialFromArray( stdinarr_.data() + kSzint, datasize ) ) {
      error_log << QObject::tr("Получено сериализованное сообщение, но его не удалось десериализовать. Сенарий завершается.");
      qApp->exit(EXIT_FAILURE);
      return false;
    }
    return true;
  }
  return false;
}

bool StdinReader::runJob( const meteo::map::proto::Job& job, meteo::map::proto::ExportResponse* result )
{
  
  Weather weather;
  Document* doc = Weather::documentFromProto( &( job.document() ) );
  if ( nullptr == doc ) {
    error_log << QObject::tr("Не удалось создать документ по proto-структуре");
    return false;
  }
  QByteArray arr;
  bool res = weather.createMap( doc, job.map() );
  if ( false == res ) {
    error_log << QObject::tr("При созднии слоев документа произошла ошибка. Документ может содержать некорректные данные");
    result->set_result(false);
    delete doc;
    doc = nullptr;
    return false;
  }


  res = doc->exportDocument( job.map(), &arr );
  if ( false == res ) {
    error_log << QObject::tr("Ошибка экспорта документа");
    delete doc;
    doc = nullptr;
    return false;
  }

  result->set_result(true);
  if ( proto::kSaveClientFile == ( job.storetype() & proto::kSaveClientFile ) ) {
    result->set_data( arr.data(), arr.size() );
  }
  if ( proto::kSaveServerFile == ( proto::kSaveServerFile & job.storetype() ) ) {
    res = doc->saveDocumentToMongo( job.map(), arr, doc->fileName( QString::fromStdString( job.name() ), job.format() ), job.format() );
  }
  if ( proto::kSaveServerClient == job.storetype() || proto::kSaveClientFile == job.storetype() ) {
    saveFile(job, arr, QString::fromStdString(job.clientsavepath()));
  }
  for (int i = 0, sz = job.user_save_path_size(); i < sz; ++i ) {
    saveFile(job, arr, QString::fromStdString(job.user_save_path(i)));
  }

  return res;
}

bool StdinReader::saveFile(const proto::Job &job, const QByteArray &data, const QString &path)
{
  QString ext;
  ext = meteo::global::getDocumentFormatExt( job.format() );

  QString locpath = path;

  QDateTime dt = QDateTime::fromString( QString::fromStdString(job.map().datetime()), Qt::ISODate );

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

  QFile file( locpath + '/' + QString::fromStdString(job.title()) +
   dt.toString("yyyy_MM_dd_hh_mm_ss") + '.'+QString::number(job.map().hour())+"." + ext );
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

void StdinReader::slotTimeout()
{
  if ( false == parseAndRunJob() && 3000 > begdt_.msecsTo( QDateTime::currentDateTime() ) ) {
    tm_->start(200);
  }
  else {
    qApp->exit(EXIT_SUCCESS);
  }
}

}
}
