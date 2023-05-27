#include "uhdcontrol.h"
#include "ui_uhdcontrol.h"
#include "layerspectr.h"
#include "ramkaspectr.h"
#include "axisspectr.h"
#include "filterkalman.h"
#include "funcs.h"
#include "uhdwindow.h"

#include <cross-commons/debug/tlog.h>

#include <uhd/device.hpp>
#include <uhd/exception.hpp>
#include <uhd.h>
#include <uhd/usrp/usrp.h>
#include <uhd/types/tune_request.hpp>
#include <uhd/utils/thread.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/transport/udp_simple.hpp>

#include <fftw3.h>

#include <iostream>
#include <cstdlib>
#include <complex>
#include <math.h>
#include <QTimer>
#include <QWidget>
#include <QSettings>
#include <qdir.h>
#include <qmessagebox.h>
#include <qevent.h>
#include <QKeyEvent>

#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/proto/map_document.pb.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/textproto/pbtools.h>
#include <commons/geobasis/generalproj.h>
#include <commons/geobasis/projection.h>
#include <commons/geobasis/geopoint.h>
#include <meteo/commons/ui/map/view/actions/action.h>



class Action;

UhdControl::UhdControl(UhdWindow *parent) :
  QWidget(parent),
  ui_(new Ui::UhdControl)
{
  ui_->setupUi(this);
  timerUpdate_= new QTimer(this);
  graph_=parent;
  layer_ = new meteo::map::LayerSpectr(graph_->document(), QObject::tr("АЧХ"));
  proj_ = meteo::projection_cast<meteo::GeneralProj*>( graph_->document()->projection() );
  proj_->setXfactor(0.15);
  proj_->setYfactor(-1.0/130.0);
  oldfftSize_=0;
  meteo::map::Action* action = graph_->mapscene()->getAction("scaleaction");
  if (0 != action && 0 != action->item()) {
    action->item()->setVisible(false);
  }
  graph_->document()->setMinScale(0);
  graph_->document()->setMaxScale(9.0);
  delete graph_->mapscene()->takeAction("focusaction");
  delete graph_->mapscene()->takeAction("layersaction");
  delete graph_->mapscene()->takeAction("savesxfacion");
  delete graph_->mapscene()->takeAction("opensxfaction" );
  delete graph_->mapscene()->takeAction("printdocaction");
  delete graph_->mapscene()->takeAction("savebmpacion" );
  delete graph_->mapscene()->takeAction("mousecoordaction");
  delete graph_->mapscene()->takeAction("incutaction");
  delete graph_->mapscene()->takeAction("legendaction");
  delete graph_->mapscene()->takeAction("hidebuttonsaction");
  delete graph_->mapscene()->takeAction("informaction");
  QObject::connect( timerUpdate_, SIGNAL(timeout()), this, SLOT( slotUpdate() ) );
  QObject::connect( ui_->searchButton, SIGNAL(clicked()), SLOT( slotSearchButton() ) );
  QObject::connect( ui_->makeButton, SIGNAL(clicked(bool)), SLOT( slotMakeButton(bool) ) );
  QObject::connect( ui_->setRamkaButton, SIGNAL(clicked()), SLOT( slotSetRamka() ) );
  QObject::connect( ui_->setButton, SIGNAL(clicked()), SLOT( slotSetInfo() ) );
  QObject::connect( ui_->graphButton, SIGNAL(clicked()), SLOT( slotStopTimer() ) );
  QObject::connect( ui_->graphButton, SIGNAL(clicked()), SLOT( slotGetGraph() ) );
  ui_->searchButton->setIcon(QIcon(":/mappi/icons/refresh.gif"));
  ui_->searchButton->setIconSize(QSize(16,16));
  ui_->makeButton->setIcon(QIcon(":/mappi/icons/connect.png"));
  ui_->makeButton->setIconSize(QSize(16,16));
  ui_->addrLabel->setText("Не подключено");
  ui_->addrLabel->setStyleSheet("QLabel {  color : red; }");
}

