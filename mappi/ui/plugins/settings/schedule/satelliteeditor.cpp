#include "satelliteeditor.h"
#include "ui_satelliteeditor.h"
#include "receivereditor.h"

#include <qdir.h>
#include <qfile.h>
#include <qsettings.h>
#include <mappi/schedule/schedulehelper.hpp>

static QMap<mappi::conf::RateMode, QString> kRecvMode;
static const QString kSettings     = QDir::homePath() + "/.meteo/mappi/settings.ini";
static const float kMHz = 1000000.0f;
static const QString kTextMHz = QObject::tr("МГц");

namespace mappi {

SatelliteEditor::SatelliteEditor(QWidget *parent)
: QDialog(parent)
, ui_(new Ui::SatelliteEditor)
, receiverEditor_(new ReceiverEditor(this))
{
  kRecvMode.insert(conf::kUnkRate, "Неизвестная скорость");
  kRecvMode.insert(conf::kLowRate, "Низкая скорость");
  kRecvMode.insert(conf::kHiRate,  "Высокая скорость");

  ui_->setupUi(this);

  connect(receiverEditor_,        &ReceiverEditor::saveReceiver,    this, &SatelliteEditor::slotSaveReceiver);
  connect(ui_->receiverTbl,       &QTableWidget::cellDoubleClicked, this, &SatelliteEditor::slotEditReceiver);
  connect(ui_->addReceiverBtn,    &QPushButton::clicked,            this, &SatelliteEditor::slotAddReceiver);
  connect(ui_->removeReceiverBtn, &QPushButton::clicked,            this, &SatelliteEditor::slotRemoveReceiver);

  connect(ui_->saveBtn,           &QPushButton::clicked,            this, &SatelliteEditor::slotSaveSatellite);
  connect(ui_->cancelBtn,         &QPushButton::clicked,            this, &SatelliteEditor::close);

  loadSettings(kSettings);
}

SatelliteEditor::~SatelliteEditor()
{
  saveSettings(kSettings);
  delete ui_;
}

void SatelliteEditor::loadSettings(const QString& filename)
{
  QFile file(filename);
  if(false == file.exists()) return;
  QSettings settings(filename, QSettings::IniFormat);
  ui_->receiverTbl->horizontalHeader()->restoreState(settings.value("schedule/receiver").toByteArray());
  receiverEditor_->restoreGeometry(settings.value("schedule/receivereditor").toByteArray());
  loadedSettings_ = true;
}
void SatelliteEditor::saveSettings(const QString& filename) const
{
  QSettings settings(filename, QSettings::IniFormat);
  settings.setValue("schedule/receiver", ui_->receiverTbl->horizontalHeader()->saveState());
  settings.setValue("schedule/receivereditor", receiverEditor_->saveGeometry());
}

void SatelliteEditor::open(const conf::ReceptionParam& sat, QMap<mappi::conf::SatType, QString>& satType, const QStringList& existSat)
{
  satType_ = satType;
  ui_->typeCbox->clear();
  for(auto it = satType_.begin(); it != satType_.end(); ++it)
    if(it.key() > conf::kUnkSatType)
      ui_->typeCbox->insertItem(it.key(), it.value());

  satellite_.CopyFrom(sat);

  ui_->satCbox->clear();


  if(satellite_.has_name()) {
    QString satName = QString::fromStdString(satellite_.name());
    ui_->satCbox->insertItem(0, satName);
    ui_->satCbox->setCurrentText(satName);
    ui_->satCbox->setEnabled(false);
  }
  else {
    auto satellites = schedule::ScheduleHelper::getAllSatellites();
    satellites = satellites.toSet().subtract(existSat.toSet()).toList();
    ui_->satCbox->insertItems(0, satellites);
    ui_->satCbox->setEnabled(true);
  }

  if(satellite_.has_type())
    ui_->typeCbox->setCurrentText(satType_.value(satellite_.type()));

  receivers_.clear();
  int row = 0;
  for(auto recv : satellite_.receiver()) {
    receivers_.append(recv);
    if(row >= ui_->receiverTbl->rowCount())
      insertRow(ui_->receiverTbl, recv);
    else
      fillRow(ui_->receiverTbl, row, recv);
    ++row;
  }
  while(row < ui_->receiverTbl->rowCount())
    ui_->receiverTbl->removeRow(row);
  if(false == loadedSettings_)
    ui_->receiverTbl->resizeColumnsToContents();

  show();
}

void SatelliteEditor::slotEditReceiver(int row, int col)
{
  Q_UNUSED(col);
  if(row >= 0 && row < receivers_.size()) {
    editRow_ = row;
    receiverEditor_->open(receivers_.at(row), kRecvMode);
  }
}
void SatelliteEditor::slotAddReceiver()
{
  editRow_ = receivers_.size();
  receiverEditor_->open(conf::ReceiverParam(), kRecvMode);
}
void SatelliteEditor::slotRemoveReceiver()
{
  const int row = ui_->receiverTbl->currentRow();
  if(row >= 0 && row < receivers_.size()) {
    receivers_.removeAt(row);
    ui_->receiverTbl->removeRow(row);
  }
}
void SatelliteEditor::slotSaveReceiver(const conf::ReceiverParam& recv)
{
  if(editRow_ < receivers_.size())
    receivers_[editRow_].CopyFrom(recv);
  else
    receivers_.append(recv);

  if(editRow_ >= ui_->receiverTbl->rowCount())
    insertRow(ui_->receiverTbl, recv);
  else
    fillRow(ui_->receiverTbl, editRow_, recv);
}

void SatelliteEditor::slotSaveSatellite()
{
  satellite_.set_name(ui_->satCbox->currentText().toUtf8().data());
  satellite_.set_type(satType_.key(ui_->typeCbox->currentText()));
  satellite_.clear_receiver();
  for(auto recv : receivers_) {
    satellite_.add_receiver()->CopyFrom(recv);
  }
  Q_EMIT(saveSatellite(satellite_));
  hide();
}

void SatelliteEditor::insertRow(QTableWidget* table, const conf::ReceiverParam& recv)
{
  if(nullptr == table) return;
  int row = table->rowCount();
  table->insertRow(row);
  table->setItem(row, kMode, createCell(kMode, recv));
  table->setItem(row, kFreq, createCell(kFreq, recv));
  table->setItem(row, kGain, createCell(kGain, recv));
  table->setItem(row, kRate, createCell(kRate, recv));
}
void SatelliteEditor::fillRow(QTableWidget* table, int row, const conf::ReceiverParam& recv)
{
  if(nullptr == table) return;
  fillCell(table->item(row, kMode), kMode, recv);
  fillCell(table->item(row, kFreq), kFreq, recv);
  fillCell(table->item(row, kGain), kGain, recv);
  fillCell(table->item(row, kRate), kRate, recv);
}

QTableWidgetItem* SatelliteEditor::createCell(Column col, const conf::ReceiverParam& recv)
{
  QTableWidgetItem* item = new QTableWidgetItem();
  fillCell(item, col, recv);
  return item;
}
void SatelliteEditor::fillCell(QTableWidgetItem *item, Column col, const conf::ReceiverParam& recv)
{
  if(nullptr == item) return;
  switch(col) {
  case kMode:     //Ячейка  режима приёма
    if(recv.has_mode() && kRecvMode.end() != kRecvMode.find(recv.mode()))
      item->setText(kRecvMode.value(recv.mode()));
    break;
  case kFreq:       //Ячейка частоты
    if(recv.has_freq())
      item->setText(QString::number(recv.freq() / kMHz) + kTextMHz);
    break;
  case kGain:       //Ячейка усиления
    if(recv.has_gain())
      item->setText(QString::number(recv.gain()));
    break;
  case kRate:      //Ячейка частоты дескритизации
    if(recv.has_rate())
      item->setText(QString::number(recv.rate() / kMHz) + kTextMHz);
    break;
  }
}

} //mappi
