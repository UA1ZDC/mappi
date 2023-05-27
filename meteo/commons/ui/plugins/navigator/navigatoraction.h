#ifndef METEO_COMMONS_UI_PLUGINS_NAVIGATORACTION_H
#define METEO_COMMONS_UI_PLUGINS_NAVIGATORACTION_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/proto/field.pb.h>

class ActionButton;

class QEvent;
class QGraphicsProxyWidget;
class QMenu;
class QMouseEvent;

namespace obanal {
class TField;
} // obanal

namespace meteo {

class GeoPoint;
class GeoVector;

namespace map {

class NavigatorWidget;

class NavigatorAction : public Action
{
  Q_OBJECT

public:
  explicit NavigatorAction(MapScene* scene);
  ~NavigatorAction();

  void addActionsToMenu(Menu* menu) const;
  void mousePressEvent(QMouseEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void resizeEvent(QResizeEvent* event);

private slots:
  void slotTurnNavigator(bool active);
  void slotUpdateCurrentPosition(const meteo::GeoPoint& pos);

  void slotNavigatorOnTopLeft();
  void slotNavigatorOnTopRight();
  void slotNavigatorOnBottomLeft();
  void slotNavigatorOnBottomRight();
  void slotNavigatorOnTop();
  void slotNavigatorOnBottom();
  void slotNavigatorOnLeft();
  void slotNavigatorOnRight();
  void slotHideNavigator();

  void slotAboutToShowParentMenu();

private:
  void turnOnNavigator();
  void turnOffNavigator();
  void showNavigatorOnPos(Position pos);

  ActionButton* createActionButton(QWidget* parent = 0) const;

  bool eventFilter(QObject* object, QEvent* event);

  QPoint navigatorWidgetPosition() const;
  GeoVector displayRamka() const;

  GeoVector getPointsFromFieldService(const field::SimpleDataRequest& request) const;

  GeoVector getStationsFromSurfaceService(const surf::DataRequest& request) const;
  GeoVector fieldPoints(const obanal::TField& srcfield) const;

private:
  NavigatorWidget* navigator_;
  ActionButton* activateButton_;
  QMenu* navmenu_;
  QAction* hideaction_;

  QGraphicsProxyWidget* proxy_;
  Position navPosition_;
  bool drag_;

};

} // map
} // meteo

#endif // METEO_COMMONS_UI_PLUGINS_NAVIGATORACTION_H
