
#include "prepeartransferwidget.h"
#include "ui_prepeartransferwidget.h"

#include <cross-commons/debug/tlog.h>
#include <commons/obanal/tfield.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/custom/stationwidget.h>

namespace meteo {
namespace map {

enum {
  Num,
  Date,
  Hour
};

PrepearTransferWidget::PrepearTransferWidget( Mode mode, QWidget* parent ) :
  QDialog(parent),
  ui_(new Ui::PrepearTransferWidget),
  mode_(mode),
  opt_(new QSettings(QDir::homePath() + "/." + MnCommon::applicationName() + "/transferwidget.ini", QSettings::IniFormat))
{
  ui_->setupUi(this);
  ui_->timeBox->setCurrentIndex(6);
  ui_->intervalBox->setCurrentIndex(3);
  ui_->timeEdit->setDisplayFormat("hh:mm");
  ui_->fieldBtn->setIcon(QIcon(":/meteo/icons/tools/find.png"));
  ui_->fieldBtn->setIconSize(QSize(16,16));
  ui_->fieldBtn->setToolTip(QObject::tr("Обновить"));
  ui_->dateEdit->setDateTime(QDateTime::currentDateTime());
  QTime time = QDateTime::currentDateTime().time();
  time.setHMS(time.hour(), 0, 0);
  ui_->timeEdit->setTime(time);
  QStringList lst;
  lst << QObject::tr("Номер") << QObject::tr("Срок") << QObject::tr("Срок прогноза");
  ui_->fieldsTree->setHeaderLabels(lst);
  ui_->fieldsTree->setRootIsDecorated(false);
  ui_->fieldsTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  createServices();
  slotDateTimeChanged();
  loadSettings();
  slotLabelBtnIconChange();

  QObject::connect( ui_->dateEdit, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(slotDateTimeChanged()) );
  QObject::connect( ui_->timeEdit, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(slotDateTimeChanged()) );
  QObject::connect( ui_->addBtn, SIGNAL(clicked()), SLOT(slotApplyClose()));
  QObject::connect( ui_->closeBtn, SIGNAL(clicked()), SLOT(slotClose()));
  QObject::connect( ui_->fieldBox, SIGNAL(currentIndexChanged(int)), SLOT(slotLevelChanged(int)));
  QObject::connect( ui_->labelBtn, SIGNAL(clicked(bool)), SLOT(slotLabelBtnIconChange()));
  QObject::connect( ui_->fieldBtn, SIGNAL(clicked(bool)), SLOT(slotFillFieldTree()));
}

PrepearTransferWidget::~PrepearTransferWidget()
{
  if(nullptr != advObj_reply_) {
    delete advObj_reply_;
    advObj_reply_= nullptr;
  }
  if(nullptr != ctrl_field_ ) {
    delete ctrl_field_;
    ctrl_field_ = nullptr;
  }
}

void PrepearTransferWidget::slotLevelChanged(int )
{
  int lvl = ui_->fieldBox->currentText().toInt();
  int cur_koef_indx = 0;
  switch(lvl){
    case 850:
      cur_koef_indx = ui_->koefBox->findText("0.9");
      break;
    case 700:
      cur_koef_indx = ui_->koefBox->findText("0.8");
      break;
    case 500:
      cur_koef_indx = ui_->koefBox->findText("0.6");
      break;
    default:
      cur_koef_indx = 0;
      break;
  }
  ui_->koefBox->setCurrentIndex(cur_koef_indx);
}

void PrepearTransferWidget::slotLabelBtnIconChange()
{
  if ( true == ui_->labelBtn->isChecked()) {
    ui_->labelBtn->setIcon(QIcon(":/meteo/icons/drawtools/tag_on.png"));
    ui_->labelBtn->setToolTip(QObject::tr("Убрать метки времени"));
  }
  else {
    ui_->labelBtn->setIcon(QIcon(":/meteo/icons/drawtools/tag_off.png"));
    ui_->labelBtn->setToolTip(QObject::tr("Установить метки времени"));
  }
}

void PrepearTransferWidget::slotFillFieldTree()
{
  ui_->fieldsTree->clear();

  QApplication::setOverrideCursor(Qt::WaitCursor);
  if ( true != calcObjectsAdvect()||
       nullptr == advObj_reply_) {
      QApplication::restoreOverrideCursor();
      return;
    }
    for(int i =0; i< advObj_reply_->descr_size(); ++i) {
      QTreeWidgetItem* item = new QTreeWidgetItem(ui_->fieldsTree);
      const meteo::field::DataDesc& d = advObj_reply_->descr(i);
      QString date_str = QString::fromStdString (d.date());
      QDateTime dt = QDateTime::fromString(date_str, Qt::ISODate);
      item->setText(Num, QString::number(i+1));
      item->setText(Date, dt.toString("dd.MM.yy hh:mm"));
      if(d.hour() > 0){
          item->setText(Hour, dt.addSecs(d.hour()).toString("dd.MM.yy hh:mm"));
        } else {
          item->setText(Hour, "Анализ");
        }
    }
  QApplication::restoreOverrideCursor();
}

bool PrepearTransferWidget::calcObjectsAdvect()
{
  if( !createServices() ) {
    return false;
  }
  if ( true == skeletMap_.isEmpty()  ) {
    return false;
  }
  meteo::field::AdvectObjectRequest request;
  request.set_date( dateTime().toString(Qt::ISODate).toStdString() );
  request.set_need_field_descr( true );
  request.set_need_prom_points( true );
  request.set_level(field());
  request.set_center(center());
  request.set_adv_time(trajectory()*time() * 3600);
  request.set_time_step(interval()*3600);
  request.set_koef(koef());
  foreach (QString uuid, skeletMap_.keys()) {
    meteo::field::Skelet* skelet = request.add_skelet();
    skelet->set_uuid(uuid.toStdString());
    GeoVector skel = skeletMap_[uuid];
    for (int i = 0, sz = skel.size(); i < sz; ++i ) {
      meteo::field::Skelet_Dot* dot = skelet->add_dots();
      dot->set_number(i);
      meteo::surf::Point* p = dot->add_coord();
      p->set_fi(skel.at(i).fi());
      p->set_la(skel.at(i).la());
      p->set_height(skel.at(i).alt());
      p->set_date_time(dateTime().toString(Qt::ISODate).toStdString());
    }
  }

  if(nullptr != advObj_reply_) {
    delete advObj_reply_;
    advObj_reply_ = nullptr;
  }
  advObj_reply_ = ctrl_field_->remoteCall( &meteo::field::FieldService::GetAdvectObject, request,  300000);
  if ( 0 == advObj_reply_ ) {
    error_log << QObject::tr("При попытке выполнить расчет перемещения частицы ответ от сервиса не получен");
    return false;
  }
  return true;
}

void PrepearTransferWidget::saveSettings()
{
  if ( nullptr != opt_) {
    opt_->setValue("time", ui_->timeBox->currentIndex());
    opt_->setValue("field", ui_->fieldBox->currentIndex());
    opt_->setValue("koef", ui_->koefBox->currentIndex());
    opt_->setValue("interval", ui_->intervalBox->currentIndex());
    opt_->setValue("label", ui_->labelBtn->isChecked());
  }
}

void PrepearTransferWidget::loadSettings()
{
  if ( nullptr != opt_) {
    if ( true == opt_->contains("time") ) {
      ui_->timeBox->setCurrentIndex( opt_->value("time").toInt() );
    }
    if ( true == opt_->contains("field") ) {
      ui_->fieldBox->setCurrentIndex( opt_->value("field").toInt() );
    }
    if ( true == opt_->contains("koef") ) {
      ui_->koefBox->setCurrentIndex( opt_->value("koef").toInt() );
    }
    if ( true == opt_->contains("interval") ) {
      ui_->intervalBox->setCurrentIndex( opt_->value("interval").toInt() );
    }
    if ( true == opt_->contains("label") ) {
      ui_->labelBtn->setChecked( opt_->value("label").toFloat() );
    }
  }
}

bool PrepearTransferWidget::createServices()
{
  if (nullptr != ctrl_field_) {
    delete ctrl_field_;
    ctrl_field_ = nullptr;
  }
  ctrl_field_ = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if(nullptr == ctrl_field_) {
    error_log.msgBox() << QObject::tr("Не удалось установить соединение с сервисом получения данных");
    return false;
  }
  return true;
}

GeoPoint PrepearTransferWidget::getStartGeoPoint( )
{
  return startPoint_;
}

void PrepearTransferWidget::setStartGeoPoint( const GeoPoint &agp)
{
  startPoint_ = agp;
  QMap<QString,GeoVector> skeletMap;
  skeletMap.insert("",GeoVector()<<agp);
  setSkeletMap( skeletMap);
}

void PrepearTransferWidget::slotDateTimeChanged ()
{
  if( nullptr == ctrl_field_ ){
    error_log << QObject::tr("Ошибка. Отсутствует подключение к сервису");
    return;
  }
  QDateTime dt = dateTime();
  QApplication::setOverrideCursor(Qt::WaitCursor);

  meteo::field::DataRequest request;
  if(trajectory() > 0){
    request.set_date_start(dt.toUTC().toString(Qt::ISODate).toStdString());
    request.set_date_end(dt.toUTC().addSecs(trajectory()*time() * 3600).toString(Qt::ISODate).toStdString());
  } else {
    request.set_date_end(dt.toUTC().toString(Qt::ISODate).toStdString());
    request.set_date_start(dt.toUTC().addSecs(trajectory()*time() * 3600).toString(Qt::ISODate).toStdString());
  }
  request.add_meteo_descr(10009);
  request.add_level(field());
  request.add_type_level(100);

  meteo::field::CentersResponse* reply = ctrl_field_->remoteCall(
        &meteo::field::FieldService::GetAvailableCentersForecast, request,  30000);
  if ( nullptr == reply ) {
    error_log << QObject::tr("При попытке выполнить расчет перемещения частицы ответ от сервиса не получен");
    QApplication::restoreOverrideCursor();
    return ;
  }

  ui_->ccenterComboBox->blockSignals(true);
  ui_->ccenterComboBox->clear();
  if(reply->info_size() < 1) {
    ui_->ccenterComboBox->clear();
    ui_->ccenterComboBox->blockSignals(false);
    QApplication::restoreOverrideCursor();
    return;
  }
  ui_->ccenterComboBox->clear();
  for(int i =0; i< reply->info_size(); ++i){
    if(reply->info(i).has_short_name() && reply->info(i).has_number())
      ui_->ccenterComboBox->addItem(QString::fromStdString(reply->info(i).short_name()),reply->info(i).number() );
  }
  ui_->ccenterComboBox->setCurrentIndex(0);
  ui_->ccenterComboBox->blockSignals(false);
  QApplication::restoreOverrideCursor();
}

QDateTime  PrepearTransferWidget::dateTime()
{
  QDateTime dataTime;
  dataTime.setDate(ui_->dateEdit->date());
  dataTime.setTime(ui_->timeEdit->time());
  return dataTime;
}

double PrepearTransferWidget::koef()
{
  return ui_->koefBox->currentText().toDouble();
}

int PrepearTransferWidget::center()
{
  return ui_->ccenterComboBox->currentData().toInt();
}

float PrepearTransferWidget::time()
{
  return ui_->timeBox->currentText().toFloat();
}

float PrepearTransferWidget::interval()
{
  return ui_->intervalBox->currentText().toFloat();
}

int PrepearTransferWidget::field()
{
  return ui_->fieldBox->currentText().toInt();
}

int PrepearTransferWidget::trajectory()
{
  if( ui_->backRadio->isChecked() ) {
    return -1;
  }
  else {
    return 1;
  }
}

void PrepearTransferWidget::slotApplyClose()
{
  saveSettings();
  QApplication::setOverrideCursor(Qt::WaitCursor);
  if ( nullptr != advObj_reply_ || true == calcObjectsAdvect() ) {
    if ( Mode::kObjects == mode_) {
        emit applyObjectsTraj();
      } else  emit applyTraj();
  }
  QApplication::restoreOverrideCursor();
  close();
}

void PrepearTransferWidget::slotClose()
{
  if(nullptr != advObj_reply_) {
    delete advObj_reply_;
    advObj_reply_=nullptr;
  }
  close();
}

bool PrepearTransferWidget::markersIsVisible()
{
  return ui_->labelBtn->isChecked();
}

}
}
