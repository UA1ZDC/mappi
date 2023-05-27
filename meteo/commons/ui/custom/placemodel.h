#ifndef METEO_COMMONS_UI_CUSTOM_PLACEMODEL_H
#define METEO_COMMONS_UI_CUSTOM_PLACEMODEL_H

#include <qmap.h>
#include <qdatetime.h>
#include <qabstractitemmodel.h>

#include <commons/geobasis/geopoint.h>

class PlaceInfo
{
public:
  static PlaceInfo fromString(const QString& s)
  {
    QStringList l = s.split(" - ");

    PlaceInfo info;
    if ( l.size() == 6 ) {
      info.index = l[0];
      info.name  = l[1];
      info.type  = l[2];
      info.coord.setPoint(l[3].toDouble(), l[4].toDouble());
      info.echelon = l[5].toInt();
    }
    else {
      //var(s);
    }
    return info;
  }

public:
  PlaceInfo()
  {
    id = -1;
    echelon = -1;
  }

  int id;
  QString type;
  QString name;
  QString index;
  int echelon;
  meteo::GeoPoint coord;

  QVariant data(int column) const
  {
    switch ( column ) {
      case 0: return QVariant(type);
      case 1: return QVariant(name);
      case 2: return QVariant(index);
      case 3: return QVariant(echelon);
      case 4: return QVariant::fromValue(coord);
      case 5: return QVariant(id);
    }
    return QVariant();
  }

  QString toString() const
  {
    return QString("%1 - %2 - %3 - %4 - %5 - %6")
              .arg(index)
              .arg(name,type)
              .arg(coord.lat(),12,'f',12)
              .arg(coord.lon(),12,'f',12)
              .arg(echelon);
  }
};

Q_DECLARE_METATYPE( meteo::GeoPoint )
Q_DECLARE_METATYPE( PlaceInfo )

class ForecastPlaceInfo
{
public:
  ForecastPlaceInfo()
  {
    echelon = -1;
  }

  int echelon;
  QDateTime dt;
  PlaceInfo fromPlace;
  PlaceInfo toPlace;
};
Q_DECLARE_METATYPE( ForecastPlaceInfo )

typedef QList<ForecastPlaceInfo>  ForecastPlaceInfoList;
Q_DECLARE_METATYPE( ForecastPlaceInfoList )

class PlaceModel : public QAbstractTableModel
{
  Q_OBJECT

  Q_PROPERTY( int columnCount READ columnCount WRITE setColumnCount )
  Q_PROPERTY( ForecastPlaceInfoList forecastPlaces READ forecastPlaces )

public:
  enum Column {
    kTypeColumn,
    kNameColumn,
    kIndexColumn,
    kEchelonColumn,
    kCoordColumn,
    kIdColumn
  };

  enum Role {
    PlaceInfoRole = Qt::UserRole + 1,
    ForecastRole,
  };

  explicit PlaceModel(QObject *parent = 0);

  void setColumnCount(int n) { columnCount_ = n; }

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

  virtual bool insertRows(int row, int count, const QModelIndex &parent);
  virtual bool removeRows(int row, int count, const QModelIndex &parent);

  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  virtual QVariant data(const QModelIndex &index, int role) const;
  virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

  void swapRows(int row1, int row2);

  int findLastRow(const QVariant& value, int column) const;
  int findRow(const QVariant& value, int column, int offset = 0) const;
  QList<PlaceInfo> findPlaces(const QVariant& value, int column, int offset = 0) const;

  void clear();

  QList<PlaceInfo> places() const { return data_; }

  void removeForecastInfo(const meteo::GeoPoint& from, const meteo::GeoPoint& to);
  void setForecastInfo(const ForecastPlaceInfo& info);
  ForecastPlaceInfoList forecastPlaces() const;
  bool findForecastInfo(ForecastPlaceInfo* info, const meteo::GeoPoint& from, const meteo::GeoPoint& to);

private:
  // параметры
  int columnCount_;

  // данные
  int idSeq_;
  QList<PlaceInfo> data_;
  QList<ForecastPlaceInfo> forecast_;
};

#endif // METEO_COMMONS_UI_CUSTOM_PLACEMODEL_H
