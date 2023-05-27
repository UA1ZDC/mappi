#include "docdlg.h"

#include <cross-commons/debug/tlog.h>

#include <qevent.h>

namespace meteo {
namespace map {

DocDlg::DocDlg( QWidget* p )
  : QDialog(p)
{
}

DocDlg::~DocDlg()
{
}

void DocDlg::keyReleaseEvent( QKeyEvent* e )
{
  if ( Qt::Key_Escape != e->key() ) {
    return;
  }
  QWidget::close();
}

}
}
