#ifndef METEO_COMMONS_UI_MAP_VIEW_WIDGETS_INCUTPARAMS_H
#define METEO_COMMONS_UI_MAP_VIEW_WIDGETS_INCUTPARAMS_H

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>

namespace Ui {
class IncutSetup;
}

namespace meteo {
namespace map {

class IncutParams : public MapWidget
{
  Q_OBJECT
  public:
    IncutParams( MapWindow* window );
    ~IncutParams();

  private:
    Ui::IncutSetup* ui_;
};

}
}

#endif
