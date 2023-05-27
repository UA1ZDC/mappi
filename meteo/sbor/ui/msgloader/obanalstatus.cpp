#include "obanalstatus.h"
#include "ui_obanalstatus.h"
#include <cross-commons/app/paths.h>

#include <qmessagebox.h>
#include <QCloseEvent>
#include <qdir.h>

namespace meteo {

ObanalStatus::ObanalStatus(QDialog *parent) :
  QDialog(parent, Qt::WindowCloseButtonHint),
  ui_(new Ui::ObanalStatus),
  settings_(new QSettings(QDir::homePath() + "/." + MnCommon::applicationName() + "/dataloader.ini", QSettings::IniFormat))
{
  ui_->setupUi(this);
  setWindowTitle("Статус");
  loadSettings();
  ui_->aeroStat->setText("<font color='red'>Выполняется...</font>");
  ui_->oceanStat->setText("<font color='red'>Выполняется...</font>");
  ui_->surfStat->setText("<font color='red'>Выполняется...</font>");
  ui_->gribStat->setText("<font color='red'>Выполняется...</font>");
  QObject::connect(ui_->closeBtn, SIGNAL(clicked()), SLOT(close()));
  QObject::connect(ui_->closeCheck, SIGNAL(stateChanged(int)), SLOT(slotSaveSettings()));
}

ObanalStatus::~ObanalStatus()
{
  slotSaveSettings();
  delete ui_;
  ui_ = nullptr;
  delete settings_;
  settings_ = nullptr;
}

void ObanalStatus::slotAeroDone()
{
  aeroSt_ = true;
  ui_->aeroStat->setText("<font color='green'>Завершено</font>");
  checkAllDone();
}

void ObanalStatus::slotOceanDone()
{
  oceanSt_ = true;
  ui_->oceanStat->setText("<font color='green'>Завершено</font>");
  checkAllDone();
}

void ObanalStatus::slotSurfDone()
{
  surfSt_ = true;
  ui_->surfStat->setText("<font color='green'>Завершено</font>");
  checkAllDone();
}

void ObanalStatus::slotGribDone()
{
  gribSt_ = true;
  ui_->gribStat->setText("<font color='green'>Завершено</font>");
  checkAllDone();
}

void ObanalStatus::slotSetAllNotDone()
{
  ui_->aeroStat->setText("<font color='red'>Выполняется...</font>");
  ui_->oceanStat->setText("<font color='red'>Выполняется...</font>");
  ui_->surfStat->setText("<font color='red'>Выполняется...</font>");
  ui_->gribStat->setText("<font color='red'>Выполняется...</font>");
  aeroSt_ = false;
  oceanSt_ = false;
  surfSt_ = false;
  gribSt_ = false;
}

void ObanalStatus::slotSaveSettings()
{
  if ( nullptr == settings_ ) {
    return;
  }
  settings_->setValue("obanalStatus", ui_->closeCheck->isChecked());
}

void ObanalStatus::loadSettings()
{
  if ( nullptr == settings_ ) {
    return;
  }
  if ( true == settings_->contains("obanalStatus") ) {
    ui_->closeCheck->setChecked(settings_->value("obanalStatus").toBool());
  }
}

void ObanalStatus::checkAllDone()
{
  if ( ui_->closeCheck->isChecked()
       && aeroSt_
       && oceanSt_
       && surfSt_
       && gribSt_ ) {
    close();
  }
}

void ObanalStatus::closeEvent(QCloseEvent *e)
{
  if ( !aeroSt_
       || !oceanSt_
       || !surfSt_
       || !gribSt_ ) {
    QMessageBox dlg;
    dlg.setWindowTitle(QObject::tr("Внимание!"));
    dlg.setText(QObject::tr("Объективный анализ запущен. Завершить?"));
    dlg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    dlg.setDefaultButton(QMessageBox::Yes);
    int ret = dlg.exec();
    if ( ret == QMessageBox::Yes ) {
      emit stopObanal();
      QDialog::closeEvent(e);
    }
    else {
      e->ignore();
    }
  }
  else {
    emit finished();
    QDialog::closeEvent(e);
  }
}

}
