#ifndef MAPPI_THEMATIC_ALGS_NDVI_H
#define MAPPI_THEMATIC_ALGS_NDVI_H

#include <mappi/thematic/algs_calc/themalg.h>
#include <mappi/proto/thematic.pb.h>
#include <qvector.h>

namespace mappi {
  namespace to {

    class Index: public ThemAlg {
    public:
      Index( mappi::conf::ThemType type, const std::string &them_name, QSharedPointer<DataStore> &ds);
      ~Index();

      virtual bool process();
//      virtual bool saveImage(const QString& name, const QSharedPointer<Channel>& channel, QImage::Format format);

    private:

    };

  }
}

#endif
