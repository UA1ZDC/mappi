#ifndef METEO_COMMONS_UI_MAINWINDOW_EMBEDWINDOW_APPCONTAINER_WINDOWS_H
#define METEO_COMMONS_UI_MAINWINDOW_EMBEDWINDOW_APPCONTAINER_WINDOWS_H

#include <qwidget.h>

namespace meteo {
namespace app {
class Preloader;
} // app
} // meteo

class AppContainer : public QWidget
{
  Q_OBJECT
public:
  explicit AppContainer(QWidget *parent = 0);
  virtual ~AppContainer();
  meteo::app::Preloader* preloader() { return preloader_; }

private:
  meteo::app::Preloader* preloader_;
};

#endif // METEO_COMMONS_UI_MAINWINDOW_EMBEDWINDOW_APPCONTAINER_WINDOWS_H
