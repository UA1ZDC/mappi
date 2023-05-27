#include "faxbuilder.h"

#include <sql/dbi/dbiquery.h>
#include <meteo/commons/global/dbnames.h>
#include <sql/nosql/nosqlquery.h>
#include <sql/dbi/gridfs.h>

FaxBuilder::FaxBuilder()
{
  setObjectName(QObject::tr("Сборщик факсов"));
}


QStringList FaxBuilder::subscribeMsgTypes()
{
  return { QObject::tr("fax") };
}

QString FaxBuilder::loaderUid()
{
  return QObject::tr("Fax image loader");
}

bool FaxBuilder::saveChain(const ChunkStats& chain, const QByteArray& compiledBuffer)
{
  static const QString& gridfsPreffix = QObject::tr("fax");
  const QDateTime &dt = chain.convertedDt_;
  const QString& dbFileName = generateFileName(dt, chain.header_ );

  meteo::FaxBuffer buffer(compiledBuffer, chain.knownChunks_.values().first().formatString().lpm());
  const QByteArray& tiffImage = buffer.tiffBuffer();

  if ( false == saveFaxDataToGridFs(dbFileName, tiffImage, gridfsPreffix) ){
    error_log << QObject::tr("Ошибка при сохранении факса в GRIDFS");
    return false;
  }

  if ( false == saveFaxInfo(dt, chain.header_, chain.imageid_, dbFileName) ){
    error_log << QObject::tr("Ошибка при сохранении информации о факсе в базе данных");
    return false;
  }
  return true;
}

bool FaxBuilder::saveFaxInfo(const QDateTime &dt,
                             const meteo::tlg::Header &header,
                             int magic,
                             const QString& imagePath)
{
  QString queryName = QObject::tr("insert_fax");

  std::unique_ptr<meteo::Dbi> db( meteo::global::dbDocument() );
  std::unique_ptr<meteo::DbiQuery> query(db->queryptrByName(queryName));
  if (nullptr == query) {
    error_log << meteo::msglog::kDbRequestNotFound.arg(queryName);
    return false;
  }

  query->arg( "t1", header.t1() )
      .arg( "t2", header.t2() )
      .arg( "a1", header.a1() )
      .arg( "a2", header.a2() )
      .arg( "ii", header.ii() )
      .arg( "cccc", header.cccc() )
      .arg( "yygggg", header.yygggg() )
      .arg( "magic", magic )
      .arg( "image_path", imagePath )
      .arg( "dt", dt );

  if ( false == query->exec() ) {
    error_log << QObject::tr("Ошибка при выполнении запроса в базе данных");
    return false;
  }

  const meteo::DbiEntry& doc = query->result();
  int ok = qRound(doc.valueDouble("ok"));  
  if ( 1 != ok ){
    error_log << meteo::msglog::kDbRequestFailed;
    return false;
  }
  return true;
}

bool FaxBuilder::saveFaxDataToGridFs(const QString& fileName, const QByteArray& data, const QString &prefixName )
{
  QString fileByNameQuery = QObject::tr("get_grid_file_by_filename");
  if ( false == meteo::global::kMongoQueriesNew.contains(fileByNameQuery) ){
    error_log << meteo::msglog::kDbRequestNotFound.arg(fileByNameQuery);
    return false;
  }

  meteo::GridFs gridfs;
  auto conf = meteo::global::mongodbConfDocument();

  if ( false == gridfs.connect(conf) ) {
    error_log << gridfs.lastError();
    return false;
  }
  gridfs.use(meteo::db::kDbDocuments, prefixName);

  if ( true == gridfs.findOneByName(fileName).hasFile() ){
    warning_log << QObject::tr("Внимание: файл %1 будет перезаписан")
                   .arg(fileName);
  }
  gridfs.put( fileName, data);
  return true;
}

QString FaxBuilder::generateFileName(const QDateTime &dt,const meteo::tlg::Header &header)
{

  QString t1 = QString::fromStdString(header.t1());
  QString t2 = QString::fromStdString(header.t2());
  QString a1 = QString::fromStdString(header.a1());
  QString a2 = QString::fromStdString(header.a2());
  int ii = header.ii();
  QString cccc = QString::fromStdString(header.cccc());
  QString yygggg =  QString::fromStdString(header.yygggg());
  return QObject::tr("fax_%1_%2%3%4%5_%6_%7_%8.tiff")
      .arg(dt.toString(Qt::ISODate).replace("z","", Qt::CaseInsensitive ))
      .arg(t1)
      .arg(t2)
      .arg(a1)
      .arg(a2)
      .arg(ii)
      .arg(cccc)
      .arg(yygggg);
}

QString FaxBuilder::buildObjectName()
{
  return QObject::tr("Факсимильная карта");
}


