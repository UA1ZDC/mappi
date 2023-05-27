#ifndef METEO_COMMONS_UI_VERTICALCUT_MAP_VERTICALCUTACTION_H
#define METEO_COMMONS_UI_VERTICALCUT_MAP_VERTICALCUTACTION_H

#include <commons/geobasis/geovector.h>
#include <meteo/commons/ui/map/view/actions/traceaction.h>

namespace meteo {
namespace map {

class VerticalCutAction : public TraceAction
{
  Q_OBJECT
public:
  static const QString kName;

public:
  explicit VerticalCutAction(MapScene* scene);
  ~VerticalCutAction();

  void addActionsToMenu(Menu* menu) const;

  void appendNode(const GeoPoint& point, const QString& title, bool move = true);
  void clearTrace();

public slots:
  void slotRequestCoord(bool enable);
  void slotRemovePath();
  void slotItemAdded(int num, const GeoPoint& point);
  void slotPlaceChanged(int num, const GeoPoint& point, const QString& text);
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_VERTICALCUT_MAP_VERTICALCUTACTION_H
