#include "receivereditor.h"
#include "ui_receivereditor.h"

static const float  kMHz = 1000000.0f;

namespace mappi {

ReceiverEditor::ReceiverEditor(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::ReceiverEditor)
{
  ui_->setupUi(this);
  connect(ui_->saveBtn,   &QPushButton::clicked, this, &ReceiverEditor::slotSaveReceiver);
  connect(ui_->cancleBtn, &QPushButton::clicked, this, &ReceiverEditor::close);
}

ReceiverEditor::~ReceiverEditor()
{
  delete ui_;
}

void ReceiverEditor::open(const conf::ReceiverParam& recv, const QMap<mappi::conf::RateMode, QString>& recvMode)
{
  recvMode_ = recvMode;
  ui_->modeCbox->clear();
  for(auto it = recvMode_.begin(); it != recvMode_.end(); ++it)
    if(it.key() > conf::kUnkRate)
      ui_->modeCbox->insertItem(it.key(), it.value());

  recv_.CopyFrom(recv);

  if(recv_.has_mode() && recvMode_.end() != recvMode_.find(recv_.mode()))
    ui_->modeCbox->setCurrentText(recvMode.value(recv.mode()));
  if(recv_.has_freq())
    ui_->freqEdt->setValue(recv_.freq() / kMHz);
  if(recv_.has_gain())
    ui_->gainEdt->setValue(recv_.gain());
  if(recv_.has_rate())
    ui_->rateEdt->setValue(recv_.rate() / kMHz);

  show();
}

void ReceiverEditor::slotSaveReceiver()
{
  recv_.set_mode(recvMode_.key(ui_->modeCbox->currentText()));
  recv_.set_freq(ui_->freqEdt->value() * kMHz);
  recv_.set_gain(ui_->gainEdt->value());
  recv_.set_rate(ui_->rateEdt->value() * kMHz);
  Q_EMIT(saveReceiver(recv_));
  hide();
}

} // mappi
