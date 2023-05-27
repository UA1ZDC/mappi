#ifndef MAPPI_THEMATIC_ALGS_SEATEMPERATURE_H
#define MAPPI_THEMATIC_ALGS_SEATEMPERATURE_H


#include <mappi/thematic/algs_calc/themalg.h>
#include <mappi/proto/thematic.pb.h>
#include <qvector.h>

namespace mappi {
  namespace to {

    class SeaTempr: public ThemAlg {
    public:

      SeaTempr( mappi::conf::ThemType type,const std::string &them_name, QSharedPointer<DataStore> &ds);
      ~SeaTempr();
      
      virtual bool process();

    private:
      double LST_process(double T5, double T6, double theta, int daytime);
      double SST_process(double T5, double T6, double sectheta, int daytime);

      float LST_process_noaa( double T5, double T6, double sectheta, int daytime);
      float SST_process_noaa( double T5, double T6, double sectheta, int daytime);

    };

  }
}


#endif
  
