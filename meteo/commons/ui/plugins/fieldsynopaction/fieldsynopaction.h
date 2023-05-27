#ifndef FIELDSYNOPACTION_H
#define FIELDSYNOPACTION_H

//#include "fieldsynopwidg.h"
//#include "fieldsynoproxy.h"
#include <meteo/commons/ui/map/view/actions/action.h>

namespace meteo {
namespace map {

class MapView;

class FieldSynopAction : public Action
{
  Q_OBJECT

public:
  FieldSynopAction( MapScene* scene);


  ~FieldSynopAction();


  /*

  void mouseMoveEvent( QMouseEvent* e );
  void mouseReleaseEvent(QMouseEvent *e);
//  void mousePressEvent(QMouseEvent *e);
  void wheelEvent(QWheelEvent *event);

private:
  FieldSynopWidget* fieldSynWidget_;
  FieldSynoProxy* fieldSynProxy_;
  bool drag_;
  bool press_;
  bool ok_;
  QPoint fixPos_;
  QMenu* menu_;

  void initFieldAction();
  QString fieldsResult(const GeoPoint& gp) const;
  MapView* mapview();
  bool proxyIsVisible();
  void setDefaultPosition();



private slots:
  void slotFields();
  void slotWidgetOpen(const QString& name, const QString& uuid);





protected:

*/

};



}
}
#endif // FIELDSYNOPACTION_H
