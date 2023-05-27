#include "ui_ftpstreamoptions.h"
#include "tftpstreamoptions.h"

#include <cross-commons/debug/tlog.h>

#include <qobject.h>
#include <qevent.h>
#include <qdebug.h>
#include <qstring.h>

TFtpStreamOptions::TFtpStreamOptions(QWidget * p):
  QWidget(p)
{
  _ui = new Ui::FtpStreamOptions;
  _ui->setupUi( this );
  _ui->encodeCombo->addItem("utf-8","utf-8");
  _ui->encodeCombo->addItem("koi8-r","koi8-r");
  _ui->encodeCombo->addItem("cp-1251","cp-1251");
}

TFtpStreamOptions::~TFtpStreamOptions()
{ 
}

void TFtpStreamOptions::showSender()
{
  _ui->groupBoxFtpReceiver->hide();
  _ui->groupBoxFtpSender->show();
  adjustSize();
}

void TFtpStreamOptions::showReceiver()
{
  _ui->groupBoxFtpSender->hide();
  _ui->groupBoxFtpReceiver->show();
  adjustSize();
}

bool TFtpStreamOptions::setPath(const QString& value)
{
  _ui->pathIOEdit->setText(value);
  return true;
}

bool TFtpStreamOptions::setSrvHost(const QString& value)
{
  _ui->hostSrvEdit->setText(value);
  return true;
}

bool TFtpStreamOptions::setSrvPort(int value)
{
  _ui->portSrvSpinbox->setValue(value);
  return true;
}

bool TFtpStreamOptions::setSrvUser(const QString& value)
{
  _ui->userSrvEdit->setText(value);
  return true;
}

bool TFtpStreamOptions::setSrvPass(const QString& value)
{
  _ui->passSrvEdit->setText(value);
  return true;
}

bool TFtpStreamOptions::setReceiverFileTempl(const QString& value)
{
  _ui->fileTemplEdit->setText(value);
  return true;
}

bool TFtpStreamOptions::setReceiverDeleteFiles(const QString& value)
{
  if (value.startsWith("t")){
    _ui->deleteFilesCheck->setChecked(true);
  }
  else {
    _ui->deleteFilesCheck->setChecked(false);
  }
  return true;
}

bool TFtpStreamOptions::setReceiverEncoding(const QString& value)
{
  _ui->encodeCombo->setCurrentIndex(_ui->encodeCombo->findText(value));
  return true;
}

bool TFtpStreamOptions::setReceiverMetaTempl(const QString& value)
{
  _ui->metaTemplEdit->setText(value);
  return true;
}

bool TFtpStreamOptions::setReceiverWaitForRead(int value)
{
  _ui->waitReadSpinbox->setValue(value);
  return true;
}

bool TFtpStreamOptions::setReceiverSessionTimeout(int value)
{
  _ui->timeoutSpinbox->setValue(value);
  return true;
}

bool TFtpStreamOptions::setReceiverUnprocessedPath(const QString& value)
{
  _ui->unprocPathEdit->setText(value);
  return true;
}

bool TFtpStreamOptions::setSenderMsgLimit(int value)
{
  _ui->limitMsgSpinbox->setValue(value);
  return true;
}

bool TFtpStreamOptions::setSenderSizeLimit(int value)
{
  _ui->limitSizeSpinbox->setValue(value);
  return true;
}

bool TFtpStreamOptions::setSenderTimeLimit(int value)
{
  _ui->limitTimeSpinbox->setValue(value);
  return true;
}

bool TFtpStreamOptions::setSenderMask(const QString& value)
{
  _ui->maskEdit->setText(value);
  return true;
}

QString TFtpStreamOptions::getPath()
{
  return _ui->pathIOEdit->text();
}

QString TFtpStreamOptions::getSrvHost()
{
  return _ui->hostSrvEdit->text();
}

int TFtpStreamOptions::getSrvPort()
{
  return _ui->portSrvSpinbox->value();
}

QString TFtpStreamOptions::getSrvUser()
{
  return _ui->userSrvEdit->text();
}

QString TFtpStreamOptions::getSrvPass()
{
  return _ui->passSrvEdit->text();
}

QString TFtpStreamOptions::getReceiverFileTempl()
{
  return _ui->fileTemplEdit->text();
}

bool TFtpStreamOptions::getReceiverDeleteFiles()
{
  return _ui->deleteFilesCheck->isChecked();
}

QString TFtpStreamOptions::getReceiverEncoding()
{
  return _ui->encodeCombo->currentText();
}

QString TFtpStreamOptions::getReceiverMetaTempl()
{
  return _ui->metaTemplEdit->text();
}

int TFtpStreamOptions::getReceiverWaitForRead()
{
  return _ui->waitReadSpinbox->value();
}

int TFtpStreamOptions::getReceiverSessionTimeout()
{
  return _ui->timeoutSpinbox->value();
}

QString TFtpStreamOptions::getReceiverUnprocessedPath()
{
  return _ui->unprocPathEdit->text();
}

int TFtpStreamOptions::getSenderMsgLimit()
{
  return _ui->limitMsgSpinbox->value();
}

int TFtpStreamOptions::getSenderSizeLimit()
{
  return _ui->limitSizeSpinbox->value();
}

int TFtpStreamOptions::getSenderTimeLimit()
{
  return _ui->limitTimeSpinbox->value();
}

QString TFtpStreamOptions::getSenderMask()
{
  return _ui->maskEdit->text();
}
