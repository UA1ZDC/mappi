#ifndef METEO_COMMONS_UI_PLUGINS_RADARMAP_H
#define METEO_COMMONS_UI_PLUGINS_RADARMAP_H

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>
#include <meteo/commons/proto/map_radar.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>

namespace Ui {
class FirLoader;
}

class QAbstractButton;
class QMenu;

namespace meteo {
namespace map {

class Layer;

class FirLoader : public MapWidget
{
  Q_OBJECT
  public:
    FirLoader( MapWindow* window );
    ~FirLoader();

  private:
    Ui::FirLoader* ui_;
    QString firpath_;

  private slots:
    void slotLoadFirFile();
};

}
}

#endif
