#ifndef SETTINGSPLUGIN_H
#define SETTINGSPLUGIN_H

#include <QtGui>
#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>

namespace meteo{
namespace map{

class SettingsWidget;

class SettingsPlugin : public app::MainWindowPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "settingsplugin" FILE "settingsplugin.json" )
  public:
    SettingsPlugin();
    ~SettingsPlugin();

    QWidget* createWidget( app::MainWindow* mw, const QString& option ) const ;

  private slots:
    void slotOpenWindow();
    void slotWgtDel();

private:
    SettingsWidget* wgt_;
};

}
}
#endif