UhdControl::~UhdControl()
{
  delete ui_;
}

void UhdControl::init()
{
  uhd::set_thread_priority_safe();
  loadSettings();
  ui_->setButton->setDisabled(true);
  ui_->makeButton->setDisabled(true);
  ui_->graphButton->setDisabled(true);
  slotSearchButton();
  graph_->setFocus();
  QObject::connect(ui_->deviceBox, SIGNAL(currentIndexChanged(int)), SLOT(slotChangeDevice(int)));
}



void UhdControl::slotSearchButton()
{
  findDevices();
  if (0 == amountDevices_) {
    ui_->addrLabel->setText("Не подключено");
    ui_->addrLabel->setStyleSheet("QLabel {  color : red; }");
    ui_->deviceBox->clear();
    ui_->deviceBox->addItem("Устройств нет");
    ui_->setButton->setDisabled(true);
    ui_->makeButton->setDisabled(true);
    ui_->graphButton->setDisabled(true);
  }
  else {
    ui_->deviceBox->clear();
    QString text = "IP: ";
    uhd::device_addr_t fullAddr;
    for (int i=0; i<amountDevices_;i++) {
      fullAddr = device_addrs_[i];
      QString stringAddr = QString::fromStdString( fullAddr.to_string());
      QStringList addrlist = stringAddr.split(",");
      stringAddr =addrlist[1];
      ui_->deviceBox->addItem(text + stringAddr.remove(0,5),i);
    }
    ui_->addrLabel->setText( getInfoString() );
    ui_->addrLabel->setStyleSheet("QLabel {  color : grey; }");
    ui_->makeButton->setEnabled(true);
  }
}

void UhdControl::slotMakeButton(bool isSet)
{
  if (true == isSet) {
    try {
      if (0 != device_addrs_.size()) {
        makeDevice(ui_->deviceBox->currentData().toInt());
        ui_->setButton->setEnabled(true);
        ui_->graphButton->setEnabled(true);
        ui_->addrLabel->setText( getInfoString() );
        ui_->addrLabel->setStyleSheet("QLabel {  color : green; }");
        ui_->makeButton->setToolTip("Отключение от устройства");
      }
    }
    catch (const std::exception &exp) {
      Q_UNUSED(exp);
      error_msg.operator <<("Подключение не удалось");
      ui_->makeButton->setChecked(false);
      ui_->makeButton->setToolTip("Подключение к устройству");
    }
  }
  else {
    slotDisconnect();
  }
}

QString UhdControl::getInfoString()
{
  QString text = pbtools::toQString( device_addrs_[ui_->deviceBox->currentData().toInt()].to_string() );
  QStringList infoList= text.split(",");
  int i = 0;
  for (i = 0; i < infoList.size()-1;i++) {
    text = infoList[i];
    infoList[i] = text.remove(0,5);
  }
  text = infoList[i];
  infoList[i] = text.remove(0,7);
  text.clear();
  text = QString("Тип: ").append(infoList[0]).append("\n");
  text = text.append( "IP: ").append(infoList[1]).append("\n");
  text = text.append("Имя: ").append(infoList[2]).append("\n");
  text = text.append("Серийный № ").append(infoList[3]).append("\n");
  return text;
}

void UhdControl::slotChangeDevice(int index)
{
  Q_UNUSED(index);
  slotDisconnect();
}

void UhdControl::slotSetRamka()
{
  setRamka();
}

void UhdControl::slotSetInfo()
{
  if (0 != usrp_) {
    setInfo();
    getInfo();
    setRamka();
    saveSettings();
    ui_->graphButton->setChecked(true);
    slotGetGraph();
  }
}

void UhdControl::slotGetGraph()
{
  if( true == ui_->graphButton->isChecked() ) {
    setRamka();
    timerUpdate_->start(75);
  }

}


