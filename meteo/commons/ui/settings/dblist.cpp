#include "dblist.h"
#include "ui_dblist.h"
#include "dbsettings.h"

#include <meteo/commons/settings/settings.h>
#include <qdiriterator.h>
#include <meteo/commons/global/global.h>
#include <commons/textproto/tprototext.h>


enum kColumns {
  kName = 0,
  kValue,
  kEditBtn
};

enum Properties {
  kConnectionName = 0,
  kHumanName = 1,
  kDbName = 2,
  kHostName = 3,
  kPort = 4,
  kLogin = 5
};

static const QString& kTextConnectionName = QObject::tr("Имя соединения");
static const QString& kTextHumanName = QObject::tr("Имя");
static const QString& kTextDbName = QObject::tr("Имя базы данных");
static const QString& kTextkHostName = QObject::tr("Имя хоста");
static const QString& kTextPort = QObject::tr("Порт");
static const QString& kTextLogin = QObject::tr("Логин");

static const QString& kProtoParseFailed = QObject::tr("Не удается прочитать данные из файла %1");

namespace meteo {

DbListWidget::DbListWidget(QWidget* parent ) :
  SettingsWidget(parent),
  ui(new ::Ui::DbList)
{
  ui->setupUi(this);  
  this->setObjectName(QObject::tr("Базы данных"));
  ui->dbList->setColumnHidden(kLogin, true);

  auto dbConfs = meteo::gSettings()->dbConfs();

  for ( int i = 0; i < dbConfs.db_connection_size(); ++i ) {
    auto conf = new meteo::settings::DbConnection(dbConfs.db_connection(i));
    auto connectionName = QString::fromUtf8(conf->conn_name().c_str(),
                                            conf->conn_name().size());
    auto ptr  = std::shared_ptr< ::meteo::settings::DbConnection >(conf);
    this->connections_.insert(connectionName, ptr);
  }

  ui->dbList->header()->setStretchLastSection(false);
  ui->dbList->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  connect(ui->dbList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
          SLOT(slotItemDoubleClicked(QTreeWidgetItem*,int)));

  updateTree();

  for ( int i = 0; i < this->ui->dbList->columnCount(); ++i ){
    ui->dbList->header()->setSectionResizeMode(i, QHeaderView::Stretch);
  }
}

DbListWidget::~DbListWidget()
{
  delete ui;  
  connections_.clear();
}

void DbListWidget::save()
{
  QDir confDir(meteo::global::Settings::instance()->dbConfPath());
  if ( false == confDir.exists() ){
    error_log.msgBox() << meteo::msglog::kDirNotFound.arg(confDir.path());
  }
  auto mask = meteo::global::Settings::instance()->dbConfMask();
  QDirIterator dirIterator(confDir.path(),
                           QStringList() << mask,
                           QDir::Files);
  int filesCount = 0;
  while ( true == dirIterator.hasNext() ){
    ++filesCount;
    auto filePath = dirIterator.next();
    QFile file(filePath);
    if ( false == file.open(QIODevice::ReadOnly)){
      error_log.msgBox() << meteo::msglog::kFileReadFailed.arg(filePath);
      continue;
    }
    settings::DbSettings settings;
    auto data = QString::fromUtf8(file.readAll());
    file.close();
    if ( false == TProtoText::fillProto(data, &settings) ){
      error_log.msgBox() << kProtoParseFailed;
      continue;
    }

    for (int i = 0; i < settings.db_connection_size(); ++i ){
      auto conf = settings.mutable_db_connection(i);
      auto id = QString::fromUtf8(conf->conn_name().c_str(),
                                  conf->conn_name().length());
      conf->CopyFrom(*connections_[id].get());
    }

    auto updatedSettings = TProtoText::toText(settings);
    if ( false == file.open(QIODevice::WriteOnly) ){
      error_log.msgBox() << meteo::msglog::kFileWriteFailed.arg(filePath);
      continue;
    }

    if ( updatedSettings.length() != file.write(updatedSettings) ){
      error_log.msgBox() << meteo::msglog::kFileWriteFailed.arg(filePath);
      continue;
    }
  }

  if ( 0 == filesCount ){
    error_log << QObject::tr("Ошибка: ожидался как минимум 1 файл");
  }
  else {
    this->changed_ = false;
  }

}

void DbListWidget::load()
{

}

void DbListWidget::updateTree()
{
  this->ui->dbList->clear();
  QList<QTreeWidgetItem*> parents;  
  for ( auto connectionName: this->connections_.keys() ){
    auto connection = this->connections_[connectionName];
    QTreeWidgetItem *item = new QTreeWidgetItem();
    parents << item;

    auto humanName = QString::fromStdString(connection->human_name());
    auto dbName = QString::fromStdString(connection->name());
    auto hostName = QString::fromStdString(connection->host());
    auto port = QString::number(connection->port());
    auto login = QString::fromStdString(connection->login());

    item->setText(kConnectionName, connectionName);
    item->setData(kConnectionName, Qt::UserRole, connectionName);
    item->setText(kHumanName, humanName);
    item->setText(kDbName, dbName);
    item->setText(kHostName, hostName);
    item->setText(kPort, port);
    item->setText(kLogin, login);
  }

  this->ui->dbList->addTopLevelItems(parents);
}

void DbListWidget::setUpdated()
{
  this->changed_ = true;
  emit changed();
}

void DbListWidget::slotItemDoubleClicked(QTreeWidgetItem* item, int)
{
  auto dbName = item->data(kConnectionName, Qt::UserRole).toString();
  auto conf = this->connections_[dbName];
  DbSettings sett(*conf.get());
  if ( QDialog::Accepted == sett.exec() ){
    if ( false == sett.isChanged() ){
      return;
    }
    auto connection = new meteo::settings::DbConnection();
    connection->set_conn_name(conf->conn_name());
    connection->set_human_name(sett.getHumanName().toStdString());
    connection->set_name(sett.getName().toStdString());
    connection->set_host(sett.getHost().toStdString());
    connection->set_port(sett.getPort());
    connection->set_login(sett.getLogin().toStdString());
    connection->set_password(sett.getPassword().toStdString());
    auto ptr = std::shared_ptr<meteo::settings::DbConnection>(connection);
    this->connections_.insert(dbName, ptr);
    this->setUpdated();
    this->updateTree();
  }
}

}

