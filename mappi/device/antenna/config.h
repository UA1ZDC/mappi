#pragma once

#include "profile.h"
#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/coords.h>


namespace mappi {

namespace antenna {

class Configuration
{
public :
  struct serialport_t {
    QString tty;
    int baudRate;
    float txTimeout;
    bool DTR;
    bool RTS;
  };

public :
  Configuration();
  ~Configuration();

  bool load(const QString& filePath);
  bool isValid() const { return isValid_; }
  QString toString() const;

public :
  bool demoMode;

  QString siteName;
  meteo::GeoPoint point;
  Coords::GeoCoord coord;

  serialport_t serialPort;
  Profile profile;
  float azimutCorrect;

private :
  bool isValid_;
};

}

}
