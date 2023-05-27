#ifndef METEO_COMMONS_UI_CITY_CITYPLUGINHANDLER_H
#define METEO_COMMONS_UI_CITY_CITYPLUGINHANDLER_H

#include <QtGui>
#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>



namespace meteo{
namespace map{

class Document;
class CityWidget;

class CityPluginHandler : public QObject
{
  Q_OBJECT
public:
  CityPluginHandler(app::MainWindow* mw);
  virtual ~CityPluginHandler() override;
  QWidget* createWidget( app::MainWindow* mw, const QString& option ) const ;

private slots:
    void slotOpenWindow();
    void slotWgtDel();

private:
  app::MainWindow* mw_;
  CityWidget* wgt_;
};

}
}
#endif

