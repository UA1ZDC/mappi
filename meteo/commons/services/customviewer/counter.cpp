#include "counter.h"

#include <meteo/commons/global/global.h>


namespace meteo {

qint64 countTelegrams(const QList<Condition> &conditions)
{
  Q_UNUSED(conditions);
  meteo::proto::ViewerConfig out;

  auto queryName = QObject::tr("count_all_msg");
  auto prop = global::mongodbConfTelegram();

  std::unique_ptr<Dbi> db(global::db(prop));
  if ( nullptr == db ){
    error_log << msglog::kDbConnectFailed;
    return 0;
  }
  std::unique_ptr<DbiQuery> query(db->queryByName(queryName));
  if ( nullptr ==query ){
    error_log << msglog::kDbRequestNotFound.arg(queryName);
    return 0;
  }

  if ( false == query->exec() ){
    error_log << msglog::kDbRequestFailed.arg(query->query());
    return 0;
  }

  const DbiEntry& result = query->result();
  bool status;
  qint64 count = result.valueInt32("n", &status);
  if ( false == status ){
    count = result.valueInt64("n", &status);
    return 0;
  }
  return count;
}

qint64 countJounral(const QList<Condition>& conditions)
{
  Q_UNUSED(conditions);

  ConnectProp prop = global::mongodbConfJournal();
  QString queryName = QObject::tr("count_journal");

  std::unique_ptr<Dbi> db(global::db(prop));

  if ( nullptr == db ){
    error_log << msglog::kDbConnectFailed;
    return 0;
  }

  std::unique_ptr<DbiQuery> query(db->queryByName(queryName));
  if ( false == query->exec() ){
    error_log << msglog::kDbRequestFailed.arg(query->query());
    return 0;
  }

  const DbiEntry& result = query->result();
  bool status;
  qint64 count = result.valueInt32("n", &status);

  if ( false == status ){
    count = result.valueInt64("n", &status);
    return 0;
  }
  return count;
}

qint64 countForecast(const QList<Condition>& conditions)
{
  Q_UNUSED(conditions);

  ConnectProp prop = global::mongodbConfMeteo();
  QString queryName = QObject::tr("count_forecast");

  std::unique_ptr<Dbi> db(global::db(prop));

  if ( nullptr == db ){
    error_log << msglog::kDbConnectFailed;
    return 0;
  }

  std::unique_ptr<DbiQuery> query(db->queryByName(queryName));
  if ( false == query->exec() ){
    error_log << msglog::kDbRequestFailed.arg(query->query());
    return 0;
  }

  const DbiEntry& result = query->result();
  bool status;
  qint64 count = result.valueInt32("n", &status);

  if ( false == status ){
    count = result.valueInt64("n", &status);
    return 0;
  }
  return count;
}

qint64 countForecastAccuracy(const QList<Condition>& conditions)
{
  Q_UNUSED(conditions);

  ConnectProp prop = global::mongodbConfMeteo();
  QString queryName = QObject::tr("count_forecast_accuracy");

  std::unique_ptr<Dbi> db(global::db(prop));

  if ( nullptr == db ){
    error_log << msglog::kDbConnectFailed;
    return 0;
  }

  std::unique_ptr<DbiQuery> query(db->queryByName(queryName));
  if ( false == query->exec() ){
    error_log << msglog::kDbRequestFailed.arg(query->query());
    return 0;
  }

  const DbiEntry& result = query->result();
  bool status;
  qint64 count = result.valueInt32("n", &status);

  if ( false == status ){
    count = result.valueInt64("n", &status);
    return 0;
  }
  return count;
}

qint64 countSendMsg(const QList<Condition>& conditions)
{
  Q_UNUSED(conditions);

  auto prop = global::mongodbConfTelegram();
  QString queryName = QObject::tr("count_all_sended_tlg");

  std::unique_ptr<Dbi> db(global::db(prop));

  if ( nullptr == db ){
    error_log << msglog::kDbConnectFailed;
    return 0;
  }

  std::unique_ptr<DbiQuery> query(db->queryByName(queryName));
  query->arg("param", global::hmsId());

  if ( false == query->exec() ){
    error_log << msglog::kDbRequestFailed.arg(query->query());
    return 0;
  }

  const DbiEntry& result = query->result();
  bool status;
  qint64 count = result.valueInt32("n", &status);

  if ( false == status ){
    count = result.valueInt64("n", &status);
    return 0;
  }
  return count;
}

qint64 countRecvMsg(const QList<Condition>& conditions)
{
  Q_UNUSED(conditions);

  auto prop = global::mongodbConfTelegram();
  QString queryName = QObject::tr("count_all_recv_tlg");

  std::unique_ptr<Dbi> db(global::db(prop));

  if ( nullptr == db ){
    error_log << msglog::kDbConnectFailed;
    return 0;
  }

  std::unique_ptr<DbiQuery> query(db->queryByName(queryName));
  query->arg("param", global::hmsId());
  if ( false == query->exec() ){
    error_log << msglog::kDbRequestFailed.arg(query->query());
    return 0;
  }

  const DbiEntry& result = query->result();
  bool status;
  qint64 count = result.valueInt32("n", &status);

  if ( false == status ){
    count = result.valueInt64("n", &status);
    return 0;
  }
  return count;
}



}

