#ifndef METEO_COMMONS_UI_MAP_AXIS_LAYERAXIS_H
#define METEO_COMMONS_UI_MAP_AXIS_LAYERAXIS_H

#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/ui/map/layer.h>

namespace meteo {
namespace map {

class Document;
class AxisSimple;

} // map
} // meteo

namespace meteo {
namespace map {

class LayerAxis : public Layer
{
public:
  enum { Type = LayerType_MAX + 201 };

  LayerAxis( Document* d, const QString& name = QString(), int customtype = -1 );
  virtual ~LayerAxis();

  virtual int type() const         { return Type; }

  QRect mapRect() const            { return mapRect_; }

  AxisSimple* xAxis()              { return x1_; }
  AxisSimple* yAxis()              { return y1_; }
  AxisSimple* xAxis2()             { return x2_; }
  AxisSimple* yAxis2()             { return y2_; }

  const AxisSimple* xAxis() const  { return x1_; }
  const AxisSimple* yAxis() const  { return y1_; }
  const AxisSimple* xAxis2() const { return x2_; }
  const AxisSimple* yAxis2() const { return y2_; }

  QList<AxisSimple*> axes() const  { return { x1_, y1_, x2_, y2_ }; }

  //! Существующая шкала будет удалена.
  //! Для шкалы axis будет установлено mapSize, offset, length и geoPosition (в соответствии
  //! с позицией отображения (слева, справа, сверху, снизу)).
  void setAxisX(AxisSimple* axis);
  void setAxisY(AxisSimple* axis);
  void setAxisX2(AxisSimple* axis);
  void setAxisY2(AxisSimple* axis);

private:
  void setupAxis(AxisSimple* axis);

private:
  QRect mapRect_;

  AxisSimple* x1_ = nullptr;
  AxisSimple* x2_ = nullptr;
  AxisSimple* y1_ = nullptr;
  AxisSimple* y2_ = nullptr;
};

}
}

#endif // METEO_COMMONS_UI_MAP_AXIS_LAYERAXIS_H
