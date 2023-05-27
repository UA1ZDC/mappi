#ifndef METEO_COMMONS_UI_PLUGINS_MULTIRADAR_MULTIRADAR_H
#define METEO_COMMONS_UI_PLUGINS_MULTIRADAR_MULTIRADAR_H

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>

#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/proto/map_radar.pb.h>
#include <meteo/commons/ui/map/view/mapview.h>

namespace Ui{
 class MultiRadar;
}

class QMovie;

namespace meteo {

namespace puanson {
namespace proto {
  class Puanson;
}
}

namespace map {

class Puanson;

class MultiRadar : public MapWidget
{
  Q_OBJECT
  public:
    MultiRadar( MapWindow* window );
    ~MultiRadar();

    void setCurrentLayer( Layer* layer );

    bool animation() const { return animation_; }

    void stopAnimation();

  private:
    Ui::MultiRadar* ui_;
    proto::WeatherLayer current_;
    proto::RadarColor currentcolor_;
    Layer* currentlayer_;
    QDateTime curdt_;
    QByteArray arr_;
    QBuffer buf_;
    QMovie* movie_;
    QGraphicsItem* item_;

    bool animation_;

    QList<QDateTime> getTerms();
    void buildImage( Document* doc, QImage* img, const QDateTime& dt );
    void buildProxyMovie();

  private slots:
    void slotPastSpinChanged( int value );
    void slotFutureSpinChanged( int value );
    void slotAccept();
    void slotReject();
};

}
}

#endif // DATAWIDGET_H
