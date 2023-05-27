#ifndef MAPPI_UI_PLUGINS_POINTVALUE_POINTVALUEACTION_H
#define MAPPI_UI_PLUGINS_POINTVALUE_POINTVALUEACTION_H

#include <qset.h>
#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {

class GeoPoint;

} // meteo

namespace meteo {
namespace map {

class GeoRastr;
class SatelliteImage;
class Layer;
class SatLayer;
class MapView;
class WidgetItem;
class BalloonItem;

class PointValueAction : public Action
{
  Q_OBJECT

public:
  static const QString kName;

  explicit PointValueAction(MapScene* scene);

  void mouseMoveEvent(QMouseEvent* e);
  void mousePressEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void wheelEvent(QWheelEvent* e);
  void addActionsToMenu(Menu* menu) const;

  void deactivate();

private slots:
  void slotActionToggled(bool toggled);
  void slotDeleteTriggered();

  void slotLayerChanged(map::Layer* layer, int ch);

protected:
  virtual bool eventFilter(QObject* obj, QEvent* event);

private:
  void updateCloseBtnPos();
  void updateContent();

  inline bool isActive() const;

  bool isWidgetUnderMouse(const QPointF& screenPos) const;

  QList<GeoRastr*> imageObjects() const;
  QStringList imageTexts(const GeoRastr* o, const meteo::GeoPoint& pos) const;

private:
  QAction* menuAct_   = nullptr;
  QAction* deleteAct_ = nullptr;
  map::WidgetItem* closeBtnItem_ = nullptr;
  //! Расстояние пройденое курсором между событиями press и release, для исключения
  //! ложного срабатывания события mouseClickEvent(), при смещении указателя на
  //! большое расстояние.
  QLineF moveDistance_;

  QSet<Layer*> layers_;
  BalloonItem* balloon_ = nullptr;
};

} // map
} // meteo

#endif // MAPPI_UI_PLUGINS_POINTVALUE_POINTVALUEACTION_H
