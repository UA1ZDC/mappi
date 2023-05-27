#ifndef METEO_COMMONS_UI_APPCLIENT_TABWIDGETAPPCLIENT_H
#define METEO_COMMONS_UI_APPCLIENT_TABWIDGETAPPCLIENT_H

#include <qsettings.h>
#include <qtablewidget.h>

namespace meteo {

class TabBarAppClient : public QTabBar
{
  Q_OBJECT

public:
  TabBarAppClient(QWidget *parent = 0, QSettings* settings = 0);
  void mouseDoubleClickEvent(QMouseEvent *e);
  void setHost(const QString& host);

private:
  QString host_;
  QSettings* settings_;
};

class TabWidgetAppClient : public QTabWidget
{
  Q_OBJECT

public:
  TabWidgetAppClient(QWidget *parent = 0, QSettings* settings = 0);
  void setHost(const QString& host);

private:
  TabBarAppClient* bar_;
};

} // meteo

#endif // METEO_COMMONS_UI_APPCLIENT_TABWIDGETAPPCLIENT_H
