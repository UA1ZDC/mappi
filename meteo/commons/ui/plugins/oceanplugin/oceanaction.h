#ifndef METEO_COMMONS_UI_OCEANDIAGRAM_OCEANCORE_OCEANACTION_H
#define METEO_COMMONS_UI_OCEANDIAGRAM_OCEANCORE_OCEANACTION_H

#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/ui/map/view/actionbutton.h>
#include <meteo/commons/ui/map/view/actions/traceaction.h>
#include <qwidget.h>

class QToolButton;
class QEvent;
class QTreeWidgetItem;

namespace meteo {
  namespace odiag {
    class CreateOceanDiagWidget;
  }
}

namespace meteo {
namespace map {

class MapView;
class Map;
class Document;

  class OceanAction : public Action
  {
    Q_OBJECT
    public:
    OceanAction( MapScene* scene );
    virtual ~OceanAction();
    //    void addActionsToMenu( Menu* menu ) const ;

  public slots:
    void slotShow( bool fl );
  private:
    ActionButton* btnocean_;
    QGraphicsProxyWidget* btnitem_;
    meteo::odiag::CreateOceanDiagWidget* oceanwidget_;
    bool deletevisible_;
		       
  private slots:
    void slotLayerWidgetClosed();
  };
  
}
}

#endif
