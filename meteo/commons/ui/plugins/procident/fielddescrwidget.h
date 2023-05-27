#ifndef FieldDescrWidgett_H
#define FieldDescrWidget_H

#include <QtWidgets>

#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/field.pb.h>
#include <commons/obanal/obanal_struct.h>
#include <commons/obanal/tfield.h>

namespace Ui{
  class FieldDescrWidget;
}

namespace meteo {
namespace map {

class Document;

class FieldDescrWidget : public QDialog
{
  Q_OBJECT
public:
  FieldDescrWidget(MapView* view );
  ~FieldDescrWidget();
  
  void setProcess(const QMap<int,QString>&  );
  void fillProcessList(const QStringList & ); 
    
  void setProcName(const QString & a);
  void setFnum(const QString & a); 
  void setSeason(const QString & a);
  void setHour(const QString & a); 
  void setBtnTitle(const QString & a); 
    
  
  int numField();
  int hour();
  int getProc(QString *);
  QString season();
  int is_ok_;
private:
  
  Ui::FieldDescrWidget* ui_;
private slots:
  void slotOk();
  void slotCancel();
  
};

}
}

#endif
