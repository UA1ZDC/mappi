#include "variableeditor.h"
#include "ui_variableeditor.h"

#include <qsettings.h>
#include <qmessagebox.h>
#include <qinputdialog.h>

#include <mappi/settings/mappisettings.h>

static QString kSettings  = QDir::homePath() + "/.meteo/mappi/settings.ini";

namespace mappi {

QMap<VariableEditor::InstrType, QString> VariableEditor::instrumentsName_;


VariableEditor::VariableEditor(QWidget *parent) :
  QDialog(parent)
, ui_(new Ui::VariableEditor)
, channelEditor_(new ChannelEditor(this))
, currentVariable_(nullptr)
{
  ui_->setupUi(this);

  QObject::connect(ui_->varLst, &QListWidget::currentItemChanged, this, &VariableEditor::slotVarChange);

  QObject::connect(ui_->addVarBtn,    &QPushButton::clicked,      this, &VariableEditor::slotAddVar);
  QObject::connect(ui_->removeVarBtn, &QPushButton::clicked,      this, &VariableEditor::slotRemoveVar);

  QObject::connect(ui_->addChBtn,     &QPushButton::clicked,      channelEditor_, &ChannelEditor::slotOpen);
  QObject::connect(ui_->removeChBtn,  &QPushButton::clicked,      this, &VariableEditor::slotRemoveCh);
  QObject::connect(channelEditor_,    &ChannelEditor::complite,   this, &VariableEditor::slotAddCh);

  QObject::connect(ui_->completeBtn,  &QPushButton::clicked,      this, &VariableEditor::slotCompleteEdit);

  init();
  loadSettings(kSettings);
}

VariableEditor::~VariableEditor()
{
  saveSettings(kSettings);
  delete ui_;
}

void VariableEditor::init()
{
  auto satellites = ::mappi::inter::Settings::instance()->satellites();
  auto instruments = ::mappi::inter::Settings::instance()->instruments();
  for(const auto & sat : satellites.satellite()) {
    if(false == sat.has_name()) continue;
    //QString satName(sat.name().data());
    QSet<conf::InstrumentType> instrTypes;

    for(const auto & instr : sat.instr())
      if(instr.has_type())
        instrTypes.insert(instr.type());

    for(auto type : instrTypes) {
      for(const auto & instr : instruments.instrs()) {
        if(isValid(instr) && instr.type() == type) {
          QString instrName(instr.name().data());
          instrumentsName_.insert(type, instrName);
        }
      }
    }
  }
}

void VariableEditor::loadSettings(const QString &filename)
{
  QFile file(filename);
  if(false == file.exists()) return;

  QSettings settings(filename, QSettings::IniFormat);
  ui_->chTbl->horizontalHeader()->restoreState(settings.value("variableeditor/table").toByteArray());
  ui_->varLst->restoreGeometry(settings.value("variableeditor/list").toByteArray());
}
void VariableEditor::saveSettings(const QString &filename)
{
  QSettings settings(filename, QSettings::IniFormat);
  settings.setValue("variableeditor/table", ui_->chTbl->horizontalHeader()->saveState());
  settings.setValue("variableeditor/list", ui_->varLst->saveGeometry());
}

void VariableEditor::open(const QMap<QString, Variable>& vars)
{
  vars_ = vars;
  ui_->varLst->clear();
  if(false == vars_.isEmpty()) {
    int row = 0;
    for(auto it = vars_.begin(); it != vars_.end(); ++it) {
      if(ui_->varLst->count() > row)
        ui_->varLst->item(row)->setText(it.key());
      else
        ui_->varLst->insertItem(row, it.key());
      ++row;
    }
    while(row > ui_->varLst->count())
      delete ui_->varLst->takeItem(row);

    ui_->varLst->setCurrentRow(0);

    currentVariable_ = &vars_[ui_->varLst->currentItem()->text()];
    fillTable(ui_->chTbl, *currentVariable_);
  }
  else {
    while(0 > ui_->chTbl->rowCount())
      ui_->chTbl->removeRow(0);
  }

  QWidget::show();
}

void VariableEditor::slotVarChange(QListWidgetItem* current)
{
  if(nullptr == current) return;
  QString name = current->text();
  auto it = vars_.find(name);
  if(vars_.end() != it) {
    ui_->nameEdt->setText(name);
    currentVariable_ = &it.value();
    fillTable(ui_->chTbl, *currentVariable_);
  }
}

void VariableEditor::slotAddVar()
{
  bool bOk;
  QString varName = QInputDialog::getText(this, QObject::tr("Добавление переменной")
                                        , QObject::tr("Название"), QLineEdit::Normal
                                        , "", &bOk);
  if (!bOk) return; // Была нажата кнопка Cancel
  if(vars_.end() != vars_.find(varName)) {
    QMessageBox::critical(this, QObject::tr("Ошибка"), QObject::tr("Переменная %1 уже существует").arg(varName));
    return;
  }
  vars_.insert(varName, Variable());
  int row = ui_->varLst->count();
  ui_->varLst->insertItem(row, varName);
  ui_->varLst->setCurrentRow(row);
  ui_->chTbl->clear();
}

void VariableEditor::slotAddCh(const ChannelEditor::Channel& channel)
{
  //Поиск инструментов с одним именем
  QVector<conf::InstrumentType> instrs;
  {
  for(auto instr = instrumentsName_.begin(); instrumentsName_.end() != instr; ++instr)
   if(instr.value() == channel.instrument)
     instrs.append(instr.key());
  }
  //Проверка наличия добавляемых каналов(нужно для ситуации где несколько инструментов называются одинаково)
  for(const Channel &ch : currentVariable_->channel()) {
    QString satelliteName(ch.satellite().data()), channelName(ch.channel().data());
    if(channel.satellite != satelliteName || channel.channel != channelName)
      continue;
    for(auto instr = instrs.begin(); instrs.end() != instr;)
      ch.instrument() == *instr ? instr = instrs.erase(instr) : ++instr;
  }
  //Добовление переменной для всех найденых инструментов
  conf::ThematicChannel* ch = nullptr;
  for(auto instr : instrs) {
    ch = currentVariable_->add_channel();
    ch->set_satellite(channel.satellite.toStdString());
    ch->set_instrument(instr);
    ch->set_channel(channel.channel.toStdString());
  }
  //Заполнение GUI
  if(nullptr != ch) {
    insertRow(ui_->chTbl, *ch);
  }
}

void VariableEditor::slotRemoveVar()
{
  int row = ui_->varLst->currentRow();
  if(0 > row || row > vars_.size()) return;
  auto it_vars = vars_.erase(vars_.find(ui_->varLst->item(row)->text()));
  delete ui_->varLst->takeItem(row);
  if(vars_.isEmpty())
    currentVariable_ = nullptr;
  else if(vars_.end() == it_vars)
    currentVariable_ = &vars_.last();
  else
    currentVariable_ = &*it_vars;

  ui_->varLst->setCurrentRow(row < ui_->varLst->count() ? row : 0);
}

void VariableEditor::slotRemoveCh()
{
  int index = ui_->chTbl->currentRow();
  if(0 > index || nullptr == currentVariable_ || index > currentVariable_->channel_size()) return;

  //Поиск инструментов с одним именем
  QVector<conf::InstrumentType> instrs;
  {
    for(auto instr = instrumentsName_.begin(); instrumentsName_.end() != instr; ++instr)
     if(instr.value() == ui_->chTbl->item(index, kInstr)->text())
       instrs.append(instr.key());
  }

  //Копируем в variable только нужные каналы
  Variable variable;
  variable.set_name(currentVariable_->has_name() ? currentVariable_->name() : "");

  //Удаление канала
  QString satellite = ui_->chTbl->item(index, kSat)->text()
      ,   channel  = ui_->chTbl->item(index, kChannel)->text();
  for(const auto &ch : currentVariable_->channel()) {
    bool has = true;
    if(satellite == QString(ch.satellite().data()) && channel == QString(ch.channel().data()))
      for(auto instr : instrs)
        if(true == (ch.instrument() == instr))
          has &= false;
    if(has)
      variable.add_channel()->CopyFrom(ch);
  }

  currentVariable_->CopyFrom(variable);
  ui_->chTbl->removeRow(index);
}

void VariableEditor::slotCompleteEdit()
{
  if(nullptr != currentVariable_) {
    currentVariable_->set_name(ui_->nameEdt->text().toStdString());
    conf::ThematicVariable variable;
    variable.CopyFrom(*currentVariable_);
    vars_.insert(ui_->nameEdt->text(), variable);
  }
  Q_EMIT(compliteVars(vars_));
  hide();
}

void VariableEditor::fillTable(QTableWidget* table, const Variable& variable)
{
  int row = 0;
  for(const auto & ch : variable.channel()) {
    if(row < table->rowCount())
      fillRow(table, row, ch);
    else
      insertRow(table, ch);
    ++row;
  }
  while (row < table->rowCount())
    table->removeRow(row);
}

void VariableEditor::insertRow(QTableWidget* table, const Channel& ch)
{
  if(nullptr == table) return;

  //поиск переменной с повторяющимся текстом в строке таблицы(из-за разных инструментов с одинаковым именем)
  for(int row = 0; row < table->rowCount(); ++row) {
    bool has = true;
    for(auto col : { kSat, kInstr, kChannel }) {
      switch (col) {
      case kSat:     has &= table->item(row, col)->text() == QString::fromStdString(ch.satellite());  break;
      case kInstr:   has &= table->item(row, col)->text() == instrumentsName_.value(ch.instrument()); break;
      case kChannel: has &= table->item(row, col)->text() == QString::fromStdString(ch.channel());    break;
      default: has &= false; break;
      }
    }
    if(has) return;
  }

  //Добавление строки
  int row = table->rowCount();
  table->insertRow(row);
  for(auto col : { kSat, kInstr, kChannel })
    table->setItem(row, col, createCell(col, ch));
}
void VariableEditor::fillRow(QTableWidget* table, int row, const Channel& ch)
{
  if(nullptr == table) return;
  for(auto col : { kSat, kInstr, kChannel })
    fillCell(table->item(row, col), col, ch);
}

QTableWidgetItem* VariableEditor::createCell(Column col, const Channel& ch)
{
  QTableWidgetItem* item = new QTableWidgetItem();
  fillCell(item, col, ch);
  return item;
}
void VariableEditor::fillCell(QTableWidgetItem* item, Column col, const Channel& ch)
{
  if(nullptr == item) return;
  switch (col) {
  case kSat:
    if(ch.has_satellite())
      item->setText(QString(ch.satellite().data()));
    break;
  case kInstr:
    if(ch.has_instrument())
      item->setText(instrumentsName_.value(ch.instrument()));
    break;
  case kChannel:
    if(ch.has_channel())
      item->setText(QString(ch.channel().data()));
    break;
  }
}

bool VariableEditor::isValid(const conf::Instrument &instrument)
{
  return instrument.has_type() && instrument.has_name();
}

}//mappi
