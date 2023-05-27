#ifndef METEO_NOVOST_UI_PLUGINS_CITYPLUGIN_CITYWIDGET_H
#define METEO_NOVOST_UI_PLUGINS_CITYPLUGIN_CITYWIDGET_H

#include <qtreewidget.h>
#include <qcombobox.h>

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/proto/map_isoline.pb.h>

#include <meteo/commons/ui/mainwindow/mainwindow.h>

#include <meteo/commons/proto/map_city.pb.h>

#include "meteo/commons/ui/map/commongrid.h"

namespace Ui{
class CityWidget;
}


namespace meteo {
namespace map {

class CityWidget : public QWidget
{
  Q_OBJECT
public:
  CityWidget( app::MainWindow* parent = 0);
  ~CityWidget();

private:
  Ui::CityWidget* ui_;

  /*QMap<QString, meteo::map::proto::CitySettings_new> cityMap;
  QList <meteo::map::proto::DisplaySetting> displaySettingList;
  bool cityMapModified;
  bool displaySettingListModified;*/
  QMap <int, QPair<QTreeWidgetItem*, QTreeWidgetItem*> > cityItemMap_;
  DisplaySettingsOperator * dsOperator_;
  QMenu * contextMenu_;

  void init();
  void initMenu();
  void updateMenu(bool allowEditCityAction, bool visibilityActionShow, bool alwaysVisibleActionChecked);
  void filterList(QLineEdit * filterLE,
                  QTreeWidget* filteredTW,
                  QComboBox* filterCB);
  /*meteo::map::proto::DisplaySetting defaultDisplaySetting();
  meteo::map::proto::DisplaySetting modeDisplaySetting(meteo::map::proto::CityMode mode);
  meteo::map::proto::DisplaySetting filledCityDisplaySetting(QString cityTitle);
  meteo::map::proto::DisplaySetting differedCityDisplaySetting(meteo::map::proto::DisplaySetting cityDS);*/
  void fillCityListsFromMap();
  void contextMenuEvent(QContextMenuEvent *event);
  void resizeEvent(QResizeEvent *event);
  void resizeTreeWidgetsColumns();
  void refitCitiesItems();
  void refitCitiesItems(int cityMode, QWidget *sender);
  void refitCityItems(int cityIndex);
  void connectModeDisplaySettingsWidgets();
  void disconnectModeDisplaySettingsWidgets();

private slots:
  void editCity(QTreeWidgetItem* item);
  void editCityTranslator();
  void filterLists();
  void flushSettings();
  void changeItemMode();
  void addNewCity();
  void changeSelectedItemsModeFromMenu();
  void closeEvent (QCloseEvent * event);
  void updateModeWidget();
  void updateModeDisplaySetting();
  void toggleTwoPanels();
  void toggleCityVisibility(QTreeWidgetItem* item, int column);
  void setSelectedCitiesVisibility();
  void toggleAlwaysVisible();
  void slotEditFinished(int res);
  void slotAddFinished(int res);
};

}
}

#endif
