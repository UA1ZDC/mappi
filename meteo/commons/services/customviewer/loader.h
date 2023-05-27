#ifndef LOADER_H
#define LOADER_H

#include <qhash.h>
#include <qlist.h>
#include <sql/dbi/dbiquery.h>
#include <meteo/commons/ui/conditionmaker/conditiongeneric.h>

//В этом файле описаны функции для загрузки данных из таблиц

namespace meteo {

QString queryLoadTelegramsDistinct(const QList<Condition>& conditions, const QString &sortColumn, Qt::SortOrder order, int skip,
                                   int limit, const QString &distinct);
QString queryLoadTelegrams(const QList<Condition>& conditions, const QString & sortColumn, Qt::SortOrder order, int skip,
                                   int limit, const QString &distinct);

QString queryLoadJournal(const QList<Condition>& conditions, const QString & sortColumn, Qt::SortOrder order, int skip,
                         int limit, const QString &distinct);

QString queryLoadForecast(const QList<Condition>& conditions, const QString & sortColumn, Qt::SortOrder order, int skip,
                          int limit, const QString &distinct);
QString queryLoadForecastAccuracy(const QList<Condition>& conditions, const QString & sortColumn, Qt::SortOrder order, int skip,
                                  int limit, const QString &distinct);

}
#endif // LOADER_H
