#ifndef MAPPI_THEMATIC_ALGS_NDVI_H
#define MAPPI_THEMATIC_ALGS_NDVI_H

#include <mappi/thematic/algs/themalg.h>
#include <mappi/proto/thematic.pb.h>
#include <qvector.h>

namespace mappi {
  namespace to {

    class Index: public ThemAlg {
    public:
      Index(proto::ThemType type);
      ~Index();

      virtual proto::ThemType type() const { return _themType; }

      bool process();
      bool process(const proto::ThematicProc& thematic);
      bool saveImage(const QString& baseName);
      bool saveData(const QString& baseName);

    private:
      void fillPalette();

    private:
      proto::ThemType _themType;
      QVector<float>  _data;
      QVector<uchar>  _pixels;
      QVector<QRgb>   _palette;

    private:
      static bool isValid(const proto::ThematicProc& thematic);
    };

  }
}

#endif
