#include "choosedir.h"
#include "ui_choosedir.h"

#include <qgridlayout.h>
#include <qlayout.h>
#include <qfiledialog.h>

#include <cross-commons/debug/tlog.h>


ChooseDir::ChooseDir(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::ChooseDir)
{
   ui_->setupUi(this);
   ui_->dir->setCursorPosition(0);
   connect(ui_->chooseBtn,SIGNAL(clicked()),this,SLOT(slotOpenDialog()));
   connect(ui_->dir,SIGNAL(textChanged(QString)),this,SIGNAL(changed()));
}

ChooseDir::~ChooseDir()
{
  delete ui_; ui_ = 0;
}

void ChooseDir::slotOpenDialog()
{

  QFileDialog* dialog = new QFileDialog(this,QString::fromUtf8("Выбрать директорию"), ui_->dir->text());
  dialog->setOption(QFileDialog::DontUseNativeDialog);
  dialog->setFileMode(QFileDialog::Directory);
  if (QDialog::Accepted == dialog->exec())
  {
    if (0 == dialog) { return; }
    QDir dir = dialog->directory();
    if (false == dir.exists()) { return; }
    ui_->dir->setText(dir.absolutePath());
  }
  dialog->close();
}

const QString ChooseDir::dir() const
{
  return ui_->dir->text();
}

void ChooseDir::setDir(const QString &dir)
{
  ui_->dir->setText(dir);
}


