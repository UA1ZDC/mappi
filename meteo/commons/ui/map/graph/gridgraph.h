#ifndef METEO_COMMONS_UI_MAP_GRAPH_GRIDGRAPH_H
#define METEO_COMMONS_UI_MAP_GRAPH_GRIDGRAPH_H

#include <qobject.h>
#include <qhash.h>

namespace meteo {
class AxisGraph;
} // meteo

namespace meteo {
namespace map {
class Document;
class Object;
} // map
} // meteo

namespace meteo {

//! Класс GridGraph отвечает за формирование координатной сетки документа.
class GridGraph : public QObject
{
  Q_OBJECT
public:
  explicit GridGraph(map::Document* doc = 0, QObject *parent = 0);

  QList<const AxisGraph*> visibleAxes(int axisPosition = -1) const;

  void calcGrid();
  void updateGrid();

protected:
  virtual bool eventFilter(QObject* obj, QEvent* event);

private:
  map::Document* document_;

  QString layerUuid_;

  QHash<const AxisGraph*, QHash<int, QList<map::Object*> > > gridLines_;
  QHash<const AxisGraph*,QString> layerAxes_;
};

} // meteo

#endif // METEO_COMMONS_UI_MAP_GRAPH_GRIDGRAPH_H
