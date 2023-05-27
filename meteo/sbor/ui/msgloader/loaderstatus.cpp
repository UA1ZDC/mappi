#include "loaderstatus.h"
#include "ui_loaderstatus.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <qmessagebox.h>
#include <qevent.h>
#include <qdir.h>

namespace meteo {

LoaderStatus::LoaderStatus(QWidget *parent) :
  QDialog(parent, Qt::WindowCloseButtonHint),
  ui_(new Ui::LoaderStatus),
  settings_(new QSettings(QDir::homePath() + "/." + MnCommon::applicationName() + "/dataloader.ini", QSettings::IniFormat))
{
  ui_->setupUi(this);
  loadSettings();
  QObject::connect(ui_->closeBtn, SIGNAL(clicked()), SLOT(close()));
  QObject::connect(this, SIGNAL(signalPrepareToClose()), SLOT(slotClose()));
  QObject::connect(ui_->closeCheck, SIGNAL(stateChanged(int)), SLOT(slotSaveSettings()));
  ui_->statusLbl->setText("<font color='red'>Идёт загрузка...</font>");
}

LoaderStatus::~LoaderStatus()
{
  slotSaveSettings();
  delete ui_;
  ui_ = nullptr;
  delete settings_;
  settings_ = nullptr;
}

void LoaderStatus::slotSetValue(int value)
{
  if( ui_->progressBar->minimum() == value ) {
    ui_->statusLbl->setText("<font color='red'>Идёт загрузка...</font>");
  }
  else if ( ui_->progressBar->maximum() == value ) {
    ui_->statusLbl->setText("<font color='green'>Загрузка завершена.</font>");
  }
  ui_->progressBar->setValue(value);
}

void LoaderStatus::slotSetMaxValue(int maxValue)
{
  ui_->progressBar->setMaximum(maxValue);
}

void LoaderStatus::slotIncrementValue()
{
  ui_->progressBar->setValue(ui_->progressBar->value()+1);
  if ( ui_->progressBar->value() == ui_->progressBar->maximum() ) {
    ui_->statusLbl->setText("<font color='green'>Загрузка завершена.</font>");
    emit signalPrepareToClose();
  }
}

void LoaderStatus::slotClose()
{
  if ( true == ui_->closeCheck->isChecked() ) {
    close();
  }
}

void LoaderStatus::closeEvent(QCloseEvent *e)
{
  if ( ui_->progressBar->value() < ui_->progressBar->maximum() ) {
    QMessageBox dlg;
    dlg.setWindowTitle(QObject::tr("Внимание!"));
    dlg.setText(QObject::tr("Загрузка телеграмм не завершена. Прервать?"));
    dlg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    dlg.setDefaultButton(QMessageBox::Yes);
    int ret = dlg.exec();
    if ( ret == QMessageBox::Yes ) {
      emit signalStop();
      QDialog::closeEvent(e);
    }
    else {
      e->ignore();
    }
  }
  else {
    emit signalFinished();
    QDialog::closeEvent(e);
  }
}

void LoaderStatus::loadSettings()
{
  if ( nullptr == settings_ ) {
    return;
  }
  if ( true == settings_->contains("statusLoader") ) {
    ui_->closeCheck->setChecked(settings_->value("statusLoader").toBool());
  }
}

void LoaderStatus::slotSaveSettings()
{
  if ( nullptr == settings_ ) {
    return;
  }
  settings_->setValue("statusLoader", ui_->closeCheck->isChecked());
}

}
