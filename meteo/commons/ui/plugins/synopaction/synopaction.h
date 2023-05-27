#ifndef METEO_MAP_VIEW_ACTIONS_TestAction_H
#define METEO_MAP_VIEW_ACTIONS_TestAction_H

#include <qpoint.h>
#include <qdialog.h>
#include <qgraphicsproxywidget.h>
#include <commons/geobasis/geopoint.h>
#include <meteo/commons/ui/map/view/actions/action.h>
#include "synopwidget.h"

class QLabel;
class QToolButton;
class QGraphicsProxyWidget;

namespace meteo {
namespace map {

class MapView;

class SynopAction : public Action
{
  Q_OBJECT
public:
  SynopAction( MapScene* scene );
  ~SynopAction();

  void addActionsToMenu( Menu* menu ) const ;
  void mousePressEvent( QMouseEvent* e );

protected:

private:
  SynopWidget* swidget_;
  QPoint point_;
//  QPoint fixPos_;


//  void setDefaultPosition();

private slots:
  void slotSynop();
};

}
}

#endif
