#include "placemodel.h"

#include <cross-commons/debug/tlog.h>

PlaceModel::PlaceModel(QObject *parent)
  : QAbstractTableModel(parent)
{
  idSeq_ = 0;
  columnCount_ = 3;
}

int PlaceModel::rowCount(const QModelIndex& /*parent*/) const
{
  return data_.size();
}

int PlaceModel::columnCount(const QModelIndex& /*parent*/) const
{
  return columnCount_;
}

bool PlaceModel::insertRows(int row, int count, const QModelIndex& parent)
{
  int lastRow = row + count - 1;

  beginInsertRows(parent, row, lastRow);
  for ( int r=row; r<=lastRow; ++r ) { data_.insert(r, PlaceInfo()); }
  endInsertRows();

  return true;
}

bool PlaceModel::removeRows(int row, int count, const QModelIndex& parent)
{
  int lastRow = row + count - 1;

  beginRemoveRows(parent, row, lastRow);
  for ( int r=row; r<=lastRow; ++r ) {
    if ( data_.size() > row ) { data_.removeAt(row); }
  }
  endRemoveRows();

  return true;
}

QVariant PlaceModel::data(const QModelIndex& index, int role) const
{
  int row = index.row();
  int col = index.column();

  if ( data_.size() <= row ) { return QVariant(); }

  if ( Qt::DisplayRole == role ) {
    if ( kCoordColumn == col ) {
      return data_.at(row).coord.toString();
    }

    return data_.at(row).data(col);
  }
  else if ( PlaceInfoRole == role ) {
    return QVariant::fromValue(data_.at(row));
  }

  return QVariant();
}

bool PlaceModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
  if ( !idx.isValid() ) { return false; }

  int row = idx.row();

  if ( data_.size() <= row ) { return false; }

  if ( Qt::EditRole == role ) {
    PlaceInfo info = value.value<PlaceInfo>();

    if ( -1 != data_.at(row).id ) {
      info.id = data_.at(row).id;
    }
    else {
      info.id = idSeq_++;
    }

    data_[row] = info;

    emit dataChanged(index(idx.row(),0), index(idx.row(),columnCount()));

    return true;
  }

  return false;
}

void PlaceModel::swapRows(int row1, int row2)
{
  if ( row1 < 0 || row2 < 0 ) { return; }

  while ( row1 >= rowCount() || row2 >= rowCount() ) { insertRow(rowCount()); }

  PlaceInfo tmp = data_.at(row1);
  data_[row1] = data_.at(row2);
  data_[row2] = tmp;

  emit dataChanged(index(row1,0),index(row1,columnCount()));
  emit dataChanged(index(row2,0),index(row2,columnCount()));
}

int PlaceModel::findLastRow(const QVariant& value, int column) const
{
  int row = findRow(value, column);
  for ( int r=row; r!=-1; ) {
    row = r;
    r = findRow(value, column, row + 1);
  }

  return row;
}

int PlaceModel::findRow(const QVariant& value, int column, int offset) const
{
  if ( offset >= data_.size() ) { return -1; }

  for ( int i=offset,isz=data_.size(); i<isz; ++i ) {
    if ( value == data_.at(i).data(column) ) {
      return i;
    }
  }

  return -1;
}

QList<PlaceInfo> PlaceModel::findPlaces(const QVariant& value, int column, int offset) const
{
  QList<PlaceInfo> list;

  int row = findRow(value, column, offset);
  while ( -1 != row ) {
    list += data_.at(row);

    row = findRow(value, column, row + 1);
  }

  return list;
}

void PlaceModel::clear()
{
  beginResetModel();
  data_.clear();
  forecast_.clear();
  endResetModel();
}

void PlaceModel::removeForecastInfo(const meteo::GeoPoint& from, const meteo::GeoPoint& to)
{
  for ( int i=0,isz=forecast_.size(); i<isz; ++i ) {
    if ( from == forecast_[i].fromPlace.coord && to == forecast_[i].toPlace.coord ) {
      forecast_.removeAt(i);
      return;
    }
  }
}

void PlaceModel::setForecastInfo(const ForecastPlaceInfo& info)
{
  for ( int i=0,isz=forecast_.size(); i<isz; ++i ) {
    if ( info.fromPlace.coord == forecast_.at(i).fromPlace.coord
         && info.toPlace.coord == forecast_.at(i).toPlace.coord )
    {
      forecast_[i] = info;
      return;
    }
  }

  forecast_ += info;
}

ForecastPlaceInfoList PlaceModel::forecastPlaces() const
{
  // исключаем из маршрута запасные аэродромы
  QList<PlaceInfo> places;
  for ( int i=0,isz=data_.size(); i<isz; ++i ) {
    if ( tr("запасной") == data_[i].type ) { continue; }

    places << data_[i];
  }

  QList< QPair<meteo::GeoPoint,meteo::GeoPoint> > parts;
  for ( int i=1,isz=places.size(); i<isz; ++i ) {
    parts += qMakePair(places[i - 1].coord, places[i].coord);
  }

  ForecastPlaceInfoList list;
  foreach ( const ForecastPlaceInfo& info, forecast_ ) {
    QPair<meteo::GeoPoint,meteo::GeoPoint> part = qMakePair(info.fromPlace.coord, info.toPlace.coord);
    if ( parts.contains(part) ) {
      list += info;
    }
  }
  return list;
}

bool PlaceModel::findForecastInfo(ForecastPlaceInfo* info, const meteo::GeoPoint& from, const meteo::GeoPoint& to)
{
  for ( int i=0,isz=forecast_.size(); i<isz; ++i ) {
    if ( from == forecast_.at(i).fromPlace.coord && to == forecast_.at(i).toPlace.coord ) {
      if ( 0 != info ) {
        *info = forecast_.at(i);
      }
      return true;
    }
  }
  return false;
}

QVariant PlaceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ( Qt::Horizontal != orientation ) { return QVariant(); }
  if ( Qt::DisplayRole != role       ) { return QVariant(); }

  switch ( section ) {
    case 0: return tr("Тип");
    case 1: return tr("Пункт");
    case 2: return tr("Индекс");
    case 3: return tr("Эшелон");
    case 4: return tr("Координата");
    case 5: return tr("Id");
  }

  return QVariant();
}
