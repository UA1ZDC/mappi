#include "alphaedit.h"
#include "ui_alphaedit.h"

#include <qevent.h>
#include <qmenu.h>
#include <qaction.h>

#include <cross-commons/debug/tlog.h>

AlphaEdit::AlphaEdit( QWidget* p, QAction* a )
  : QWidget(p),
  ui_(new Ui::AlphaEdit),
  action_(a)
{
  ui_->setupUi(this);
}

AlphaEdit::~AlphaEdit()
{
  delete ui_; ui_ = 0;
}

int AlphaEdit::percent() const
{
  return ui_->alphaspin->value();
}

void AlphaEdit::setPercent( int prcnt )
{
  ui_->alphaspin->setValue(prcnt);
}
    
void AlphaEdit::keyPressEvent( QKeyEvent* e )
{
  if ( Qt::Key_Enter == e->key() || Qt::Key_Return == e->key() ) {
    QMenu* m = qobject_cast<QMenu*>( parent() );
    if ( 0 != m ) {
      m->setActiveAction(action_);
    }
  }
  QWidget::keyPressEvent(e);
}
