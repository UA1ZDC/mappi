#ifndef METEO_COMMONS_UI_MAP_GRAPH_RAMKAGRAPH_H
#define METEO_COMMONS_UI_MAP_GRAPH_RAMKAGRAPH_H

#include <qhash.h>

#include <meteo/commons/ui/map/ramka.h>

namespace meteo {
class AxisGraph;
class GridGraph;
} // meteo

namespace meteo {
namespace map {
class Object;
class Document;
} // map
} // meteo

namespace meteo {

//! Класс RamkaGraph обеспечивает отрисовку осей на рамке документа.
class RamkaGraph : public map::Ramka
{
public:
  RamkaGraph(map::Document* doc);
  virtual ~RamkaGraph();

  virtual Ramka* copy(map::Document* d) const { return new RamkaGraph(d); }

  virtual bool drawOnDocument(QPainter* painter);
  virtual QPolygon calcRamka(int mapindent, int docindent) const;

  QList<const AxisGraph*> visibleAxes(int axisPosition = -1) const;

private:
  QRect calcRamkaRect() const;
  QRect calcRamkaRect(int left, int top, int right, int bottom) const;

  void updateGrid();

private:
  GridGraph* grid_;

  mutable QHash<const AxisGraph*,int> cacheAxisSize_;
};

} // meteo

#endif // METEO_COMMONS_UI_MAP_GRAPH_RAMKAGRAPH_H
