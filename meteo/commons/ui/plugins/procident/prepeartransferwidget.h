#ifndef PrepearTransferWidget_H
#define PrepearTransferWidget_H

#include <QtGui>

#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/sprinf.pb.h>

namespace Ui{
 class PrepearTransferWidget;
}

namespace meteo {
namespace map {

class Document;

class PrepearTransferWidget : public QDialog
{
  Q_OBJECT
public:
   PrepearTransferWidget(QWidget *parent = 0);
  ~PrepearTransferWidget();
  
  void setStartGeoPoint( const GeoPoint &agp);
  GeoPoint getStartGeoPoint( );
  int trajectory();
  bool markersIsVisible();
  
  meteo::field::AdvectDataReply *advectReply(){return advect_reply_;}
private:
  
   Ui::PrepearTransferWidget* ui_;
   rpc::TController* ctrl_field_;
   meteo::field::AdvectDataReply *advect_reply_;
   
   double koef();
   int time();
   int interval();
   int field();
   int center();
   QDateTime dateTime(); 

   bool calcAdvectPoints();
   
   
   bool createServices();
   
public slots:
  void slotFillFieldBox(int);
  void dateTimeChanged ();
  void anyChanged ();
  void slotApplyClose();
  void slotClose();
signals:
  void applyTraj();
};

}
}

#endif
