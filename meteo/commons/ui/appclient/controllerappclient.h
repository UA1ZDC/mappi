#ifndef METEO_COMMONS_UI_APPCLIENT_CONTROLLERAPPCLIENT_H
#define METEO_COMMONS_UI_APPCLIENT_CONTROLLERAPPCLIENT_H

#include <qmap.h>
#include <qobject.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/appconf.pb.h>

namespace meteo {

class ManagerData
{
public:
  ManagerData() : channel(nullptr)
  {}

  rpc::Channel* channel;
  QList<QString> procidlist;    //!< списки идентификаторов прилоджений вида <address@id>
  int protocol = rpc::Transport::Maslo;
};

class ControllerAppClient : public QObject
{
  Q_OBJECT

public:
  explicit ControllerAppClient(QObject* parent = 0);

signals:
  void managerConnected(const QString& address);
  void managerDisconnected(const QString& managerId);
  void managerConnectionFail(const QString& address);
  void managerConnectionLost(const QString& managerId);
  void procListChanged(const QString& managerId, const app::AppState& procs);
  void procChanged(const QString& managerId, const app::AppState::Proc& proc);
  void logReceived(const QString& managerId, int64_t id, const app::AppOutReply& log);

public slots:
  void slotConnectToManager(const QString& address, int protocol = rpc::Transport::Maslo );
  void slotReconnectToManager(const QString& managerId);
  void slotDisconnectFromManager(const QString& managerId);
  void slotStartProc(const app::AppState_Proc& id, const QString& managerId);
  void slotStopProc(const app::AppState_Proc& id, const QString& managerId);

private slots:
  void slotDisconnected();

private:
  void procStateChanged(app::AppState::Proc* proc);
  void logReceived(app::AppOutReply* reply);

private:
  QMap<QString,ManagerData> managers_;
};

} // meteo

#endif // METEO_COMMONS_UI_APPCLIENT_CONTROLLERAPPCLIENT_H
