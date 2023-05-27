#include "save.h"
#include "ui_save.h"

#include <meteo/commons/ui/mainwindow/widgethandler.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <meteo/commons/global/log.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/proto/exportmap.pb.h>
#include <commons/textproto/tprototext.h>

#define DEFAULT_DIR MnCommon::etcPath() + "savedocument.conf"

namespace meteo {
namespace map {

SaveDocument::SaveDocument( MapWindow* p )
  : MapWidget(p),
  ui_(0),
  layers_(0),
  watchedEventHandler_(0)
{  

  ui_ = new Ui::SaveDocument;
  ui_->setupUi(this);

  layers_ = new LayersSelect( mapview(), this);
  QGridLayout* gridLayout2 = new QGridLayout(ui_->wlayers);
  gridLayout2->addWidget(layers_, 0, 0, 1, 1);
  gridLayout2->setContentsMargins(0, 0, 0, 0);

  int cnt = layers_->loadNonBaseLayers();
  layers_->setCheckedNonBaseLayers(true);
  if (0 != cnt) {
    ui_->info->hide();
  }

  connect(ui_->cancelButton, SIGNAL(clicked()), SLOT(cancel())); 
  connect(ui_->saveButton, SIGNAL(clicked()), SLOT(save()));

  installWatchedDocument();
}

SaveDocument::~SaveDocument()
{
  clear();
  if (0 != ui_) {
    delete ui_;
    ui_ = 0;
  }
  resetWatchedDocument();
}

void SaveDocument::clear()
{
}

Document* SaveDocument::currentDocument() const
{
  if (mapview() != 0 &&
      mapview()->mapscene() != 0 &&
      mapview()->mapscene()->document() != 0) {
    return mapview()->mapscene()->document();
  }

  return 0;
}

bool SaveDocument::installWatchedDocument()
{
  Document* current = currentDocument();

  if (0 != current &&
      current->eventHandler() != 0) {
    watchedEventHandler_ = current->eventHandler();
    watchedEventHandler_->installEventFilter(this);

    return true;
  }
  return false;
}

void SaveDocument::resetWatchedDocument()
{
  if (watchedEventHandler_ != 0) {
    this->removeEventFilter(watchedEventHandler_);
    watchedEventHandler_ = 0;
  }
}

bool SaveDocument::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == watchedEventHandler_) {
    if (event->type() == map::LayerEvent::LayerChanged) {
      int cnt = layers_->loadNonBaseLayers();
      layers_->setCheckedNonBaseLayers(true);
      if (0 != cnt) {
	ui_->info->hide();
      } else {
	ui_->info->show();
      }
    } 
  }

  return MapWidget::eventFilter(watched, event);
}

void SaveDocument::keyPressEvent( QKeyEvent * e)
{
  if (e->key() == Qt::Key_Escape) {
    clear();
    MapWidget::keyPressEvent(e);
  }
}

void SaveDocument::closeEvent(QCloseEvent*e)
{
  clear();
  MapWidget::closeEvent(e);
}

void SaveDocument::cancel()
{ 
  clear();
  MapWidget::close();
}

QString SaveDocument::chooseFile()
{
  QString caption = QString::fromUtf8("Сохранение документа");
  if (defaultDir_.isEmpty()) {
    defaultDir_ = readDefaultDir();
  }
  QString filter = QString::fromUtf8("Файл в формате AVGM (*.avgm)");
  
  QString fileName = QFileDialog::getSaveFileName(this, caption,
						  defaultDir_,
						  filter);
  
  if (!fileName.isEmpty()) {
    defaultDir_ = fileName.left(fileName.lastIndexOf("/") + 1);
    saveDefaultDir(defaultDir_);
    
    if (!fileName.endsWith(".avgm")) {
      fileName += QString(".avgm");
    }
    
  }
  return fileName;
}


void SaveDocument::saveDefaultDir(const QString& dir) const
{
  meteo::map::exportmap::ExportPath p;
  p.set_path(dir.toStdString());
  
  QString text = TProtoText::toText(p);
  if (text.isEmpty()) return;
  
  
  QFile file(DEFAULT_DIR);
  if ( !file.open(QIODevice::WriteOnly) ) {
    return;
  }
  file.write(text.toUtf8());
  file.close();
  
}

QString SaveDocument::readDefaultDir() 
{
  QString dir = QDir::homePath();
  
  QFile file(DEFAULT_DIR);
  if ( !file.open(QIODevice::ReadOnly) ) {
    return dir;
  }
  
  QString text = QString::fromUtf8(file.readAll());
  file.close();
  
  meteo::map::exportmap::ExportPath p;
  if ( !TProtoText::fillProto(text, &p) ) {
    file.remove(DEFAULT_DIR);
    return dir;
  }
  
  return QString::fromStdString(p.path());
}

void SaveDocument::save()
{
  QString fileName = chooseFile();
  if (fileName.isEmpty()) return;
  
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  bool ok = save(fileName);
  if (ok) {
    close();
  }
  QApplication::restoreOverrideCursor();    
}
      
bool SaveDocument::save(const QString& fileName)
{
  Q_UNUSED(fileName);
  return false;
}

}
}