void UhdControl::slotUpdate()
{
  try {
   update();
  }
  catch (const std::exception &exp) {
    Q_UNUSED(exp);
    debug_log<< "Привет " <<exp.what();
    error_msg.operator <<("Нет соединения с устройством");
    slotDisconnect();
    ui_->graphButton->setChecked(false);
    timerUpdate_->stop();
    graph_->setFocus();
  }
}

void UhdControl::update()
{
  QString fftSizeText;
  fftSizeText=ui_->fftBox->currentText();
  fftSize_= fftSizeText.toInt();
  QVector<std::complex<double> > samples=getSample();
  QVector<std::complex<double> > fftsamples=getfftw(samples);
  QMap <double,double> amplitude=getAmplitude(fftsamples);
  amplitude=smooth(amplitude);
  layer_->setAmplitude(amplitude);
  graph_->mapview()->setCacheMode(QGraphicsView::CacheNone);
  graph_->mapscene()->invalidate();
  if (false==first_) {
    setRamka();
    first_=true;
  }
}


void UhdControl::slotStopTimer()
{
  if ( false == ui_->graphButton->isChecked()) {
    timerUpdate_->stop();
  }
}

QVector<std::complex<double> > UhdControl::getSample()
{
  QVector<std::complex<double> > buffItog(fftSize_);
  bool flagExit=true;
  for(int j=0; j<buffItog.size() && true==flagExit;) {
    //create a receive streamer
    uhd::stream_args_t stream_args("fc32"); //complex floats
    uhd::rx_streamer::sptr rx_stream = usrp_->get_rx_stream(stream_args);

    //setup streaming
    unsigned int total_num_samps=10000;
    uhd::stream_cmd_t stream_cmd(uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE  );
    stream_cmd.num_samps = total_num_samps;
    stream_cmd.stream_now = true;
    static const double reasonable_delay = 0.1; //order of magnitude over RTT
    stream_cmd.time_spec = usrp_->get_time_now() + ::uhd::time_spec_t(reasonable_delay);
    rx_stream->issue_stream_cmd(stream_cmd);

    //loop until total number of samples reached
    size_t num_acc_samps = 0; //number of accumulated samples
    uhd::rx_metadata_t md;
    QVector<std::complex<float> > buff(rx_stream->get_max_num_samps());
    usrp_->set_rx_iq_balance(0.0);
    usrp_->set_rx_dc_offset(true);
    while((num_acc_samps < total_num_samps) && true==flagExit){
      size_t num_rx_samps = rx_stream->recv(
            &buff.front(), buff.size(), md
            );

      //handle the error codes
      switch(md.error_code){
      case uhd::rx_metadata_t::ERROR_CODE_NONE:
        break;

      case uhd::rx_metadata_t::ERROR_CODE_TIMEOUT:
        if (num_acc_samps == 0) continue;
        std::cout << "Got timeout before all samples received, possible packet loss, exiting loop..."  << std::endl;
        flagExit=false;
        break;

      default:
        std::cout << "Got error code " << md.error_code << " exiting loop..."   << std::endl;
        flagExit=false;
        error_msg.operator <<("Ошибка приёма. Проверьте парметры");
        ui_->graphButton->setChecked(false);
        timerUpdate_->stop();
        break;
      }
      for (int i=0; (i<(buff.size())) && (j<buffItog.size());i++) {
        buffItog[j]=buff[i];
        j++;
      }
      num_acc_samps += num_rx_samps;
    }
  }
  return buffItog;
}


