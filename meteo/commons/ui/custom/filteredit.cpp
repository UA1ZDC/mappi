#include "filteredit.h"

#include <qevent.h>

#include <cross-commons/debug/tlog.h>

FilterEdit::FilterEdit( QWidget* p, QAction* a )
  : QLineEdit(p),
  action_(a)
{
}

FilterEdit::~FilterEdit()
{
}

bool FilterEdit::hasFilter() const
{
  if ( true == QLineEdit::text().simplified().isEmpty() ) {
    return false;
  }
  return true;
}

void FilterEdit::setFilter( const QString& f )
{
  QLineEdit::setText(f);
}
    
void FilterEdit::keyPressEvent( QKeyEvent* e )
{
  if ( Qt::Key_Enter == e->key() || Qt::Key_Return == e->key() ) {
    QMenu* m = qobject_cast<QMenu*>( parent() );
    if ( 0 != m ) {
      m->setActiveAction(action_);
    }
  }
  QLineEdit::keyPressEvent(e);
}
