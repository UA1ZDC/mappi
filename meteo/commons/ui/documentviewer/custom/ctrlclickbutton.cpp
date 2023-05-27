#include "ctrlclickbutton.h"
#include <qevent.h>

namespace meteo {
namespace commons {

CtrlClickButton::CtrlClickButton( QWidget* parent ) : QPushButton (parent)
{

}


void CtrlClickButton::mousePressEvent(QMouseEvent *e)
{
  if ( e->modifiers() == Qt::CTRL ) {
    emit signalCtrlClick();
    e->setAccepted(true);
    return;
  }
  QPushButton::mousePressEvent(e);
}

}
}
