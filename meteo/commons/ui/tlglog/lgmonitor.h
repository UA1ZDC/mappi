#ifndef METEO_COMMONS_UI_TLGLOG_LGMONITOR_H
#define METEO_COMMONS_UI_TLGLOG_LGMONITOR_H

#include <qwidget.h>
#include <qsettings.h>
#include <meteo/commons/ui/viewheader/viewheader.h>
#include <meteo/commons/alphanum/gphdecoder.h>
#include "regexpconditiondialog.h"
#include "checkpanel.h"
#include <meteo/commons/ui/customviewer/contentwidget.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/rpc/channel.h>
#include <meteo/commons/proto/msgparser.pb.h>
#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/global/log.h>
//#include <meteo/commons/msgcenter/core/msgqueue.h>
#include <qtreewidget.h>
#include <qheaderview.h>
#include "choicecondition.h"
#include "receivemessthread.h"

namespace meteo {
namespace tlg {
class MessageNew;
} // tlg
} // meteo

class NS_PGBase;

namespace Ui {
class TlgMonitorForm;
}

class ViewTreeWidget : public QTreeWidget
{
  Q_OBJECT
public:
  ViewTreeWidget(QWidget* parent);
  void scrollContentsBy(int dx, int dy);
};

class Lgmonitor : public QWidget
{
  Q_OBJECT

public:
  explicit Lgmonitor(QWidget *parent = 0);
  ~Lgmonitor();
  QSettings* settings() { return settings_; }
  QSettings* settings_;

private:
  Ui::TlgMonitorForm* ui_;
  int Maxmessages_;
  QMap<int,QList< QPair<QString, QString> > > menu_;
  QMap<QString, QCheckBox*> checkmap_;
  QMap<int, QString> conditions_;
  QMap<int, QString> condmap_;
  QMap<int, QString> checkpanelcond_;
  QStringList columns_;

  QMap<QString, QMap<int,QString> > group_conditions_;
  QMap<QString, QMap<int,QString> > group_human_conditions_;
  QString currentcondition_;
  QStringList namescond_;


  QSettings* group_settings_;
  RegExpConditionDialog* conditionsWidget_;
  choicecondition* choiceCondition_;

  CheckPanel* checkpanel_;
  meteo::bank::ViewHeader* header_;
  meteo::ContentWidget* contentWidget_;
  meteo::rpc::Channel* channel_;
  meteo::tlg::MessageNew* msg_;
  ViewTreeWidget* tree_;
  QThread* thread_;
  ReceiveMessThread* threadWork_;

  void createBottomPanel();
  void addItem( QTreeWidgetItem *item, int col );
  void createConditionByText( int col, const QString& text );
  void treeFilter();
  void loadCheckBox();
  bool filteritems( QStringList* strlst );
  int findValue( int col, const QString& value );

private slots:
  void slotPause();
  void slotPlay();
  void slotMaxmessages( int );
  void slotSaveGeometryAndState();
  void slotRestoreGeometryAndState();
  void slotOpenFile( QTreeWidgetItem*,int );
  void slotColumnFilterMenuActivated( int clmn, QMenu* action );
  void slotAboutColumnMenuActivated( int clmn, QAction* action);
  void slotSqlConditionChecked( bool on );
  void slotPanelClosed();
  void slotServiceDisconnected();
  void slotInit();
  void slotGetMsg(meteo::tlg::MessageNew msg);
  void initThread();

  void slotOpenDialCond();

  void slotLoadCondList();
  void slotSaveCondList();

  QMap<int,QString> restoreCondition(QString str,QString regexporhuman);

  virtual void setVisible( bool );
};

#endif // LG_MONITOR
