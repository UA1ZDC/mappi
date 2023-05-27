#pragma once

#include "antenna.h"
#include <QtDebug>


namespace mappi {

namespace antenna {

/**
 * Сценарий (скрипт) позиционирования антенны.
 * Новое положение антенны формируетcя с определенной периодичностью заданной параметром time_slot.
 * Скрипт может работать бесконечно или завершиться самостоятельно по внутреннему условию либо быть прерванным.
 *
 * Основной метод скрипта - exec. Для завершения работы скрипта метод должен вернуть false (выход по внутреннему условию).
 * Интерфейс сценария реализован так, что запускать его каждый раз можно с разной антенной.
 *
 * Походу работы скрипта собирается следующая статистика:
 * - количество итерации скрипта;
 * - среднее и максимальное отклонение по времени (мсек.) относительно time_slot между итерациями.
 *
 * Статистика доступна за последний запуск скрипта, метод trace.
 */
class Script :
  public QObject
{
  Q_OBJECT
public :
  enum id_t {
    CATCHER = 1,
    TRACKER,
    DIRECT,
    MONKEY_TEST
  };
  Q_ENUM(id_t)

public :
  explicit Script(QObject* parent = nullptr);
  virtual ~Script();

  virtual id_t id() const = 0;
  virtual bool exec(Antenna* antenna) = 0;

  virtual bool run(Antenna* antenna);
  virtual void terminate();     // остановка по требованию

  bool isRunning() const;
  QString trace();

signals :
  void finished();      // завершение по внутреннему условию

protected :
  virtual void timerEvent(QTimerEvent* event);

  QDateTime now() const;
  QString toString(const QDateTime& timeStamp);
  QString toString(const QDateTime& timeStamp, float azimut, float elevat) const;
  QString toString(float azimut, float elevat) const;

  QString toString(const QDateTime& timeStamp, const Drive& azimut, const Drive& elevat) const;
  QString toString(const Drive& azimut, const Drive& elevat) const;

protected :
  // текущее положение антенны
  Drive azimut;
  Drive elevat;

private :
  int timerId_;
  int timeSlot_;        // msec
  Antenna* antenna_;

  // статистика
  int calls_;           // количество итераций (вызовов exec)
  int max_;             // максимальное отклонение в мсек.
  int average_;         // среднее отклонение в мсек.
};

}

}
