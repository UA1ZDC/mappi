#ifndef METEO_COMMONS_UI_MAP_AXIS_RAMKAAXIS_H
#define METEO_COMMONS_UI_MAP_AXIS_RAMKAAXIS_H

#include <meteo/commons/ui/map/ramka.h>

namespace meteo {
namespace map {

class Document;
class AxisSimple;

}
}

namespace meteo {
namespace map {

//! Класс RamkaAxis отвечает за отрисовку рамки документа со шкалами.
class RamkaAxis : public Ramka
{
public:
  RamkaAxis(map::Document* doc);
  virtual ~RamkaAxis();

  virtual Ramka* copy(map::Document* d) const { return new RamkaAxis(d); }

  virtual bool drawOnDocument(QPainter* painter);
  virtual QPolygon calcRamka(int mapindent, int docindent) const;

  QList<AxisSimple*> visibleAxes(int axisPosition = -1) const;

private:
  //! Вычисляет внешнюю границу рамки.
  QRect calcOuterRect(int left, int top, int right, int bottom) const;
  //! Вычисляет внутреннюю границу рамки.
  QRect calcInnerRect() const;
};

}
}

#endif // METEO_COMMONS_UI_MAP_AXIS_RAMKAAXIS_H
