#include "printdocaction.h"
#include "../widgets/printdoc.h"

#include <qevent.h>
#include <qgraphicssceneevent.h>
#include <qobject.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qpalette.h>
#include <qgraphicslinearlayout.h>
#include <qgraphicsproxywidget.h>
#include <qdebug.h>
#include <qmatrix.h>
#include <qmath.h>
#include <qcoreapplication.h>
#include <qcursor.h>
#include <qprintdialog.h>
#include <qaction.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/global/global.h>

#include "../mapview.h"
#include "../mapscene.h"
#include "../mapwindow.h"
#include "../widgetitem.h"

namespace meteo {
namespace map {

static const auto kMenuItem = QPair<QString, QString>("printdoc", QObject::tr("Печать"));
static const auto kMenuPath = QList<QPair<QString, QString>>
    ({QPair<QString, QString>("document", QObject::tr("Документ"))});

PrintdocAction::PrintdocAction( MapScene* scene )
  : Action(scene, "printdocaction" ),
  dlg_(0)
{
  QAction* a = view()->window()->addActionToMenu(kMenuItem, kMenuPath);
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotOpenPrintDialog() ) );
}

PrintdocAction::~PrintdocAction()
{
}

void PrintdocAction::slotOpenPrintDialog()
{
  if ( 0 != dlg_ ) {
    dlg_->show();
    dlg_->setFocus( Qt::OtherFocusReason );
    return;
  }

  PrintDoc* tab = new PrintDoc( view()->window() );
  dlg_ = new QPrintDialog(tab->printer(), view()->window());
  dlg_->setOption(QAbstractPrintDialog::PrintShowPageSize, false);
  dlg_->setOptionTabs(QList<QWidget*>() << tab);
  dlg_->setWindowTitle(QObject::tr("Печать"));
  foreach( QPushButton* b, dlg_->findChildren<QPushButton*>() ){
    if( "&Параметры >>" == b->text() ){
      b->click();
      break;
    }
  }
  QTabWidget* tw = dlg_->findChild<QTabWidget*>();
  if( 0 != tw ){
    tw->setCurrentWidget(tab);
  }
  dlg_->adjustSize();
  dlg_->installEventFilter(tab);

  QObject::connect( dlg_, SIGNAL( destroyed( QObject* ) ), this, SLOT( slotDlgDestroyed( QObject* ) ) );
  QObject::connect( dlg_, SIGNAL( finished(int) ), tab, SLOT( finished(int) ) );
  dlg_->show();
}

void PrintdocAction::slotDlgDestroyed( QObject* o )
{
  Q_UNUSED(o);
  dlg_ = 0;
}

}
}
