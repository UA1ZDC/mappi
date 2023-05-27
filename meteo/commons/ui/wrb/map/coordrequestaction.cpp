#include "coordrequestaction.h"

#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>

namespace meteo {
namespace map {

const QString CoordRequestAction::kName = QObject::tr("coord_request_action");

CoordRequestAction::CoordRequestAction(MapScene* scene)
  : Action(scene, kName)
{
}

void CoordRequestAction::mouseReleaseEvent(QMouseEvent* e)
{
  if ( !hasView() ) { return; }

  QPoint pnt = scene_->mapview()->mapToScene( e->pos() ).toPoint();
  GeoPoint p = scene_->document()->screen2coord(pnt);
  emit coordinate(p);

  Action::mouseReleaseEvent(e);
}

} // map
} // meteo
