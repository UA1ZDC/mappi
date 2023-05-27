#include "settingsplugin.h"
#include "base/settingswidget.h"
#include "base/settingplugin.h"

#include <qaction.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <meteo/commons/ui/mainwindow/mainwindow.h>

namespace meteo {
namespace app {

static const QString kPluginPath = MnCommon::pluginPath("mappi.app.widgets");

SettingsPlugin::SettingsPlugin()
  : MainWindowPlugin("settingsplugin")
  , widget_(nullptr)
{
  //  QAction* action = mainwindow()->actionFromFile(MnCommon::etcPath() + "/application.menu.conf", "settings");
  static const QString& actionName = QObject::tr("settings");
  QAction* action = mainwindow()->findAction(actionName);
  if(nullptr == action) {
    error_log << "PATH: " << MnCommon::etcPath() + "/application.menu.conf";
    error_log << QObject::tr("Не найден пункт меню с идентификатором settings");
    return;
  }
  QObject::connect(action, &QAction::triggered, this, &SettingsPlugin::slotOpenWindow);
}

SettingsPlugin::~SettingsPlugin() { }

void SettingsPlugin::loadPlugins(const QString& path)
{
  QDir dir(path);
  if(false == dir.exists()) {
    warning_log << QObject::tr("Директория %1 с плагинами не обнаружена")
                   .arg(dir.absolutePath());
    return;
  }

  QStringList files = dir.entryList( QStringList() << "*.so" << "*.dll" );
  for(int j = 0, jsz = files.size(); j < jsz; ++j)
  {
    QString pname( dir.absoluteFilePath(files[j]));
    QPluginLoader loader( pname, this );
    if ( false == loader.isLoaded() ) {
      if ( false == loader.load() ) {
        error_log << QObject::tr("Не удалось загрузить плагин %1.\n\tОшибка = %2")
                     .arg(pname)
                     .arg(loader.errorString());
      }
    }
    SettingPlugin* plg = dynamic_cast<SettingPlugin*>(loader.instance());
    if ( 0 != plg ) {
      if ( false == plugins_.contains( plg->name() ) ) {
        plugins_.insert( plg->name(), plg );
      }
    }
  }
}

QWidget* SettingsPlugin::createWidget(MainWindow *mw)
{
  if(nullptr == widget_) {
    loadPlugins(kPluginPath);
    
   // mw->addPluginGroup("mappi.app.widgets");
    widget_ = new SettingsWidget(mw);
    widget_->setWindowFlags(Qt::Dialog);
    for(auto plg : qAsConst(plugins_)) {
      widget_->addSetting(plg);
    }
    QObject::connect(widget_, &SettingsWidget::destroyed, this, &SettingsPlugin::slotWidgetDelete);
  }
  return widget_;
}

QWidget* SettingsPlugin::createWidget(MainWindow *mw, const QString &option)
{
  Q_UNUSED(option);
  return createWidget(mw);
}

void SettingsPlugin::slotOpenWindow() {
  qApp->setOverrideCursor(Qt::WaitCursor);

  if(nullptr == widget_) {
    widget_ = dynamic_cast<SettingsWidget*>(createWidget(mainwindow()));
    if(nullptr != mainwindow()) {
      mainwindow()->toMdi(widget_);
    }
  }
  else {
    widget_->show();
    widget_->setFocus();
  }

  qApp->restoreOverrideCursor();
}

void SettingsPlugin::slotWidgetDelete() { widget_ = nullptr; }

} // app
} // meteo
