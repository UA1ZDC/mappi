#include "coordsaction.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>

#include "../actionbutton.h"
#include "../mapview.h"
#include "../mapscene.h"
#include "../widgetitem.h"

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/geopoint.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/custom/geopointeditor.h>


#include <unistd.h>


namespace meteo {
namespace map {

//! Расстояние пройденое курсором между событиями press и release, для исключения
//! ложного срабатывания события mouseClickEvent(), при смещении указателя на
//! большое расстояние.
static QLineF gMoveDistance;

CoordsAction::CoordsAction( MapScene* scene )
  : Action(scene, "coordsaction" )
{
}

CoordsAction::~CoordsAction()
{
}

void CoordsAction::mousePressEvent(QMouseEvent* e)
{
  if ( !gMoveDistance.isNull() ) {
    gMoveDistance.setP1(e->pos());
  }
  else {
    gMoveDistance = QLine(e->pos(), e->pos());
  }

  Action::mousePressEvent(e);
}

void CoordsAction::mouseReleaseEvent(QMouseEvent *e)
{
  gMoveDistance.setP2(e->pos());

  if ( isWidgetUnderMouse(e->pos()) ) { return; }

  if ( gMoveDistance.length() < 20) { mouseClickEvent(e); }

  Action::mouseReleaseEvent(e);
}

void CoordsAction::mouseClickEvent(QMouseEvent* e)
{
  if ( !hasView() || nullptr == parent() || nullptr == scene_->document() ) { return; }
  if ( meteo::GENERAL == scene_->document()->projection()->type()) { return; }

  for ( auto pointeditor: GeoPointEditor::getAllEditors() ){
    if ( true == pointeditor->isMapButtonPressed() ){
      QPoint pnt = scene_->mapview()->mapToScene( e->pos() ).toPoint();
      pointeditor->setCoord(scene_->document()->screen2coord(pnt));
    }
  }
}

bool CoordsAction::isWidgetUnderMouse(const QPointF& screenPos) const
{
  QPointF scenePos = scene_->mapview()->mapToScene(screenPos.toPoint());
  QList<QGraphicsItem*> items = scene_->items(scenePos);

  for ( int i=0,isz=items.size(); i<isz; ++i ) {
    if ( nullptr != qgraphicsitem_cast<QGraphicsProxyWidget*>(items.at(i)) ) {
      return true;
    }
  }

  return false;
}


} // map
} // meteo
