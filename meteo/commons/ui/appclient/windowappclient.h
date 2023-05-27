#ifndef METEO_COMMONS_UI_APPCLIENT_WINDOWAPPCLIENT_H
#define METEO_COMMONS_UI_APPCLIENT_WINDOWAPPCLIENT_H

#include <qstring.h>
#include <qwidget.h>

#include <meteo/commons/proto/appconf.pb.h>

class QSettings;

namespace rpc {
class Address;
} // rpc

namespace meteo {
class ControllerAppClient;
class TabWidgetAppClient;
class HomePageWidget;
class MonitorPageWidget;
class ProcMonitorWidget;
} // meteo

namespace meteo {

class WindowAppClient : public QWidget
{
  Q_OBJECT

public:
  explicit WindowAppClient(QWidget* parent = 0);
  virtual ~WindowAppClient();

  void setTabBarVisible(bool visible);
  void setAppIconVisible(bool visible);
  void setLogDefaultFontSize(int size);

  void setController(ControllerAppClient* c);

signals:
  void managerConnected(const QString& addr);

public slots:
  void slotRemoteConnection();

private slots:
  void slotStop(int64_t id, const QString& managerId);
  void slotInit();
  void slotUpdateServices();
  void slotReconnectManager();
  void slotCurrentTabChanged();
  void slotTabClose(int index);
  void slotSaveState();
  void slotRemoteConnect(const QString& addr);
  void slotRestoreGeometryAndState();

  void slotConnectedToManager(const QString& addr);
  void slotManagerConnectionFail(const QString& addr);
  void slotManagerDisconnected(const QString& managerId);
  void slotProcListChanged(const QString& managerId, const app::AppState& procs);
  void slotProcChanged(const QString& managerId, const app::AppState::Proc& proc);
  void slotStart(int64_t id, const QString& managerId);
  void slotLogReceived(const QString& managerId, int64_t procId, const app::AppOutReply& log);

private:
  bool appIconVisible_ = true;
  int logFontSize_ = 11;
  TabWidgetAppClient* tabWidget_;
  QSettings* settings_;
  HomePageWidget* home_;

  ControllerAppClient* controller_;
};

} // meteo

#endif // METEO_COMMONS_UI_APPCLIENT_WINDOWAPPCLIENT_H
