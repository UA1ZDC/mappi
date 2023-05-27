#ifndef FrontIdentAction_H
#define FrontIdentAction_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/ui/map/view/actionbutton.h>
#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/ui/map/view/actionbutton.h>

#include "frontidentwidget.h"

#include <qpoint.h>

namespace meteo {
namespace map {


  class FrontIdentAction  : public Action
{
  Q_OBJECT
public:
  FrontIdentAction ( MapScene* scene );
  ~FrontIdentAction (  );
  void addActionsToMenu( Menu* menu ) const ;

private:
  
  FrontIdentWidget* dlg_;
  
private slots:
  void slotAddFronts();
    
};

}
}

#endif
