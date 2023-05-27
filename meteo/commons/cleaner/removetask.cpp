#include "removetask.h"
#include "param.h"

#include <sql/nosql/nosqlquery.h>
#include <meteo/commons/global/global.h>
#include <qdiriterator.h>
#include <qelapsedtimer.h>


namespace meteo {

namespace cleaner {

RemoveTask::RemoveTask(QObject* parent)
  : PlannerTask(parent) {
}

void RemoveTask::setOptions(const FileDbRemove& options)
{
  opt_ = options;
}

void RemoveTask::run()
{
  const ConnectProp param = Global::instance()->dbConf(QString::fromStdString(opt_.conn_name()));
  if (param.driver() == settings::kMongo) {
    jobRemoveDocsGridFsMongodb();
  } else {
    jobRemove();
  }
}

static void removeGridFsFileById(GridFs* gridfs, const QString& fileId)
{
  GridFile file;
  file = gridfs->findOneById(fileId);

  if ( false == file.isValid() ) {
    error_log << gridfs->lastError();
    return;
  }

  if ( true == file.hasFile() ) {
    file.remove();
  }
  else {
    warning_log << QObject::tr("Файл '%1'( id = %i ) не найден")
                   .arg(file.fileName())
                   .arg(file.id());
  }
  return;
}

static void removeGridFsFileByName(GridFs* gridfs, const QString& fileName)
{
  GridFile file;
  file = gridfs->findOneByName(fileName);

  if ( false == file.isValid() ) {
    error_log << gridfs->lastError();
    return;
  }

  if ( true == file.hasFile() ) {
    file.remove();
  }
  else {
    warning_log << QObject::tr("Файл '%1'( id = %i ) не найден")
                   .arg(file.fileName())
                   .arg(file.id());
  }
  return;
}

void RemoveTask::jobRemoveDocsGridFsMongodb()
 {
   const QString kGridPrefix = QString::fromStdString(opt_.gridfs_prefix());
   const QString kCollectionName = QString::fromStdString(opt_.collection());
   const ConnectProp param = meteo::Global::instance()->dbConf(QString::fromStdString(opt_.conn_name()));

   std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
   if ( nullptr == db.get() ) {
     error_log << msglog::kDbConnectFailed.arg(param.name());
     return;
   }

   GridFs gridfs;
   if ( false == gridfs.connect(param)) {
     error_log << gridfs.lastError();
     return;
   }
   gridfs.use( param.name(),kGridPrefix);

   QString dtField = QString::fromStdString(opt_.dt_field());
   if ( true == dtField.isEmpty() ) {
     error_log << tr("Не задан параметр dt_field");
     return;
   }

   if ( false == opt_.has_gridfs_field() && false == opt_.has_gridfs_path() ){
     error_log << QObject::tr("Необходимо задать путь к идентификатору файла в GRIDFS");
     return;
   }

   bool rmById = opt_.has_gridfs_field();

   QString gridField =  ( true == rmById) ? QString::fromStdString(opt_.gridfs_field())
                                          : QString::fromStdString(opt_.gridfs_path());


   if ( gridField.isEmpty() ) {
     error_log << tr("Необходимо указать параметр gridfs_field или gridfs_path");
     return;
   }
   QDateTime currentDateTime = QDateTime::currentDateTimeUtc();

   QString dtWriteField;
   QDateTime storeDt;
   if ( true == opt_.has_store_period() && true == opt_.has_dt_write_field()  ) {
     dtWriteField = QString::fromStdString(opt_.dt_write_field());
     storeDt = currentDateTime.addDays( -opt_.store_period() );
   }

   QDateTime enddt = currentDateTime.addDays( -opt_.delete_period() );

   static auto queryName = QObject::tr("cleaner_remove_task_get_documents");
   auto query = db->queryptrByName(queryName);
   if(nullptr == query) {
     error_log << msglog::kDbRequestNotFound.arg(queryName);
     return;
   }

   query->arg("collection", kCollectionName);
   query->arg("dtField", dtField);
   query->arg("enddt", enddt);
   query->arg("gridField", gridField);

   if ( true == storeDt.isValid() && false == dtWriteField.isEmpty() ) {
     query->arg("dtWriteField", dtWriteField);
     query->arg("storeDt", storeDt);
   }

   QString err;
   if ( false == query->execInit(&err)){
     error_log << msglog::kDbRequestFailed << err;
     return;
   }

   QElapsedTimer timer;
   timer.start();

   int n = 0;
   while ( true == query->next() ) {
     const meteo::DbiEntry& doc = query->entry();

     QString oid; // для ObjectId
     QString sid; // для String
     int64_t iid = -1; // для Int

     bson_type_t type = doc.typeField("_id");
     switch ( type ) {
       case BSON_TYPE_OID:
         oid = doc.valueOid("_id");
         break;
       case BSON_TYPE_INT64:
         iid = doc.valueInt64("_id");
         break;
       case BSON_TYPE_UTF8:
         sid = doc.valueString("_id");
         break;
       default:
         error_log << tr("Удаление доскументов с типом идентификатора %1 не поддерживается").arg(type);
         continue;
     }

     QString fileKey;
     type = doc.typeField(gridField);
     switch ( type ) {
       case BSON_TYPE_OID:
         fileKey = doc.valueOid(gridField);
         break;
       case BSON_TYPE_UTF8:
         fileKey = doc.valueString(gridField);
         break;
       default:
         error_log << tr("Удаление файлов с типом идентификатора %1 не поддерживается").arg(type);
         continue;
     }

     QString q;
     if ( rmById ) {
       removeGridFsFileById(&gridfs, fileKey);
     }
     else {
       removeGridFsFileByName(&gridfs, fileKey);
     }

     QString deleteQName = QObject::tr("rm_any_by_id");
     auto deleteQ = db->queryptrByName("rm_any_by_id");
     if(nullptr == deleteQ) {
       return;
     }

     deleteQ->arg("collection",kCollectionName);
     if ( -1 != iid ) {
       QList<qint64> ids = { iid };
       deleteQ->arg("id",ids);
     }
     else if ( !oid.isEmpty() ) {
       deleteQ->argOid("id",QStringList(oid));
     }
     else if ( !sid.isEmpty() ) {
       deleteQ->arg("id",QStringList(sid));
     }

     if ( !deleteQ->exec() ){
       error_log << QObject::tr("Не удалось выполнить запрос = %1")
         .arg( deleteQ->query() );
       return;
     }

     ++n;
     debug_log_if( (n%1000) == 0 ) << tr("УДАЛЕНО %1 за %2 мсек.").arg(1000, 4, 10, QChar(' ')).arg(timer.restart());
   }
   debug_log_if( (n%1000) != 0 ) << tr("УДАЛЕНО %1 за %2 мсек.").arg((n%1000), 4, 10, QChar(' ')).arg(timer.restart());
 }

void RemoveTask::jobRemove()
{
  std::time_t ts_start = std::time(nullptr);

  if (opt_.has_gridfs_prefix() && !opt_.has_gridfs_field()) {
    error_log << QObject::tr("Ошибка, отсутствует описание поля gridfs_field");
    return;
  }

  Param param(opt_);
  ConnectProp prop = global::dbConf(param.dbName());
  std::unique_ptr<Dbi> db(global::db(prop));
  if (db == nullptr) {
    error_log << msglog::kDbConnectFailed.arg(prop.name());
    return;
  }

  auto query = db->queryptrByName("cleaner_remove_documents_by_dt");
  if (query == nullptr) {
    return;
  }

  query->arg("msgTable", param.msgTable());
  query->arg("msgFsField", param.msgFsField());
  query->arg("dtField", param.dtField());
  query->arg("dt", param.dt());
  query->arg("dtWriteField", param.dtWriteField());
  query->arg("dtWrite", param.dtWrite());
  query->arg("fsTable", param.fsTable());
  query->arg("fsField", param.fsField());
  if (!query->exec()) {
    error_log << meteo::msglog::kDbRequestFailed;
    return;
  }

  const meteo::DbiEntry& result = query->result();
  int64_t fs_rec_count = result.valueInt64("fs_rec_count");
  int64_t fs_rm_count = 0;

  debug_log << "param.isFsTable() = " << param.isFsTable();
  debug_log << "fs_rec_count = " << fs_rec_count;

  if (param.isFsTable() && 0 < fs_rec_count) {
    if (param.isCompress()) {
      int64_t fs_rm_arch = flushExpired(param.compressPath(), param.dt());
      info_log_if(0 < fs_rm_arch) << QObject::tr("Удалено архивов: %1").arg(fs_rm_arch);

      ArchUtil archUtil;
      fs_rm_count = archUtil.storageRotate(param);
    } else
      fs_rm_count = flushExpired(param.fsPath(), param.dt());
  }

  info_log << QObject::tr("Удалено файлов (db;fs): (%1;%2), затраченно времени: %3 сек")
                .arg(fs_rec_count)
                .arg(fs_rm_count)
                .arg(std::time(nullptr) - ts_start);
}

int64_t RemoveTask::flushExpired(const QString& root, const QDateTime& expired)
{
  // отладка выводит первых 100 удаляемых файлов
  // debug_log << "root = " << root << " expired = " << expired;
  // int i = 100;

  int64_t res = 0;
  QDirIterator it(root, (QDir::AllEntries | QDir::NoDotAndDotDot), QDirIterator::Subdirectories);
  while (it.hasNext()) {
    QString path = it.next();
    QFileInfo info(path);
    // if (0 < i) {
    //   debug_log << path;
    //   --i;
    // }

    if (info.isFile() && (info.birthTime() < expired)) {
      QFile::remove(path);
      ++res;
      continue;
    }

    if (info.isDir()) {
      QDir dir(path);
      if (dir.isEmpty())
        dir.rmdir(path);
    }
  }

  return res;
}

int64_t RemoveTask::flushExpired(const QDir& dir, const QDateTime& expired)
{
  int64_t res = 0;
  QStringListIterator it(dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot));
  while (it.hasNext()) {
    QFileInfo info(dir.absoluteFilePath(it.next()));
    QString path = info.filePath();
    if (info.isFile() && (info.birthTime() < expired)) {
      QFile::remove(path);
      ++res;
      continue;
    }

    if (info.isDir()) {
      QDir subDir(path);
      if (subDir.isEmpty())
        dir.rmdir(path);
      else
        res += flushExpired(subDir, expired);
    }
  }

  return res;
}

}

}
