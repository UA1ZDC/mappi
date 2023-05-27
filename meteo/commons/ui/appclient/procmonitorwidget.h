#ifndef METEO_COMMONS_UI_APPCLIENT_PROCPARAM_H
#define METEO_COMMONS_UI_APPCLIENT_PROCPARAM_H

#include <qsettings.h>
#include <qsyntaxhighlighter.h>
#include <qtablewidget.h>
#include <qtreewidget.h>
#include <qwidget.h>

#include <meteo/commons/proto/appconf.pb.h>
#include <meteo/commons/rpc/rpc.h>

namespace Ui { class ProcMonitorWidget; }

namespace meteo {

class TextHightlighter : public QSyntaxHighlighter
{
  Q_OBJECT

public:
  TextHightlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
  {}

  virtual void highlightBlock(const QString& text)
  {
    if( text.contains("[E]") ){
      setFormat(0, text.length(), Qt::red);
    }else if( text.contains("[W]") ){
      setFormat(0, text.length(), Qt::yellow);
    }else if(text.contains("[I]")){
      setFormat(0, text.length(), Qt::green);
    }
  }
};

class ProcMonitorWidget : public QWidget
{
  Q_OBJECT


  enum Role {
    kRoleParamState = Qt::UserRole + 1,
  };

public:
  explicit ProcMonitorWidget(QWidget *parent = 0);
  virtual ~ProcMonitorWidget();

  void update(const app::AppState::Proc& proc);
  void setCurrentTab(int index);
  void setScrollBackSize(int value);
  void setLogFontSize(int size);

  bool isRunning() const { return running_; }

  void logReceived(const app::AppOutReply& res);

signals:
  void start();
  void stop();
  void stateUpdate(const app::AppState::Proc proc);
  void currentTabChange(int index);
  void scrollbackChange(int value);

public slots:
  void slotAdjustParamColumns();
  void slotZoomIn();
  void slotZoomOut();

private slots:
  void slotCurrentTabChange(int index);
  void updateSizeScrollback();
  void slotSaveState();
  void slotEditingFinished();

private:
  void restoreState();
  void setStateIcon(QTreeWidgetItem* item, app::OperationState state) const;

private:
  Ui::ProcMonitorWidget* ui_;
  QSettings* settings_;
  bool running_;

  QMap<app::OperationState,QIcon> icoState_;
};

} // meteo

#endif // METEO_COMMONS_UI_APPCLIENT_PROCPARAM_H
