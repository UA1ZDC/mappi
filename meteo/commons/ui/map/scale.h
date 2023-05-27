#ifndef METEO_COMMONS_UI_VERTICALCUT_CORE_SCALE_H
#define METEO_COMMONS_UI_VERTICALCUT_CORE_SCALE_H

#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>

#include <meteo/commons/ui/map/geogroup.h>

namespace meteo {
namespace map {

//!
struct ScaleOptions
{
  enum Side { kNoneSide   = 0,
              kLeftSide   = 1,
              kTopSide    = 1,
              kBottomSide = 2,
              kRightSide  = 2,
              kAllSide    = kLeftSide | kRightSide,
            };

  ScaleOptions()
  {
    orientation = Qt::Vertical;
    length = 100;
    tickSide  = kAllSide;
    showLabel = kAllSide;
  }

  Qt::Orientation orientation;
  GeoPoint point;
  float length;
  QVector<float> tickValues;
  QVector<QString> tickLabels;

  int tickSide;
  int showLabel;
};

//!
class Scale
{
public:
  Scale(){}
  Scale(const ScaleOptions& opt);

  void setOptions(const ScaleOptions& opt) { opt_ = opt; }

  GeoVector body() const;

  GeoPoint tickPoint(float value) const;
  GeoPoint tickLabel(float value, float padding = 10) const;
  GeoVector tick(float value, float size = 10) const;

  GeoGroup* createObject(Layer* layer) const;

  bool isHorizontal() const { return ( Qt::Horizontal == opt_.orientation ); }

private:
  GeoPoint getPoint(float value) const;

private:
  ScaleOptions opt_;
};



} // cut
} // meteo

#endif // METEO_COMMONS_UI_VERTICALCUT_CORE_SCALE_H
