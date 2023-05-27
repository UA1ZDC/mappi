
#include "reobanalwidget.h"
#include "ui_reobanalwdgt.h"

//#include <commons/obanal/tfield.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/view/actions/selectaction.h>
#include <meteo/commons/global/gradientparams.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/layerpunch.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/ui/map/geotext.h>
#include <commons/textproto/pbtools.h>

#include <commons/obanal/haosmain.h>
#include <commons/obanal/func_obanal.h>
#ifdef debug_log
#undef debug_log
#endif


namespace meteo {
namespace map {

ReobanalWidget::ReobanalWidget(MapView* view) :
  QDialog(view),
  ui_(new Ui::ReobanalWidget),
  layer_(nullptr),
  all_data_(nullptr)
{
  //Qt::WindowFlags flags = Qt::Dialog;
  //setWindowFlags(flags);
  ui_->setupUi(this);
  QObject::connect(ui_->runBtn, SIGNAL(clicked(bool)), SLOT(slotRunObanal()));
  QObject::connect(ui_->cancelBtn, SIGNAL(clicked(bool)), SLOT(slotStopObanal()) );
  QObject::connect(ui_->closeBtn, SIGNAL(clicked(bool)), SLOT(slotClose()) );
  thread_ = new QThread;
  qRegisterMetaType<meteo::surf::DataRequest>();
  qRegisterMetaType<meteo::map::proto::WeatherLayer>();
  createObnl();
}

ReobanalWidget::~ReobanalWidget()
{
  thread_->quit();
  thread_->wait();
  delete all_data_;all_data_=nullptr;
  QApplication::restoreOverrideCursor();
}

void ReobanalWidget::setLayer( Layer *layer ){
  layer_ = layer;
  if(nullptr == layer_ ){ return;}
  if (kLayerPunch == layer_->type()) {
    ui_->nanoska_cB->setEnabled(true);
    ui_->nanoska_cB->setChecked(true);
  }else {
    ui_->nanoska_cB->setEnabled(false);
    ui_->nanoska_cB->setChecked(false);
  }
}

void ReobanalWidget::createObnl(){
  meteo::obanal::ObanalRuner *obnl = new meteo::obanal::ObanalRuner;
  obnl->moveToThread(thread_);
  connect(thread_, &QThread::finished, obnl, &QObject::deleteLater);
  connect(this, &ReobanalWidget::operate, obnl, &meteo::obanal::ObanalRuner::analyse);
  connect(this, &ReobanalWidget::operateData, obnl, &meteo::obanal::ObanalRuner::analyseData);
  connect(obnl, &meteo::obanal::ObanalRuner::resultReady, this, &ReobanalWidget::handleResults);
  ui_->cancelBtn->setEnabled(false);
  thread_->start();
}

void ReobanalWidget::slotStopObanal()
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor)) ;
  ui_->cancelBtn->setEnabled(false);
  thread_->quit();
  thread_->wait();
  ui_->statulabel->setText(QObject::tr("Обработка прервана"));
  createObnl();
  ui_->runBtn->setEnabled(true);
  ui_->closeBtn->setEnabled(true);
  QApplication::restoreOverrideCursor();
}

void ReobanalWidget::setWidget( ){
  QString ltype;
  int ntypel = info_.type_level();
  QMap< int, QString > ltypes = meteo::global::kLevelTypes();
  if(ltypes.contains(ntypel)) {
    ltype = ltypes[ntypel];
  }
  ui_->type_level_lbl->setText(ltype);

  QString lvl;
  if(0 == info_.level()){
    lvl =QObject::tr("Поверхность");
  } else {
    switch (ntypel) {
    case 100:
      lvl = QObject::tr(" %1 мбар").arg(info_.level());
    break;
    case 20:
    case 160:
    case 102:
    case 103:
    case 106:
    case 15000:
      lvl = QObject::tr(" %1 м").arg(info_.level());
    break;
    }
  }
  ui_->level_lbl->setText(lvl);
  QString hour = QString::number(info_.hour());
  if(0 == info_.hour()){
    ui_->hourCombo->setCurrentIndex(0);
  } else {
    for(int i = 0; i< ui_->hourCombo->count(); ++i){
      if(hour == ui_->hourCombo->itemText(i)){
        ui_->hourCombo->setCurrentIndex(i);
        break;
      }
    }
  }
  ui_->hourCombo->setEnabled(false);
  QDateTime dt;
  dt = QDateTime::fromString(QString::fromStdString(info_.datetime()),Qt::ISODate);
  ui_->dateEdit->setDate(dt.date());
  ui_->timeEdit->setTime(dt.time());
  ui_->dateEdit->setEnabled(false);
  ui_->timeEdit->setEnabled(false);
  ui_->value_combo->clear();
  for(int i = 0; i< info_.meteo_descr_size(); ++i){
    QString ds = TMeteoDescriptor::instance()->name(info_.meteo_descr(i));
    if(!ds.isEmpty()) ui_->value_combo->addItem(ds);
  }
}

