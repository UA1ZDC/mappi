#include "saveiface.h"
#include "ui_saveiface.h"
#include "filedialogrus.h"

#include <commons/textproto/tprototext.h>



using namespace meteo;

SaveIface::SaveIface(const QString& confFile, QWidget* parent/* = 0*/, Qt::WindowFlags fl /*= 0*/):
  QDialog(parent, fl),
  _confFile(confFile)
{
  _ui = new Ui::SaveIface;
  _ui->setupUi(this);

  connect(_ui->btnfile, SIGNAL(clicked()), SLOT(chooseFile()));
  connect(_ui->cancelButton, SIGNAL(clicked()), SLOT(reject()));
  connect(_ui->saveButton, SIGNAL(clicked()), SLOT(save()));

  readConfig();
}

SaveIface::~SaveIface()
{
  if (0 != _ui) {
    delete _ui;
    _ui = 0;
  }
}

//! название документа
void SaveIface::setDefaultDoc(const QString& name)
{
  _ui->lename->setText(name);
}

//! имя файла и фильтр для расширений
void SaveIface::setDefaultFile(const QString& name, const QString& filter)
{
  _filter = filter;
  QString txt = QString::fromStdString(_conf.path()) + '/' + name;
  while ( -1 != txt.indexOf("//") ) {
    txt.replace("//","/");
  }
  _ui->lefile->setText(txt);
}

//! true - необходимо сохранить в БД, name - название документа
bool SaveIface::isSaveInner(QString* name /*= 0*/)
{
  if (_ui->dbcheck->checkState() == Qt::Checked) {
    if (0 != name) {
      *name = _ui->lename->text();
    }
    return true;
  }

  return false;
}

//! true - необходимо сохранить в файл, name - название файла
bool SaveIface::isSaveFile(QString* name /*= 0*/)
{
  if (_ui->filecheck->checkState() == Qt::Checked) {
    if (0 != name) {
      *name = _ui->lefile->text();
    }
    return true;
  }

  return false;  
}

void SaveIface::chooseFile()
{
  QString caption = QString::fromUtf8("Сохранение данных");

  QString  selectedFilter;
  QString fileName = FileDialog::getSaveFileName(this, caption,
						 _ui->lefile->text(),
						 _filter, 
						 &selectedFilter);

  if (!fileName.isEmpty()) {
    QRegExp rx("\\w\\s[(*]{2}(.\\w{3,4})");
    rx.indexIn(selectedFilter);
    
    selectedFilter = rx.cap(1);
    
    if (fileName.endsWith(selectedFilter) == false) {
      fileName += QString(selectedFilter);
    }
    _ui->lefile->setText(fileName);
  }
}

void SaveIface::save()
{
  QFileInfo fi(_ui->lefile->text());
  QString path = fi.dir().path();
  QDir dir;
  dir.mkpath(path);

  saveConfig(path);

  emit saveClicked();
}

void SaveIface::readConfig()
{
  _conf.set_path(QDir::homePath().toStdString());
  
  QFile file(_confFile);
  if ( !file.open(QIODevice::ReadOnly) ) {
    return;
  }
  
  QString text = QString::fromUtf8(file.readAll());
  file.close();
  
  meteo::map::exportmap::ExportPath p;
  if ( !TProtoText::fillProto(text, &p) ) {
    file.remove(_confFile);
  } else {
    _conf = p;
  }
}

void SaveIface::saveConfig(const QString& dir)
{
  _conf.set_path(dir.toStdString());
  _conf.set_save_fs(_ui->filecheck->checkState() == Qt::Checked);
  _conf.set_save_db(_ui->dbcheck->checkState() == Qt::Checked);

  QString text = TProtoText::toText(_conf);
  if (text.isEmpty()) return;


  QFile file(_confFile);
  if ( !file.open(QIODevice::WriteOnly) ) {
    return;
  }
  file.write(text.toUtf8());
  file.close();
}

QString SaveIface::defaultDir() const
{
  return QString::fromStdString(_conf.path());
}

void SaveIface::setDefaultDir(const QString& name)
{
  saveConfig(name);
}
