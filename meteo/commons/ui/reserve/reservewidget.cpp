#include <qscrollbar.h>
#include <qdir.h>
#include <qdatetime.h>
#include <qmessagebox.h>

//#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/global/global.h>

#include "reservewidget.h"
#include "ui_reserve.h"
#include "datasaver.h"

namespace meteo{

ReserveWidget::ReserveWidget(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ReserveWidget),
  saveDir_(QDir::homePath() + "/restore/")
{
  ui->setupUi(this);
  new TextHightlighter(ui->console->document());
  ui->saveDir->setDir(saveDir_);
  ui->restoreDir->setDir(saveDir_);
  connect(ui->execBtn, SIGNAL(clicked(bool)), SLOT(save()));
  connect(ui->restoreBtn, SIGNAL(clicked(bool)), SLOT(restore()));
  connect(ui->saveDir, SIGNAL(changed()), SLOT(slotChangeSaveDir()));
  connect(ui->restoreDir, SIGNAL(changed()), SLOT(slotChangeRestoreDir()));
  connect(ui->closeBtn, SIGNAL(clicked(bool)), SLOT(slotClose()));
  connect(ui->closeBtn_2, SIGNAL(clicked(bool)), SLOT(slotClose()));
}

ReserveWidget::~ReserveWidget()
{
  delete ui;
}

void ReserveWidget::closeEvent(QCloseEvent* e)
{
  slotClose();
  QWidget::closeEvent(e);
}

void ReserveWidget::save()
{
  QStringList dirList;
  if( ui->projectBox->isChecked() ){
    dirList.append(MnCommon::projectPath());
  }
  if( ui->iniSettingsBox->isChecked() ){
    dirList.append(MnCommon::userSettingsPath());
  }

  QStringList dbs;
  for ( auto check : ui->buttonGroup->buttons() ) {
    if ( true == check->isChecked() ) {
      dbs.append(check->objectName());
    }
  }

//  QString pgFlag;
//  if(false == ui->dumpDataCheck->isChecked()){
//    pgFlag = "--schema-only";
//  }
//  else if(ui->nodumpRadio->isChecked()){
//    pgFlag = "nodump";
//  }

  DataSaver* saver = new DataSaver(dirList, dbs, ui->saveDir->dir());
  saverThread = new QThread(this);

  connect(saverThread, SIGNAL(started()), saver, SLOT(save()));
  connect(saverThread, SIGNAL(finished()), saver, SLOT(deleteLater()));
  connect(saver, SIGNAL(newMessage(QString)), SLOT(log(QString)));
  connect(saver, SIGNAL(saveComplete()), SLOT(slotSaveComplete()));

  saver->moveToThread(saverThread);
  saverThread->start();

  ui->execBtn->setDisabled(true);
  ui->restoreBtn->setDisabled(true);
  ui->tab_2->setDisabled(true);
}

void ReserveWidget::restore()
{
  DataSaver* rest = new DataSaver(QStringList(), "", ui->restoreDir->dir(), ui->hostBox->currentText());
  restoreThread = new QThread(this);
  connect(restoreThread, SIGNAL(started()), rest, SLOT(restore()));
  connect(restoreThread, SIGNAL(finished()), rest, SLOT(deleteLater()));
  connect(rest, SIGNAL(newMessage(QString)), SLOT(log(QString)));
  connect(rest, SIGNAL(restoreComplete()), SLOT(slotRestoreComplete()));
  rest->setDropBeforeRestore(ui->dropBeforeRestoreBox->isChecked());

  rest->moveToThread(restoreThread);
  restoreThread->start();

  ui->restoreBtn->setDisabled(true);
  ui->execBtn->setDisabled(true);
  ui->tab->setDisabled(true);
}

void ReserveWidget::log(const QString& text)
{
  QScrollBar* sb = ui->console->verticalScrollBar();
  bool needAutoScroll = ( sb->value() == sb->maximum() );
  int v = sb->value();
  ui->console->appendPlainText( QDateTime::currentDateTime().toString("HH:mm ") + text);
  if ( needAutoScroll ) { sb->setValue(sb->maximum()); } else { sb->setValue(v); }
}

void ReserveWidget::slotSaveComplete()
{
  if ( saverThread != nullptr ){
    saverThread->quit();
  }
  log(QObject::tr("[I] Сохранение завершено."));
  QMessageBox::information(0,QObject::tr("Внимание"), "Сохранение завершено", QObject::tr("Принять"));
  ui->execBtn->setDisabled(false);
  ui->restoreBtn->setDisabled(false);
  ui->tab_2->setDisabled(false);
}

void ReserveWidget::slotRestoreComplete()
{
  if ( restoreThread != nullptr ){
    restoreThread->quit();
  }
  log(QObject::tr("[I] Восстановление данных завершено."));
  QMessageBox::information(0,QObject::tr("Внимание"), "Восстановление данных завершено", QObject::tr("Принять"));
  ui->restoreBtn->setDisabled(false);
  ui->execBtn->setDisabled(false);
  ui->tab->setDisabled(false);
}

void ReserveWidget::slotChangeSaveDir()
{
  saveDir_ = ui->saveDir->dir() + "/";
}

void ReserveWidget::slotChangeRestoreDir()
{
  restoreDir_ = ui->restoreDir->dir() + "/";
}

void ReserveWidget::slotClose()
{
  if( saverThread && saverThread->isRunning() ){
    QMessageBox* msg = new QMessageBox(QMessageBox::Warning,
                                       tr("Внимание"),
                                       tr("Процесс сохранения данных не завершен. "
                                              "Экспортируемые данные будут непригодны для восстановления. "
                                              "Все равно продолжить?"));
    msg->addButton(tr("Принять"),QMessageBox::YesRole);
    msg->addButton(tr("Отмена"),QMessageBox::NoRole);
    if( false == msg->exec() ){
      qApp->quit();
    }
  }
  else if( restoreThread && restoreThread->isRunning() ){
    QMessageBox* msg = new QMessageBox(QMessageBox::Warning,
                                       tr("Внимание"),
                                       tr("Процесс восстановления данных не завершен. "
                                              "Работа СПО ГМО будет невозможна. Все равно продолжить?"));
    msg->addButton(tr("Принять"),QMessageBox::YesRole);
    msg->addButton(tr("Отмена"),QMessageBox::NoRole);
    if( false == msg->exec() ){
      qApp->quit();
    }
  }
  else{
    qApp->quit();
  }
}

}
