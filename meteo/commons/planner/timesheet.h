#ifndef METEO_PLANNER_TIMESHEET_H
#define METEO_PLANNER_TIMESHEET_H

#include <qpair.h>
#include <qstring.h>
#include <qdatetime.h>

namespace meteo {

//! Timesheet описывает периодичность запуска задачи.
class Timesheet
{
public:
  enum Section { kMinute, kHour, kDay, kMonth, kDayOfWeek };

  Timesheet() {}

  bool setMinute(const QString& minute);
  bool setHour(const QString& hour);
  bool setDay(const QString& day);
  bool setMonth(const QString& month);
  bool setDayOfWeek(const QString& dayOfWeek);

  QString minute() const;
  QString hour() const;
  QString day() const;
  QString month() const;
  QString dayOfWeek() const;

  bool isValid() const;
  //! Возвращает true, если время dateTime соответствует шаблону.
  bool match(const QDateTime& dateTime) const;

  QString toString() const;
  QDateTime nextRun(const QDateTime& from = QDateTime::currentDateTimeUtc()) const;
  QDateTime prevRun(const QDateTime& from = QDateTime::currentDateTimeUtc()) const;

  //! Формат timesheet: minute hour day month day-of-week
  //! minute: 0-59
  //! hour: 0-23
  //! day: 1-31
  //! month: 1-12
  //! day-of-week: 1-7 (7 - воскресенье)
  //! Разделитель: табуляция, пробел
  //! Пример: 0 * 1-15 */2 1,2 # каждый час, в ноль минут, с 1 по 15 число, каждого второго месяца, выпадающие
  //!                          # на понедельник и вторник
  static Timesheet fromString(const QString& timesheet);

private:
  enum FieldFormat {
    kInvalid,    //!< Зачение не соответствует формату
    kAny,        //!< Любое значение - '*'
    kValue,      //!< Конкретное значение (цифра)
    kRage,       //!< Диапазон значений (от - до)
    kList,       //!< Список значений (1,3,10)
    kDivisible   //!< Кратное значение (*/3)
  };

  QString processField(const QString& value, int min, int max) const;
  FieldFormat parseFormat(const QString& value) const;

  bool checkValue(const QString& value, int min, int max) const;
  bool checkRange(const QString& value1, const QString& value2, int min, int max) const;
  bool checkList(const QStringList& values, int min, int max) const;

  bool matchSection(Section section, const QDateTime& dt) const;
  bool matchValue(const QString& fieldValue, FieldFormat format, int value) const;

  void clearCache();

private:
  // данные
  QString data_[5];
  FieldFormat format_[5];

  // служебные
  mutable QPair<QDateTime,QDateTime> nextRunCache_; // second - ближайшее время, начиная с first
};

} // meteo

#endif // METEO_PLANNER_TIMESHEET_H
