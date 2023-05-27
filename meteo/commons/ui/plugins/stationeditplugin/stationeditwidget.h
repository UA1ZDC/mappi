#ifndef STATIONEDITWIDGET_H
#define STATIONEDITWIDGET_H

#include <QtWidgets>
#include <qsplitter.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>

#include <meteo/commons/proto/sprinf.pb.h>

#include "stationdetailsdialog.h"

namespace Ui{
class StationEditWidget;
}

class StationEditWidget;

namespace meteo {
namespace map {

class StationEditWidget : public QWidget
{
  Q_OBJECT
public:
  StationEditWidget(meteo::app::MainWindow *parent = nullptr);
  ~StationEditWidget();
  bool init();  
  void saveStationToDb(sprinf::Station st);

private:
  bool loadStationTypes(bool forceReload = false);
  bool loadStations(bool forceReload = false);
  bool loadCountries();
  void fillStationTypeCombo();
  void fillStationsTree();
  QTreeWidgetItem *addStationToTree(const sprinf::Station *st);
  void updateStationInTree(int stationIdx);
  void fillCountriesMap();
  void resizeTreeColumns();
  int getStationIdx(std::string stationId);
  int errorMsgBoxForUser(QString text, QString informationText, QString title);
  bool stationExists(sprinf::Station st);

  void closeEvent(QCloseEvent *event);


  Ui::StationEditWidget* ui_;
  meteo::app::MainWindow *mainWindow_;

  meteo::sprinf::StationTypes* stationTypes_;
  QMap <QString, int> stationTypesMap_;
  meteo::sprinf::Stations* stations_;
  QMap<QTreeWidgetItem*, int> stationTreeItemsMap_;
  meteo::sprinf::Countries* countries_;
  QMap<int, QString> countriesMap_;
  meteo::map::StationDetailsDialog* detailsDialog_;
  QSplitter* splitter_;
  int requestTimeout_;
  bool closing_; // досрочное прекращение обращений к службе справочной информации при ошибке и нажатии пользователем кнопки "Завершить"

private slots:
  void showStationDetails(QTreeWidgetItem *item);
  void showNewStationDetails();
  void deleteStation();
  void updateButtonsAvailability();
  void filterStations();


};

}
}

#endif // STATIONEDITWIDGET_H
