#ifndef METEO_COMMONS_UI_APPCLIENT_MONITORPAGEWIDGET_H
#define METEO_COMMONS_UI_APPCLIENT_MONITORPAGEWIDGET_H

#include "procmonitorwidget.h"

#include <qwidget.h>

#include <meteo/commons/ui/viewheader/viewheader.h>
#include <meteo/commons/proto/appconf.pb.h>
#include <meteo/commons/rpc/rpc.h>

class QCheckBox;
class NotFoundWidget;

namespace Ui { class MonitorPageWidget; }

namespace meteo {

class MonitorPageWidget : public QWidget
{
  Q_OBJECT

public:
  explicit MonitorPageWidget(const QString& managerId, QWidget *parent = 0);
  virtual ~MonitorPageWidget();

  void setAppIconVisible(bool visible);
  void setLogDefaultFontSize(int size);

  QString managerId() const { return managerId_; }
  void setProcList(const app::AppState& list);
  app::AppState_Proc procById(int64_t id) const;

  ProcMonitorWidget* procw(int64_t id);
  void setConnectionLost(bool lost);
  bool isConnectionLost() const;
  void setTabName(const QString& tabName) { tabName_ = tabName; }

signals:
  void start(int64_t id, QString host);
  void stop(int64_t id, QString host);
  void updateServices();

public slots:
  void slotRestoreGeometryAndState();

private slots:
  void slotItemClicked();
  void slotItemDoubleClicked();
  void slotUpdate(const app::AppState::Proc& proc);
  void slotColumnsClicked();
  void slotColumnCheckToggled(bool on);
  void slotHeaderCheckToggled(bool on);
  void slotContextMenu(QPoint point);
  void slotMoveColumn(int logicalIndex, int oldVisualIndex, int newVisualIndex);
  void slotStop();
  void slotStart();
  void slotRestart();
  void slotChangeTabIndex(int index);
  void slotRestartServices();

  void slotSaveSplitterState();
  void slotSaveButtonState();
  void slotSaveTableState();
  void slotScrollBackChange(int);
  void slotMenuActivated(int col, QAction* act);

private:
  QString toolTipFromState(app::AppState::ProcState state);
  QString toolTipFromStatus(app::OperationState state);
  void setCellPixmap(const QPixmap& pix, int row, int col);
  void buildMenu();
  void createHeader();
  void createAdditionalMenu();
  void filterProcsList();

private:
  QString managerId_;
  bool appIconVisible_ = true;
  int logFontSize_ = 11;

  Ui::MonitorPageWidget* ui_;
  app::AppState* state_;
  QMap<int64_t, QTableWidgetItem*> itemmap_;
  QMap<int64_t, int> windexmap_;
  QMap<int64_t, ProcMonitorWidget*> procwidgetmap_;
  QMap<int64_t, app::AppState::ProcState> statemap_;
  QMenu* _settingsMenu;
  QList<QAction*> actlist_;
  QList<QCheckBox*> checklist_;
  QMap<int, QCheckBox*> checkmap_;
  QSettings* settings_;
  QMenu *menu_;
  QList<QAction*> acttmp;
  int currentTab_;
  QList<int64_t> restartlist_;
  NotFoundWidget* notFound_;
  bank::ViewHeader* header_;
  QStringList spoList_;
  QString spoFilter_;

  QMap<app::OperationState,QPixmap> icoProcStatus_;
  QMap<app::AppState_ProcState,QPixmap> icoProcState_;
  QString tabName_;
};

} // meteo

#endif // METEO_COMMONS_UI_APPCLIENT_MONITORPAGEWIDGET_H
