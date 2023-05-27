#pragma once

#include <mappi/proto/schedule.pb.h>
#include <qdatetime.h>


namespace mappi {

namespace schedule {

class SessionData
{
public :
  static QString dateToString(const QDateTime& dt);
  static QDateTime dateFromString(const QString& str);
  static QString info(const QString& satellite, unsigned long revol);

public :
  SessionData();
  ~SessionData();

  bool isDefault() const;
  void setDefault();

  QString toString(bool isPretty = false) const;
  bool fromString(const QString& str);

  QString info() const;
  QString fileName() const;

public :
  bool operator ==(const SessionData& other) const;
  bool operator <(const SessionData& other) const;
  bool operator !=(const SessionData& other) const;

private :
  QString directionName() const;
  QString conflStateName() const;

public :
  QString satellite;                  // имя спутника
  QDateTime aos;                      // момент времени, когда сигнал будет получен
  QDateTime los;                      // момент времени, когда сигнал будет потерян
  double elevatMax;                   // момент кульминации (максимальный угол места)
  unsigned long revol;                // номер витка на момент времени aos
  conf::SatDirection direction;       // направление спутника (-1 - неизвестно,0 - C->Ю, 1 - Ю->С)
  conf::ConflState conflState;        // состояние конфликта
};

}

}
