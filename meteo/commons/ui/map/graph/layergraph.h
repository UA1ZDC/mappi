#ifndef METEO_COMMONS_UI_MAP_GRAPH_LAYERGRAPH_H
#define METEO_COMMONS_UI_MAP_GRAPH_LAYERGRAPH_H

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/ui/map/graph/axisgraph.h>
#include <meteo/commons/ui/map/layer.h>

namespace meteo {
namespace map {
class GeoPolygon;
class Property;
class Document;
} // map
} // meteo

namespace meteo {
namespace map {

class LayerGraph: public Layer
{
public:
  enum { Type = LayerType_MAX + 25 };

  LayerGraph( Document* d, const QString& name = QString());
  virtual ~LayerGraph();

  virtual int type() const { return Type; }

  GeoPoint nearestByX(float x, bool* ok = 0) const;
  GeoPoint nearestByY(float y, bool* ok = 0) const;

  QList<GeoPoint> crossByX(double x) const;
  QList<GeoPoint> crossByY(double y) const;

  // note: если xDescr,yDescr пустые строки, то в качестве значения будет использоваться дата
  void setData(const QList<TMeteoData>& data, const QString& xDescr, const QString& yDescr, bool recalcAxisRange = true);
  void update(bool recalcAxisRange = false);

  void setPuanson(const ::meteo::puanson::proto::Puanson& puanson) { puanson_ = puanson; }

  QStringList puansonLabels(float x = NAN, float y = NAN) const;

  meteo::Property graphProperty() const                 { return prop_; }
  void setGraphProperty(const meteo::Property& prop);

  AxisGraph* xAxis()              { return &x_; }
  AxisGraph* yAxis()              { return &y_; }
  const AxisGraph* xAxis() const  { return &x_; }
  const AxisGraph* yAxis() const  { return &y_; }

  GeoVector graphCoords() const;

  virtual int32_t dataSize() const;
  virtual int32_t serializeToArray(char* data) const;
  virtual int32_t parseFromArray(const char* data);

private:
  QVector<float> getDtValues(const QList<TMeteoData>& mdList) const;
  QVector<float> getValues(const QList<TMeteoData>& mdList, const QString& descr) const;

  QPointF nearest(float value, const QVector<float>& values, bool* ok = 0) const;

  void calcAxisRange();

  void printLayerInfo() const;

  float minElement(const QVector<float>& v) const;
  float maxElement(const QVector<float>& v) const;

private:
  QList<TMeteoData> input_;

  QVector<float> xVals_;
  QVector<float> yVals_;

  AxisGraph x_;
  AxisGraph y_;

  QString graphUuid_;
  QStringList puansonUuids_;
  meteo::Property prop_;
  ::meteo::puanson::proto::Puanson puanson_;

  QRect mapRect_;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_MAP_GRAPH_LAYERGRAPH_H
