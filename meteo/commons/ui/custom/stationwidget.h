#ifndef METEO_COMMONS_UI_CUSTOM_STATIONWIDGET_H
#define METEO_COMMONS_UI_CUSTOM_STATIONWIDGET_H


#include <qwidget.h>
#include <qboxlayout.h>
#include <qtreeview.h>
#include <QTimer>
#include <QtWidgets>

#include <commons/geobasis/geopoint.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/ui/map/view/mapview.h>


class QCompleter;
class QModelIndex;
class QAbstractItemModel;
class QSortFilterProxyModel;
class QItemSelection;

namespace Ui {
  class StationWidget;
}

namespace meteo {
  namespace sprinf {
    class MultiStatementRequest;
    class Station;
    class Stations;
    class StationTypes;
  }
}
namespace meteo {
namespace map {
    class TraceAction;
}

}
class StationFilterProxyModel;


//! Класс StationWidget отображает информацию о метерологической станции и позволяет осуществлять поиск и выбор станции
//! по данным хранящимся в модели.
class StationWidget : public QWidget
{
  Q_OBJECT

  // Q_PROPERTY(QString ruName READ ruName WRITE setRuName)
  // Q_PROPERTY(QString enName READ enName WRITE setEnName)
  // Q_PROPERTY(QString stationIndex READ stationIndex WRITE setStationIndex)
  Q_PROPERTY(bool isMapButtonVisible READ isMapButtonVisible WRITE setMapButtonVisible)
  Q_PROPERTY(bool isCoordVisible READ isCoordVisible WRITE setCoordVisible)
  Q_PROPERTY(bool isSavedStationsVisible READ isSavedStationsVisible WRITE setSavedStationsVisible)
  Q_PROPERTY(bool isHorisontal READ isHorisontal WRITE setHorisontal)
  Q_PROPERTY(bool isAltitudeVisible READ isAltitudeVisible WRITE setIsAltitudeVisible)
  Q_PROPERTY(bool isFilterVisible READ isFilterVisible WRITE setFilterVisible)
  

public:
  //! Информация для автодополнения берётся из ролей CompleterRole, а отображаемый список из Qt::DisplayRole
  enum CompleterRole {
    kCompleterRole = Qt::UserRole,
    kStationRole,
    kStationTypeRole,
    kCoordinateRole
  };

  enum CompleterColumn {
    kUndefColumn   = -1,
    kStationColumn = 0,
    kRuNameColumn = 1,
    kEnNameColumn = 2,
    kCoordColumn  = 3,
    kStationTypeCol = 4
};
  //!
  explicit StationWidget(QWidget* parent = nullptr, Qt::WindowFlags fl = nullptr);
  //!
  virtual ~StationWidget();


  /**
   * @brief isAltitudeVisible
   * @return
   * true - Поле для ввода высоты видно пользователю
   * false - Поле ввода высоты скрыто от пользователя
   */
  bool isAltitudeVisible();
  /**
   * @brief isAltitudeVisible
   * @param isVisible: устанавливает видимость поля ввода высоты
   */
  void setIsAltitudeVisible(bool isVisible);

  virtual void setObjectName(const QString& name);


  void setSizePolicy(QSizePolicy pol);
  void setSizePolicy(QSizePolicy::Policy hor, QSizePolicy::Policy ver);

  //! Запрашивает список станций у сервиса sprinf
  //! autoDeleteModel - если true (по умолчанию), модель будет удалена при удалении виджета
  //! \note Если была установлена модель через setModel(), она не будет удалена
  bool loadStation(const QList<meteo::sprinf::MeteostationType>& types, bool autoDeleteModel= true,
		   const QList<meteo::sprinf::MeteostationType>& show_types = QList<meteo::sprinf::MeteostationType>());

  meteo::map::TraceAction* getAssociatedAction();
  meteo::map::MapScene* getAssociatedScene();

  
  //-- заполнение/запрос параметров станции

  void setFromProto(const meteo::surf::Point &);
  bool findStation(const QString &station, int stationType);
  //! Изменить станцию (station - индекс, икао или название)
  void setStation(const QString &station, const meteo::GeoPoint& coord, const QString& name = "");
  //! Изменяет координаты в форме ввода на coord
  void setCoord(const meteo::GeoPoint& coord);
  
  //! Возвращает введённый в форму индекс станции
  QString stationIndex() const;
  //! Возвращает введённое в форму название станции
  QString ruName() const;
  //! Возвращает введённое в форму название станции
  QString enName() const;
  //! Возвращает координаты выбранной станции
  meteo::GeoPoint coord() const;
  int stationType() const { return cur_.type(); }
  
