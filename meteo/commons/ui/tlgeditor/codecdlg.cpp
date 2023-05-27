#include "codecdlg.h"
#include "ui_codecdlg.h"

namespace meteo {

CodecDlg::CodecDlg(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::CodecDlg)
{
  ui_->setupUi(this);

  connect( ui_->list, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
           SLOT(slotCurrentChanged(QListWidgetItem*,QListWidgetItem*)) );
  connect( ui_->saveBtn, SIGNAL(clicked(bool)), SLOT(slotSave()) );
  connect( ui_->reloadBtn, SIGNAL(clicked(bool)), SLOT(slotReload()) );
}

CodecDlg::~CodecDlg()
{
  delete ui_;
}

QString CodecDlg::codecName() const
{
  return ui_->list->currentItem()->text();
}

void CodecDlg::setCurrentCodecName(const QString& name)
{
  QList<QListWidgetItem*> items = ui_->list->findItems(name, Qt::MatchExactly);
  if ( items.size() == 0 ) {
    ui_->list->setCurrentRow(0);
  }
  else {
    ui_->list->setCurrentItem(items.first());
  }
}

CodecDlg::Button CodecDlg::run()
{
  if ( exec() == QDialog::Rejected ) {
    button_ = kCancelButton;
  }

  return button_;
}

void CodecDlg::slotCurrentChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
  if ( 0 == previous ) {
    previous = ui_->list->item(0);
  }

  if ( 0 == current ) {
    ui_->list->blockSignals(true);
    ui_->list->setCurrentItem(previous);
    ui_->list->blockSignals(false);
  }
}

void CodecDlg::slotReload()
{
  button_ = kReloadButton;
  accept();
}

void CodecDlg::slotSave()
{
  button_ = kSaveButton;
  accept();
}

} // meteo
