#pragma once

#include <mappi/thematic/algs_calc/themalg.h>
#include <mappi/proto/thematic.pb.h>
#include <qvector.h>

namespace mappi {
  namespace to {

    class TotalPrecip: public ThemAlg {
    public:
      TotalPrecip( mappi::conf::ThemType type, const std::string &them_name, QSharedPointer<DataStore> &ds);
      ~TotalPrecip();

      virtual bool process();
//      virtual bool saveImage(const QString& name, const QSharedPointer<Channel>& channel, QImage::Format format);

    private:
      float emissivity(float t1, float t2, float t3, float mu);//retrieved emissivity at 23.8 GHz
      float waterEmissivity(float mu);

    };

  }
}

