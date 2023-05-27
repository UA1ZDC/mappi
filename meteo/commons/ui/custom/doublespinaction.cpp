#include "doublespinaction.h"

#include <qevent.h>
#include <qmenu.h>
#include <qaction.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace map {

DoubleSpinAction::DoubleSpinAction( QWidget* p, QAction* a )
  : QDoubleSpinBox(p),
  action_(a)
{
}

DoubleSpinAction::~DoubleSpinAction()
{
}

  
void DoubleSpinAction::keyPressEvent( QKeyEvent* e )
{
  if ( Qt::Key_Enter == e->key() || Qt::Key_Return == e->key() ) {
    QMenu* m = qobject_cast<QMenu*>( parent() );
    if ( 0 != m ) {
      m->setActiveAction(action_);
    }
  }
  QDoubleSpinBox::keyPressEvent(e);
}

}
}
