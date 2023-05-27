#ifndef METEO_COMMONS_UI_PLUGINS_METEOGRAM_METEOGRAMPLUGIN_H
#define METEO_COMMONS_UI_PLUGINS_METEOGRAM_METEOGRAMPLUGIN_H

#include <qmap.h>

#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>
#include <meteo/commons/proto/meteogram.pb.h>
#include <meteo/commons/ui/meteogram/meteogramwindow.h>

namespace meteo {

class MeteogramWindow;

class MeteogramPlugin : public app::MainWindowPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA( IID "meteogram" FILE "meteogram.json" )
public:
  MeteogramPlugin();
  virtual ~MeteogramPlugin();

  static MeteogramPlugin* instance();

  void addCallback(const QString &key, const meteo::ui::proto::MeteogramWindowState &args);
  meteo::ui::proto::MeteogramWindowState getCallback(const QString &key);

public slots:
  MeteogramWindow* slotCreateDocument();
private slots:
  void slotDBusHandler(quint32 , const QString& key);

private:
  static QString basePath ;
  MeteogramWindow* findMeteogramWindow();
  static MeteogramPlugin* instance_;
  QMap<QAction*, QString> args_;

  //QHash<QString, meteo::ui::proto::MeteogramWindowState*> callBackList;
};

} // meteo

#endif // METEO_COMMONS_UI_PLUGINS_METEOGRAM_METEOGRAMPLUGIN_H

