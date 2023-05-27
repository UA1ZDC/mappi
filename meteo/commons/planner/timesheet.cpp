#include "timesheet.h"

#include <cross-commons/debug/tlog.h>

#include <QRegExp>
#include <QDateTime>
#include <QStringList>


namespace meteo {

bool Timesheet::setMinute(const QString& minute)
{
  clearCache();
  data_[kMinute] = processField(minute, 0, 59);
  format_[kMinute] = parseFormat(data_[kMinute]);
  return !data_[kMinute].isNull();
}

bool Timesheet::setHour(const QString& hour)
{
  clearCache();
  data_[kHour] = processField(hour, 0, 23);
  format_[kHour] = parseFormat(data_[kHour]);
  return !data_[kHour].isNull();
}

bool Timesheet::setDay(const QString& day)
{
  clearCache();
  data_[kDay] = processField(day, 1, 31);
  format_[kDay] = parseFormat(data_[kDay]);
  return !data_[kDay].isNull();
}

bool Timesheet::setMonth(const QString& month)
{
  clearCache();
  data_[kMonth] = processField(month, 1, 12);
  format_[kMonth] = parseFormat(data_[kMonth]);
  return !data_[kMonth].isNull();
}

bool Timesheet::setDayOfWeek(const QString& dayOfWeek)
{
  clearCache();
  data_[kDayOfWeek] = processField(dayOfWeek, 1, 7);
  format_[kDayOfWeek] = parseFormat(data_[kDayOfWeek]);
  return !data_[kDayOfWeek].isNull();
}

QString Timesheet::minute() const
{
  return data_[kMinute];
}

QString Timesheet::hour() const
{
  return data_[kHour];
}

QString Timesheet::day() const
{
  return data_[kDay];
}

QString Timesheet::month() const
{
  return data_[kMonth];
}

QString Timesheet::dayOfWeek() const
{
  return data_[kDayOfWeek];
}

bool Timesheet::isValid() const
{
  return !data_[kMinute].isEmpty() && !data_[kHour].isEmpty() && !data_[kDay].isEmpty() && !data_[kMonth].isEmpty() && !data_[kDayOfWeek].isEmpty();
}

bool Timesheet::match(const QDateTime& dateTime) const
{
  if ( !isValid() || !dateTime.isValid() ) { return false; }

  if ( !matchSection(kMinute, dateTime) ||
       !matchSection(kHour, dateTime) ||
       !matchSection(kDay, dateTime) ||
       !matchSection(kMonth, dateTime) ||
       !matchSection(kDayOfWeek, dateTime)
       )
  {
    return false;
  }

  return true;
}

QString Timesheet::toString() const
{
  return !isValid() ? QString() : data_[kMinute] + ' ' + data_[kHour] + ' ' + data_[kDay] + ' ' + data_[kMonth] + ' ' + data_[kDayOfWeek];
}

QDateTime Timesheet::nextRun(const QDateTime& from) const
{
  if ( !isValid() || !from.isValid() ) { return QDateTime(); }

  if ( from >= nextRunCache_.first && from <= nextRunCache_.second ) {
    return nextRunCache_.second;
  }

  QDateTime dt = from;

  while ( !match(dt) ) {
    dt = dt.addSecs(60);
  }

  nextRunCache_.first  = from;
  nextRunCache_.second = dt;

  return dt;
}

QDateTime Timesheet::prevRun(const QDateTime& from) const
{
  if ( !isValid() || !from.isValid() ) { return QDateTime(); }

  QDateTime dt = from;

  dt = dt.addSecs(-60);

  while ( !match(dt) ) {
    dt = dt.addSecs(-60);
  }

  return dt;
}

Timesheet Timesheet::fromString(const QString& timesheet)
{
  QStringList fields = timesheet.split(QRegExp("[\t ]+"), QString::SkipEmptyParts);
  if ( fields.size() < 5 ) {
    return Timesheet();
  }

  Timesheet t;
  t.setMinute(fields.at(0));
  t.setHour(fields.at(1));
  t.setDay(fields.at(2));
  t.setMonth(fields.at(3));
  t.setDayOfWeek(fields.at(4));

  return t;
}

QString Timesheet::processField(const QString& value, int min, int max) const
{
  QString v = value.trimmed();

  FieldFormat format = parseFormat(v);
  switch ( format ) {
    case kAny: {} break;
    case kValue: {
      if ( !checkValue(v, min, max) ) {
        return QString();
      }
    } break;
    case kRage: {
      if ( !checkRange(v.section("-", 0, 0), v.section("-", 1, 1), min, max) ) {
        return QString();
      }
    } break;
    case kList: {
      if ( !checkList(v.split(","), min, max) ) {
        return QString();
      }
    } break;
    case kDivisible: {
      int val = v.section("/", 1, 1).toInt();
      if ( val < 0 ) {
        return QString();
      }
    } break;
    case kInvalid: {
      return QString();
    }
  }

  return v;
}

Timesheet::FieldFormat Timesheet::parseFormat(const QString& value) const
{
  if ( "*" == value ) {
    return kAny;
  }
  else if ( QRegExp("\\d+").exactMatch(value) ) {
    return kValue;
  }
  else if ( QRegExp("\\d+[-]\\d+").exactMatch(value) ) {
    return kRage;
  }
  else if ( QRegExp("(\\d+[,]?)+").exactMatch(value) ) {
    return kList;
  }
  else if ( QRegExp("[*][/]\\d+").exactMatch(value) ) {
    return kDivisible;
  }

  return kInvalid;
}

bool Timesheet::checkValue(const QString& value, int min, int max) const
{
  bool ok = false;
  int v = value.toInt(&ok);

  if ( !ok || v < min || v > max ) {
    return false;
  }

  return true;
}

bool Timesheet::checkRange(const QString& value1, const QString& value2, int min, int max) const
{
  bool ok1 = false;
  int v1 = value1.toInt(&ok1);

  bool ok2 = false;
  int v2 = value2.toInt(&ok2);

  if ( !ok1 || !ok2 || v1 < min || v1 > max || v2 < min || v2 > max || v1 > v2 ) {
    return false;
  }

  return true;
}

bool Timesheet::checkList(const QStringList& values, int min, int max) const
{
  foreach ( const QString& value, values ) {
    bool ok = false;
    int v = value.toInt(&ok);
    if ( !ok || v < min || v > max ) {
      return false;
    }
  }

  return true;
}

bool Timesheet::matchSection(Timesheet::Section section, const QDateTime& dt) const
{
  int v = -1;
  switch ( section ) {
    case kMinute: v = dt.time().minute(); break;
    case kHour:   v = dt.time().hour(); break;
    case kDay:    v = dt.date().day(); break;
    case kMonth:  v = dt.date().month(); break;
    case kDayOfWeek: v = dt.date().dayOfWeek(); break;
  }
  return matchValue(data_[section], format_[section], v);
}

bool Timesheet::matchValue(const QString& fieldValue, FieldFormat format, int value) const
{
  switch ( format ) {
    case kAny: {} break;
    case kValue: {
      if ( fieldValue.toInt() != value ) { return false; }
    } break;
    case kRage: {
      int from = fieldValue.section("-", 0, 0).toInt();
      int to = fieldValue.section("-", 1, 1).toInt();
      if ( value < from || value > to ) { return false; }
    } break;
    case kList: {
      bool ok = false;
      QStringList list = fieldValue.split(",");
      foreach ( const QString& item, list ) {
        if ( item.toInt() == value ) {
          ok = true;
        }
      }
      if ( !ok ) { return false; }
    } break;
    case kDivisible: {
      int div = fieldValue.section("/", 1, 1).toInt();
      int v = value % div;
      if ( 0 != v ) { return false; }
    } break;
    case kInvalid: {
      return false;
    } break;
  }

  return true;
}

void Timesheet::clearCache()
{
  nextRunCache_.first = QDateTime();
  nextRunCache_.second = QDateTime();
}

} // meteo
