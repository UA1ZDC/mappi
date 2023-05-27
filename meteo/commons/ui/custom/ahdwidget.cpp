#include "ahdwidget.h"
#include "ui_ahdwidget.h"

AhdWidget::AhdWidget(QWidget *parent)
  : QWidget(parent),
    ui_(new Ui::AhdWidget)
{
  ui_->setupUi(this);

  connect( ui_->tt, SIGNAL(textChanged(QString)), SIGNAL(changed()) );
  connect( ui_->aa, SIGNAL(textChanged(QString)), SIGNAL(changed()) );
  connect( ui_->cccc, SIGNAL(textChanged(QString)), SIGNAL(changed()) );
  connect( ui_->yygggg, SIGNAL(textChanged(QString)), SIGNAL(changed()) );
  connect( ui_->bbb, SIGNAL(textChanged(QString)), SIGNAL(changed()) );
  connect( ui_->ii, SIGNAL(valueChanged(QString)), SIGNAL(changed()) );
}

AhdWidget::~AhdWidget()
{
  delete ui_;
}

QString AhdWidget::toString() const
{
  return  ui_->tt->text() + ui_->aa->text() + ui_->ii->text() + " " +
          ui_->cccc->text() + " " +
          ui_->yygggg->text() + " " +
          ui_->bbb->text();
}

QString AhdWidget::toString(const QString& format) const
{
  QString tt = ui_->tt->text();
  QString aa = ui_->aa->text();
  QString ii = ui_->ii->text();
  QString cccc = ui_->cccc->text();
  QString yygggg = ui_->yygggg->text();
  QString bbb = ui_->bbb->text();

  QString s = format;
  s.replace("TT", tt);
  s.replace("AA", aa);
  s.replace("ii" , ii);
  s.replace("CCCC", cccc);
  s.replace("YYGGgg", yygggg);
  s.replace("BBB", bbb);
  return s;
}
