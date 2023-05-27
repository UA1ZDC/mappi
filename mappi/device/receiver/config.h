#pragma once

#include "parameters.h"
#include <commons/geobasis/coords.h>
#include <qhostaddress.h>


namespace mappi {

namespace receiver {

class Configuration
{
public :
  Configuration();
  ~Configuration();

  bool load(const QString& filePath);
  bool isValid() const { return isValid_; }
  QString toString() const;

  bool hasParameters(const QString& satellite) const;
  Parameters parameters(const QString& satellite) const;

public :
  bool demoMode;

  QString siteName;
  meteo::GeoPoint point;
  Coords::GeoCoord coord;

  conf::RateMode mode;
  conf::ReceiverEngine engine;
  bool apch;
  QString appName;
  QHostAddress host;
  quint16 port;

  QString sessionDir;
  QString dumpDir;
  QString rawFile;

private :
  bool isValid_;

  Parameters::pool_t param_;
  static Parameters default_;
};

}

}
