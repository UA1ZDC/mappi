#ifndef TGRADACIIDB_H
#define TGRADACIIDB_H

#include <meteo/commons/proto/forecast.pb.h>

namespace meteo{
  namespace forecast{
  class TGradaciiDB {
  public:
    static bool getGradacii(int level, int levelType, int descr, int , meteo::forecast::Gradacii* responce);
  };

  }
}
#endif // TGRADACIIDB_H
