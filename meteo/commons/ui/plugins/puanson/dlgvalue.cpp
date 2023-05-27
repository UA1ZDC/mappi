#include "dlgvalue.h"

#include <commons/mathtools/mnmath.h>

#include "ui_dlgvalue.h"

namespace meteo {
namespace map {

DlgValue::DlgValue( const QString& title, double val, QWidget* parent )
  : QDialog(parent),
  ui_( new Ui::DlgValue ),
  oldval_(val)
{
  ui_->setupUi(this);
  ui_->spinvalue->setValue(val);
  ui_->lblparam->setText(title);

  QObject::connect( ui_->okbtn, SIGNAL( clicked() ), this, SLOT( slotOkClicked() ) );
  QObject::connect( ui_->nobtn, SIGNAL( clicked() ), this, SLOT( slotNoClicked() ) );
}

DlgValue::~DlgValue()
{
  delete ui_; ui_ = 0;
}

double DlgValue::value() const
{
  return ui_->spinvalue->value();
}

control::QualityControl DlgValue::qual() const
{
  control::QualityControl q = control::HAND_CORRECTED;
  if ( 1 == ui_->cmbqual->currentIndex() ) {
    q = control::MISTAKEN;
  }
  return q;
}

void DlgValue::slotOkClicked()
{
  if ( true == MnMath::isEqual( oldval_, ui_->spinvalue->value() ) ) {
    QDialog::reject();
  }
  QDialog::accept();
}

void DlgValue::slotNoClicked()
{
  QDialog::reject();
}

}
}
