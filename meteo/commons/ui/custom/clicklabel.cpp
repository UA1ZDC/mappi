#include "clicklabel.h"

#include <qevent.h>
#include <qspinbox.h>

ClickLabel::ClickLabel( QWidget* p, Qt::WindowFlags fl )
  : QLabel( p, fl )
{
}

ClickLabel::ClickLabel( const QString& t, QWidget* p, Qt::WindowFlags fl )
  : QLabel( t, p, fl )
{
}

ClickLabel::~ClickLabel()
{
}

void ClickLabel::mouseDoubleClickEvent( QMouseEvent* e )
{
  emit doubleclick();
  QWidget::mouseDoubleClickEvent(e);
}
