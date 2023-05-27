#include "schedulesettingwidget.h"
#include "ui_schedulesettingwidget.h"
#include "satelliteeditor.h"

#include <qsettings.h>

#include <mappi/settings/mappisettings.h>
#include <mappi/schedule/schedulehelper.hpp>

namespace mappi {
/*

//! Способ разрешения конфликта (сессия)
enum ConflResol {
  kUnkResol = -1;       //!< Неизвестно
  kStartResol = 0;      //!< Начало сеанса
  kDurationResol = 1;   //!< Продолжительность сеанса
  kWestResol = 2;       //!< Западный регион
  kEastResol = 3;       //!< Восточный регион
};

//! Состояние конфликта (сессия)
enum ConflState {
  kUnkState = -1;       //!< Неизвестно
  kNormalState = 0;     //!< Не конфликтует
  kSelfState = 1;       //!< Конфликт решён в пользу этого спутника
  kOtherState = 2;      //!< Конфликт решён в пользу другого спутника (т.е. этот д.б. исключён из расписания)
  kUserSelfState = 3;   //!< Конфликт решён пользователем в пользу этого спутника
  kUserOtherState = 4;  //!< Конфликт решён пользователем в пользу другого спутника
};
*/

  //static const QString kReceptionConf = "/reception.conf";
static const QString kSettings      = QDir::homePath() + "/.meteo/mappi/settings.ini";

