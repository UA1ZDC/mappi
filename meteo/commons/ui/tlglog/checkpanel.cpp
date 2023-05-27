#include "checkpanel.h"

CheckPanel::CheckPanel(QWidget *parent) :
  QWidget(parent)
{
  hbl_ = new QHBoxLayout(this);
  hbl_->setAlignment(Qt::AlignLeft);
  hbl_->setContentsMargins(3,3,3,3);
  btn_ = new QToolButton(this);
  btn_->setMinimumSize(QSize(26,26));
  btn_->setMaximumSize(QSize(26,26));
  btn_->setIcon(QIcon(":/meteo/icons/close.png"));
  hbl_->addWidget(btn_);
  connect(btn_, SIGNAL(clicked()), SLOT(close()));
}

void CheckPanel::addCheckBox(QCheckBox* check)
{
  hbl_->addWidget(check);
}

void CheckPanel::closeEvent(QCloseEvent* e)
{
  emit panelClosed();
  QWidget::closeEvent(e);
}
