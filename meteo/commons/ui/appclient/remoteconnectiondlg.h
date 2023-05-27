#ifndef METEO_COMMONS_UI_APPCLIENT_REMOTECONNECTION_H
#define METEO_COMMONS_UI_APPCLIENT_REMOTECONNECTION_H

#include <qdialog.h>
#include <meteo/commons/rpc/rpc.h>

namespace Ui { class RemoteConnection; }

namespace meteo {

class RemoteConnectionDlg : public QDialog
{
  Q_OBJECT

public:
  explicit RemoteConnectionDlg(QWidget *parent = 0);

  const QString host() const ;
  const QString port() const ;

  rpc::Transport::Protocol proto() const ;

signals:
  void connectToHost(const QString& address);

public slots:
  void slotShowError();
  void slotConnectClicked();

private slots:
  void slotTextChanged();

private:
  Ui::RemoteConnection* ui_;
};

}

#endif
