#include "savebmpaction.h"
#include "../widgets/savebitmap.h"

#include <qevent.h>
#include <qgraphicssceneevent.h>
#include <qobject.h>
#include <qtoolbutton.h>
#include <qpalette.h>
#include <qgraphicslinearlayout.h>
#include <qgraphicsproxywidget.h>
#include <qdebug.h>
#include <qmatrix.h>
#include <qmath.h>
#include <qcoreapplication.h>
#include <qcursor.h>
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

static const auto kMenuItem = QPair<QString, QString>("savebitmap", QObject::tr("Изображение"));
static const auto kMenuPath = QList<QPair<QString, QString>>
({QPair<QString, QString>("document", QObject::tr("Документ")), QPair<QString, QString>("saveas", QObject::tr("Сохранить как"))});

SavebmpAction::SavebmpAction( MapScene* scene )
  : Action(scene, "savebmpacion" ),
  dlg_(0)
{
  QAction* a = view()->window()->addActionToMenu(kMenuItem, kMenuPath);
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotOpenSaveBmpDialog() ) );
}

SavebmpAction::~SavebmpAction()
{
}

void SavebmpAction::slotOpenSaveBmpDialog()
{
  if ( 0 != dlg_ ) {
    dlg_->show();
    dlg_->setFocus( Qt::OtherFocusReason );
    return;
  }
  dlg_ = new SaveBitmap( view()->window() );
  QObject::connect( dlg_, SIGNAL( destroyed( QObject* ) ), this, SLOT( slotDlgDestroyed( QObject* ) ) );
  dlg_->show();
}

void SavebmpAction::slotDlgDestroyed( QObject* o )
{
  Q_UNUSED(o);
  dlg_ = 0;
}

}
}
