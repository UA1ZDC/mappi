#ifndef MAPPI_UI_PLUGINS_UHDCONTROL_RAMKASPECTR_H
#define MAPPI_UI_PLUGINS_UHDCONTROL_RAMKASPECTR_H

#include <qhash.h>

#include <meteo/commons/ui/map/ramka.h>
#include "gridspectr.h"

namespace meteo {
class AxisSpectr;
class GridSpectr;
} // meteo

namespace meteo {
namespace map {
class Object;
class Document;
class MapScene;
} // map
} // meteo

namespace meteo {
//! Класс RamkaSpectr обеспечивает отрисовку осей на рамке документа.
class RamkaSpectr : public map::Ramka
{
public:
  RamkaSpectr(map::Document* doc);
  virtual ~RamkaSpectr();

  virtual Ramka* copy(map::Document* d) const { return new RamkaSpectr(d); }

  virtual bool drawOnDocument(QPainter* painter);
  virtual QPolygon calcRamka(int mapindent, int docindent) const;

  QList<const AxisSpectr*> visibleAxes(int axisPosition = -1) const;

  void updateGrid();

  void setMapScene(map::MapScene* scene) { scene_ = scene; }

private:
  QRect calcRamkaRect() const;
  QRect calcRamkaRect(int left, int top, int right, int bottom) const;


private:
  map::MapScene* scene_ = nullptr;
  GridSpectr* grid_;

  mutable QHash<const AxisSpectr*,int> cacheAxisSize_;
};

} // meteo


#endif // MAPPI_UI_PLUGINS_UHDCONTROL_RAMKASPECTR_H
