#ifndef MAPPI_THEMATIC_ALGS_CLOUDMASK_H
#define MAPPI_THEMATIC_ALGS_CLOUDMASK_H


#include <mappi/thematic/algs_calc/themalg.h>
#include <mappi/proto/thematic.pb.h>
#include <qvector.h>

namespace mappi {
  namespace to {

    class CloudMask: public ThemAlg {
    public:


      CloudMask( mappi::conf::ThemType type, const std::string &them_name,  QSharedPointer<DataStore> &ds);
      ~CloudMask();
      
      virtual bool process();

    protected:

    ThemAlg::pixelType cloudTest(float a1, float a2, float t3,float t4, int landmask);

    };

  }
}


#endif
  
