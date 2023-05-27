#ifndef METEO_APP_SETTINGS_SETTINGBASE_H
#define METEO_APP_SETTINGS_SETTINGBASE_H

#include <qwidget.h>


namespace meteo {
namespace app {

class SettingsWidget;
class SettingWidget;

/*!
 * \brief The SettingBase class
 * Класс предназначен для того, чтобы создавать плагины,
 * которые загружаются во время создания главного окна приложения
 * и добавляют пункты настроек в окно настроек с помощью функции
 * meteo::app::SettingsWidget::addSetting
 */
class SettingPlugin : public QObject
{
public:
  explicit SettingPlugin(const QString& name);
  virtual ~SettingPlugin();

  virtual const QString name() const = 0;

  virtual SettingWidget* createWidget(SettingsWidget* settings) = 0;

protected:
  const QString name_;
};

} // app
} // meteo


#endif // METEO_APP_SETTINGS_SETTINGBASE_H
