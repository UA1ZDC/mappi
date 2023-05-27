#include "sateliteimagebuilder.h"

#include <meteo/commons/faxes/datachain.h>
#include <sql/nosql/nosqlquery.h>
#include <sql/dbi/gridfs.h>
#include <qcryptographichash.h>
#include <meteo/commons/global/dbnames.h>

SateliteImageBuilder::SateliteImageBuilder()
{
  setObjectName(QObject::tr("Сборщик спутниковых изображений"));
}

SateliteImageBuilder::~SateliteImageBuilder()
{

}

QStringList SateliteImageBuilder::subscribeMsgTypes()
{
  return { QObject::tr("doc") };
}

bool SateliteImageBuilder::saveChain(const ChunkStats& chain, const QByteArray& data)
{
  return saveImage(chain.header_, data);
}

QString SateliteImageBuilder::loaderUid()
{
  return QObject::tr("Satelite image loader");
}

QString SateliteImageBuilder::gridfsPreffix()
{
  return QObject::tr("image_satelite");
}

bool SateliteImageBuilder::saveImage( const meteo::tlg::Header& header,
                                      const QByteArray &data)
{
  QDateTime dt = QDateTime::currentDateTimeUtc();
  QString t1 = QString::fromStdString(header.t1());
  QString t2 = QString::fromStdString(header.t2());
  QString a1 = QString::fromStdString(header.a1());
  QString a2 = QString::fromStdString(header.a2());
  QString cccc = QString::fromStdString(header.cccc());
  QString yygggg = QString::fromStdString(header.yygggg());
  int ii = header.ii();

  QDateTime dtWrite = QDateTime::currentDateTimeUtc();

  meteo::FileBuffer fileBuffer(data);
  QString md5 = QCryptographicHash::hash(fileBuffer.data(), QCryptographicHash::Md5).toHex();

  QString fileName = QObject::tr("%1_%2")
      .arg( fileBuffer.fileName())
      .arg(md5);

  bool isDup = false;
  auto fileid = saveImageToGridFs( fileBuffer.data(), fileName, &isDup );

  if ( true == fileid.isEmpty() ){
    error_log << QObject::tr("Ошибка сохранения файла в gridfs");
    return false;
  }

  QString insertQueryName = QObject::tr("insert_image_satelite");
  if ( false == meteo::global::kMongoQueriesNew.contains(insertQueryName) ){
    error_log << meteo::msglog::kDbRequestNotFound.arg(insertQueryName);
    return false;
  }
  auto db = meteo::global::dbDocument();
  auto  query = db->queryptrByName(insertQueryName);
  if(nullptr == query) {return false;}
  query->arg( "t1", t1 )
      .arg( "t2",t2 )
      .arg( "a1", a1 )
      .arg( "a2", a2 )
      .arg( "ii", ii )
      .arg( "cccc", cccc )
      .arg( "yygggg", yygggg )
      .argOid( "fileid", fileid )
      .arg( "dt", dt )
      .arg( "dt_write", dtWrite );
  bool res = query->exec();
  delete db;

  if ( false == res ){
    error_log << meteo::msglog::kDbRequestFailed.arg( query->query() );
    return false;
  }

  return true;
}


QString SateliteImageBuilder::saveImageToGridFs( const QByteArray& data, const QString& fileName, bool* isDup )
{
  QString fileByNameQuery = QObject::tr("get_grid_file_by_filename");
  if ( false == meteo::global::kMongoQueriesNew.contains(fileByNameQuery) ){
    error_log << meteo::msglog::kDbRequestNotFound.arg(fileByNameQuery);
    return QString();
  }

  meteo::GridFs fs;
  auto conf = meteo::global::mongodbConfDocument();
  if ( false == fs.connect(conf )) {
    error_log << fs.lastError();
    return QString();
  }
  fs.use( meteo::db::kDbDocuments, gridfsPreffix() );
  /*meteo::GridFile file;

  const QString md5summ = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();

  meteo::NosqlQuery query;
  query.setQuery(meteo::global::kMongoQueriesNew[fileByNameQuery]);
  query.arg( "id", fileName);
  auto gridfsFile = fs.findOne(query.query());

  if ( true == gridfsFile.hasFile() && 0 == md5summ.compare(gridfsFile.md5()) ){
    warning_log << QObject::tr("Предупреждение: файл %1 является дубликатом")
                   .arg(fileName);
    *isDup = true;
  }
  else {
    *isDup = false;
  }
*/

  //TODO - добавить проверку на дубликаты через функции gridfs->findByName, gridfs->findById
  *isDup = false;
  meteo::GridFile file;
  fs.put( fileName, data, &file, false );
  return file.id();
}

QString SateliteImageBuilder::buildObjectName()
{
  return QObject::tr("Спутниковое изображение");
}
