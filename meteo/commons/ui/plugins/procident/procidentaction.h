#ifndef ProcIdentAction_H
#define ProcIdentAction_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/ui/map/view/actionbutton.h>
#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/ui/map/view/actionbutton.h>
#include "procidentwidget.h"
#include "object_trajection.h"

#include <qpoint.h>

namespace meteo {
namespace map {

class PrepearTrajWidget;

  class ProcIdentAction  : public Action
{
  Q_OBJECT
public:
  ProcIdentAction ( MapScene* scene );
  ~ProcIdentAction (  );
  void addActionsToMenu( Menu* menu ) const ;

private:
  
  ProcIdentWidget* dlg_;
  PrepearTrajWidget* obj_traj_widget_; 
  
  
private slots:
  
  void slotRecognFields();
  void slotAddFields();
  void slotAddObjTrajectory();
  void getField(const QRect&);
    
};

}
}

#endif
