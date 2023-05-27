#pragma once

// #include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <mappi/proto/reception.pb.h>

#define CONF_FILE MnCommon::etcPath("mappi") + "reception.conf"


namespace mappi {

namespace pretreatment {

class Configuration
{
public :
  Configuration();
  ~Configuration();

  bool load(const QString& filePath);

  QString toString() const;
  bool isValid() const { return isValid_; }

public :
  QString dataDir;
  QString sessionDir;

private :
  bool isValid_;
};

}

}
