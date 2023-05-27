#ifndef PrepearTransferWidget_H
#define PrepearTransferWidget_H

#include <QtGui>

#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/widgets/mapwidget.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/sprinf.pb.h>

namespace Ui{
 class PrepearTrajWidget;
}

namespace meteo {
namespace map {

class ObjectTrajection;

class Document;

class PrepearTrajWidget : public MapWidget
{
  Q_OBJECT
public:
  PrepearTrajWidget(meteo::map::Document* doc, MapWindow *parent = nullptr);
   ~PrepearTrajWidget();
  
  
private:
  void hasData(meteo::field::ExtremumTrajReply*);
  
  bool getObjTrajectory();
  
   Ui::PrepearTrajWidget* ui_;
   
   int timeStart();
   int timeEnd();

   meteo::map::Document* doc_;
   ObjectTrajection *objtraj_;
   Layer *cur_layer_;
   QDateTime dt_start_;
   QDateTime dt_end_;
   meteo::field::DataRequest request_;
public slots:
  void anyChanged ();
  void slotApplyClose();
  void slotClose();
  void slotLayerChanged( map::Layer* , int );
};

}
}

#endif
