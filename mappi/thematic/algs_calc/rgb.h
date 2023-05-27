#ifndef MAPPI_THEMATIC_ALGS_RGB_H
#define MAPPI_THEMATIC_ALGS_RGB_H

#include <mappi/thematic/algs_calc/themalg.h>
#include <mappi/proto/thematic.pb.h>
#include <qvector.h>

namespace mappi {
  namespace to {
  enum class Color { kRed, kGreen, kBlue };

    class RgbAlg: public ThemAlg {
    public:
      RgbAlg( mappi::conf::ThemType type,const std::string &them_name, QSharedPointer<DataStore> &ds);
      ~RgbAlg();

      virtual bool process();

    private:
      virtual bool saveImage(const QString& name, const QSharedPointer<Channel>& channel, QImage::Format format);
      void setData(Color color_channel, const QVector<uchar> &data);

      QMap<Color, QVector<uchar>> pixels_;
    };

  }
}

#endif
