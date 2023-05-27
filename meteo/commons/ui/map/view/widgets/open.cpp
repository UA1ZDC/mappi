#include "open.h"
#include "ui_open.h"

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

OpenDocument::OpenDocument( MapWindow* w )
  : MapWidget(w),
  ui_(new Ui::OpenDocument)
{  
  ui_->setupUi(this);

  connect(ui_->loadButton, SIGNAL(clicked()), SLOT(open()));
  connect(ui_->cancelButton, SIGNAL(clicked()), SLOT(cancel()));
  connect(ui_->chooseBtn, SIGNAL(clicked()), SLOT(chooseFile()));
}

OpenDocument::~OpenDocument()
{
  clear();
}

void OpenDocument::keyPressEvent( QKeyEvent * e)
{
  if (e->key() == Qt::Key_Escape) {
    clear();
    MapWidget::keyPressEvent(e);
  }
}

void OpenDocument::closeEvent(QCloseEvent * e)
{
  clear();
  MapWidget::closeEvent(e);
}

void OpenDocument::clear()
{
  ui_->layers->clear();
  ui_->file->clear();
}

void OpenDocument::cancel()
{
  clear();
  MapWidget::close();
}

Document* OpenDocument::currentDocument() const
{
  if ( mapview() != 0 &&
      mapview()->mapscene() != 0 &&
      mapview()->mapscene()->document() != 0) {
    return mapview()->mapscene()->document();
  }

  return 0;
}

void OpenDocument::chooseFile()
{
  QString caption = QString::fromUtf8("Открытие документа");
  if (defaultDir_.isEmpty()) {
    defaultDir_ = readDefaultDir();
  }
  QString filter = QString::fromUtf8("Файл в формате AVGM (*.avgm)");
  
  QString fileName = QFileDialog::getOpenFileName(this, caption,
						  defaultDir_,
						  filter);
  
  if (!fileName.isEmpty()) {
    defaultDir_ = fileName.left(fileName.lastIndexOf("/") + 1);
  }
  
  ui_->file->setText(fileName);

  readLayers();
}

void OpenDocument::readLayers()
{
  ui_->layers->clear();

  QString fileName = ui_->file->text();
  if (fileName.isEmpty()) return;
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    error_log << meteo::msglog::kFileReadWriteFailed.arg(fileName).arg(file.error());
    return;
  }


  QDataStream ds(&file);
  char id[5];
  ds.readRawData(id, 5);
  // var(id);
  if (QString(id) != "AVGM") {
    error_log << QObject::tr("Ошибка загрузки данных: несоответствующий формат файла");
    file.close();
    return;
  }

  int layCount;
  ds >> layCount;
  //  var(layCount);
  for (int idx = 0; idx < layCount; idx++) {
    QString str;
    ds >> str;
    //var(str);
    QTreeWidgetItem* item = new QTreeWidgetItem(ui_->layers);
    item->setExpanded(false);
    item->setText(0, str);
    item->setSizeHint(0, QSize(32, 32));
  }
  file.close();
}

QString OpenDocument::readDefaultDir() 
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

bool OpenDocument::open()
{
  QString fileName = ui_->file->text();
  if (fileName.isEmpty()) return false;
  
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  bool ok = open(fileName);
  if (ok) {
    close();
  }
  QApplication::restoreOverrideCursor();    
  return ok;
}
      
bool OpenDocument::open(const QString& fileName)
{
  Q_UNUSED(fileName);
  return false;
}

}
}
