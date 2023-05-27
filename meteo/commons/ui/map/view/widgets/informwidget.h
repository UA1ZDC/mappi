#ifndef METEO_COMMONS_UI_MAP_VIEW_WIDGETS_INFORMWIDGET_H
#define METEO_COMMONS_UI_MAP_VIEW_WIDGETS_INFORMWIDGET_H

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>

namespace Ui {
  class DocInfo;
}

namespace meteo {
namespace map {

class InformWidget : public MapWidget
{
  Q_OBJECT
  public:
    InformWidget( MapWindow* parent );
    ~InformWidget();

  protected:
    void showEvent( QShowEvent* e );

  private:
    Ui::DocInfo* ui_;

  private slots:
    void slotCloseClicked();
};


}
}

#endif
