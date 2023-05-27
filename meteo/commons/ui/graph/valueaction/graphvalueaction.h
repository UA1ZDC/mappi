#ifndef METEO_COMMONS_UI_GRAPH_VALUEACTION_GRAPHVALUEACTION_H
#define METEO_COMMONS_UI_GRAPH_VALUEACTION_GRAPHVALUEACTION_H

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
class MarkerItem;
} // meteo

namespace meteo {
namespace graph {
class LineItem;
class ComboLabelItem;
} // graph
} // meteo

namespace meteo {
namespace map {

class MapView;
class WidgetItem;

class GraphValueAction : public Action
{
  Q_OBJECT

public:
  static const QString kName;
  enum GraphType { kGraph, kOcean };

  explicit GraphValueAction(MapScene* scene, GraphType type = kGraph);
  ~GraphValueAction();

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

private:
  QAction* menuAct_;
  QAction* deleteAct_;
  graph::LineItem* lineItem_;
  graph::ComboLabelItem* labelItem_;
  QList<MarkerItem*> markers_;
  map::WidgetItem* closeBtnItem_;
  GraphType type_;

  //! Расстояние пройденое курсором между событиями press и release, для исключения
  //! ложного срабатывания события mouseClickEvent(), при смещении указателя на
  //! большое расстояние.
  QLineF moveDistance_;
};

}
}

#endif
