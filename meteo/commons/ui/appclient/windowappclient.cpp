#include "windowappclient.h"

#include <qlayout.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qmessagebox.h>

#include <cross-commons/debug/tlog.h>

#include <commons/textproto/tprototext.h>

#include <meteo/commons/global/global.h>

#include "remoteconnectiondlg.h"
#include "notfoundwidget.h"
#include "homepagewidget.h"
#include "monitorpagewidget.h"
#include "procmonitorwidget.h"
#include "tabwidgetappclient.h"
#include "controllerappclient.h"

namespace meteo
{

WindowAppClient::WindowAppClient( QWidget* p )
  : QWidget(p)
{
  controller_ = 0;

  setObjectName("meteo.app.client");
  setWindowTitle(tr("Контроль и диагностика"));

  settings_ = new QSettings(QDir::homePath() + "/.meteo/" + this->objectName() + ".ini", QSettings::IniFormat);

  tabWidget_ = new TabWidgetAppClient(this, settings_);
  tabWidget_->setTabPosition(QTabWidget::North);
  tabWidget_->setTabsClosable(true);
  tabWidget_->setDocumentMode(true);
  tabWidget_->setObjectName("ProcViewTab");

  home_ = new HomePageWidget;
  connect(home_, SIGNAL(updateServices()), SLOT(slotUpdateServices()));
  connect(home_, SIGNAL(removeConnection()), SLOT(slotRemoteConnection()));
  tabWidget_->addTab(home_, QObject::tr("Главная"));
  tabWidget_->setTabIcon(0, QIcon(":/meteo/appclient/icons/home.png"));

  QHBoxLayout* hbl = new QHBoxLayout(this);
  hbl->addWidget(tabWidget_);
  connect(tabWidget_, SIGNAL(tabCloseRequested(int)), SLOT(slotTabClose(int)));
  connect(tabWidget_, SIGNAL(currentChanged(int)), SLOT(slotCurrentTabChanged()));
  connect(tabWidget_, SIGNAL(currentChanged(int)), SLOT(slotSaveState()));

  QSize sz = qApp->desktop()->availableGeometry().size() * 0.75;
  setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, sz, qApp->desktop()->availableGeometry()));
}

WindowAppClient::~WindowAppClient()
{
}

void WindowAppClient::setTabBarVisible(bool visible)
{
  tabWidget_->tabBar()->setVisible(visible);
}

void WindowAppClient::setAppIconVisible(bool visible)
{
  appIconVisible_ = visible;

  for ( int i=0,isz=tabWidget_->count(); i<isz; ++i ) {
    MonitorPageWidget* t = qobject_cast<MonitorPageWidget*>(tabWidget_->widget(i));
    if ( 0 != t ) {
      t->setAppIconVisible(appIconVisible_);
    }
  }
}

void WindowAppClient::setLogDefaultFontSize(int size)
{ logFontSize_ = size; }

void WindowAppClient::setController(ControllerAppClient* c)
{
  if ( 0 != controller_ ) {
    disconnect(controller_, 0, 0, 0);
  }

  controller_ = c;

  if ( 0 != controller_ ) {
    connect( controller_, SIGNAL(managerConnected(QString)), SLOT(slotConnectedToManager(QString)) );
    connect( controller_, SIGNAL(managerConnectionFail(QString)), SLOT(slotManagerConnectionFail(QString)) );
    connect( controller_, SIGNAL(managerDisconnected(QString)), SLOT(slotManagerDisconnected(QString)) );
    connect( controller_, SIGNAL(managerConnectionLost(QString)), SLOT(slotManagerDisconnected(QString)) );
    connect( controller_, SIGNAL(procListChanged(QString,app::AppState)),
             SLOT(slotProcListChanged(QString,app::AppState)) );
    connect( controller_, SIGNAL(procChanged(QString,app::AppState::Proc)),
             SLOT(slotProcChanged(QString,app::AppState::Proc)) );
    connect( controller_, SIGNAL(logReceived(QString,int64_t,app::AppOutReply)),
             SLOT(slotLogReceived(QString,int64_t,app::AppOutReply)) );
  }
}

