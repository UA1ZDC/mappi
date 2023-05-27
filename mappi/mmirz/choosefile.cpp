#include "choosefile.h"
#include "ui_choosefile.h"

#include <qgridlayout.h>
#include <qlayout.h>
#include <qfiledialog.h>

#include <cross-commons/debug/tlog.h>


ChooseFile::ChooseFile(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::ChooseFile)
{
   ui_->setupUi(this);
   ui_->file->setCursorPosition(0);
   connect(ui_->chooseBtn,SIGNAL(clicked()),this,SLOT(slotOpenDialog()));
   connect(ui_->file,SIGNAL(textChanged(QString)),this,SIGNAL(changed()));
}

ChooseFile::~ChooseFile()
{
  delete ui_; ui_ = 0;
}

void ChooseFile::slotOpenDialog()
{

  QFileDialog* dialog = new QFileDialog(this,QString::fromUtf8("Выбрать файл"), ui_->file->text());
  if (QDialog::Accepted == dialog->exec())
  {
    if (0 == dialog) { return; }
    QStringList list = dialog->selectedFiles();
    if (true == list.isEmpty()) { return; }
    QString str = list[0];
    ui_->file->setText(str);
  }
  dialog->close();
}

const QString ChooseFile::file() const
{
  return ui_->file->text();
}

void ChooseFile::setFile(const QString &file)
{
  ui_->file->setText(file);
}


