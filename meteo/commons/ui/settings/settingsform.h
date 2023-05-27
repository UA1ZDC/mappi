#ifndef METEO_MASLO_UI_SETTINGS_SETTINGSFORM_H
#define METEO_MASLO_UI_SETTINGS_SETTINGSFORM_H

#include <qmap.h>
#include <qwidget.h>
class QTabWidget;
class QTreeWidgetItem;
enum PageNumber {
  APP_START,
  LOC,
  CLIMAT_STATIONS,
  MSG,
  DB_LIST_PAGE,
  STORM_RING_SETTINGS,
  POGODAINPUT_SETTINGS,
  CENTER_SETTINGS
};

enum GroupNumber {
  GENERAL,
  DB_LIST,
  MSG_GROUP
};

namespace Ui{
  class Settings;
}

namespace meteo {
  class SettingsWidget;

namespace internal {
  class TAdminSettings;
}

class TSettingsForm : public QWidget
{
  Q_OBJECT
public:
  TSettingsForm( QWidget* parent = nullptr );
  ~TSettingsForm();
  void init();
  void load();
  bool canClose();

protected:
  void closeEvent(QCloseEvent* event );
  void keyPressEvent( QKeyEvent* e );

signals:
  void needClosed();

protected:
  void showEvent( QShowEvent* event );

private slots:
  void slotSwitchPage();
  void slotSave();
  void slotActivateBtn();
  void slotDeactivateBtn();

private:
  void showMsg(const QString& err);
  int showQuestion(const QString& questionTitle, const QString& questionText);

  void registerPages();
  void addPage(GroupNumber gnum, PageNumber pnum, meteo::SettingsWidget* page);

  bool widgetsIsChanged() const;
  bool saveWidgets();

private:
  Ui::Settings* ui_ = nullptr;
  QMap<GroupNumber, QTabWidget* > tabs_; //!< все группы и табы со страницами для каждой группы
  QList<SettingsWidget*> settWgts_;
};

}
#endif
