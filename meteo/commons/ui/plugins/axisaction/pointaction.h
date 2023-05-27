#ifndef METEO_COMMONS_UI_PLUGINS_AXISACTION_POINTACTION_H
#define METEO_COMMONS_UI_PLUGINS_AXISACTION_POINTACTION_H

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
namespace map {

class MapView;
class WidgetItem;
class LayerProfile;
class Puanson;

} // map
} // meteo

namespace meteo {
namespace graph {

class PuansonItem;

}
}

namespace meteo {
namespace map {

class PointAction : public Action
{
  Q_OBJECT

  enum PropertyType {
    kRemoveUuid,
  };

public:
  static const QString kName;

  explicit PointAction(MapScene* scene);
  ~PointAction();

  virtual void mouseMoveEvent(QMouseEvent* e);
  virtual void mousePressEvent(QMouseEvent* e);
  virtual void mouseReleaseEvent(QMouseEvent* e);
  virtual void wheelEvent(QWheelEvent* e);
  virtual void addActionsToMenu(Menu* am) const;

  void deactivate();

private slots:
  void slotActionToggled(bool toggled);
  void slotDeleteClicked();

private:
  bool isActive() const;

  LayerProfile* findLayerProfile() const;
  bool hasLayerProfile() const;
  bool isWidgetUnderMouse(const QPointF& screenPos) const;

  void updateScenePos();

  bool ensureItemCreated();
  Puanson* puansonUnderCursor(const QPoint& screenPos) const;

  void setSelected(bool flag);

private:
  QAction* menuAct_ = nullptr;
  graph::PuansonItem* puansonItem_ = nullptr;
  map::WidgetItem* removeBtnItem_ = nullptr;

  //! Расстояние пройденое курсором между событиями press и release, для исключения
  //! ложного срабатывания события mouseClickEvent(), при смещении указателя на
  //! большое расстояние.
  QLineF moveDistance_;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_PLUGINS_AXISACTION_POINTACTION_H
