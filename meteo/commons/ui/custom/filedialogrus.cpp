#include "filedialogrus.h"

namespace meteo {

FileDialog::FileDialog(QWidget * parent, Qt::WindowFlags flags):
  QFileDialog(parent, flags) 
{ 
  setButtons(); 
}
FileDialog::FileDialog(QWidget * parent /*= 0*/, const QString & caption /*= QString()*/, 
		       const QString & directory /*= QString()*/, const QString & filter /*= QString()*/):
      QFileDialog(parent, caption, directory, filter)
{
  setButtons();
}

FileDialog::~FileDialog()
{
}

void FileDialog::setButtons()
{
  setLabelText(QFileDialog::LookIn, QObject::tr("Перейти к"));
  setLabelText(QFileDialog::FileName, QObject::tr("Имя файла"));
  setLabelText(QFileDialog::FileType, QObject::tr("Типы файлов"));	
  setLabelText(QFileDialog::Accept, QObject::tr("Выбрать"));
  setLabelText(QFileDialog::Reject, QObject::tr("Закрыть"));	
}


QString FileDialog::getSaveFileName(QWidget * parent /*= 0*/, const QString & caption /*= QString()*/, 
				    const QString & dir /*= QString()*/, const QString & filter /*= QString()*/, 
				    QString * selectedFilter /*= 0*/, Options options /*= 0*/)
{
  FileDialog dialog(parent, caption, dir, filter);
  if (options != 0) {
    dialog.setOptions(options);
  }

  dialog.setAcceptMode(AcceptSave);
  dialog.setButtons();
  
  if (selectedFilter && !selectedFilter->isEmpty()) {
    dialog.selectNameFilter(*selectedFilter);
  }

  if (dialog.exec() == QDialog::Accepted) {
    if (selectedFilter)
      *selectedFilter = dialog.selectedNameFilter();
    return dialog.selectedFiles().value(0);
  }
  
  return QString();
}


QString FileDialog::getOpenFileName(QWidget * parent /*= 0*/, const QString & caption /*= QString()*/, 
					   const QString & dir /*= QString()*/, const QString & filter /*= QString()*/, 
					   QString * selectedFilter /*= 0*/, Options options /*= 0*/)
{
  FileDialog dialog(parent, caption, dir, filter);
  if (options != 0) {
    dialog.setOptions(options);
  }

  dialog.setFileMode(QFileDialog::ExistingFile);

  if (selectedFilter && !selectedFilter->isEmpty()) {
    dialog.selectNameFilter(*selectedFilter);
  }

  if (dialog.exec() == QDialog::Accepted) {
    if (selectedFilter)
      *selectedFilter = dialog.selectedNameFilter();
    return dialog.selectedFiles().value(0);
  }
  return QString();
}

}
