#ifndef MAPPI_UI_PLUGINS_UHDCONTROL_UHDCONTROL_H
#define MAPPI_UI_PLUGINS_UHDCONTROL_UHDCONTROL_H

#include <QWidget>
#include <uhd.h>
#include <uhd/usrp/multi_usrp.hpp>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <commons/geobasis/generalproj.h>
#include <commons/geobasis/projection.h>
#include <QSettings>
#include <signal.h>
#include "layerspectr.h"
#include "uhdwindow.h"

#include <qevent.h>
#include <QKeyEvent>


namespace Ui {
class UhdControl;
}

class UhdControl : public QWidget
{
  Q_OBJECT

public:
  explicit UhdControl(UhdWindow *parent = 0);
  ~UhdControl();
  void init();
  void findDevices();
  void makeDevice(int i);
  void getInfo();
  void setInfo();


private:
  size_t total_num_samps_;
  double rate_, freq_, gain_, bw_;
  int amountDevices_=0, fftSize_=0, oldfftSize_=0;
  bool first_=false;
  QMap <double,double> oldSample_;
  uhd::device_addrs_t device_addrs_;
  uhd::usrp::multi_usrp::sptr usrp_;
  QTimer* timerUpdate_;
  Ui::UhdControl* ui_;
  QSettings settings_;
  meteo::map::LayerSpectr* layer_;
  UhdWindow* graph_;
  meteo::map::GeoPolygon* line_;
  meteo::GeneralProj* proj_;

private:
  QVector<std::complex<double> > getSample();
  QVector<std::complex<double> > getfftw(QVector<std::complex<double> >& sample);
  QMap <double,double> getAmplitude(QVector<std::complex<double> >& fftsample);
  QMap <double,double> smooth(QMap <double,double>& sample);
  QString getInfoString();
  void setRamka();
  void saveSettings();
  void loadSettings();
  void update();
  void keyPressEvent(QKeyEvent *event);
  void closeEvent(QCloseEvent* ev);



private slots:
  void slotSearchButton();
  void slotMakeButton(bool isSet);
  void slotSetRamka();
  void slotSetInfo();
  void slotGetGraph();
  void slotUpdate();
  void slotStopTimer();
  void slotDisconnect();
  void slotChangeDevice(int index);

};

#endif // MAPPI_UI_PLUGINS_UHDCONTROL_UHDCONTROL_H
