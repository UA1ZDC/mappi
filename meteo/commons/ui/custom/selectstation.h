#ifndef METEOPRODUCTION_UI_SELECTSTATION_H
#define METEOPRODUCTION_UI_SELECTSTATION_H

#include <qmenu.h>
#include <qwidget.h>
#include <qboxlayout.h>

#include <commons/geobasis/geopoint.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/ui/custom/geopointeditor.h>

class QCompleter;
class QModelIndex;
class QAbstractItemModel;
class QSortFilterProxyModel;

namespace Ui {
class SelectStation;
}

namespace meteo {
namespace sprinf {
class MultiStatementRequest;
class Station;
class Stations;
class StationTypes;
} // sprinf
} // meteo

//! Класс SelectStation отображает информацию о метерологической станции и позволяет осуществлять поиск и выбор станции
//! по данным хранящимся в модели.

class SelectStation : public QWidget
{
  Q_OBJECT
public:
  //! Информация для автодополнения берётся из ролей CompleterRole, а отображаемый список из Qt::DisplayRole
  enum CompleterRole {
    kRuNameRole = Qt::UserRole,
    kEnNameRole,
    kIndexOrICAORole,
    kCoordinateRole,
    kDataTypeRole,
    kStationTypeRole,
  };
  //! Данные в модели должны располагаться в соответствующих колонках
  enum Column {
    kIndexOrICAOColumn,
    kRuNameColumn,
    kEnNameColumn,
    kCoordinateColumn,
  };

  //!
  explicit SelectStation(QWidget* parent = 0, Qt::WindowFlags fl = 0);
  //!
  virtual ~SelectStation();

  //! Модель должна состоять из четырёх колонок.
  void setModel(QAbstractItemModel* model);
  //! Возвращает текущую модель.
  QAbstractItemModel* model() const { return model_; }
  //! Поиск элемента по индексу станции/ICAO, типу станции, типу данных.
  //! Возвращает первый найденый.
  QModelIndex findItem(const QString& indexOrICAO, int stationType = -1, int dataType = -1) const;
  //! Поиск элемента по названию станции (ru или en в зависимости от роли role). Возвращает первый найденый.
  QModelIndex findItem(const QString& name, CompleterRole role) const;
  //! Поиск по координате. Возвращает первый найденый.
  QModelIndex findItem(const meteo::GeoPoint& coord, double delta = .0000000001) const;
  //! Возвращает введённый в форму индекс станции.
  QString stationIndex() const;
  //! Изменяет индекс станции в форме ввода на index.
  void setStationIndex(const QString &index);
  //! Возвращает введённое в форму название станции.
  QString ruName() const;
  //! Изменяет название станции в форме ввода на name.
  void setRuName(const QString& name);
  //! Возвращает введённое в форму название станции.
  QString enName() const;
  //! Изменяет название станции в форме ввода на name.
  void setEnName(const QString& name);
  //! Возвращает введённые в форму координаты станции.
  meteo::GeoPoint coord() const;
  //! Изменяет координаты в форме ввода на coord.
  void setCoord(const meteo::GeoPoint& coord);

  int dataType() const;
  int stationType() const { return stationType_; }

  //! Запрашивает список станций у сервиса sprinf.
  //! autoDeleteModel - если true (по умолчанию), модель будет удалена при удалении виджета.
  //! \note Если была установлена модель через setModel(), она не будет удалена.
  bool loadStation(const meteo::sprinf::MultiStatementRequest& req, bool autoDeleteModel = true);
  bool loadStation(const QList<int>& types, bool autoDeleteModel= true);

  GeoPointEditor* geopointEditor() const ;

  void setFromProto(const meteo::surf::Point &);
  void setEnabled(bool);

  // Возвращает true, если последняя загрузка станций завершилась корректно.
  bool stationsLoaded() const { return stationsLoaded_; }
  QString lastError() const { return lastError_; }

  //! Возвращает станцию в виде прото-структуры
  meteo::sprinf::Station toStation() const;

public slots:
  void slotUpdateByFirstCompletion();
  void slotClear();
  void slotSetupGui(const QModelIndex& idx);

signals:
  void completed();

private:
  void beginUpdateGui();
  void endUpdateGui();
  void saveStations();
  void loadSavedStations();
  void toProto(meteo::surf::Point *);
  QString toString(const meteo::surf::Point& p);
  void setStations(const meteo::sprinf::Stations& list, bool autoDeleteModel);
  bool loadStationTypes(meteo::sprinf::StationTypes* types, const meteo::sprinf::MultiStatementRequest& req);
  void setLatDegree(double lat);
  void setLonDegree(double lon);
  void setAltitudeMeter(double alt);

private:
  meteo::GeoPoint coord_;
  Ui::SelectStation*  ui_;
  QAbstractItemModel* model_;
  QCompleter* indexCompleter_;
  QCompleter* ruCompleter_;
  QCompleter* enCompleter_;
  meteo::surf::Polygon stationList_;
  int dataType_;
  int stationType_;
  bool cacheOn_;
  QString lastError_;
  bool stationsLoaded_;
  QSortFilterProxyModel* filterModel_;
};


#endif // METEOPRODUCTION_UI_SELECTSTATION_H
