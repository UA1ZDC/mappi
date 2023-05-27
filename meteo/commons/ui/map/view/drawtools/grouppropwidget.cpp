#include "grouppropwidget.h"
#include "ui_grouppropwidget.h"

namespace meteo {
namespace map {

GroupPropWidget::GroupPropWidget(QWidget* parent)
  : PropWidget(parent)
{
  ui_ = new Ui::GroupPropWidget;
  ui_->setupUi(this);

  setWindowFlags(Qt::Tool);

  connect( ui_->finishBtn, SIGNAL(clicked(bool)), SIGNAL(finish()) );
  connect( ui_->valueCheck, SIGNAL(toggled(bool)), SIGNAL(valueChanged()) );
  connect( ui_->valueSpin, SIGNAL(valueChanged(double)), SIGNAL(valueChanged()) );
  connect( ui_->valueUnitEdit, SIGNAL(textChanged(QString)), SIGNAL(valueChanged()) );

  connect( ui_->delBtn, SIGNAL(clicked(bool)), SIGNAL(deleteCurrent()) );
}

GroupPropWidget::~GroupPropWidget()
{
  delete ui_;
  ui_ = 0;
}

bool GroupPropWidget::hasValue() const
{
  return ui_->valueCheck->isChecked();
}

void GroupPropWidget::setValue(double value, bool enable)
{
  ui_->valueCheck->setChecked(enable);
  if ( enable ) {
    ui_->valueSpin->setValue(value);
  }
}

double GroupPropWidget::value() const
{
  return ui_->valueSpin->value();
}

QString GroupPropWidget::unit() const
{
  return ui_->valueUnitEdit->text();
}

} // map
} // meteo

