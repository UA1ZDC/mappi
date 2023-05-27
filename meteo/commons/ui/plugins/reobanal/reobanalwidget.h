#ifndef ReobanalWidget_H
#define ReobanalWidget_H

#include <QtWidgets>

#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/actions/selectaction.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/field.pb.h>
#include <commons/obanal/obanal_struct.h>
#include <commons/obanal/tfield.h>
#include <meteo/commons/ui/map/layeriso.h>

#include <meteo/commons/services/obanal/obanalmaker.h>

namespace Ui{
  class ReobanalWidget;
}

namespace meteo {
namespace map {

class LayerIso;

  enum frontType{
    fCold = 0,
    fWarm = 1,
    fColdRazm = 10,
    fWarmRazm = 11,
    fNoType = 12
  };
  
  
class Document;

class ReobanalWidget : public QDialog
{
  Q_OBJECT
public:
  ReobanalWidget(MapView* view );
  ~ReobanalWidget();
  
  void closeEvent(QCloseEvent *);
  
  void setLayer( Layer *layer );
  void setProp( const proto::WeatherLayer &info_);
  void setInfo(::obanal::TField * field);

private:
  void obanalFromPuanson(descr_t descr);
  void obanalFromDb(descr_t descr);

  void setWidget();
  void createObnl();
   Ui::ReobanalWidget* ui_;
   Layer *layer_;
   meteo::map::proto::WeatherLayer info_;
   QThread *thread_;
   GeoData *all_data_;

public slots:
  void slotClose();
  void slotStopObanal();
  void slotRunObanal();

public slots:
    void handleResults(const QString &);
signals:
    void operate(const meteo::surf::DataRequest &);
    void operateData(meteo::GeoData *, const meteo::map::proto::WeatherLayer &);


};

}
}

#endif
