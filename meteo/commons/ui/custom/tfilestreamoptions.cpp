#include "ui_filestreamoptions.h"
#include "tfilestreamoptions.h"

#include <cross-commons/debug/tlog.h>

#include <qobject.h>
#include <qevent.h>
#include <qdebug.h>
#include <qstring.h>

TFileStreamOptions::TFileStreamOptions(QWidget * p):
  QWidget(p)
{
  _ui = new Ui::FileStreamOptions;
  _ui->setupUi( this );

  _ui->encodeCombo->addItem("utf-8","utf-8");
  _ui->encodeCombo->addItem("koi8-r","koi8-r");
  _ui->encodeCombo->addItem("cp-1251","cp-1251");
}

TFileStreamOptions::~TFileStreamOptions()
{

}

void TFileStreamOptions::showSender()
{
  _ui->groupBoxFileReceiver->hide();
  _ui->groupBoxFileSender->show();
  adjustSize();
}

void TFileStreamOptions::showReceiver()
{
  _ui->groupBoxFileSender->hide();
  _ui->groupBoxFileReceiver->show();
  adjustSize();
}

bool TFileStreamOptions::setPath(const QString& value)
{
  _ui->pathIOEdit->setText(value);
  return true;
}

bool TFileStreamOptions::setReceiverFileTempl(const QString& value)
{
  _ui->fileTemplEdit->setText(value);
  return true;
}

bool TFileStreamOptions::setReceiverDeleteFiles(const QString& value)
{
  if (value.startsWith("t")){
    _ui->deleteFilesCheck->setChecked(true);
  }
  else {
    _ui->deleteFilesCheck->setChecked(false);
  }
  return true;
}

bool TFileStreamOptions::setReceiverEncoding(const QString& value)
{
  _ui->encodeCombo->setCurrentIndex(_ui->encodeCombo->findText(value));
  return true;
}

bool TFileStreamOptions::setReceiverMetaTempl(const QString& value)
{
  _ui->metaTemplEdit->setText(value);
  return true;
}

bool TFileStreamOptions::setReceiverWaitForRead(int value)
{
  _ui->waitReadSpinbox->setValue(value);
  return true;
}

bool TFileStreamOptions::setSenderMsgLimit(int value)
{
  _ui->limitMsgSpinbox->setValue(value);
  return true;
}

bool TFileStreamOptions::setSenderSizeLimit(int value)
{
  _ui->limitSizeSpinbox->setValue(value);
  return true;
}

bool TFileStreamOptions::setSenderTimeLimit(int value)
{
  _ui->limitTimeSpinbox->setValue(value);
  return true;
}

bool TFileStreamOptions::setSenderMask(const QString& value)
{
  _ui->maskEdit->setText(value);
  return true;
}

QString TFileStreamOptions::getPath()
{
  return _ui->pathIOEdit->text();
}

QString TFileStreamOptions::getReceiverFileTempl()
{
  return _ui->fileTemplEdit->text();
}

bool TFileStreamOptions::getReceiverDeleteFiles()
{
  return _ui->deleteFilesCheck->isChecked();
}

QString TFileStreamOptions::getReceiverEncoding()
{
  return _ui->encodeCombo->currentText();
}

QString TFileStreamOptions::getReceiverMetaTempl()
{
  return _ui->metaTemplEdit->text();
}

int TFileStreamOptions::getReceiverWaitForRead()
{
  return _ui->waitReadSpinbox->value();
}

int TFileStreamOptions::getSenderMsgLimit()
{
  return _ui->limitMsgSpinbox->value();
}

int TFileStreamOptions::getSenderSizeLimit()
{
  return _ui->limitSizeSpinbox->value();
}

int TFileStreamOptions::getSenderTimeLimit()
{
  return _ui->limitTimeSpinbox->value();
}

QString TFileStreamOptions::getSenderMask()
{
  return _ui->maskEdit->text();
}
