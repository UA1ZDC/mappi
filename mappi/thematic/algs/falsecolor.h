#ifndef MAPPI_THEMATIC_ALGS_FALSECOLOR_H
#define MAPPI_THEMATIC_ALGS_FALSECOLOR_H

#include <mappi/thematic/algs/themalg.h>
#include <mappi/proto/thematic.pb.h>
#include <qvector.h>

namespace mappi {
namespace to {

class FalseColor: public ThemAlg {
private:
  enum class Color { kRed, kGreen, kBlue };
public:
  FalseColor();
  ~FalseColor();

  enum {
    Type = proto::kFalseColor
  };
  virtual proto::ThemType type() const { return proto::kFalseColor; }

  bool process();
  bool process(const proto::ThematicProc &thematic);
  bool saveImage(const QString& baseName);
  bool saveData(const QString& baseName);

private:
  QMap<Color, QVector<float>> _data;
  QMap<Color, QVector<uchar>> _pixels;
  QVector<uchar>              _bitmap;

private:
  static bool isValid(const proto::ThematicProc& thematic);
};

}
}

#endif
