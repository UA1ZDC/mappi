#include "tfaxdb.h"
#include <qstring.h>
#include <qiodevice.h>
#include <qfile.h>

#include <sql/nosql/nosqlquery.h>
#include <meteo/commons/global/global.h>
#include <sql/nosql/document.h>
#include <sql/dbi/gridfs.h>

namespace meteo {
namespace tfaxdb {

static const QString prefixName = "fax";

bool getAllFaxes(const QDateTime& dtStart, const QDateTime &dtEnd, meteo::map::proto::FaxReply *reply)
{
  std::unique_ptr<Dbi> db(meteo::global::dbDocument());
  if ( nullptr == db.get() ) {
      error_log<<QObject::tr("Не удалось подключиться к БД");
      return false;
    }
  auto query = db->queryptrByName("find_fax_by_dt");
  if(nullptr == query) {
      error_log<< QObject::tr("Не найдена функция для получения данных");
      return false;
    }
  query->arg("start_dt",dtStart);
  query->arg("end_dt",dtEnd);

  QString error;
  if(false == query->execInit( &error)){
      //error_log << error;
      return false;
    }

  while ( true == query->next() ) {
    const ::meteo::DbiEntry& doc = query->entry();
    auto newFax = reply->add_faxes();
    QString id = doc.valueOid("_id");
    QString t1 = doc.valueString("t1");
    QString t2 = doc.valueString("t2");
    QString a1 = doc.valueString("a1");
    QString a2 = doc.valueString("a2");
    int ii = doc.valueInt32("ii");
    QString cccc = doc.valueString("cccc");
    QString yygggg = doc.valueString("yygggg");
    QDateTime dt = doc.valueDt("dt");

    newFax->set_id(id.toStdString());
    newFax->set_a1(a1.toStdString());
    newFax->set_a2(a2.toStdString());
    newFax->set_t1(t1.toStdString());
    newFax->set_t2(t2.toStdString());
    newFax->set_ii(ii);
    newFax->set_cccc(cccc.toStdString());
    newFax->set_yygggg(yygggg.toStdString());
    newFax->set_dt_start(dt.toString(Qt::ISODate).toStdString());
  }
  return true;
}

bool getFaxById(const QString& id, QByteArray *array){
  if ( nullptr == array ){
    error_log << QObject::tr("Ошибка: использовать null в качества параметра недопустимо!");
    return false;
  }
    std::unique_ptr<Dbi> db(meteo::global::dbDocument());
  if ( nullptr == db.get() ) {
      error_log<<QObject::tr("Не удалось подключиться к БД");
      return false;
    }
  auto query = db->queryptrByName("find_fax_by_id");
  if(nullptr == query) {
      error_log<< QObject::tr("Не найдена функция для получения данных");
      return false;
    }
  query->arg("id",id);

  QString error;
  if(false == query->execInit( &error)){
      //error_log << error;
      return false;
    }
  int dataCount = 0;
  QByteArray result;
  auto conf = meteo::global::mongodbConfDocument();
    GridFs gridfs;
    if ( false == gridfs.connect( conf ) ) {
      error_log << gridfs.lastError();
      return false;
    }
    gridfs.use(conf.name(),prefixName);

  while ( true == query->next() ) {
    ++dataCount;
    auto fileName = query->entry().valueString("image_path");
    QString fileNameQuery = QString("{ \"filename\": \"%1\" }").arg(fileName);
    GridFile file = gridfs.findOne(fileNameQuery);
    if ( false == file.isValid() ) {
      error_log << gridfs.lastError();
      return false;
    }
    if ( false == file.hasFile() ) {
      error_log << meteo::msglog::kFileNotFound.arg(fileName);
      return false;
    }
    else {
      bool ok = false;
      result = file.readAll(&ok);
      if ( false == ok ) {
        error_log << file.lastError();
        return false;
      }
    }
  }
  if (dataCount != 1) {
    error_log << QObject::tr("Ошибка: ожидалось получить 1 набор данных, получено %1").arg(dataCount);
    return false;
  }
  *array = result;
  return  true;
}


}
}
