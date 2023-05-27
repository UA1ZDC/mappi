#include "dateformat.h"

namespace meteo {

QString dateToHumanFull(const QDateTime& dt)
{
  return dt.toString(meteo::dtHumanFormatFull);
}

QString dateToHumanTimeShort(const QDateTime& dt)
{
  return dt.toString(meteo::dtHumanFormatTimeShort);
}

QString dateToHumanDateOnly(const QDateTime& dt)
{
  return dt.toString(meteo::dtHumanFormatDateOnly);
}

QString dateToHumanDateOnly(const QDate& date)
{
  return date.toString(meteo::dtHumanFormatDateOnly);
}

QString dateToISODateWithoutZ(const QDateTime& dt)
{
  return dt.toString(meteo::dtISODateWithoutZ);
}

QString dateToISODateWithoutZ(const QDate& date)
{
  return date.toString(meteo::dtISODateWithoutZ);
}
}
