#include "opendoc.h"

#include <qsettings.h>
#include <qapplication.h>
#include <qmenu.h>
#include <qcolordialog.h>
#include <qmessagebox.h>

#include <cross-commons/app/paths.h>

//#include <sql/psql/psqlquery.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/proto/msgparser.pb.h>
#include <meteo/commons/proto/msgcenter.pb.h>

#include "ui_opendoc.h"

namespace meteo {
namespace map {

OpenDoc::OpenDoc( MapWindow* view )
  : MapWidget(view),
  ui_(new Ui::OpenDoc),
  ext_("map_ptkpp")
{
  ui_->setupUi(this);
  if ( 0 != mapdocument() ) {
    switch ( mapdocument()->property().doctype() ) {
      case proto::kGeoMap:
        ext_ = "map_ptkpp";
        break;
      case proto::kVerticalCut:
        ext_ = "cut_ptkpp";
        break;
      case proto::kAeroDiagram:
        ext_ = "aero_ptkpp";
        break;
      case proto::kOceanDiagram:
        ext_ = "ocean_ptkpp";
        break;
      case proto::kMeteogram:
        ext_ = "mg_ptkpp";
        break;
      case proto::kFormalDocument:
        ext_ = "frml_ptkpp";
        break;
    }
  }

  setDefaultName();
  turnSignals();
}

OpenDoc::~OpenDoc()
{
  delete ui_;
  ui_ = 0;
}

void OpenDoc::muteSignals()
{
  QObject::disconnect( ui_->okbtn, SIGNAL( clicked() ), this, SLOT( slotAccept() ) );
  QObject::disconnect( ui_->nobtn, SIGNAL( clicked() ), this, SLOT( slotReject() ) );
  QObject::disconnect( ui_->btnfile, SIGNAL( clicked() ), this, SLOT( slotSelectFile() ) );
  QObject::disconnect( ui_->lefile, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotPathChanged() ) );
}

void OpenDoc::turnSignals()
{
  QObject::connect( ui_->okbtn, SIGNAL( clicked() ), this, SLOT( slotAccept() ) );
  QObject::connect( ui_->nobtn, SIGNAL( clicked() ), this, SLOT( slotReject() ) );
  QObject::connect( ui_->btnfile, SIGNAL( clicked() ), this, SLOT( slotSelectFile() ) );
  QObject::connect( ui_->lefile, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotPathChanged() ) );
}

QString OpenDoc::lastPath() const
{
  return global::lastDocPath();
}
void OpenDoc::saveLastPath() const
{
  if ( true == ui_->lefile->text().isEmpty() ) {
    return;
  }
  QFile file( ui_->lefile->text() );
  global::saveLastDocPath( QFileInfo(file).absolutePath() );
}

void OpenDoc::setDefaultName()
{
  QString txt = lastPath();
  while ( -1 != txt.indexOf("//") ) {
    txt.replace("//","/");
  }
  ui_->lefile->setText(txt);
}

void OpenDoc::openFile()
{
  QFile file( ui_->lefile->text() );
  if ( false == file.exists() ) {
    error_log.msgBox() << QObject::tr("Выберите файл с документом");
    qApp->restoreOverrideCursor();
    return;
  }
  if ( false == file.open( QIODevice::ReadOnly ) ) {
    error_log.msgBox() << QObject::tr("Не удалось открыть файл для для чтения %1")
      .arg( file.fileName() );
    qApp->restoreOverrideCursor();
    return;
  }
  QByteArray arr = file.readAll();

  openMap(arr);

  QWidget::close();
}

void OpenDoc::slotAccept()
{

  qApp->setOverrideCursor( Qt::WaitCursor );
  openFile();
  qApp->restoreOverrideCursor();
}

void OpenDoc::slotReject()
{
  QWidget::close();
}

void OpenDoc::slotSelectFile()
{
  QFileDialog* dlg = new QFileDialog( this, windowTitle(), QFileInfo( ui_->lefile->text() ).absoluteFilePath(), "*."+ext_ );
  dlg->setFileMode( QFileDialog::ExistingFile );
  dlg->selectFile( QFileInfo( ui_->lefile->text() ).absoluteFilePath() );
  dlg->setLabelText( QFileDialog::Accept, QObject::tr("Выбрать") );
  dlg->setLabelText( QFileDialog::Reject, QObject::tr("Закрыть") );
  int res = dlg->exec();
  QStringList list = dlg->selectedFiles();
  delete dlg;
  if ( QDialog::Accepted != res || 0 == list.size() ) {
    return;
  }
  muteSignals();
  ui_->lefile->setText( list[0] );
  turnSignals();
}

void OpenDoc::slotPathChanged()
{
  saveLastPath();
}

void OpenDoc::openMap( const QByteArray& arr )
{
  Document* doc = mapscene()->document();
  if ( 0 == doc ) {
    return;
  }
  doc->parseFromArray(arr);
}

}
}
