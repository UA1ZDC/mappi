#ifndef ProcIdentWidget_H
#define ProcIdentWidget_H

#include <QtWidgets>

#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/actions/selectaction.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/field.pb.h>
#include <commons/obanal/obanal_struct.h>
#include <commons/obanal/tfield.h>

//#include <fftw.h>
//#include <rfftw.h>
//#include <fftw.h>
//#include <fftw3.h>
namespace Ui{
 class ProcIdentWidget;
}

namespace meteo {
namespace map {

class Document;

class ProcIdentWidget : public QDialog, public SelectActionIface
{
  Q_OBJECT
public:
  ProcIdentWidget(MapView* view, obanal::TField *field, int type );
  ~ProcIdentWidget();
  
  void closeEvent(QCloseEvent *);
  
  void setField( obanal::TField *field ){ field_ = field;}
  void setDocument( Document *document ){ document_ = document;}
  void fillTree();
    
  //void findMaxCor(obanal::TField *srcfield,obanal::TField *afield);
    
  void test(obanal::TField *field);
private:
  bool calcAdvectPoints();
  void setPoints(const GeoPoint& nw, const GeoPoint& ne, const GeoPoint& sw, const GeoPoint& se, const QRect& rect);        
  bool saveProcessField(const ::meteo::field::ProcessFieldData& );
  bool deleteProcessField(int );
  bool editProcessField(const ::meteo::field::ProcessFieldData& );
  bool getProcField(obanal::TField *field);
  
  //void prepField(const obanal::TField *,const obanal::TField *, int *w, int *h);
  //fftw_complex * prepField(const obanal::TField *,int w, int h);
  
 // void visualizeFFT(fftw_complex* fft,  int WIDTH,  int HEIGHT, const QString & path) ;
    
   Ui::ProcIdentWidget* ui_;
   MapView* view_;
  
   RegionParam reg_par_;
   obanal::TField *field_;
   QRect mapBand_;
   Document *document_;
   QStringList procList_;
   int type_;
   //fftw_complex *pattern_in;
   //fftw_complex *src_in;
public slots:
  void slotSelectRegion(bool);
  void slotDelField();
  void slotEditField();
  void slotAddField();
  void slotClose();
  void slotTreeChanged();
  void slotSearch();
  
signals:
  void addRect(const QRect& );
};

}
}

#endif
