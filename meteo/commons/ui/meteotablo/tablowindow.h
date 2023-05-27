#ifndef METEO_COMMONS_UI_METEOTABLO_TABLOWINDOW_H
#define METEO_COMMONS_UI_METEOTABLO_TABLOWINDOW_H

#include <qmutex.h>
#include <qmainwindow.h>
#include <qdatetime.h>
#include <qtreewidget.h>
#include "tablowidget.h"

class QFileSystemWatcher;

namespace Ui {
class TabloWindow;
}

namespace meteo {

class BoolLocker
{
public:
  BoolLocker(bool* lock) : lock_(lock) { *lock_ = true; }
  ~BoolLocker() { *lock_ = false; }

private:
  bool* lock_;
};

class TabloWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit TabloWindow(QWidget *parent = 0);
  virtual ~TabloWindow();

  virtual void setVisible(bool visible);

public slots:
  void slotUpdateData();
  void slotSaveSettings();
  void slotLoadSettings();
  void slotDtChanged( const QDateTime& dt );
  void slotClearDtClicked();
  void slotArchive(QTreeWidgetItem* item, int column);
  void slotAddPreset();
  void slotDeletePreset();
  void slotSavePreset();
  void slotPresetChange();
  void slotOnSortButton();

private:
  void setStatus(const QString& text);
  void save();
  void loadPresets();
  void loadSettings(TabloWidget* wgt);
  void setDescription( TabloWidget* wgt);

private:
  QFileSystemWatcher* watcher_;
  Ui::TabloWindow* ui_;
  bool loading_;
  QDateTime dt_selected_;
  TabloWidget* archive_ = nullptr;
  tablo::Settings opt_;
  tablo::Presets presets_;
};

}

#endif
