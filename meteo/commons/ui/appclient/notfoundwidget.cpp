#include "notfoundwidget.h"
#include "ui_notfound.h"

NotFoundWidget::NotFoundWidget(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::NotFoundForm)
{
  ui_->setupUi(this);
}

void NotFoundWidget::setText(const QString& text)
{
  ui_->textEdit->setText(text);
}