void WindowAppClient::slotRemoteConnection()
{
  RemoteConnectionDlg dlg;
  connect( &dlg, SIGNAL(connectToHost(QString)), SLOT(slotRemoteConnect(QString)) );
  connect( this, SIGNAL(managerConnected(QString)), &dlg, SLOT(accept()) );
  connect( controller_, SIGNAL(managerConnected(QString)), &dlg, SLOT(accept()) );
  connect( controller_, SIGNAL(managerConnectionFail(QString)), &dlg, SLOT(slotShowError()) );

  dlg.exec();
}

void WindowAppClient::slotConnectedToManager(const QString& addr)
{
  debug_log << tr("Подключено к %1").arg(addr);

  QString managerId = addr;

  MonitorPageWidget* tab = 0;
  for ( int i=0,isz=tabWidget_->count(); i<isz; ++i ) {
    MonitorPageWidget* t = qobject_cast<MonitorPageWidget*>(tabWidget_->widget(i));
    if ( 0 == t ) {
      continue;
    }
    if ( t->managerId() == managerId ) {
      tab = t;
      break;
    }
  }

  if ( 0 == tab ) {
    tab = new MonitorPageWidget(addr);
    tab->setTabName(addr);
    tab->setLogDefaultFontSize(logFontSize_);
    int idx = tabWidget_->addTab(tab, addr);
    tabWidget_->setTabIcon(idx, QIcon(":/meteo/appclient/icons/desktop.png"));
    tabWidget_->setCurrentIndex(idx);
    connect(tab, SIGNAL(start(int64_t,QString)), SLOT(slotStart(int64_t,QString)));
    connect(tab, SIGNAL(stop(int64_t,QString)), SLOT(slotStop(int64_t,QString)));
    connect(tab, SIGNAL(updateServices()), SLOT(slotReconnectManager()) );
  }
  tab->setAppIconVisible(appIconVisible_);
  tab->setConnectionLost(false);
}

void WindowAppClient::slotManagerConnectionFail(const QString& addr)
{
  debug_log << tr("Не удалось подключиться к %1").arg(addr);

  QString managerId = addr;

  MonitorPageWidget* tab = 0;
  for ( int i=0,isz=tabWidget_->count(); i<isz; ++i ) {
    MonitorPageWidget* t = qobject_cast<MonitorPageWidget*>(tabWidget_->widget(i));
    if ( 0 == t ) {
      continue;
    }
    if ( t->managerId() == managerId ) {
      tab = t;
      break;
    }
  }

  if ( 0 == tab ) {
    tab = new MonitorPageWidget(addr);
    tab->setTabName(addr);
    int idx = tabWidget_->addTab(tab, addr);
    tabWidget_->setTabIcon(idx, QIcon(":/meteo/appclient/icons/desktop.png"));
    tabWidget_->setCurrentIndex(idx);

    connect(tab, SIGNAL(start(int64_t,QString)), SLOT(slotStart(int64_t,QString)));
    connect(tab, SIGNAL(stop(int64_t,QString)), SLOT(slotStop(int64_t,QString)));
    connect( tab, SIGNAL(updateServices()), SLOT(slotReconnectManager()) );
  }
  tab->setConnectionLost(true);
}

void WindowAppClient::slotManagerDisconnected(const QString& managerId)
{
  for ( int i=0,isz=tabWidget_->count(); i<isz; ++i ) {
    MonitorPageWidget* tab = qobject_cast<MonitorPageWidget*>(tabWidget_->widget(i));
    if ( 0 == tab ) {
      continue;
    }
    if ( tab->managerId() == managerId ) {
      tab->setConnectionLost(true);
      break;
    }
  }
}

void WindowAppClient::slotProcListChanged(const QString& managerId, const app::AppState& procs)
{
  for ( int i=0,isz=tabWidget_->count(); i<isz; ++i ) {
    MonitorPageWidget* tab = qobject_cast<MonitorPageWidget*>(tabWidget_->widget(i));
    if ( 0 == tab ) {
      continue;
    }
    if ( tab->managerId() == managerId ) {
      tab->setProcList(procs);
      break;
    }
  }
}

void WindowAppClient::slotProcChanged(const QString& managerId, const app::AppState::Proc& proc)
{
  for ( int i=0,isz=tabWidget_->count(); i<isz; ++i ) {
    MonitorPageWidget* tab = qobject_cast<MonitorPageWidget*>(tabWidget_->widget(i));
    if ( 0 == tab ) {
      continue;
    }
    if ( tab->managerId() == managerId ) {
      tab->procw(proc.id());
      ProcMonitorWidget* pp = tab->procw(proc.id());
      pp->update(proc);
      break;
    }
  }
}

