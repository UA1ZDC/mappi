#ifndef METEO_UI_SETTINGS_SETTINGSWIDGET_H
#define METEO_UI_SETTINGS_SETTINGSWIDGET_H

#include <QWidget>

namespace meteo {

class SettingsWidget : public QWidget
{
  Q_OBJECT
public:
  explicit SettingsWidget(QWidget* parent = nullptr);
  virtual ~SettingsWidget();

  virtual void init();
  virtual void load() = 0;

  bool isChanged() const;

signals:
   void changed();

public slots:
  void slotSave();

protected:
  virtual void save() = 0;


protected:
  bool changed_;

};

}

#endif
