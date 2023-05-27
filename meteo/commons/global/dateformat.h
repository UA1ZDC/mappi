#ifndef METEO_COMMONS_GLOBAL_DATEFORMAT_H
#define METEO_COMMONS_GLOBAL_DATEFORMAT_H
#include <qdatetime.h>
#include <qstring.h>
#include <qobject.h>

namespace meteo {

const QString dtHumanFormatFull = QObject::tr("dd.MM.yyyy hh:mm:ss");
const QString dtHumanFormatTimeShort = QObject::tr("dd.MM.yyyy hh:mm");
const QString dtHumanFormatDateOnly = QObject::tr("dd.MM.yyyy");
const QString dtHumanFormatTimeOnly = QObject::tr("hh:mm");
const QString dtISODateWithoutZ = QObject::tr("yyyy-MM-ddThh:mm:ss");

/**
 * @brief dateToHumanFull - полный формат даты
 * @param dt - дата
 * @return
 */
QString dateToHumanFull(const QDateTime& dt);

/**
 * @brief dateToHumanTimeShort - дата, часы, минуты(без секунд)
 * @param dt
 * @return
 */
QString dateToHumanTimeShort(const QDateTime& dt);

/**
 * @brief dateToHumanDateOnly - дата без времени
 * @param dt
 * @return
 */
QString dateToHumanDateOnly(const QDateTime& dt);
QString dateToHumanDateOnly(const QDate& date);

QString dateToISODateWithoutZ(const QDateTime& date);
QString dateToISODateWithoutZ(const QDate& date);

}

#endif
