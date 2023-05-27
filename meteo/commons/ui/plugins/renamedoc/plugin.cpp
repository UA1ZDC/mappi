#include "plugin.h"

#include <qdialog.h>
#include <qaction.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/mapview.h>

#include "ui_renamedoc.h"

namespace meteo {
namespace map {

Plugin::Plugin()
  : ActionPlugin("renamedocaction")
{
}

Plugin::~Plugin()
{
}

Action* Plugin::create( MapScene* scene ) const
{
  return new RenameDoc(scene);
}

RenameDoc::RenameDoc( MapScene* scene )
  : Action( scene, "renamedocaction" )
{
  auto path = QList<QPair<QString, QString>> ({QPair<QString, QString>("document", "Документ")});
  auto title = QPair<QString, QString> ("renamedoc", "Переименовать");
  QAction* a = view()->window()->addActionToMenu(title, path);
  QObject::connect( a, SIGNAL( triggered() ), this, SLOT( slotRenameDoc() ) );
}

RenameDoc::~RenameDoc()
{
}

void RenameDoc::slotRenameDoc()
{
  if ( 0 == scene_->document() ) {
    error_log.msgBox() << QObject::tr("Документ не создан. Переименование невозможно.");
    return;
  }
  QDialog* dlg = new QDialog();
  Ui::RenameDoc ui;
  ui.setupUi(dlg);
  ui.ledocname->setText( scene_->document()->name() );
  int res = dlg->exec();
  if ( QDialog::Accepted == res ) {
    scene_->document()->setName( ui.ledocname->text() );
  }
  delete dlg;
}

}
}
