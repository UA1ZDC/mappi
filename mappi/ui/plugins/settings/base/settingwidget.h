#ifndef METEO_APP_SETTINGS_SETTINGWIDGET_H
#define METEO_APP_SETTINGS_SETTINGWIDGET_H

#include <functional>
#include <qwidget.h>

namespace meteo {
namespace app {

class SettingsWidget;

class SettingWidget : public QWidget
{
  Q_OBJECT

public:
  SettingWidget(QWidget* parent = nullptr);
  virtual ~SettingWidget();

  QString title() const;
  QPixmap icon() const;

signals:
  void changed(bool) const;

public slots:
  virtual void slotLoadConf();
  virtual void slotSaveConf() const;
  virtual void slotChanged() const;

protected:
  SettingsWidget* settings_;
  QString title_;
};

} // app
} // meteo

#endif // METEO_APP_SETTINGS_SETTINGWIDGET_H
