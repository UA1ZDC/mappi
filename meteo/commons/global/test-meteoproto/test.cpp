#include "test.h"
#include <sql/nosql/document.h>
#include <sql/nosql/nosqlquery.h>
#include <meteo/commons/global/log.h>

using namespace meteo;

static const auto dbName = "test";
static const auto kCollectionName = "getmeteodatawreptest";

static const auto kHost = "localhost";
static const int kPort = 27017;

static auto kQueryNameDropCollection = QObject::tr("drop_collection");
static auto kQueryNameCountCollection = QObject::tr("count_collection");
static auto kQueryNameFindCollection = QObject::tr("find_collection");


QString GetMeteoDataWrapTest::readFile(const QString& filename)
{
  QFile file(filename);
  if ( false == file.open(QIODevice::ReadOnly) ){
    error_log << msglog::kFileNotFound.arg(filename);
    return QString();
  }

  auto data = file.readAll();
  return QString::fromUtf8(data.data(), data.size());
}

void GetMeteoDataWrapTest::initTestCase()
{
  this->dropCollection(dbName, kCollectionName);
  static const QString& insertRequest = readFile(":/query.json");
  static const QString& data = readFile(":/test.json");

  nosql::NosqlQuery query(insertRequest);
  query.arg(kCollectionName)
      .argJson(data);

  nosql::NoSql db(kHost, kPort);
  QVERIFY(db.connect());
  QVERIFY(db.execQuery(dbName, query.query()));
  debug_log << db.result().jsonExtendedString();
}

int GetMeteoDataWrapTest::countCollection(const QString& kDbName, const QString& collection)
{
  nosql::NosqlQuery query(global::kMongoQueriesNew[kQueryNameCountCollection]);
  query.arg(collection);
  nosql::NoSql db(kHost, kPort);
  if (false == db.connect()){
    return -1;
  }
  if (false == db.execQuery(kDbName, query.query())){
    return -1;
  }
  auto result = db.result();
  bool res = false;
  auto ok = qRound(result.valueDouble("ok", &res));
  if ( false == res ){
    return -1;
  }
  auto n = result.valueInt32("n", &res);
  if ( false == res){
    return -1;
  }
  if ( 1 != ok ){
    return -1;
  }
  return n;
}


bool GetMeteoDataWrapTest::dropCollection(const QString& kDbName, const QString& collectionName)
{
  if ( 0 !=  countCollection(kDbName, collectionName) ){
    return true;
  }
  nosql::NosqlQuery query(meteo::global::kMongoQueriesNew[kQueryNameDropCollection]);
  query.arg(collectionName);

  nosql::NoSql db(kHost, kPort);
  if ( false == db.connect() )
  {
    return false;
  }
  if (false== db.execQuery(kDbName, query.query())){
    return false;
  }
  if ( 0 !=  countCollection(kDbName, collectionName) ){
    return false;
  }
  return true;
}

void GetMeteoDataWrapTest::doMeteodataTest()
{
  QVERIFY(global::kMongoQueriesNew.contains(kQueryNameFindCollection));
  nosql::NosqlQuery query(global::kMongoQueriesNew[kQueryNameFindCollection]);
  query.arg(kCollectionName);

  nosql::NoSql db(kHost, kPort);
  QVERIFY(db.connect());
  QVERIFY(db.execQuery(dbName, query.query()));
  QVERIFY(db.toCursor());

  QVERIFY(db.next());
  auto doc = db.document();

  QTime ttt; ttt.start();
  static const int n = 10000;
  for (int i = 0; i < n; ++i ){
    surf::MeteoDataProto proto;
    global::json2meteoproto(&doc, &proto, true);
  }

  debug_log << "Прошедшее время: " << ttt.elapsed();
  debug_log << "Время на 1 документ: " << (double) ttt.elapsed() / n;
}


QTEST_MAIN(GetMeteoDataWrapTest)
