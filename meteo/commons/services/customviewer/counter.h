#ifndef COUNTER_H
#define COUNTER_H

#include <qobject.h>
#include <meteo/commons/ui/conditionmaker/conditiongeneric.h>

namespace meteo {

qint64 countTelegrams(const QList<Condition>& conditions);
qint64 countJounral(const QList<Condition>& conditions);
qint64 countForecast(const QList<Condition>& conditions);
qint64 countForecastAccuracy(const QList<Condition>& conditions);

qint64 countSendMsg(const QList<Condition>& conditions);
qint64 countRecvMsg(const QList<Condition>& conditions);


}

#endif
