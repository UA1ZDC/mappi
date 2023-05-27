#include "savedoc.h"
#include "ui_savedoc.h"

#include <qfiledialog.h>

#include <cross-commons/app/paths.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/layeritems.h>
#include <meteo/commons/ui/map/event.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/proto/msgparser.pb.h>
#include <meteo/commons/global/global.h>

namespace meteo {
namespace map {

static const QString kEmptyDefaultName = QObject::tr("Пустой документ");

SaveDoc::SaveDoc(MapWindow* view ) :
  MapWidget(view),
  ui_(new Ui::SaveDoc),
  ext_("map_ptkpp")
{
  ui_->setupUi(this);
  updateLayers();
  if ( 0 != mapdocument() && 0 != mapdocument()->eventHandler() ) {
    mapdocument()->eventHandler()->installEventFilter(this);
  }
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
  ui_->lefile->clear();

  setDefaultName();
  turnSignals();
}

SaveDoc::~SaveDoc()
{
  delete ui_; ui_ = 0;
}

bool SaveDoc::eventFilter(QObject *watched, QEvent* event)
{
  if ( meteo::map::LayerEvent::LayerChanged == event->type() ) {
    LayerEvent* e = reinterpret_cast<LayerEvent*>(event);
    if ( LayerEvent::Added == e->changeType() ||  LayerEvent::Deleted == e->changeType() ) {
      updateLayers();
    }
  }
  return MapWidget::eventFilter(watched, event);
}

void SaveDoc::showEvent( QShowEvent* e )
{
  muteSignals();
  ui_->lefile->clear();
  setDefaultName();
  turnSignals();
  setWindowTitle( QObject::tr("Сохранить документ") + " '" + mapname() + "\'" );
  MapWidget::showEvent(e);
}

void SaveDoc::muteSignals()
{
  QObject::disconnect( ui_->okbtn, SIGNAL( clicked() ), this, SLOT( slotOkBtn() ) );
  QObject::disconnect( ui_->nobtn, SIGNAL( clicked() ), this, SLOT( slotNoBtn() ) );
  QObject::disconnect( ui_->btnfile, SIGNAL( clicked() ), this, SLOT( slotSelectFile() ) );
  QObject::disconnect( ui_->layerstree, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ), this, SLOT( slotItemChanged( QTreeWidgetItem*, int ) ) );
  QObject::disconnect( ui_->lefile, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotPathChanged() ) );
}

void SaveDoc::turnSignals()
{
  QObject::connect( ui_->okbtn, SIGNAL( clicked() ), this, SLOT( slotOkBtn() ) );
  QObject::connect( ui_->nobtn, SIGNAL( clicked() ), this, SLOT( slotNoBtn() ) );
  QObject::connect( ui_->btnfile, SIGNAL( clicked() ), this, SLOT( slotSelectFile() ) );
  QObject::connect( ui_->layerstree, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ), this, SLOT( slotItemChanged( QTreeWidgetItem*, int ) ) );
  QObject::connect( ui_->lefile, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotPathChanged() ) );
}

void SaveDoc::updateLayers()
{
  muteSignals();
  ui_->layerstree->clear();
  if ( 0 == mapdocument() ) {
    turnSignals();
    return;
  }
  Document* doc = mapdocument();
  QList<Layer*> list = doc->layers();
  QList<Layer*> keys = checkedlayers_.keys();
  for ( int i = 0, sz = keys.size(); i < sz; ++i ) {
    Layer* l = keys[i];
    if ( mapdocument()->itemsLayer() == l ) {
      continue;
    }
    if ( -1 == list.indexOf(l) ) {
      checkedlayers_.remove(l);
    }
  }
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    Layer* l = list[i];
    if ( mapdocument()->itemsLayer() == l ) {
      continue;
    }
    if ( true == l->isBase() ) {
      continue;
    }
    QTreeWidgetItem* item = new QTreeWidgetItem( ui_->layerstree );
    item->setFlags( item->flags() | Qt::ItemIsUserCheckable );
    item->setText( 0, l->name() );
    item->setData( 0, Qt::UserRole, l->uuid() );
    if ( false == checkedlayers_.contains(l) ) {
      checkedlayers_.insert( l, false == l->isBase() );
    }
    if ( true == checkedlayers_[l] ) {
      item->setCheckState( 0, Qt::Checked );
    }
    else {
      item->setCheckState( 0, Qt::Unchecked );
    }
  }
  turnSignals();
}

