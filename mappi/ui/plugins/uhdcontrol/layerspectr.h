#ifndef MAPPI_UI_PLUGINS_UHDCONTROL_LAYERSPECTR_H
#define MAPPI_UI_PLUGINS_UHDCONTROL_LAYERSPECTR_H

#include "axisspectr.h"

#include <meteo/commons/proto/map_document.pb.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/geopolygon.h>

#include <cross-commons/debug/tlog.h>


namespace meteo {
namespace map {
class GeoPolygon;
class Property;
class Document;
} // map
} // meteo

namespace meteo {
namespace map {

class LayerSpectr: public Layer
{
public:
  enum { Type = meteo::LayerType_MAX + 26 };

  LayerSpectr( Document* d, const QString& name = QString());
  virtual ~LayerSpectr() {}

  virtual int type() const { return Type; }


  QList<GeoPoint> crossByX(double x) const;

  void setAmplitude(QMap<double, double> &coord );
  void updateSkelet( meteo::GeoVector& skelet );


  const meteo::Property& grProperty() const;
  void setGraphProperty(const meteo::Property& prop);

  AxisSpectr* xAxis()              { return &x_; }
  AxisSpectr* yAxis()              { return &y_; }
  const AxisSpectr* xAxis() const  { return &x_; }
  const AxisSpectr* yAxis() const  { return &y_; }
  double minAmpl();
  double maxAmpl();
  void calcYAxisRange();
  void calcXAxisRange(double lower, double upper);

private:
  QVector<double> getValuesX(QMap<double, double> coord);
  QVector<double> getValuesY(QMap<double,double> coord);
  void printLayerInfo() const;
  double minElement(const QVector<double>& v) const;
  double maxElement(const QVector<double>& v) const;


private:

  QVector<double> xVals_;
  QVector<double> yVals_;

  AxisSpectr x_;
  AxisSpectr y_;

  QString graphUuid_;
  meteo::Property prop_;
  meteo::map::GeoPolygon* line_;
  QRect mapRect_;
};

} // map
} // meteo


#endif // MAPPI_UI_PLUGINS_UHDCONTROL_LAYERSPECTR_H
