#include "obanaloptions.h"
#include "ui_obanaloptions.h"

#include "selectcenters.h"
#include "selectdatatypes.h"
#include "selectlevels.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/planner/timesheet.h>
#include <meteo/commons/proto/obanal.pb.h>
#include <meteo/commons/ui/obanalsettings/timesheeteditor/timesheeteditor.h>

#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

namespace {

const QMap<int, QString>& basicShedules()
{
  static QMap<int, QString> ts;
  if (ts.isEmpty() == true) {
    ts.insert(1, QString::fromUtf8("каждый 1 час"));
    ts.insert(3, QString::fromUtf8("каждые 3 часа"));
    ts.insert(6 ,QString::fromUtf8("каждые 6 часов"));
    ts.insert(12, QString::fromUtf8("каждые 12 часов"));
    ts.insert(24, QString::fromUtf8("каждые 24 часа"));
  }
  return ts;
}

int defaultShedule() { return 6; }
QString defaultLineEditText() { return QString::fromUtf8("Все"); }
meteo::Timesheet defaultTimesheet() { return meteo::Timesheet::fromString("* * * * *"); }

meteo::Timesheet everyHourTimesheet()
{
  meteo::Timesheet ts = defaultTimesheet();
  ts.setMinute("0");
  return ts;
}

meteo::Timesheet everyDayTimesheet()
{
  meteo::Timesheet ts = everyHourTimesheet();
  ts.setHour("0");
  return ts;
}

meteo::Timesheet everyNthHourTimesheet(int hour)
{
  meteo::Timesheet ts = everyHourTimesheet();
  if (hour == 1) {
    qt_noop();
  }
  else if (hour == 24) {
    ts = everyDayTimesheet();
  }
  else {
    ts.setHour(QString("*/%1").arg(hour));
  }
  return ts;
}

}

//using namespace novost;

