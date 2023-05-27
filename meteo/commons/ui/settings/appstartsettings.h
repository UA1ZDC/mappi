#ifndef METEO_UI_SETTINGS_APPSTART_SETTINGS_H
#define METEO_UI_SETTINGS_APPSTART_SETTINGS_H

#include "settingswidget.h"

#include <qvariant.h>
#include <qmap.h>
#include <qsharedpointer.h>
#include <qstringlist.h>
#include <qtreewidget.h>

class QEvent;
class QPixmap;
class QString;

namespace Ui {
 class AppStartSettings;
}

namespace meteo {
namespace app {
  class Application;
} // app

class AppStartSettingsWidget : public SettingsWidget
{
  Q_OBJECT

  enum Column {
    TITLE_ICON = 0,
    STATUS,
    AUTOSTART,
    SPO_NAME
  };

public:
  explicit AppStartSettingsWidget(QWidget* parent = nullptr);
  virtual ~AppStartSettingsWidget() override;

private slots:
  void slotClone();
  void slotClone(const QString& appid);
  void slotAdd();
  bool slotAdd(const QString& appid);
  void slotRemove();
  void slotRemove(const QString& appid);
  void slotEdit();
  void slotEdit(const QString& row);
  void slotEnableActions();  
  void slotCbSpoNameCurrentIndexChanged(int);
  void slotTableContextMenuRequested(const QPoint&);
  void slotAppListSelectionChanged();
  void slotItemEdit(QTreeWidgetItem* item, int column);

protected:
  virtual void keyPressEvent(QKeyEvent* event) override;

private:
  virtual void save() override;
  virtual void load() override;
  QStringList spoNames();

  void updateAppTable();
  void setChange();

  bool isApptableNeedFullUpdate();
  void updateAppTableRow(const app::Application& config, QTreeWidgetItem* row);
  QString appProcessString(const app::Application& config) const;
  QPixmap stateIcon(const app::Application& config) const;
  QPixmap autostartIcon(const app::Application& config) const;

  void setCellPixmap(const QPixmap& pixmap, QTreeWidgetItem* row, int column);

  bool isValidConfig(const app::Application& config, QStringList* errors = nullptr) const;

private:
  Ui::AppStartSettings* ui_;

  QMap<QString, QSharedPointer<app::Application> > allApps_;
  QVariant spoFilterData_;
};

}

#endif
