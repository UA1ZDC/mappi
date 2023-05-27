#include "dbsettings.h"
#include "ui_dbsettings.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <meteo/commons/proto/cleaner.pb.h>
#include <meteo/commons/settings/settings.h>

#include <QLayout>
#include <QMessageBox>
#include <QString>
#include <QLineEdit>

namespace meteo {

DbSettings::DbSettings(const ::meteo::settings::DbConnection& originalSettings,
                       QWidget* parent) :
  QDialog(parent),
  ui_(new Ui::DbSettings)
{
  ui_->setupUi(this);
  ui_->lbLogin->setVisible(false);
  ui_->lbPassword->setVisible(false);
  ui_->lePassword->setVisible(false);
  ui_->leLogin->setVisible(false);

  auto connName = QString::fromUtf8(originalSettings.conn_name().c_str(),
                                    originalSettings.conn_name().length());
  auto humanName = QString::fromUtf8(originalSettings.human_name().c_str(),
                                     originalSettings.human_name().length());
  auto name = QString::fromUtf8(originalSettings.name().c_str(),
                                originalSettings.name().length());
  auto host = QString::fromUtf8(originalSettings.host().c_str(),
                                originalSettings.host().length());
  int port = originalSettings.port();
  auto login = QString::fromUtf8(originalSettings.login().c_str(),
                                 originalSettings.login().length());
  auto password = QString::fromUtf8(originalSettings.password().c_str(),
                                    originalSettings.password().length());

  this->setWindowTitle(QObject::tr("База данных: %1").arg(connName));
  this->ui_->leHumanName->setText(humanName);
  this->ui_->leDbName->setText(name);
  this->ui_->leHostIp->setText(host);
  this->ui_->sbPort->setValue(port);
  this->ui_->leLogin->setText(login);
  this->ui_->lePassword->setText(password);
  this->layout()->setSizeConstraint( QLayout::SetFixedSize );

  for ( auto wgt: { this->ui_->leHumanName,
                    this->ui_->leDbName,
                    this->ui_->leHostIp,
                    this->ui_->leLogin,
                    this->ui_->lePassword } ){
    QObject::connect(wgt, SIGNAL(textChanged(const QString&)),
                     this, SLOT(slotChanged()));
  }

  connect( this->ui_->sbPort, SIGNAL(valueChanged(const QString&)),
           (SLOT(slotChanged())));

  connect( this->ui_->pbAccept, SIGNAL(clicked(bool)),
           SLOT(accept()));
  this->isChanged_ = false;
}

DbSettings::~DbSettings()
{
  delete ui_;
  ui_ = nullptr;
}

bool DbSettings::isChanged()
{
  return this->isChanged_;
}

QString DbSettings::getHumanName()
{
  return this->ui_->leHumanName->text();
}

QString DbSettings::getName()
{
  return this->ui_->leDbName->text();
}

QString DbSettings::getHost()
{
  return this->ui_->leHostIp->text();
}

int DbSettings::getPort()
{
  return this->ui_->sbPort->value();
}

QString DbSettings::getLogin()
{
  return this->ui_->leLogin->text();
}

QString DbSettings::getPassword()
{
  return this->ui_->lePassword->text();
}

void DbSettings::slotChanged()
{
  this->isChanged_ = true;
}

}
