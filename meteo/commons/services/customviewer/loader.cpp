#include "loader.h"
#include <meteo/commons/global/global.h>

namespace meteo {

static QString generateQuery( const ConnectProp& dbConf,
                              const QString& queryName,
                              const QList<Condition>& conditions,
                              const QString& sortColumn,
                              Qt::SortOrder sortOrder,
                              int skip,
                              int limit,
                              const QString& distinct)
{
  int sortOrderValue = sortOrder == Qt::AscendingOrder? 1:
                                                        -1;
  std::unique_ptr<meteo::DbiQuery> query( global::dbqueryByName(dbConf, queryName) );
  if ( nullptr == query ){
    error_log << meteo::msglog::kDbRequestNotFound.arg(queryName);
    return nullptr;
  }

  query->arg("sort_column", sortColumn);
  query->arg("sort_order", sortOrderValue);
  query->arg("skip", skip);
  query->arg("limit", limit);
  if(false == distinct.isEmpty()){
    query->arg("distinct_on", distinct);
  }

  if ( 0 != conditions.size() ){
    QStringList conditionJsons;
    for ( auto condition: conditions ){
      conditionJsons << condition.getJsonCondition();
    }

    query->argJson("filter",  conditionJsons);
  }
  return query->query();
}

QString queryLoadTelegramsDistinct(const QList<Condition>& conditions, const QString& sortColumn,
                                   Qt::SortOrder sortOrder, int skip, int limit, const QString& distinct)
{
  auto dbConf = global::mongodbConfTelegram();
  QString queryName = QObject::tr("find_from_msg_viewer_distinct");
  return generateQuery(dbConf, queryName, conditions, sortColumn, sortOrder, skip, limit, distinct);
}

QString queryLoadTelegrams(const QList<Condition>& conditions, const QString& sortColumn,
                           Qt::SortOrder sortOrder, int skip, int limit, const QString &distinct)
{
  auto dbConf = global::mongodbConfTelegram();
  QString queryName = QObject::tr("find_from_msg_viewer");
  return generateQuery(dbConf, queryName, conditions, sortColumn, sortOrder, skip, limit, distinct);
}

QString queryLoadJournal(const QList<Condition>& conditions, const QString& sortColumn,
                         Qt::SortOrder order, int skip, int limit, const QString &distinct)
{
  auto dbConf = global::mongodbConfJournal();
  QString queryName = QObject::tr("journal_find_records");
  return generateQuery(dbConf, queryName, conditions, sortColumn, order, skip, limit, distinct);
}


  QString queryLoadForecast(const QList<Condition>& conditions, const QString& sortColumn,
                            Qt::SortOrder order, int skip, int limit, const QString &distinct)
  {
    auto dbConf = global::mongodbConfMeteo();
    QString queryName = QObject::tr("find_forecast_records");
    return generateQuery(dbConf, queryName, conditions, sortColumn, order, skip, limit, distinct);
  }
  
  QString queryLoadForecastAccuracy(const QList<Condition>& conditions, const QString& sortColumn,
                                    Qt::SortOrder order, int skip, int limit, const QString &distinct)
  {
    auto dbConf = global::mongodbConfMeteo();
    QString queryName = QObject::tr("find_forecast_accuracy_records");
    return generateQuery(dbConf, queryName, conditions, sortColumn, order, skip, limit, distinct);
  }
  
}
