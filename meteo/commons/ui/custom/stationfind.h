#ifndef METEO_COMMONS_UI_CUSTOM_STATIONFIND_H
#define METEO_COMMONS_UI_CUSTOM_STATIONFIND_H


#include <qwidget.h>
#include <qboxlayout.h>
#include <qtreeview.h>
#include <QTimer>
#include <QtWidgets>

#include <commons/geobasis/geopoint.h>
#include <meteo/commons/proto/surface_service.pb.h>

class QCompleter;
class QModelIndex;
class QAbstractItemModel;
class QSortFilterProxyModel;
class QItemSelection;

class GeoPointEditor;

namespace Ui {
  class StationFind;
}

namespace meteo {
  namespace sprinf {
    class MultiStatementRequest;
    class Station;
    class Stations;
    class StationTypes;
  }
}

//! Класс StationWidget отображает информацию о метерологической станции и позволяет осуществлять поиск и выбор станции
//! по данным хранящимся в модели.
class StationFind : public QWidget
{
  Q_OBJECT

  // Q_PROPERTY(QString ruName READ ruName WRITE setRuName)
  // Q_PROPERTY(QString enName READ enName WRITE setEnName)
  // Q_PROPERTY(QString stationIndex READ stationIndex WRITE setStationIndex)
  // Q_PROPERTY(bool isMapButtonVisible READ isMapButtonVisible WRITE setMapButtonVisible)
  // Q_PROPERTY(bool isHorisontal READ isHorisontal WRITE setHorisontal)

public:
  //! Информация для автодополнения берётся из ролей CompleterRole, а отображаемый список из Qt::DisplayRole
  enum CompleterRole {
    kCompleterRole = Qt::UserRole,
    kStationRole,
    kStationTypeRole,
    kCoordinateRole
  };

  enum CompleterColumn {
    kStationColumn = 0,
    kRuNameColumn = 1,
    kEnNameColumn = 2,
    kCoordColumn  = 3,
    kStationTypeCol = 4
};
  //!
  explicit StationFind(QWidget* parent = nullptr, Qt::WindowFlags fl = nullptr);
  //!
  virtual ~StationFind();

  
  void setFromProto(const meteo::surf::Point &);
  void setEnabled(bool);

  void checkInputed();

  /**
   * Устанавливаем задержку в отправке сигнала об изменении полей
   */
  void setDelaySignal() { isNeedDelay_ = true; }
  void setNoDelaySignal() { isNeedDelay_ = false; }

  meteo::sprinf::Station toStation() const;
  QModelIndex findItem(const meteo::GeoPoint& coord, double delta = .0000000001) const;
  //! Изменить станцию (station - индекс, икао или название)
  //  void setStation(const QString &station);
    //! Возвращает введённый в форму индекс станции
  QString stationIndex() const;
  //! Возвращает введённое в форму название станции
  QString ruName() const;
  //! Возвращает введённое в форму название станции
  QString enName() const;
  //! Возвращает координаты выбранной станции
  meteo::GeoPoint coord() const;
  //! Изменяет координаты в форме ввода на coord
  void setCoord(const meteo::GeoPoint& coord);

  GeoPointEditor* geopointEditor() const;
  
  //! Запрашивает список станций у сервиса sprinf
  //! autoDeleteModel - если true (по умолчанию), модель будет удалена при удалении виджета
  //! \note Если была установлена модель через setModel(), она не будет удалена
  bool loadStation(const QList<int>& types, bool autoDeleteModel= true);

  
signals:
  void add();
  void remove();
  void curChanged();
  void changed();

  void mapBtnToggled(bool);	
			 
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

  void slotUpdateByCoordinate();
  //void slotUpdateByFirstCompletion();
  
private:
  void beginUpdateGui(); //!< блокируем сигналы
  void endUpdateGui();   //!<  разблокируем сигналы
  
  void saveStations();
  void loadSavedStations();
  QString toString(const meteo::surf::Point& p);
  void fillStationsCombo();

  void setStations(const meteo::sprinf::Stations& list, const QMap<QString,int>& types, bool autoDeleteModel);
  QString stationText(const QString& index, const QString& ru, const QString en);

  bool findNearestStation(meteo::sprinf::Station* station, const meteo::GeoPoint& point, int radius) const;
  
private:

  meteo::surf::Polygon stationList_; //!< Список сохраненных
  meteo::surf::Point cur_; //!< Текущая выбранная //TODO может sprinf::Station?
  
  Ui::StationFind*  ui_ = nullptr;

  //для автодополнения
  QAbstractItemModel* model_ = nullptr;
  QCompleter* completer_ = nullptr;
  QSortFilterProxyModel* filterModel_ = nullptr;
  QMenu stTypeMenu_;
  
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

};

class StationFindPopup : public QTreeView
{
  Q_OBJECT
public:
  StationFindPopup();

public slots:
  void resizeWithText();  
  
protected:
  void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
  
signals:
  void signalSelectionChanged();

private:
  int columns_ = 4;
};

#endif
