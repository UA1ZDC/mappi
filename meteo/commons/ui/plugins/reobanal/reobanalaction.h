#ifndef ReobanalIdentAction_H
#define ReobanalIdentAction_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/ui/map/view/actionbutton.h>
#include <meteo/commons/ui/map/layerpunch.h>

#include "reobanalwidget.h"

#include <qpoint.h>

namespace meteo {
namespace map {


  class ReobanalAction  : public Action
{
  Q_OBJECT
public:
  ReobanalAction ( MapScene* scene );
  ~ReobanalAction (  );
  void addActionsToMenu( Menu* menu ) const ;

private:
  
  ReobanalWidget* dlg_;
  
private slots:
  void slotNeedReanalyse();
    
};

}
}

#endif
