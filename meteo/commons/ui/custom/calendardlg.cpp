#include "calendardlg.h"

#include <qlayout.h>
#include <qtoolbutton.h>
#include <qevent.h>

#include <cross-commons/debug/tlog.h>

#include "ui_calendardlg.h"

CalendarDlg::CalendarDlg( QWidget* prnt )
  : QDialog(prnt),
  ui_( new Ui::CalendarDlg ),
  dt_( QDateTime::currentDateTime() )
{
  ui_->setupUi(this);
  setWindowFlags(Qt::Popup);
  QObject::connect( ui_->toolButton, &QAbstractButton::clicked, this, &QDialog::accept );
}

CalendarDlg::~CalendarDlg()
{
  delete ui_; ui_ = nullptr;
}

const QDateTime CalendarDlg::currentDt() const
{
  return QDateTime( ui_->dateselect->selectedDate(), QTime( ui_->hourselect->time().hour(), ui_->hourselect->time().minute(), 0 ) );
}

void CalendarDlg::setDt(const QDate& date, const QTime &time )
{
  ui_->dateselect->setSelectedDate(date);
  ui_->hourselect->setTime(time);
}

void CalendarDlg::keyReleaseEvent( QKeyEvent* e )
{
  QWidget::keyReleaseEvent(e);
  switch ( e->key() ) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Space:
      QDialog::accept();
      break;
    default:
      break;
  }
}