  //TODO так не подгружаются, вбито в коде ниже при вызове
// static QPixmap kEnableAdopt (":/mappi/icons/run_satellite_receiver.png");
// static QPixmap kDisableAdopt(":/mappi/icons/no_satellite_receiver.png");

static QMap<mappi::conf::ConflResol, QString> kPriorType;
static QMap<mappi::conf::SatType,   QString> kSatType;

ScheduleSettingWidget::ScheduleSettingWidget(QWidget *parent)
  : SettingWidget(parent)
  , ui_(new Ui::ScheduleSettingWidget)
  , satelliteEditor_(new SatelliteEditor(this))
{
  title_ = QObject::tr("Расписание");

  kPriorType.insert(conf::kUnkResol,    QObject::tr("Приоритет неопределен"));
  kPriorType.insert(conf::kStartResol,  QObject::tr("Начало сеанса"));
  kPriorType.insert(conf::kDurationResol, QObject::tr("Продолжительность сеанса"));
  kPriorType.insert(conf::kWestResol,   QObject::tr("Западный регион"));
  kPriorType.insert(conf::kEastResol,   QObject::tr("Восточный регион"));

  kSatType.insert(conf::kUnkSatType, "Неизвестный тип орбиты");
  kSatType.insert(conf::kOrbitalSat, "Солнечно-синхронная");
  kSatType.insert(conf::kGeostatSat, "Геостационарная");
  kSatType.insert(conf::kAnySatType, "Любой тип орбиты");

  ui_->setupUi(this);

  for(auto it = kPriorType.begin(); it != kPriorType.end(); ++it) {
    if(it.key() > conf::kUnkResol) {
      ui_->conflictCbox->insertItem(it.key(), it.value());
    }
  }
  ui_->satelliteTbl->hideColumn(kUseles);

  slotLoadConf();
  loadSettings(kSettings);

  QObject::connect(ui_->satelliteTbl, &QTableWidget::cellClicked,       this, &ScheduleSettingWidget::slotToggleSat);
  QObject::connect(ui_->satelliteTbl, &QTableWidget::cellDoubleClicked, this, &ScheduleSettingWidget::slotEditSat);
  QObject::connect(ui_->addSatBtn,    &QPushButton::clicked,            this, &ScheduleSettingWidget::slotAddSat);
  QObject::connect(ui_->removeSatBtn, &QPushButton::clicked,            this, &ScheduleSettingWidget::slotRemoveSat);
  QObject::connect(satelliteEditor_,  &SatelliteEditor::saveSatellite,  this, &ScheduleSettingWidget::slotSaveSatellite);

  QObject::connect(ui_->periodSpn,      static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                   this, &ScheduleSettingWidget::slotChanged);
  QObject::connect(ui_->conflictCbox,   static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                   this, &ScheduleSettingWidget::slotChanged);
  QObject::connect(ui_->culminationSpn, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                   this, &ScheduleSettingWidget::slotChanged);
  QObject::connect(ui_->placeSpn,       static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                   this, &ScheduleSettingWidget::slotChanged);

}

ScheduleSettingWidget::~ScheduleSettingWidget()
{
  saveSettings(kSettings);
  delete ui_;
}

void ScheduleSettingWidget::loadSettings(const QString& filename)
{
  QFile file(filename);
  if(false == file.exists()) return;
  QSettings settings(filename, QSettings::IniFormat);
  ui_->satelliteTbl->horizontalHeader()->restoreState(settings.value("schedule/satellite").toByteArray());
  satelliteEditor_->restoreGeometry(settings.value("schedule/satelliteeditor").toByteArray());
  loadedSettings_ = true;
}
void ScheduleSettingWidget::saveSettings(const QString& filename) const
{
  QSettings settings(filename, QSettings::IniFormat);
  settings.setValue("schedule/satellite", ui_->satelliteTbl->horizontalHeader()->saveState());
  settings.setValue("schedule/satelliteeditor", satelliteEditor_->saveGeometry());
}

void ScheduleSettingWidget::slotToggleSat(int row, int col)
{
  static QPixmap kEnableAdopt (":/mappi/icons/run_satellite_receiver.png");
  static QPixmap kDisableAdopt(":/mappi/icons/no_satellite_receiver.png");

  if(kEnable == col && row >= 0 && row < satellites_.size()) {
    if(ui_->satelliteTbl->item(row, kUseles)->checkState() == Qt::Checked) {
      ui_->satelliteTbl->item(row, col)->setIcon(kDisableAdopt);
      ui_->satelliteTbl->item(row, kUseles)->setCheckState(Qt::Unchecked);
    }
    else {
      ui_->satelliteTbl->item(row, col)->setIcon(kEnableAdopt);
      ui_->satelliteTbl->item(row, kUseles)->setCheckState(Qt::Checked);
    }
    Q_EMIT(changed(true));
  }
}

void ScheduleSettingWidget::slotEditSat(int row, int col)
{
  Q_UNUSED(col);
  editName_ = ui_->satelliteTbl->item(row, kName)->text();
  if(satellites_.end() != satellites_.find(editName_)) {
    editRow_ = row;
    satelliteEditor_->open(satellites_[editName_], kSatType, satellites_.keys());
  }
}

void ScheduleSettingWidget::slotAddSat()
{
  editRow_ = ui_->satelliteTbl->rowCount();
  editName_ = "";
  satelliteEditor_->open(conf::ReceptionParam(), kSatType, satellites_.keys());
}

void ScheduleSettingWidget::slotRemoveSat()
{
  const int row = ui_->satelliteTbl->currentRow();
  auto it = satellites_.find(ui_->satelliteTbl->item(row, kName)->text());
  if(satellites_.end() != it) {
    satellites_.erase(it);
    ui_->satelliteTbl->removeRow(row);
  }
}

void ScheduleSettingWidget::slotSaveSatellite(const conf::ReceptionParam& sat)
{
  satellites_.remove(editName_);
  satellites_.insert(QString::fromStdString(sat.name()), sat);
  if(editRow_ >= ui_->satelliteTbl->rowCount()) {
    ui_->satelliteTbl->insertRow(editRow_);
    ui_->satelliteTbl->setItem(editRow_, kName, createCell(kName, sat));
    ui_->satelliteTbl->setItem(editRow_, kType, createCell(kType, sat));
    ui_->satelliteTbl->setItem(editRow_, kEnable, createCell(kEnable, sat, false));
    ui_->satelliteTbl->setItem(editRow_, kUseles, createCell(kUseles, sat, false));
  }
  else
  {
    fillCell(ui_->satelliteTbl->item(editRow_, kName), kName, sat);
    fillCell(ui_->satelliteTbl->item(editRow_, kType), kType, sat);
  }
  Q_EMIT(changed(true));
}

void ScheduleSettingWidget::slotLoadConf()
{
  auto reception = ::mappi::inter::Settings::instance()->reception();
  int row = 0;
  //bool has_sched = false;//reception.has_sched();

  satellites_.clear();
  for(auto sat : reception.satellite()) {
    if(false == sat.has_name()) continue;
    if(ui_->satelliteTbl->rowCount() <= row)
      ui_->satelliteTbl->insertRow(row);
    satellites_.insert(QString::fromStdString(sat.name()), sat);
    ui_->satelliteTbl->setItem(row, kName, createCell(kName, sat));
    ui_->satelliteTbl->setItem(row, kType, createCell(kType, sat));
    bool isUses = /*has_sched ? isUsesSatellite(sat, reception.sched()) :*/ false;
    ui_->satelliteTbl->setItem(row, kEnable, createCell(kEnable, sat, isUses));
    ui_->satelliteTbl->setItem(row, kUseles, createCell(kUseles, sat, isUses));
    row++;
  }
  if(ui_->satelliteTbl->rowCount() != row)
    ui_->satelliteTbl->setRowCount(row);
  if(false == loadedSettings_)
    ui_->satelliteTbl->resizeColumnsToContents();
/*
  if(has_sched) {
    if(reception.sched().has_priority())
      ui_->conflictCbox->setCurrentText(kPriorType.value(reception.sched().priority()));
    if(reception.sched().has_elmax())
      ui_->culminationSpn->setValue(reception.sched().elmax());
    if(reception.sched().has_elmin())
      ui_->placeSpn->setValue(reception.sched().elmin());
    if(reception.sched().has_period())
      ui_->periodSpn->setValue(reception.sched().period());
  }
*/
  Q_EMIT(changed(false));
}
void ScheduleSettingWidget::slotSaveConf() const
{
  auto reception = ::mappi::inter::Settings::instance()->reception();

  reception.clear_satellite();
  for(auto sat : satellites_)
    reception.add_satellite()->CopyFrom(sat);

  conf::ScheduleConf sched;
  for(int row = 0, sz = ui_->satelliteTbl->rowCount(); row < sz; ++row)
    if(ui_->satelliteTbl->item(row, kUseles)->checkState() == Qt::Checked)
      *sched.mutable_satellite()->add_item() = ui_->satelliteTbl->item(row, kName)->text().toStdString();

  sched.mutable_elevation()->set_max(ui_->culminationSpn->value());
  sched.mutable_elevation()->set_min(ui_->placeSpn->value());
  sched.set_period(ui_->periodSpn->value());
  sched.set_resol(kPriorType.key(ui_->conflictCbox->currentText()));
//  reception.mutable_sched()->CopyFrom(sched);

  ::mappi::inter::Settings::instance()->saveReception(&reception); 
  //meteo::mappi::TMeteoSettings::instance()->save(MnCommon::etcPath() + kReceptionConf, &reception);

  schedule::ScheduleHelper::refreshSchedule();

  Q_EMIT(changed(false));
}

bool ScheduleSettingWidget::isUsesSatellite(const conf::ReceptionParam& sat, const conf::ScheduleConf& sched)
{
  if(false == sat.has_name())
    return false;
/*
  for(auto satName : sched.satellite())
    if(0 == satName.compare(sat.name()))
      return true;
*/
  return false;
}

QTableWidgetItem* ScheduleSettingWidget::createCell(Column col, const conf::ReceptionParam& sat, bool isUses)
{
  QTableWidgetItem* item = new QTableWidgetItem();
  fillCell(item, col, sat, isUses);
  return item;
}

void ScheduleSettingWidget::fillCell(QTableWidgetItem* item, Column col, const conf::ReceptionParam& sat, bool isUses)
{
  static QPixmap kEnableAdopt (":/mappi/icons/run_satellite_receiver.png");
  static QPixmap kDisableAdopt(":/mappi/icons/no_satellite_receiver.png");

  switch(col) {
  case kEnable:     //Ячейка  использования в расписании
    item->setIcon(isUses ? kEnableAdopt  :kDisableAdopt);
    break;
  case kUseles:     //Служебная ячейка использования в расписании (не знаю как спрятать checkbox в ячейки таблицы)
    item->setCheckState(isUses ? Qt::Checked : Qt::Unchecked);
    break;
  case kName:       //Ячейка названия спутника
    if(sat.has_name())
      item->setText(sat.name().data());
    break;
  case kType:       //Ячейка типа орбиты
    if(sat.has_type() && kSatType.end() != kSatType.find(sat.type()))
      item->setText(kSatType.value(sat.type()));
    break;
  }
}

} //mappi
