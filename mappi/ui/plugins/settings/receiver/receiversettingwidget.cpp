#include "receiversettingwidget.h"
#include "ui_receiversettingwidget.h"

#include <mappi/settings/mappisettings.h>
#include <mappi/schedule/schedulehelper.hpp>

//static const QString kReceptionConf = "/reception.conf";

namespace mappi {

ReceiverSettingWidget::ReceiverSettingWidget(QWidget *parent)
  : SettingWidget(parent)
  , ui_(new Ui::ReceiverSettingWidget)
{
  title_ = QObject::tr("Приёмник");
  ui_->setupUi(this);

  slotLoadConf();

  QObject::connect(ui_->nameEdt,      &QLineEdit::textChanged,          this, &SettingWidget::slotChanged);
  QObject::connect(ui_->geoPoinEdt,   &GeoPointEditor::tryChanged,      this, &SettingWidget::slotChanged);
  QObject::connect(ui_->antennaCbox,  static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged)
                 , this, &SettingWidget::slotChanged);
  QObject::connect(ui_->azimutSpn,    static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged)
                 , this, &SettingWidget::slotChanged);
  QObject::connect(ui_->compasSpn,    static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged)
                 , this, &SettingWidget::slotChanged);
  QObject::connect(ui_->navigatorSpn, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged)
                 , this, &SettingWidget::slotChanged);
}

ReceiverSettingWidget::~ReceiverSettingWidget()
{
  delete ui_;
}

void ReceiverSettingWidget::slotLoadConf()
{
  auto reception = ::mappi::inter::Settings::instance()->reception();
  if(reception.has_site()) {
    if(reception.site().has_name())
      ui_->nameEdt->setText(QObject::tr(reception.site().name().data()));

    if(reception.site().has_point()) {
      if(reception.site().point().has_height_meters())
        ui_->geoPoinEdt->setAltitudeMeter(reception.site().point().height_meters());
      if(reception.site().point().has_lat_radian())
        ui_->geoPoinEdt->setLatRadian(reception.site().point().lat_radian());
      if(reception.site().point().has_lon_radian())
        ui_->geoPoinEdt->setLonRadian(reception.site().point().lon_radian());
    }
  }

  if(reception.antenna().has_azimut_correct())
    ui_->azimutSpn->setValue(reception.antenna().azimut_correct());

  if(reception.has_antenna()) {
    // if(reception.antenna().has_port())
      ui_->antennaCbox->setCurrentText(reception.antenna().serial_port().tty().c_str());
    //if(reception.ports().has_compas_port())
      ui_->compasSpn->setValue(0);
      //if(reception.ports().has_nav_port())
      ui_->navigatorSpn->setValue(0);
  }

  Q_EMIT(changed(false));
}

void ReceiverSettingWidget::slotSaveConf() const
{
  auto reception = ::mappi::inter::Settings::instance()->reception();
  reception.mutable_site()->set_name(std::string(ui_->nameEdt->text().toUtf8().data()));

  reception.mutable_site()->mutable_point()->set_height_meters(ui_->geoPoinEdt->altitudeMeter());
  reception.mutable_site()->mutable_point()->set_lat_radian      (ui_->geoPoinEdt->latRadian());
  reception.mutable_site()->mutable_point()->set_lon_radian      (ui_->geoPoinEdt->lonRadian());

  reception.mutable_antenna()->set_azimut_correct(ui_->azimutSpn->value());

//  reception.mutable_antenna()->set_port(std::string(ui_->antennaCbox->currentText().toUtf8().data()));
  // reception.mutable_ports()->set_compas_port(ui_->compasSpn->value());
  // reception.mutable_ports()->set_nav_port   (ui_->navigatorSpn->value());

  ::mappi::inter::Settings::instance()->saveReception(&reception); //(MnCommon::etcPath() + kReceptionConf, &reception);

  schedule::ScheduleHelper::refreshSchedule();

  Q_EMIT(changed(false));
}


} //mappi