  QString placeName();
  meteo::sprinf::Station toStation() const;

  //настройка внешнего вида
  bool isMapButtonVisible() const;
  void setMapButtonVisible(bool visible);
  bool isMapButtonPressed() const;  
  void setSearchVisible(bool visible);
  bool isSearchVisible() const;

  void unPressMapButton() const;


  bool isFilterVisible() const;
  void setFilterVisible(bool visible);
  
  bool isCoordVisible() const;
  void setCoordVisible(bool visible);
    
  bool isSavedStationsVisible() const;
  void setSavedStationsVisible(bool visible);
  
  bool isHorisontal() const;
  void setHorisontal(bool sethor);

  void setEnabled(bool);
  
  //! Устанавливаем задержку в отправке сигнала об изменении полей
  void setDelaySignal() { isNeedDelay_ = true; }
  void setNoDelaySignal() { isNeedDelay_ = false; }


  //
  QString lastError() const { return lastError_; }
  void checkInputed();
  bool isStationValid() const;
  bool isStationsLoaded() const { return stationsLoaded_; }

 
signals:
  void add();
  void remove();
  void curChanged();
  void changed();

  void signalOnMap(bool);
			 
public slots:
  void slotUpdateByFirstCompletion();
  void slotAddStation();
  void slotDelStation();
  void slotChangeCurStation(int index);
  void slotUpdateComboVisible();

  void slotEmitChaged();
  void slotEmitFizzBuzzChaged();

  void slotShowFilterMenu();
  void slotFilterChecked(bool checked);

  void slotSetupGui(const QModelIndex& idx);
  void slotResizeCompletionPopup();
  void slotClear();
  void slotPopupIndexChanged();
  //void slotUpdateByFirstCompletion();

private slots:
  void slotOnMap(bool isEnabled);
  void slotUpdateByCoordinate(const meteo::GeoPoint& coord);

  void changeSorting();
private:
  void beginUpdateGui(); //!< блокируем сигналы
  void endUpdateGui();   //!<  разблокируем сигналы
  
  void saveStations();
  void loadSavedStations();
  QString toString(const meteo::surf::Point& p);
  void fillStationsCombo();

  void setStations(const meteo::sprinf::Stations& list, const QMap<int, QString>& types, bool autoDeleteModel);
  QString stationText(const QString& index, const QString& ru, const QString en);

  bool findNearestStation(meteo::sprinf::Station* station, const meteo::GeoPoint& point, int radius) const;
  QModelIndex findItem(const meteo::GeoPoint& coord, double delta = .0000000001) const;
  bool isIcaoName(const QString& text, int stationType);
private:

  meteo::surf::Polygon stationList_; //!< Список сохраненных
  meteo::surf::Point cur_; //!< Текущая выбранная //TODO может sprinf::Station?
  QList<meteo::sprinf::MeteostationType> types_; //!< Используемые типы станций
  
  Ui::StationWidget*  ui_ = nullptr;

  //для автодополнения
  QAbstractItemModel* model_ = nullptr;
  QCompleter* completer_ = nullptr;
  StationFilterProxyModel* filterModel_ = nullptr;
  QMenu stTypeMenu_;
  QSettings* settings_ = nullptr;
  
  
  // служебные
  QString lastError_;
  bool stationsLoaded_ = false;  
  // флаг блокировки сигнала, true - сигналы заблокированы, false - разблокированы
  bool is_signal_blocked_ = false;
  bool isLastFocused_ = false;
  
  // таймер для ввода
  // будем испускать сигналы с зарежкой, чтобы не перегрузить систему частыми запросами
  QTimer* timer_;
  // если нужна задержка в отправке сигнала на
  // изменение поля (нужно для жирных запросов в базу, чтобы пользователь успел ввести информацию)
  bool isNeedDelay_ = false;
  bool isHor_ = true; //!< горизонтальное или вертикальное расположение

  QTimer* timerSort_ = nullptr; //!< для сортировки
  int sortedCol_ = kUndefColumn;
};

class StationWidgetPopup : public QTreeView
{
  Q_OBJECT
public:
  StationWidgetPopup();

public slots:
  void resizeWithText();  
  
protected:
  void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
  
signals:
  void signalSelectionChanged();

private:
  int columns_ = 4;
};

 class StationFilterProxyModel : public QSortFilterProxyModel
 {
     Q_OBJECT

 public:
     StationFilterProxyModel(QObject *parent = 0);
						 
 public slots:
   void setText(const QString& text) {
     curText_ = text.toUpper();
   }

 protected:
     // bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
     bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

 private:
   QString curText_;
   
 };



#endif
