#include "actionbutton.h"

#include <qgraphicseffect.h>
#include <qgraphicsproxywidget.h>
#include <cross-commons/debug/tlog.h>

ActionButton::ActionButton(QWidget * parent) :
  QToolButton(parent)
{
  setAttribute( Qt::WA_TranslucentBackground, true );
  setMinimumSize( QSize(kActionBtnSize,kActionBtnSize) );
  setMaximumSize( QSize(kActionBtnSize,kActionBtnSize) );
  setIconSize(QSize(kActionBtnIconSize,kActionBtnIconSize));
  setCheckable(true);
}

void ActionButton::enterEvent(QEvent* e)
{
  emit hovered();
  QWidget::enterEvent( e );
}

void ActionButton::leaveEvent(QEvent* e)
{
  emit leave();
  QWidget::leaveEvent( e );
}