void ReobanalWidget::setProp( const meteo::map::proto::WeatherLayer &info){
  info_.CopyFrom(info);
  setWidget();
}

void ReobanalWidget::setInfo(::obanal::TField * field){
  if(nullptr == field) return ;
  info_.set_mode(proto::kIsoline);
  info_.set_source(proto::kField);
  info_.set_center(field->getCenter());
  info_.set_center_name(field->getCenterName().toStdString());
  info_.set_model(field->getModel());
  QString ads = field->getDate().toString("dd.MM.yy hh:mm");
  info_.set_datetime(ads.toStdString());
  info_.set_hour(field->getHour()/3600);

}


void ReobanalWidget::obanalFromPuanson(descr_t ds){
  map::LayerPunch* l = maplayer_cast<map::LayerPunch*>(layer_);
  if(nullptr == l || (false == l->hasData())){   return;  }
  delete all_data_;
  all_data_ = new GeoData;
  l->getGeoData(all_data_,ds);
  meteo::map::proto::WeatherLayer vsinfo;
  vsinfo.CopyFrom(info_);
  vsinfo.clear_meteo_descr();
  vsinfo.add_meteo_descr(ds);
  emit operateData(all_data_, vsinfo);
  return;

  //emit operateData(all_data_, vsinfo);
}

void ReobanalWidget::obanalFromDb(descr_t ds){
  surf::DataRequest rdt;
  rdt.set_type_level(info_.type_level());
  rdt.set_level_p(info_.level());
  rdt.set_date_start(info_.datetime());
  rdt.add_meteo_descr(ds);
  /*int ds = info_.meteo_descr_size();
  for(int i=0; i< ds;++i){
      int descr = info_.meteo_descr(i);
      rdt.add_meteo_descr(descr);
  }*/
  emit operate(rdt);
}

void ReobanalWidget::slotRunObanal()
{
  if(nullptr == layer_ ){ return;}
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  QString ds_name = ui_->value_combo->currentText();
  descr_t ds  = TMeteoDescriptor::instance()->descriptor( ds_name);

  switch (layer_->type()) {
  case kLayerPunch:
    if(Qt::Checked == ui_->nanoska_cB->checkState()){
      obanalFromPuanson(ds);
    } else{
      obanalFromDb(ds);
    }
  break;
  case kLayerIso:
    obanalFromDb(ds);
  break;
  }
  ui_->runBtn->setEnabled(false);
  ui_->cancelBtn->setEnabled(true);
  ui_->closeBtn->setEnabled(false);
  ui_->statulabel->setText(tr("Выполняется анализ данных "));
  QApplication::restoreOverrideCursor();
}

void ReobanalWidget::handleResults(const QString & res){
  delete all_data_;all_data_=nullptr;
  if("1" == res){
    ui_->statulabel->setText(QObject::tr("Обработка данных завершена успешно"));
    QMessageBox::information(this, tr("Анализ данных"),
                             tr("Анализ данных завершен успешно\n\n"));
  } else {
    ui_->statulabel->setText(QObject::tr("Обработка данных завершена с ошибками"));

    QMessageBox::information(this, tr("Анализ данных"),
                             tr("Анализ данных завершен с ошибками\n\n"));
  }
  thread_->quit();
  thread_->wait();
  ui_->runBtn->setEnabled(true);
  ui_->cancelBtn->setEnabled(false);
  ui_->closeBtn->setEnabled(true);
}

void ReobanalWidget::slotClose(){

  close();
}

void ReobanalWidget::closeEvent(QCloseEvent *event){
  if(!ui_->closeBtn->isEnabled()){
    QMessageBox::information(this, tr("Анализ данных"),
                             tr("Производится анализ данных.\nНеобходимо дождаться завершения"));
    event->ignore();
  } else {
    QApplication::restoreOverrideCursor();
    event->accept();
  }
}



}
}
