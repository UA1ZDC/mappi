#ifndef METEO_MASLO_UI_SETTINGS_DBLIST_H
#define METEO_MASLO_UI_SETTINGS_DBLIST_H

#include <qwidget.h>
#include <meteo/commons/ui/settings/settingswidget.h>
#include <sql/proto/dbsettings.pb.h>
#include <qtreewidget.h>


namespace Ui {
  class DbList;
}

namespace meteo {

class DbListWidget : public SettingsWidget
{
  Q_OBJECT

public:
  explicit DbListWidget(QWidget* parent );
  virtual ~DbListWidget() override;

  virtual void save() override;
  virtual void load() override;

private slots:
  void slotItemDoubleClicked(QTreeWidgetItem*, int);

private:  
  void updateTree();
  void setUpdated();

private:
  ::Ui::DbList *ui;
  QHash<QString, std::shared_ptr< ::meteo::settings::DbConnection > > connections_;
};

}
#endif
