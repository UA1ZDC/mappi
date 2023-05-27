#ifndef FRontIdentWidget_H
#define FRontIdentWidget_H

#include <QtWidgets>

#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/actions/selectaction.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/field.pb.h>
#include <commons/obanal/obanal_struct.h>
#include <commons/obanal/tfield.h>

namespace Ui{
  class FrontIdentWidget;
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

class FrontIdentWidget : public QDialog
{
  Q_OBJECT
public:
  FrontIdentWidget(MapView* view );
  ~FrontIdentWidget();
  
  void closeEvent(QCloseEvent *);
  
  bool prepFields(obanal::TField *field);
  void setProp( obanal::TField *field);
  void slotConnect();

  void setDocument( Document *document ){ document_ = document;}
  
private:
  void setInfo(obanal::TField * field);
    
  proto::WeatherLayer getInfo(const QString&);
    
  void flushFront(Layer * layer,frontType ftype, meteo::GeoVector *tl, meteo::GeoVector *hl );
  bool getGradT( const meteo::GeoPoint &p, float *);
    
  frontType identFrontType(const meteo::GeoPoint &p);
  
  void addFrontLine(double val, const QString & aname);
  void addColdFront(Layer * layer, const meteo::GeoVector &line, frontType);
  void addWarmFront(Layer * layer, const meteo::GeoVector &line, frontType);
  frontType addFront(Layer * layer, const meteo::GeoPoint &p, meteo::GeoVector *tl, meteo::GeoVector *hl);
    
  double getTadv(const meteo::GeoPoint& p );
    
  bool getField(const ::std::string &id,obanal::TField * fd);
  bool loadFields();
  bool getFieldList(const meteo::field::DataRequest &request, QList< meteo::field::DataDesc> *afields);
  int  fillFieldList(QComboBox *, const QDateTime &acur_dt, int level,int level_type,int hour,int descr);
  void fillFieldCombos();
  QString getComboText(const meteo::field::DataDesc &d);

  bool reconnectFieldService();

  Ui::FrontIdentWidget* ui_;
   MapView* view_;
  
   Document *document_;
   float dfi_1;
   float dla_1;
   int center_;
   obanal::TField fdT850_;
   obanal::TField fdH850_;
   obanal::TField fdH5001000_;
   obanal::TField fdU850_;
   obanal::TField fdV850_;
   proto::WeatherLayer info_; 

   meteo::rpc::Channel* field_ctrl_;

public slots:
  void slotAdd();
  void slotClose();
  void slotChanged();


};

}
}

#endif
