#include "dlgspinbox.h"

#include <qevent.h>

#include "ui_dlgspinbox.h"

namespace meteo {
namespace puanson {

DlgSpinbox::DlgSpinbox( QWidget* p )
//  : QDialog( p, Qt::FramelessWindowHint ),
  : QDialog(p),
  ui_(new Ui::DlgSpinBox)
{
  ui_->setupUi(this);
}

DlgSpinbox::~DlgSpinbox()
{
  delete ui_; ui_ = 0;
}
    
double DlgSpinbox::value() const
{
  return ui_->spin->value();
}

void DlgSpinbox::setValue( double val )
{
  ui_->spin->setValue(val);
}

void DlgSpinbox::keyReleaseEvent( QKeyEvent* e )
{
  QDialog::keyReleaseEvent(e);
  if ( Qt::Key_Enter == e->key() || Qt::Key_Return == e->key() ) {
    QDialog::accept();
  }
}

}
}
