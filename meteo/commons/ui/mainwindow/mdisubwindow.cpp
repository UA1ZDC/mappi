#include "mdisubwindow.h"

#include <qevent.h>
#include <qapplication.h>
#include <qdialog.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace app {

MdiSubWindow::MdiSubWindow( QWidget* p, QWidget* w )
  : QMdiSubWindow(p)
{
  setWidget(w);
  w->installEventFilter(this);
  QMdiSubWindow::setOption( QMdiSubWindow::RubberBandResize, true );
}

MdiSubWindow::~MdiSubWindow()
{
  if ( nullptr != widget() ) {
    delete widget();
  }
}

void MdiSubWindow::closeEvent( QCloseEvent* e )
{
  QWidget* w = widget();
  bool ok = w->close();
  if( true == ok ){
    w->deleteLater();
    e->accept();
    emit mdiSubWindowClosed(this);
  }
  else{
    e->ignore();
  }
}

void MdiSubWindow::slotSetTitle(const QString & t ){
  setWindowTitle(t);
  emit(windowTitleChanged(t));
}

void MdiSubWindow::paintEvent( QPaintEvent* e )
{

  QMdiSubWindow::paintEvent(e);
}

void MdiSubWindow::showEvent( QShowEvent* e )
{
  QMdiSubWindow::showEvent(e);
}

void MdiSubWindow::focusInEvent( QFocusEvent* ){
if ( nullptr != widget() ) {
      emit(windowTitleChanged(widget()->windowTitle()));
  }
}

void MdiSubWindow::focusOutEvent( QFocusEvent* )
{
}

bool MdiSubWindow::eventFilter( QObject* o, QEvent* e )
{
  if ( o != widget() || QEvent::Close != e->type() ) {
    return false;
  }
  o->removeEventFilter(this);
  bool res = qApp->sendEvent( o, e );
  if ( true == res ) {
    QWidget::close();
  }
  else {
    o->installEventFilter(this);
  }
  return res;
}

void MdiSubWindow::setVisibleChildWindow(bool visible)
{
  QList<QDialog*> lst = findChildren<QDialog*>();
  foreach( QDialog* w, lst ){
    if( nullptr != w ){
      if( true == visible ){
        if( true == w->property("hidden") ){
          w->move(w->property("pos").toPoint());
          w->setVisible(true);
          w->setProperty("hidden", false);
        }
      }
      else if(w->isVisible()){
        w->setProperty("hidden", true);
        w->setProperty("pos", w->pos());
        w->setVisible(false);
      }
    }
  }
}

}
}
