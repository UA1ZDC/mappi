#include "ui_dbconnection.h"
#include "tdbconnection.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <qobject.h>
#include <qevent.h>
#include <qdebug.h>
#include <qmap.h>
#include <qstring.h>
#include <qmenu.h>


//namespace {
//  static const QString iconsPath() { return MnCommon::sharePath() + "icons/"; }
//}

TDbConnection::TDbConnection(QWidget * p):
  QWidget(p)
{
  _ui = new Ui::DbConnection;
  _ui->setupUi( this );
  _menu = new QMenu;  
  _ui->editButton->setMenu(_menu);
//  _ui->editButton->setStyleSheet("QToolButton::menu-indicator { width: 0px; height: 0px;}");
//  _ui->editButton->setIcon(QIcon(iconsPath() + "bottomarrow.png" ));
  connect(_menu, SIGNAL(triggered(QAction*)),this,SLOT(updateParams(QAction*)));
  connect(_ui->dbHostEdit, SIGNAL(textChanged(QString)), this, SLOT(saveSlot()));
  connect(_ui->dbNameEdit, SIGNAL(textChanged(QString)), this, SLOT(saveSlot()));
  connect(_ui->dbLoginEdit, SIGNAL(textChanged(QString)), this, SLOT(saveSlot()));
  connect(_ui->dbPassEdit, SIGNAL(textChanged(QString)), this, SLOT(saveSlot()));
//  connect(_ui->saveButton, SIGNAL(clicked()),this,SLOT(saveSlot()));
}

TDbConnection::~TDbConnection()
{
}

QLineEdit* TDbConnection::getHostEdit(){
  return _ui->dbHostEdit;
}

bool TDbConnection::setHost(const QString& host){
  _ui->dbHostEdit->setText(host);
  return true;
}

bool TDbConnection::setDbName(const QString& name){
  _ui->dbNameEdit->setText(name);
  return true;
}

bool TDbConnection::setLogin(const QString& login){
  _ui->dbLoginEdit->setText(login);
  return true;
}

bool TDbConnection::setPass(const QString& pass){
  _ui->dbPassEdit->setText(pass);
  return true;
}

QString TDbConnection::getHost(){
  return _ui->dbHostEdit->text();
}

QString TDbConnection::getDbName(){
  return _ui->dbNameEdit->text();
}

QString TDbConnection::getLogin(){
  return _ui->dbLoginEdit->text();
}

QString TDbConnection::getPass(){
  return _ui->dbPassEdit->text();
}

void TDbConnection::updateParams(QAction* act){
  QString dbName = act->data().toString();
  emit menuClicked(dbName);
}

void TDbConnection::saveSlot(){
  emit save();
}

bool TDbConnection::setDbList(QMap<QString, QString> list){
  _menu->clear();
  foreach (const QString &str, list.keys()){
    if (!str.isEmpty()){
      QAction* act = new QAction(_menu);
      act->setData(list.key(str));
      act->setText(str);
      _menu->addAction(act);
    }
  }
  if (_menu->actions().count() == 0){
    _ui->editButton->setDisabled(true);
  } else {
    _ui->editButton->setDisabled(false);
    _menu->repaint();
  }
  return true;
}