QVector<std::complex<double> > UhdControl::getfftw(QVector<std::complex<double> >& sample)
{
  int sampleSize=sample.size();
  fftw_complex in[sampleSize], out[sampleSize];
  fftw_plan p;
  float filter=1.0;
  for ( int i=0;i<sampleSize;i++) {
    filter=0.5*(1-cos((2*3.141*i)/(sampleSize-1) ) );
    in[i][0]=(sample[i].real() * filter);
    in[i][1]=(sample[i].imag() * filter);
  }
  p=fftw_plan_dft_1d(sampleSize, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(p);
  fftw_destroy_plan(p);
  fftw_cleanup();
  int j=sampleSize/2;
  int i=0;
  QVector<std::complex<double> > fftSample(sampleSize);
  for ( ;i<sampleSize/2;i++, j++) {
    std::complex<double> buff(out[j][0],out[j][1]);
    fftSample[i]=buff;
  }
  for (j=0;i<sampleSize;i++, j++) {
    std::complex<double> buff(out[j][0],out[j][1]);
    fftSample[i]=buff;
  }
  return fftSample;
}

QMap <double,double> UhdControl::getAmplitude(QVector<std::complex<double> >& fftsample)
{
  int sampleSize=fftsample.size();
  double df, freqBegin;
  freqBegin=(usrp_->get_rx_freq())-(usrp_->get_rx_rate()/2);
  df=usrp_->get_rx_rate()/sampleSize;
  double first,second;
  QMap <double,double> ampl;
  for ( int i=0;i<sampleSize;i++) {
    second=(sqrt( (fftsample[i].real()*fftsample[i].real()) + (fftsample[i].imag()*fftsample[i].imag()) ))/fftSize_;
    if (0!=second) {
      second=10*log10(second);
    }
    first=freqBegin+i*df;
    ampl.insert(first,second);
  }
  return ampl;
}


QMap<double, double> UhdControl::smooth (QMap<double, double> &sample)
{
  if (oldfftSize_ == fftSize_) {
    QMap <double,double>::iterator itMap=sample.begin();
    QMap <double,double>::iterator itMapOld=oldSample_.begin();
    double r;
    r=ui_->rSpinBox->value();
    kalman::FilterKalman* kalman = new kalman::FilterKalman(0.1,r,1,1);
    for (itMap=sample.begin();itMap!=sample.end();itMap++,itMapOld++) {
      kalman->setState(itMapOld.value(),0.1);
      kalman->correct(itMap.value());
      itMap.value()=kalman->getState();
    }
    oldSample_=sample;
    return sample;
  }
  else {
    oldfftSize_=fftSize_;
    oldSample_=sample;
    return sample;
  }
}

void UhdControl::findDevices()
{
  uhd::device_addr_t helpAddr("");
  device_addrs_ = uhd::device::find(helpAddr);
  amountDevices_=device_addrs_.size();
}

void UhdControl::makeDevice(int i)
{
  usrp_ = uhd::usrp::multi_usrp::make(device_addrs_[i]);
  usrp_->set_rx_rate(5*1e6);
  usrp_->set_rx_bandwidth(50*1e6);
  getInfo();
}

void UhdControl::getInfo()
{
  ui_->samplRateSpineBox->setValue(usrp_->get_rx_rate()*1e-6);
  ui_->gainSpineBox->setValue(usrp_->get_rx_gain());
  //ui_->gainSpineBox->setValue(usrp_->get_normalized_rx_gain()*100);
  ui_->refFrqSpineBox->setValue(usrp_->get_rx_freq()*1e-6);
  //ui_->bandWidthSpineBox->setValue(usrp_->get_rx_bandwidth()*1e-6);

}

void UhdControl::setInfo()
{
  //usrp_->set_rx_bandwidth(ui_->bandWidthSpineBox->value()*1e6);
  usrp_->set_rx_rate(ui_->samplRateSpineBox->value()*1e6);
  usrp_->set_rx_freq(ui_->refFrqSpineBox->value()*1e6);
  usrp_->set_rx_gain(ui_->gainSpineBox->value());
  //usrp_->set_normalized_rx_gain(ui_->gainSpineBox->value()/100);
}

void UhdControl::slotDisconnect()
{
  ui_->graphButton->setChecked(false);
  slotStopTimer();
  ui_->setButton->setDisabled(true);
  ui_->graphButton->setDisabled(true);
  usrp_=nullptr;
 // ui_->addrLabel->setText("Не подключено");
  ui_->addrLabel->setStyleSheet("QLabel {  color : grey; }");
  ui_->makeButton->setChecked(false);
  ui_->makeButton->setToolTip("Подключение к устройству");
}

void UhdControl::setRamka()
{
  double freqBegin, freqEnd, minAmpl, maxAmpl;
  if ( 0 != proj_ && 0 != usrp_) {
    minAmpl=layer_->minAmpl();
    maxAmpl=layer_->maxAmpl();
    freqBegin=( (usrp_->get_rx_freq())-(usrp_->get_rx_rate()/2.0) )*1e-6;
    freqEnd=( (usrp_->get_rx_freq())+(usrp_->get_rx_rate()/2.0) )*1e-6;
    proj_->setRamka(freqBegin,minAmpl,freqEnd,maxAmpl);
    proj_->setEnd(meteo::GeoPoint(freqEnd,minAmpl,0,meteo::LA_GENERAL));
    proj_->setStart(meteo::GeoPoint(freqBegin,maxAmpl,0,meteo::LA_GENERAL));
  }
  if ( 0 != usrp_) {
    meteo::GeoPoint gp( usrp_->get_rx_freq()*1e-6, minAmpl+(maxAmpl-minAmpl)/2, 0, meteo::LA_GENERAL);
    graph_->document()->setScreenCenter(gp);
    layer_->calcYAxisRange();
    layer_->calcXAxisRange(freqBegin, freqEnd);
    graph_->mapview()->setCacheMode(QGraphicsView::CacheNone);
    graph_->mapscene()->invalidate();
  }
}

void UhdControl::saveSettings()
{
  QSettings settings( (QDir::homePath() + "/.meteo/uhdControl.ini"), QSettings::IniFormat );
  settings.beginGroup( "User settings" );
  settings.setValue( "rate", ui_->samplRateSpineBox->value() );
  settings.setValue( "freq", ui_->refFrqSpineBox->value());
  settings.setValue( "gain", ui_->gainSpineBox->value() );
  //settings.setValue( "bandWidth", ui_->bandWidthSpineBox->value() );
  settings.setValue( "smooth", ui_->rSpinBox->value());
  settings.setValue( "fftSize", ui_->fftBox->findText(ui_->fftBox->currentText()) );
  settings.endGroup();
}

void UhdControl::loadSettings()
{
  QSettings settings((QDir::homePath() + "/.meteo/uhdControl.ini"), QSettings::IniFormat );
  settings.beginGroup( "User settings" );
  ui_->samplRateSpineBox->setValue( settings.value( "rate",-1 ).toDouble() );
  ui_->refFrqSpineBox->setValue( settings.value( "freq", -1 ).toDouble() );
  ui_->gainSpineBox->setValue( settings.value( "gain", -1).toDouble() );
  //ui_->bandWidthSpineBox->setValue( settings.value( "bandWidth", -1 ).toDouble() );
  ui_->rSpinBox->setValue( settings.value("smooth",-1).toDouble());
  if (-1 !=settings.value("fftSize", -1).toInt() ) {
  ui_->fftBox->setCurrentIndex(settings.value("fftSize", -1).toInt());
  }
  else {
    ui_->fftBox->setCurrentIndex(0);
  }
  settings.endGroup();
}


void UhdControl::closeEvent(QCloseEvent *ev)
{
  ev->ignore();
  if( 0 == QMessageBox::question( this, tr("Выход"), trUtf8("Закрыть тест аппаратуры?"), trUtf8("Да"), trUtf8("Нет"))){
    graph_->setFlExit(true);
    graph_->close();
  }
  else{
    graph_->setFlExit(false);
  }
  ev->ignore();
}



void UhdControl::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Escape)
  {
    if( 0 == QMessageBox::question( this, tr("Выход"), trUtf8("Закрыть окно: \"Тест аппаратуры приёма\"?"), trUtf8("Да"), trUtf8("Нет"))){
      graph_->setFlExit(true);
      graph_->close();
    }
    else {
      graph_->setFlExit(false);
    }
  }
}
