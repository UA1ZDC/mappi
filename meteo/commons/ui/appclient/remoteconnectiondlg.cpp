#include "remoteconnectiondlg.h"
#include "ui_remoteconnection.h"

#include <qpushbutton.h>

namespace meteo {

RemoteConnectionDlg::RemoteConnectionDlg(QWidget *parent)
  : QDialog(parent),
    ui_(new Ui::RemoteConnection)
{
  ui_->setupUi(this);
  ui_->error->hide();

  connect( ui_->address, SIGNAL(textChanged(QString)), SLOT(slotTextChanged()) );
  connect( ui_->port, SIGNAL(textChanged(QString)), SLOT(slotTextChanged()) );
  connect( ui_->okBtn, SIGNAL(clicked(bool)), SLOT(slotConnectClicked()) );
}

const QString RemoteConnectionDlg::host() const
{
  if ( 0 == ui_->address->text().simplified().size() ) {
    return ui_->address->placeholderText();
  }
  return ui_->address->text();
}

const QString RemoteConnectionDlg::port() const
{
  if ( 0 == ui_->port->text().simplified().size() ) {
    return ui_->port->placeholderText();
  }
  return ui_->port->text();
}

rpc::Transport::Protocol RemoteConnectionDlg::proto() const
{
  if ( 1 == ui_->cmb_proto->currentIndex() ) {
    return rpc::Transport::Novost;
  }
  return rpc::Transport::Maslo;
}

void RemoteConnectionDlg::slotShowError()
{
  ui_->error->show();
}

void RemoteConnectionDlg::slotTextChanged()
{
//  bool hasHost = !ui_->address->text().isEmpty();
//  bool hasPort = !ui_->port->text().isEmpty();
//  ui_->okBtn->setEnabled(hasHost && hasPort);
}

void RemoteConnectionDlg::slotConnectClicked()
{
  ui_->error->hide();
  QString addr = rpc::TcpSocket::stringAddress( host(), port().toInt() );
  emit connectToHost(addr);
}

} // meteo
