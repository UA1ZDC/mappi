#ifndef METEO_MAP_VIEW_ACTIONS_FIELDSACTION_H
#define METEO_MAP_VIEW_ACTIONS_FIELDSACTION_H

#include "fieldswidget.h"
#include <meteo/commons/ui/map/view/actions/coordproxywidget.h>
#include <qpoint.h>
#include <qdialog.h>
#include <qgraphicsproxywidget.h>
#include <commons/geobasis/geopoint.h>
#include <meteo/commons/ui/map/view/actions/mousecoordaction.h>

class QLabel;
class QToolButton;
class QGraphicsProxyWidget;

namespace meteo {
namespace map {

class MapView;

class FieldsAction : public BaloonAction
{
  Q_OBJECT
public:
  FieldsAction( MapScene* scene );

  void mouseMoveEvent( QMouseEvent* e );
  void mouseReleaseEvent( QMouseEvent* e );
  void addActionsToMenu( Menu* am ) const;

protected:
  bool eventFilter( QObject* watched, QEvent* event );
  void createFWidget( );
private:
  QList<valLabel> fieldsResult(const GeoPoint& gp) const;
  void updateText();
  bool proxyIsVisible()const;
  MapView* mapview();
  QAction* action_;

private slots:
  void slotWidgetOpen(bool);
  void slotTurnCoordWidgetOff();
  void slotAboutToShowParentMenu();
};

}
}

#endif
