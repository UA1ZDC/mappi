#ifndef METEO_MASLO_UI_JOURNALPLUGIN_JOURNALPLUGIN_H
#define METEO_MASLO_UI_JOURNALPLUGIN_JOURNALPLUGIN_H

#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>
#include <meteo/commons/ui/customviewer/customviewer.h>


namespace meteo{


class JournalPlugin : public app::MainWindowPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "journalplugin" FILE "journalplugin.json" )
  public:
    JournalPlugin();
    ~JournalPlugin();

private slots:
    QWidget* slotOpenJournal();
    void slotWidgetDestroyed();

private:
    CustomViewer* widget_;
};


}
#endif
