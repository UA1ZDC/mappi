#ifndef METEO_COMMONS_UI_MAINWINDOW_EMBEDWINDOW_APPCONTAINER_LINUX_H
#define METEO_COMMONS_UI_MAINWINDOW_EMBEDWINDOW_APPCONTAINER_LINUX_H

#include <QX11Info>

namespace meteo {
namespace app {
class Preloader;
} // app
} // meteo

class AppContainer : public QX11EmbedContainer
{
  Q_OBJECT
public:
  AppContainer(QWidget *parent = 0);
  ~AppContainer();
  meteo::app::Preloader* preloader() { return preloader_; }

protected:
  void paintEvent(QPaintEvent *e);
  void resizeEvent(QResizeEvent *);

private:
  QTimer* timer_;
  meteo::app::Preloader* preloader_;
};

#endif // METEO_COMMONS_UI_MAINWINDOW_EMBEDWINDOW_APPCONTAINER_LINUX_H
