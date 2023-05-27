#pragma once

#include <commons/geobasis/coords.h>
#include <mappi/proto/reception.pb.h>


namespace mappi {

namespace schedule {

class Configuration
{
public :
  Configuration();
  ~Configuration();

  bool load(const QString& filePath);
  bool isValid() const { return isValid_; }
  QString toString() const;

public :
  conf::ConflResol conflResol;
  unsigned int period;          // период на который составлено расписание (часы)
  QStringList satellite;        // список спутников
  conf::Elevation elevation;
  QString tlePath;              // путь к телеграмме
  meteo::GeoPoint point;
  Coords::GeoCoord coord;       // координаты пункта приёма

private :
  bool isValid_;
};

}

}
