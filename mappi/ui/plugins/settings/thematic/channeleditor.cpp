#include "channeleditor.h"
#include "ui_channeleditor.h"

#include <mappi/settings/mappisettings.h>

namespace mappi {

QMap<ChannelEditor::InstrType, QString> ChannelEditor::instrumentsName_;

ChannelEditor::ChannelEditor(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::ChannelEditor)
{
  ui_->setupUi(this);
  QObject::connect(ui_->satCbox,   &QComboBox::currentTextChanged, this, &ChannelEditor::slotSatChange);
  QObject::connect(ui_->instrCbox, &QComboBox::currentTextChanged, this, &ChannelEditor::slotInstrChange);
  QObject::connect(ui_->addBtn,   &QPushButton::clicked,           this, &ChannelEditor::slotAdd);
  QObject::connect(ui_->closeBtn, &QPushButton::clicked,           this, &ChannelEditor::close);
}

ChannelEditor::~ChannelEditor()
{
  delete ui_;
}

void ChannelEditor::init()
{
  satellites_.clear();
  auto satellites = ::mappi::inter::Settings::instance()->satellites();
  for(auto sat : satellites.satellite()) {
    if(sat.has_name()) {
      satellites_.insert(QString(sat.name().data()));
    }
  }

  instruments_.clear();
  instrumentsName_.clear();
  channels_.clear();
  auto instruments = ::mappi::inter::Settings::instance()->instruments();
  for(auto sat : satellites.satellite()) {
    if(false == sat.has_name()) continue;
    QString satName(sat.name().data());
    QSet<conf::InstrumentType> instrTypes;

    for(auto instr : sat.instr())
      if(instr.has_type())
        instrTypes.insert(instr.type());

    for(auto type : instrTypes) {
      for(auto instr : instruments.instrs()) {
        if(isValid(instr) && instr.type() == type) {
          QString instrName(instr.name().data());
          instruments_[satName].insert(instr.type());
          instrumentsName_.insert(type, instrName);
          auto index = qMakePair(satName, instr.type());
          for(auto ch : instr.channel())
            channels_[index].insert(QString(ch.alias().data()));
        }
      }
    }
  }

  ui_->satCbox->clear();
  auto sats = satellites_.values();
  qSort(sats);
  ui_->satCbox->insertItems(0, sats);
}

void ChannelEditor::slotOpen() { open(Channel()); }

void ChannelEditor::open(const Channel &channel)
{
  init();
  channel_ = channel;
  if(false == channel_.satellite.isEmpty() && satellites_.contains(channel_.satellite)) {
    ui_->satCbox->setCurrentText(channel_.satellite);
  }

  if(false == channel_.instrument.isEmpty()) {
    ui_->instrCbox->setCurrentText(channel_.instrument);
  }

  if(false == channel_.channel.isEmpty()) {
    ui_->chCbox->setCurrentText(channel_.channel);
  }

  debug_log << ui_->satCbox->currentText() << ui_->instrCbox->currentText() << ui_->chCbox->currentText();

  show();
}

void ChannelEditor::slotSatChange(const QString& satellite)
{
  QStringList instruments;
  {
    QSet<QString> instrumentsSet;
    for(auto instr : instruments_.value(satellite)) {
      instrumentsSet.insert(instrumentsName_.value(instr));
    }
    instruments = instrumentsSet.values();
    qSort(instruments);
  }
  QString valueInstr   = ui_->instrCbox->currentText();
  QString valueChannel = ui_->chCbox->currentText();

  int oldCount = ui_->instrCbox->count();
  ui_->instrCbox->insertItems(0, instruments);
  if(instruments.contains(valueInstr)) {
    ui_->instrCbox->setCurrentText(valueInstr);
  }

  int newCount = instruments.count();
  for(int i = 0; i < oldCount; ++i) {
    ui_->instrCbox->removeItem(newCount);
  }
  ui_->chCbox->setCurrentText(valueChannel);
}

void ChannelEditor::slotInstrChange(const QString& instrument)
{
  auto channels = channels_.value(qMakePair(ui_->satCbox->currentText()
                                          , instrumentsName_.key(instrument))).values();
  qSort(channels);
  QString value = ui_->chCbox->currentText();

  int oldCount = ui_->chCbox->count();
  ui_->chCbox->insertItems(0, channels);
  if(channels.contains(value)) {
    ui_->chCbox->setCurrentText(value);
  }

  int newCount = channels.count();
  for(int i = 0; i < oldCount; ++i) {
    ui_->chCbox->removeItem(newCount);
  }
}

void ChannelEditor::slotAdd()
{
  channel_.satellite  = ui_->satCbox->currentText();
  channel_.instrument = ui_->instrCbox->currentText();
  channel_.channel    = ui_->chCbox->currentText();
  Q_EMIT(complite(channel_));
}

bool ChannelEditor::isValid(const conf::Instrument &instrument)
{
  return instrument.has_type() && instrument.has_name();
}

} // mappi
