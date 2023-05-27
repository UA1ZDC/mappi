#include "documentstatus.h"
#include "ui_documentstatus.h"

#include <cross-commons/app/paths.h>

#include <qsettings.h>
#include <qmessagebox.h>
#include <qevent.h>
#include <qdir.h>

namespace meteo {

DocumentStatus::DocumentStatus(QWidget *parent) :
  QDialog(parent, Qt::WindowCloseButtonHint),
  ui_(new Ui::DocumentStatus),
  settings_(new QSettings(QDir::homePath() + "/." + MnCommon::applicationName() + "/dataloader.ini", QSettings::IniFormat))
{
  ui_->setupUi(this);
  setWindowTitle("Статус");
  loadSettings();
  ui_->docStat->setText("<font color='red'>Выполняется...</font>");
  QObject::connect(ui_->closeBtn, SIGNAL(clicked()), SLOT(close()));
  QObject::connect(ui_->closeCheck, SIGNAL(stateChanged(int)), SLOT(slotSaveSettings()));
}

DocumentStatus::~DocumentStatus()
{
  slotSaveSettings();
  delete ui_;
  ui_ = nullptr;
  delete settings_;
  settings_ = nullptr;
}

void DocumentStatus::slotDocumentDone()
{
  documentSt_ = true;
  ui_->docStat->setText("<font color='green'>Завершено</font>");
  checkDone();
}

void DocumentStatus::slotSetNotDone()
{
  ui_->docStat->setText("<font color='red'>Выполняется...</font>");
  documentSt_ = false;
}

void DocumentStatus::slotSaveSettings()
{
  if ( nullptr == settings_ ) {
    return;
  }
  settings_->setValue("documentStatus", ui_->closeCheck->isChecked());
}

void DocumentStatus::loadSettings()
{
  if ( nullptr == settings_ ) {
    return;
  }
  if ( true == settings_->contains("documentStatus") ) {
    ui_->closeCheck->setChecked(settings_->value("documentStatus").toBool());
  }
}

void DocumentStatus::checkDone()
{
  if ( ui_->closeCheck->isChecked() && documentSt_ ) {
    close();
  }
}

void DocumentStatus::closeEvent(QCloseEvent *e)
{
  if ( !documentSt_ ) {
    QMessageBox dlg;
    dlg.setWindowTitle(QObject::tr("Внимание!"));
    dlg.setText(QObject::tr("Идёт формирование документов. Завершить?"));
    dlg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    dlg.setDefaultButton(QMessageBox::Yes);
    int ret = dlg.exec();
    if ( ret == QMessageBox::Yes ) {
      emit stopDocument();
      QDialog::closeEvent(e);
    }
    else {
      e->ignore();
    }
  }
  else {
    QDialog::closeEvent(e);
  }
}

}