void SaveDoc::slotItemChanged( QTreeWidgetItem* item, int clmn )
{
  if ( 0 == item ) {
    return;
  }
  if ( 0 != clmn ) {
    return;
  }
  if ( 0 == mapdocument() ) {
    return;
  }
  QString uuid = item->data( 0, Qt::UserRole ).toString();
  bool checked = false;
  if ( Qt::Checked == item->checkState(0) ) {
    checked = true;
  }
  QList<Layer*> list = mapdocument()->layers();
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    Layer* l = list[i];
    if ( l->uuid() == uuid ) {
      checkedlayers_[l] = checked;
      break;
    }
  }
}

QString SaveDoc::lastPath() const
{
  return global::lastDocPath();
}
void SaveDoc::saveLastPath() const
{
  if ( true == ui_->lefile->text().isEmpty() ) {
    return;
  }
  QFile file( ui_->lefile->text() );
  global::saveLastDocPath( QFileInfo(file).absolutePath() );
}

QString SaveDoc::mapname() const
{
  QString txt = ui_->lefile->text();
  QFileInfo fi(txt);
  if ( false == fi.isDir() ) {
    txt = fi.fileName();
    if ( false == txt.isEmpty() ) {
      if ( -1 == txt.indexOf( kEmptyDefaultName ) ) {
        return txt;
      }
      else {
        txt = "";
      }
    }
  }
  txt = QString::fromStdString( mapdocument()->info().title() );
  QString termstr;
  if ( 0 != mapdocument() ) {
    termstr = mapdocument()->dateString();
  }
  if ( true == termstr.isEmpty() ) {
    termstr = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm");
  }
  if ( false == txt.isEmpty() ) {
    return txt + '_' + termstr;
  }
  return kEmptyDefaultName + '_' + termstr;
}

void SaveDoc::setDefaultName()
{
  QString txt = lastPath() + '/' + mapname();
  while ( -1 != txt.indexOf("//") ) {
    txt.replace("//","/");
  }
  ui_->lefile->setText(txt);
  QString mapname = QString::fromStdString( mapdocument()->info().title() );
  if ( true == mapname.isEmpty() ) {
    mapname = QObject::tr("Пустой документ");
  }
}

void SaveDoc::slotSelectFile()
{
  QFileDialog* dlg = new QFileDialog( this, windowTitle(), QFileInfo( ui_->lefile->text() ).absolutePath(), "*."+ext_ );
  dlg->setFileMode( QFileDialog::AnyFile );
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

void SaveDoc::slotPathChanged()
{
  saveLastPath();
}

void SaveDoc::slotOkBtn()
{
  if ( 0 == mapdocument() ) {
    return;
  }

  qApp->setOverrideCursor( Qt::WaitCursor );
  QStringList uuids;
  QMapIterator< Layer*, bool > it(checkedlayers_);
  while ( true == it.hasNext() ) {
    it.next();
    if ( true == it.value() ) {
      uuids.append( it.key()->uuid() );
    }
  }
  if ( 0 == uuids.size() ) {
    warning_log.msgBox() << QObject::tr("Не выбран ни один слой документа для сохранения!");
    qApp->restoreOverrideCursor();
    return;
  }
  QByteArray arr( mapdocument()->dataSize(uuids), '\0' );
  int32_t res = mapdocument()->serializeToArray( arr.data(), uuids );
  arr.resize(res);
  if ( 0 >= res ) {
    error_log.msgBox() << QObject::tr("Не удалось сохранить документ");
    qApp->restoreOverrideCursor();
    return;
  }
  saveFile(arr);
  qApp->restoreOverrideCursor();
  MapWidget::accept();
}

void SaveDoc::saveFile( const QByteArray& arr )
{
  QString nm = ui_->lefile->text();
  QFileInfo fi(nm);

  if ( ext_ != fi.suffix() ) {
    nm += "."+ ext_;
  }
  QFile file(nm);
  if ( false == file.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) {
    error_log.msgBox() << QObject::tr("Не удалось открыть файл для записи %1")
      .arg( file.fileName() );
    qApp->restoreOverrideCursor();
    return;
  }
  file.write(arr);
  file.flush();
  file.close();
}

void SaveDoc::slotNoBtn()
{
  MapWidget::reject();
}

}
}
