#ifndef METEO_COMMONS_UI_GRAPH_COORDACTION_GRAPHCOORDACTION_H
#define METEO_COMMONS_UI_GRAPH_COORDACTION_GRAPHCOORDACTION_H

#include <qlist.h>

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
class MarkerItem;
} // meteo

namespace meteo {
namespace graph {
class LineItem;
class LabelItem;
} // graph
} // meteo

namespace meteo {
namespace map {
class MapView;
class WidgetItem;
} // map
} // meteo

namespace meteo {
namespace map {

class GraphCoordAction : public Action
{
  Q_OBJECT

public:
  static const QString kName;
  enum GraphType { kGraph, kOcean };

  explicit GraphCoordAction(MapScene* scene, GraphType type = kGraph);
  ~GraphCoordAction();

  void mouseMoveEvent(QMouseEvent* e);
  void mousePressEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void addActionsToMenu(Menu* am) const;

  void deactivate();

private slots:
  void slotActionToggled(bool toggled);
  void slotDeleteTriggered();

private:
  inline bool isActive() const;

  bool hasLayerGraph() const;
  bool isWidgetUnderMouse(const QPointF& screenPos) const;

  void removeItems();
  void setAlignment(QRect ramkaRect, QRect r, graph::LabelItem* lbl);

private:
  QAction* deleteAct_;
  QAction* menuAct_;
  graph::LabelItem* dtItem_;
  graph::LineItem* lineItem_;
  QList<MarkerItem*> markers_;
  QList<graph::LabelItem*> labels_;
  map::WidgetItem* closeBtnItem_;

  //! Расстояние пройденое курсором между событиями press и release, для исключения
  //! ложного срабатывания события mouseClickEvent(), при смещении указателя на
  //! большое расстояние.
  QLineF moveDistance_;
  GraphType type_;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_GRAPH_COORDACTION_GRAPHCOORDACTION_H