namespace meteo {

ObanalOptions::ObanalOptions(QWidget* parent) :
  QDialog(parent),
  ui_(new Ui::ObanalOptions()),
  currentTimesheet_(new Timesheet())
{
  ui_->setupUi(this);

  QDateTime dt(QDate::currentDate(), QTime(0,0,0));
  ui_->endDateTimeEdit->setDateTime(dt);
  ui_->startDateTimeEdit->setDateTime(dt.addDays(-1));

  ui_->timesheetComboBox->addItems(QStringList(::basicShedules().values()));

  connect(ui_->applyButton, SIGNAL(clicked()), SLOT(accept()));
  connect(ui_->cancelButton, SIGNAL(clicked()), SLOT(reject()));

  connect(ui_->surfaceRadioButton, SIGNAL(toggled(bool)), ui_->levelsGroupBox, SLOT(setDisabled(bool)));
  connect(ui_->gribRadioButton, SIGNAL(toggled(bool)), SLOT(slotSwitchStationsAndCenters(bool)));
  connect(ui_->gribRadioButton, SIGNAL(toggled(bool)), ui_->gribHourWidget, SLOT(setVisible(bool)));

  connect(ui_->timesheetComboBox, SIGNAL(currentIndexChanged(const QString&)), SLOT(slotChangeTimesheet(const QString&)));
  connect(ui_->advancedCheckBox, SIGNAL(toggled(bool)), SLOT(slotSwitchSheduleMode(bool)));
  connect(ui_->timesheetSetupButton, SIGNAL(clicked()), SLOT(slotSetupTimesheet()));
  connect(ui_->timesheetLineEdit, SIGNAL(editingFinished()), SLOT(slotChangeTimesheet()));

  connect(ui_->levelClearButton, SIGNAL(clicked()), SLOT(slotClearLevels()));
  connect(ui_->descriptorClearButton, SIGNAL(clicked()), SLOT(slotClearDescriptors()));
  connect(ui_->centerClearButton, SIGNAL(clicked()), SLOT(slotClearCenters()));
  connect(ui_->stationClearButton, SIGNAL(clicked()), SLOT(slotClearStations()));

  connect(ui_->levelSetupButton, SIGNAL(clicked()), SLOT(slotSetupLevels()));
  connect(ui_->descriptorSetupButton, SIGNAL(clicked()), SLOT(slotSetupDescriptors()));
  connect(ui_->centerSetupButton, SIGNAL(clicked()), SLOT(slotSetupCenters()));
  connect(ui_->stationSetupButton, SIGNAL(clicked()), SLOT(slotSetupStations()));

  connect(ui_->customDateRadioButton, SIGNAL(toggled(bool)), ui_->customDateTimeWidget, SLOT(setEnabled(bool)));

  init(0);
}

ObanalOptions::~ObanalOptions()
{
  delete ui_;
  ui_ = 0;
}

void ObanalOptions::init(const obanalsettings::Task* const config, const QString* const timesheet)
{
  if (config != 0) {
    switch (config->params().type()) {
      case obanalsettings::kSurface:
          ui_->surfaceRadioButton->setChecked(true);
        break;
      case obanalsettings::kAero:
          ui_->aeroRadioButton->setChecked(true);
        break;
      case obanalsettings::kRadar:
          ui_->radioRadioButton->setChecked(true);
        break;
      case obanalsettings::kOcean:
          ui_->oceanRadioButton->setChecked(true);
        break;
      case obanalsettings::kGrib:
          ui_->gribRadioButton->setChecked(true);
        break;
      default:
        break;
    }

    ui_->levelsLineEdit->setText(levelsFromTask(*config));
    ui_->descriptorsLineEdit->setText(descriptorsFromTask(*config));

    if (config->params().type() == obanalsettings::kGrib) {
      ui_->centersLineEdit->setText(centersFromTask(*config));
      ui_->gribHourSpinBox->setValue(gribHourFromTask(*config));
    }
    else {
      ui_->stationsLineEdit->setText(stationsFromTask(*config));
    }

    if (config->params().has_dt_begin() == true) {
      ui_->customDateRadioButton->setChecked(true);
      ui_->startDateTimeEdit->setDateTime(QDateTime::fromString(QString::fromStdString(config->params().dt_begin()), Qt::ISODate));
      if (config->params().has_dt_end()) {
        ui_->endDateTimeEdit->setDateTime(QDateTime::fromString(QString::fromStdString(config->params().dt_end()), Qt::ISODate));
      }
    }
    else {
      ui_->currentDateRadioButton->setChecked(true);
    }
  }
  else {
    ui_->levelClearButton->click();
    ui_->descriptorClearButton->click();
    ui_->centerClearButton->click();
    ui_->stationClearButton->click();
    ui_->surfaceRadioButton->setChecked(true);
    ui_->currentDateRadioButton->setChecked(true);

    ui_->centersGroupBox->hide();
    ui_->gribHourWidget->hide();
  }

  if (timesheet != 0) {
    *currentTimesheet_ = timesheetFromString(*timesheet);

    int hour = basicTimesheetHour(*currentTimesheet_);
    if (hour < 0) {
      updateTimesheetAdvanced();
    }
    else {
      updateTimesheetBasic(hour);
    }
  }
  else {
    updateTimesheetBasic(::defaultShedule());
  }
}

int ObanalOptions::basicTimesheetHour(const Timesheet& ts) const
{
  int result = -1;
  QString strTs = ts.toString();
  foreach (int hour, ::basicShedules().keys()) {
    if (strTs == ::everyNthHourTimesheet(hour).toString()) {
      result = hour;
      break;
    }
  }
  return result;
}

void ObanalOptions::updateTimesheetBasic(int hour)
{
  ui_->timesheetComboBox->setCurrentIndex(::basicShedules().keys().indexOf(hour));
  ui_->sheduleStackedWidget->setCurrentWidget(ui_->basicPage);
  if (ui_->advancedCheckBox->checkState() != Qt::Unchecked) {
    ui_->advancedCheckBox->setChecked(false);
  }
}

void ObanalOptions::updateTimesheetAdvanced()
{
  ui_->timesheetLineEdit->setText(currentTimesheet_->toString());
  ui_->sheduleStackedWidget->setCurrentWidget(ui_->advancedPage);
  if (ui_->advancedCheckBox->checkState() != Qt::Checked) {
    ui_->advancedCheckBox->setChecked(true);
  }
}

obanalsettings::Task ObanalOptions::config() const
{
  obanalsettings::Task result;

  result.mutable_params()->set_type(static_cast<obanalsettings::DataType>(typeToTask()));

  if (ui_->customDateRadioButton->isChecked()) {
    result.mutable_params()->set_dt_begin(ui_->startDateTimeEdit->dateTime().toString(Qt::ISODate).toStdString());
    result.mutable_params()->set_dt_end(ui_->endDateTimeEdit->dateTime().toString(Qt::ISODate).toStdString());
  }

  foreach (int each, levelsToTask()) {
    result.mutable_params()->add_level(each);
  }

  foreach (int each, descriptorsToTask()) {
    result.mutable_params()->add_descr(each);
  }

  if (typeToTask() == obanalsettings::kGrib) {
    foreach (int each, centersToTask()) {
      result.mutable_params()->add_center(each);
    }
    result.mutable_params()->set_hour(gribHourToTask());
  }
  else {
    foreach (int each, stationsToTask()) {
      result.mutable_params()->add_center(each);
    }
  }

  return result;
}

QString ObanalOptions::timesheet() const
{
  return timesheetToString();
}

void ObanalOptions::slotClearLevels()
{
  setDefaultValue(ui_->levelsLineEdit);
}

void ObanalOptions::slotClearDescriptors()
{
  setDefaultValue(ui_->descriptorsLineEdit);
}

void ObanalOptions::slotClearCenters()
{
  setDefaultValue(ui_->centersLineEdit);
}

void ObanalOptions::slotClearStations()
{
  setDefaultValue(ui_->stationsLineEdit);
}

void ObanalOptions::setDefaultValue(QLineEdit* target)
{
  Q_CHECK_PTR(target);
  target->setText(::defaultLineEditText());
}

void ObanalOptions::slotSwitchSheduleMode(bool isAdvanced)
{
  if (isAdvanced == true) {
    updateTimesheetAdvanced();
  }
  else {
    int hour = basicTimesheetHour(*currentTimesheet_);
    if (hour < 0) {
      updateTimesheetBasic(::defaultShedule());
    }
    else {
      updateTimesheetBasic(hour);
    }
  }
}

QString ObanalOptions::levelsFromTask(const obanalsettings::Task& config) const
{
  int sz = config.params().level_size();
  if (sz == 0 ||
      (sz == 1 && config.params().level(0) == -1)) {
    return ::defaultLineEditText();
  }

  QStringList levels;
  levels.reserve(sz);
  for (int i = 0; i < sz; ++i) {
    levels.append(QString::number(config.params().level(i)));
  }
  return QString("[%1]").arg(levels.join(", "));
}

QString ObanalOptions::descriptorsFromTask(const obanalsettings::Task& config) const
{
  int sz = config.params().descr_size();
  if (sz == 0 ||
      (sz == 1 && config.params().descr(0) == -1)) {
    return ::defaultLineEditText();
  }

  QStringList descriptors;
  descriptors.reserve(sz);
  for (int i = 0; i < sz; ++i) {
    descriptors.append(QString::number(config.params().descr(i)));
  }
  return QString("[%1]").arg(descriptors.join(", "));
}

QString ObanalOptions::centersFromTask(const obanalsettings::Task& config) const
{
  int sz = config.params().center_size();
  if (sz == 0 ||
      (sz == 1 && config.params().center(0) == -1)) {
    return ::defaultLineEditText();
  }

  QStringList centers;
  centers.reserve(sz);
  for (int i = 0; i < sz; ++i) {
    centers.append(QString::number(config.params().center(i)));
  }
  return QString("[%1]").arg(centers.join(", "));
}

QString ObanalOptions::stationsFromTask(const obanalsettings::Task& config) const
{
  return centersFromTask(config);
}

Timesheet ObanalOptions::timesheetFromString(const QString& str) const
{
  return str.isEmpty() ? ::defaultTimesheet()
                       : Timesheet::fromString(str);
}

int ObanalOptions::gribHourFromTask(const obanalsettings::Task& config) const
{
  return config.params().has_hour() ? config.params().hour()
                                    : 0;
}

QString ObanalOptions::timesheetToString() const
{
  return currentTimesheet_->toString();
}

int ObanalOptions::typeToTask() const
{
  if (ui_->surfaceRadioButton->isChecked()) {
    return obanalsettings::kSurface;
  }
  else if (ui_->aeroRadioButton->isChecked()) {
    return obanalsettings::kAero;
  }
  else if (ui_->radioRadioButton->isChecked()) {
    return obanalsettings::kRadar;
  }
  else if (ui_->oceanRadioButton->isChecked()) {
    return obanalsettings::kOcean;
  }
  else if (ui_->gribRadioButton->isChecked()) {
    return obanalsettings::kGrib;
  }
  return -1;
}

QList<int> ObanalOptions::levelsToTask() const
{
  QList<int> result = valuesToTask(ui_->levelsLineEdit->text());
  if (result.size() == 1 && result.first() == -1) {
    result = SelectLevelsWidget::allLevelsList(typeToTask());
  }
  return result;
}

QList<int> ObanalOptions::descriptorsToTask() const
{
  return valuesToTask(ui_->descriptorsLineEdit->text());
}

QList<int> ObanalOptions::centersToTask() const
{
  return valuesToTask(ui_->centersLineEdit->text());
}

QList<int> ObanalOptions::stationsToTask() const
{
  return valuesToTask(ui_->stationsLineEdit->text());
}

int ObanalOptions::gribHourToTask() const
{
  return ui_->gribHourSpinBox->value();
}

QList<int> ObanalOptions::valuesToTask(const QString& values) const
{
  QList<int> result;
  if (values == ::defaultLineEditText()) {
    result.append(-1);
  }
  else {
    QStringList valuesList = values.split(",");
    for (int i = 0, sz = valuesList.size(); i < sz; ++i) {
      QString& each = valuesList[i];
      if (each.startsWith('[') == true ||
          each.endsWith(']') == true) {
        each = each.remove('[').remove(']');
      }
      bool ok = false;
      int v = each.trimmed().toInt(&ok);
      if (ok == true) {
        result.append(v);
      }
    }
  }
  return result;
}

void ObanalOptions::slotSwitchStationsAndCenters(bool enabled)
{
  ui_->centersGroupBox->setVisible(enabled);
  ui_->stationsGroupBox->setVisible(!enabled);
}

void ObanalOptions::slotChangeTimesheet()
{
  *currentTimesheet_ = Timesheet::fromString(ui_->timesheetLineEdit->text());
}

void ObanalOptions::slotChangeTimesheet(const QString& str)
{
  int hour = ::basicShedules().key(str);
  *currentTimesheet_ = ::everyNthHourTimesheet(hour);
  ui_->timesheetLineEdit->setText(currentTimesheet_->toString());
}

void ObanalOptions::slotSetupLevels()
{
  SelectLevelsWidget dlg(typeToTask());
  dlg.init(ui_->levelsLineEdit->text());
  if (dlg.exec() == QDialog::Accepted) {
    QString levels = dlg.valuesToString();
    if (levels.isEmpty() == true) {
      levels = ::defaultLineEditText();
    }
    ui_->levelsLineEdit->setText(levels);
  }
}

void ObanalOptions::slotSetupCenters()
{
  SelectCentersWidget dlg;
  dlg.init(ui_->centersLineEdit->text());
  if (dlg.exec() == QDialog::Accepted) {
    QString centers = dlg.valuesToString();
    if (centers.isEmpty() == true) {
      centers = ::defaultLineEditText();
    }
    ui_->centersLineEdit->setText(centers);
  }
}

void ObanalOptions::slotSetupStations()
{
  SelectStationsWidget dlg;
  dlg.setType(typeToTask());
  dlg.init(ui_->stationsLineEdit->text());
  if (dlg.exec() == QDialog::Accepted) {
    QString stations = dlg.valuesToString();
    if (stations.isEmpty() == true) {
      stations = ::defaultLineEditText();
    }
    ui_->stationsLineEdit->setText(stations);
  }
}

void ObanalOptions::slotSetupDescriptors()
{
  SelectDataTypesWidget dlg;
  dlg.init(ui_->descriptorsLineEdit->text());
  if (dlg.exec() == QDialog::Accepted) {
    QString datatypes = dlg.valuesToString();
    if (datatypes.isEmpty() == true) {
      datatypes = ::defaultLineEditText();
    }
    ui_->descriptorsLineEdit->setText(datatypes);
  }
}

void ObanalOptions::slotSetupTimesheet()
{
  TimesheetEditor dlg;
  dlg.init(Timesheet::fromString(ui_->timesheetLineEdit->text()));
  if (dlg.exec() == QDialog::Accepted) {
    QString timesheet = dlg.timesheet().toString();
    if (timesheet.isEmpty() == true) {
      timesheet = ::defaultTimesheet().toString();
    }
    ui_->timesheetLineEdit->setText(timesheet);
    slotChangeTimesheet();
  }
}

}