void WindowAppClient::slotStart(int64_t id, const QString& managerId)
{
  for ( int i=0,isz=tabWidget_->count(); i<isz; ++i ) {
    MonitorPageWidget* tab = qobject_cast<MonitorPageWidget*>(tabWidget_->widget(i));
    if ( 0 == tab ) {
      continue;
    }
    if ( tab->managerId() == managerId ) {
      controller_->slotStartProc(tab->procById(id), managerId);
      break;
    }
  }
}

void WindowAppClient::slotLogReceived(const QString& managerId, int64_t procId, const app::AppOutReply& log)
{
  for ( int i=0,isz=tabWidget_->count(); i<isz; ++i ) {
    MonitorPageWidget* tab = qobject_cast<MonitorPageWidget*>(tabWidget_->widget(i));
    if ( 0 == tab ) {
      continue;
    }
    if ( tab->managerId() == managerId ) {
      ProcMonitorWidget* pp = tab->procw(procId);
      if ( nullptr != pp ) {
        pp->logReceived(log);
      }
      break;
    }
  }
}

void WindowAppClient::slotStop(int64_t id, const QString& managerId)
{
  for ( int i=0,isz=tabWidget_->count(); i<isz; ++i ) {
    MonitorPageWidget* tab = qobject_cast<MonitorPageWidget*>(tabWidget_->widget(i));
    if ( 0 == tab ) {
      continue;
    }
    if ( tab->managerId() == managerId ) {
      controller_->slotStopProc(tab->procById(id), managerId);
      break;
    }
  }
}

void WindowAppClient::slotInit()
{
  if ( 0 == controller_ ) { return; }

  bool ok = false;
  QString addr = global::serviceAddress(settings::proto::kDiagnostic, &ok);
  if ( ok ) {
    slotRemoteConnect(addr);
  }
}

void WindowAppClient::slotUpdateServices()
{
  slotInit();
}

void WindowAppClient::slotReconnectManager()
{
  MonitorPageWidget* tab = qobject_cast<MonitorPageWidget*>(sender());
  if ( 0 == tab ) {
    return;
  }

  controller_->slotReconnectToManager(tab->managerId());
}

void WindowAppClient::slotCurrentTabChanged()
{
  MonitorPageWidget* tab = qobject_cast<MonitorPageWidget*>(tabWidget_->widget(tabWidget_->currentIndex()));
  if ( 0 == tab ) {
    return;
  }
  tab->slotRestoreGeometryAndState();
}

void WindowAppClient::slotTabClose(int index)
{
  if( tabWidget_->count() == 1 ){
    if( 0 == QMessageBox::question( this, tr("Выход"), QString("Закрыть приложение?"), tr("Да"), tr("Нет"))){
      qApp->quit();
    }
    else {
      return;
    }
  }

  MonitorPageWidget* tab = qobject_cast<MonitorPageWidget*>(tabWidget_->widget(index));
  if ( 0 == tab ) {
    return;
  }

  controller_->slotDisconnectFromManager(tab->managerId());
  delete tab;
}

void WindowAppClient::slotSaveState()
{
}

void WindowAppClient::slotRestoreGeometryAndState()
{
}

void WindowAppClient::slotRemoteConnect(const QString& addr)
{
  for ( int i=0,isz=tabWidget_->count(); i<isz; ++i ) {
    MonitorPageWidget* tab = qobject_cast<MonitorPageWidget*>(tabWidget_->widget(i));
    if ( 0 == tab ) {
      continue;
    }
    if ( tab->managerId() == addr ) {
      tabWidget_->setCurrentWidget(tab);

      if ( !tab->isConnectionLost() ) {
        emit managerConnected(addr);
        return;
      }
    }
  }

  debug_log << QObject::tr("Подключение к %1...").arg(addr);
  int protocol = rpc::Transport::Maslo;

  RemoteConnectionDlg* dlg = qobject_cast<RemoteConnectionDlg*>( sender() );
  if ( nullptr != dlg ) {
    protocol = dlg->proto();
  }

  controller_->slotConnectToManager( addr, protocol );
}

} // meteo
