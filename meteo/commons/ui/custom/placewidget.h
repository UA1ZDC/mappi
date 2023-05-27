#ifndef METEO_COMMONS_UI_CUSTOM_PLACEWIDGET_H
#define METEO_COMMONS_UI_CUSTOM_PLACEWIDGET_H

#include <qwidget.h>
#include <qmetatype.h>
#include <qstringlist.h>

#include <meteo/commons/ui/custom/placemodel.h>

class QToolButton;
class StationWidget;

namespace Ui {
class PlaceWidget;
}

typedef QList<PlaceInfo> PlaceInfoList;
Q_DECLARE_METATYPE( PlaceInfoList )

class PlaceWidget : public QWidget
{
  Q_OBJECT

  Q_PROPERTY( int minVisibleItems READ minVisibleItems WRITE setMinVisibleItems )
  Q_PROPERTY( StationWidget* stationWidget READ stationWidget WRITE setExternalStationWidget )
  Q_PROPERTY( PlaceModel* model READ model WRITE setModel )

  Q_PROPERTY( QString departureIndex READ departureIndex )
  Q_PROPERTY( QString landIndex READ landIndex )
  Q_PROPERTY( QList<int> reserveIndexes READ reserveIndexes )
  Q_PROPERTY( QList<int> routeIndexes READ routeIndexes )
  Q_PROPERTY( PlaceInfoList placeList READ placeList )
  Q_PROPERTY( QString historyGroupName READ historyGroupName WRITE setHistoryGroupName )

  Q_PROPERTY( int maxEchelon READ maxEchelon )
  Q_PROPERTY( int minEchelon READ minEchelon )

public:
  enum Column { kType, kName, kIndex };

  explicit PlaceWidget(QWidget *parent = 0);
  virtual ~PlaceWidget();

  QString departureIndex() const;
  QString landIndex() const;
  QList<int> routeIndexes() const;
  QList<int> reserveIndexes() const;

  QString historyGroupName() const { return settingsGroup_; }
  void setHistoryGroupName(const QString& name) { settingsGroup_ = name; }

  PlaceInfoList placeList() const;

  int minVisibleItems() const { return minVisibleItems_; }
  void setMinVisibleItems(int v) { minVisibleItems_ = v; slotCalcTreeMinSize(); }

  StationWidget* stationWidget() const;
  void setExternalStationWidget(StationWidget* w);

  int maxEchelon() const;
  int minEchelon() const;

public slots:
  PlaceModel* model() const { return model_; }
  void setModel(PlaceModel* model);

signals:
  void placeAdded();
  void placeRemoved();

public slots:
  void slotAddClicked();
  void slotDelClicked();
  void slotUpClicked();
  void slotDownClicked();
  void slotPlaceTypeToggled(bool toggle);
  void slotClearHistory();
  void slotHistorySelected();

  void slotCalcTreeMinSize();
  void slotAdjustColumns();
  void slotAddPlace(const QString& type, const QString& name, const QString& index, const meteo::GeoPoint& coord, int echelon = 5500);

  void slotClear();
  void slotLoadHistory();

private:
  QString selectedType() const;

  void loadHistory();
  void saveHistory();

  void selectRow(int row);

private:
  // параметры
  int minVisibleItems_;
  QString settingsGroup_;

  // служебные
  Ui::PlaceWidget* ui_;
  QList<QToolButton*>  placeTypeButtons_;
  StationWidget* stationWidget_;
  PlaceModel* model_;
};

#endif // METEO_COMMONS_UI_CUSTOM_PLACEWIDGET_H
