#ifndef METEO_COMMONS_UI_PLUGINS_AXISACTION_AXISACTION_H
#define METEO_COMMONS_UI_PLUGINS_AXISACTION_AXISACTION_H

#include <qlist.h>

#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
namespace map {

class Object;
class MapView;
class WidgetItem;
class LayerProfile;

} // map
} // meteo

namespace meteo {
namespace graph {

class GeoAxisItem;

}
}

namespace meteo {
namespace map {

class AxisAction : public Action
{
  Q_OBJECT

  enum PropertyType {
    kRemoveUuid,
  };

public:
  static const QString kName;

  explicit AxisAction(MapScene* scene);
  ~AxisAction();

  virtual void mouseMoveEvent(QMouseEvent* e);
  virtual void mousePressEvent(QMouseEvent* e);
  virtual void mouseReleaseEvent(QMouseEvent* e);
  virtual void wheelEvent(QWheelEvent* e);
  virtual void addActionsToMenu(Menu* am) const;

  virtual bool eventFilter(QObject* obj, QEvent* e);

  void deactivate();

private slots:
  void slotActionToggled(bool toggled);
  void slotDeleteClicked();

private:
  bool isActive() const;

  LayerProfile* findLayerProfile() const;
  bool hasLayerProfile() const;
  bool isWidgetUnderMouse(const QPointF& screenPos) const;

  void updateAxisPos();

  bool ensureItemCreated();
  map::Object* axisUnderCursor(const QPoint& scenePos) const;

private:
  QAction* menuAct_ = nullptr;
  QAction* removeAct_ = nullptr;
  graph::GeoAxisItem* axisItem_ = nullptr;
  map::WidgetItem* removeBtnItem_ = nullptr;

  QSet<QString> userProfiles_; // uuid профилей добавленных пользователем

  //! Расстояние пройденое курсором между событиями press и release, для исключения
  //! ложного срабатывания события mouseClickEvent(), при смещении указателя на
  //! большое расстояние.
  QLineF moveDistance_;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_PLUGINS_AXISACTION_AXISACTION_H
