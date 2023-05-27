#include "calendarbtn.h"

#include <meteo/commons/global/common.h>

CalendarButton::CalendarButton( QWidget* p )
  : QToolButton(p),
  dt_( QDateTime::currentDateTime() )
{
  QWidget::setMinimumSize( QSize(32,32) );
  QWidget::setMaximumSize( QSize(32,32) );
  QObject::connect( this, SIGNAL( clicked() ), this, SLOT( slotClicked() ) );
  setDatetime(dt_);
  QAbstractButton::setIcon( QIcon(":/meteo/icons/calendar.png") );
}

CalendarButton::~CalendarButton()
{
}

void CalendarButton::setDatetime( const QDateTime& dt )
{
  if ( dt_ == dt ) {
    return;
  }
  dt_ = dt;
  if ( nullptr != dlg_ ) {
    dlg_->setDt( dt_.date(), dt_.time() );
  }
}

void CalendarButton::slotClicked()
{
  dlg_ = new CalendarDlg(this);
  QTime time = dt_.time();
  time.setHMS(time.hour(), 0, 0);
  dlg_->setDt( dt_.date(), time );
  dlg_->move( QWidget::mapToGlobal( QPoint(0,0) ) + QPoint( -1*dlg_->width() + width(), height() ) );
  int res = dlg_->exec();
  QDateTime dt = dlg_->currentDt();
  if ( QDialog::Accepted == res ) {
    setDatetime(dt);
    emit dtChanged(dt);
  }
  delete dlg_;
  dlg_ = nullptr;
}
