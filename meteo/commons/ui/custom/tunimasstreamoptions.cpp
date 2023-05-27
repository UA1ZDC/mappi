#include "ui_unimasstreamoptions.h"
#include "tunimasstreamoptions.h"

#include <cross-commons/debug/tlog.h>

#include <qobject.h>
#include <qevent.h>
#include <qdebug.h>
#include <qstring.h>

TUnimasStreamOptions::TUnimasStreamOptions(QWidget * p):
  QWidget(p)
{
  _ui = new Ui::UnimasStreamOptions;
  _ui->setupUi( this );
}

TUnimasStreamOptions::~TUnimasStreamOptions()
{
}


void TUnimasStreamOptions::showSender()
{
  _ui->groupBoxReceiver->hide();
  _ui->groupBoxSender->show();
  adjustSize();
}

void TUnimasStreamOptions::showReceiver()
{
  _ui->groupBoxSender->hide();
  _ui->groupBoxReceiver->show();
  adjustSize();
}

bool TUnimasStreamOptions::setTypeRec(int value)
{
  _ui->typeComboRec->setCurrentIndex(value);
  return true;
}

bool TUnimasStreamOptions::setModeRec(int value)
{
  _ui->modeComboRec->setCurrentIndex(value);
  return true;
}

bool TUnimasStreamOptions::setHostRec(const QString& value)
{
  _ui->hostEditRec->setText(value);
  return true;
}

bool TUnimasStreamOptions::setPortRec(int value)
{
  _ui->portSpinboxRec->setValue(value);
  return true;
}

bool TUnimasStreamOptions::setSessionTimeoutRec(int value)
{
  _ui->sessionTimeoutSpinboxRec->setValue(value);
  return true;
}

bool TUnimasStreamOptions::setAskTimeoutRec(int value)
{
  _ui->askTimeoutSpinboxRec->setValue(value);
  return true;
}

bool TUnimasStreamOptions::setUnprocessedPathRec(const QString& value)
{
  _ui->unprocPathEditRec->setText(value);
  return true;
}

bool TUnimasStreamOptions::setCompressModeRec(int value)
{
  _ui->compressComboRec->setCurrentIndex(value);
  return true;
}

int TUnimasStreamOptions::getTypeRec()
{
  return _ui->typeComboRec->currentIndex();
}

int TUnimasStreamOptions::getModeRec()
{
  return _ui->modeComboRec->currentIndex();
}

QString TUnimasStreamOptions::getHostRec()
{
  return _ui->hostEditRec->text();
}

int TUnimasStreamOptions::getPortRec()
{
  return _ui->portSpinboxRec->value();
}

int TUnimasStreamOptions::getSessionTimeoutRec()
{
  return _ui->sessionTimeoutSpinboxRec->value();
}

int TUnimasStreamOptions::getAskTimeoutRec()
{
  return _ui->askTimeoutSpinboxRec->value();
}

QString TUnimasStreamOptions::getUnprocessedPathRec()
{
  return _ui->unprocPathEditRec->text();
}

int TUnimasStreamOptions::getCompressModeRec()
{
  return _ui->compressComboRec->currentIndex();
}

bool TUnimasStreamOptions::setTypeSend(int value)
{
  _ui->typeComboSend->setCurrentIndex(value);
  return true;
}

bool TUnimasStreamOptions::setModeSend(int value)
{
  _ui->modeComboSend->setCurrentIndex(value);
  return true;
}

bool TUnimasStreamOptions::setPortSend(int value)
{
  _ui->portSpinboxSend->setValue(value);
  return true;
}

bool TUnimasStreamOptions::setAskTimeoutSend(int value)
{
  _ui->askTimeoutSpinboxSend->setValue(value);
  return true;
}

bool TUnimasStreamOptions::setCompressModeSend(int value)
{
  _ui->compressComboSend->setCurrentIndex(value);
  return true;
}

int TUnimasStreamOptions::getTypeSend()
{
  return _ui->typeComboSend->currentIndex();
}

int TUnimasStreamOptions::getModeSend()
{
  return _ui->modeComboSend->currentIndex();
}

int TUnimasStreamOptions::getPortSend()
{
  return _ui->portSpinboxSend->value();
}

int TUnimasStreamOptions::getAskTimeoutSend()
{
  return _ui->askTimeoutSpinboxSend->value();
}

int TUnimasStreamOptions::getCompressModeSend()
{
  return _ui->compressComboSend->currentIndex();
}
