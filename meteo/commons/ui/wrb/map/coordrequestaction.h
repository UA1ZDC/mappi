#ifndef METEO_COMMONS_UI_WRB_MAP_COORDREQUESTACTION_H
#define METEO_COMMONS_UI_WRB_MAP_COORDREQUESTACTION_H

#include <commons/geobasis/geopoint.h>
#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
namespace map {

//!
class CoordRequestAction : public Action
{
  Q_OBJECT
public:
  static const QString kName;

public:
  explicit CoordRequestAction(MapScene* scene);
  virtual void mouseReleaseEvent(QMouseEvent* e);

signals:
  void coordinate(const GeoPoint& coord);

private:
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_WRB_MAP_COORDREQUESTACTION_H
