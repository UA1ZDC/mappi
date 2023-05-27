#ifndef METEO_COMMONS_UI_MAINWINDOW_EMBEDWINDOW_APPWATCHER_H
#define METEO_COMMONS_UI_MAINWINDOW_EMBEDWINDOW_APPWATCHER_H

#include <qmap.h>
#include <qobject.h>
#include <qprocess.h>

class AppContainer;

namespace meteo {
namespace app {

class MainWindow;
class Preloader;

class AppWatcher : public QObject
{
  Q_OBJECT
public:
  explicit AppWatcher(QObject *parent = 0);
  virtual ~AppWatcher();

  void setMainWindow(MainWindow* window) { mainwindow_ = window; }

  QWidget* create( const QString& app, const QStringList& args, const QString& title = 0 );
  QMap<QProcess*, AppContainer*> procx11() { return procX11_; }

private:
  MainWindow* mainwindow_;
  QMap<QProcess*, AppContainer*> procX11_;
  QMap<QString, AppContainer*> appnameX11_;

private slots:
  void slotReadError();
  void slotReadOutput();
  void slotProcFinished(int,QProcess::ExitStatus);
  void slotContainerDestroyed(QObject* o);
};

} //meteo
} //app

#endif // METEO_COMMONS_UI_MAINWINDOW_EMBEDWINDOW_APPWATCHER_H
