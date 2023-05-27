#include "placedata.h"

namespace ocean {

PlaceData::PlaceData()
{
  clear();
}

PlaceData::PlaceData(const QString& name, const meteo::GeoPoint& coord, uint count,
		     uint maxlev, const QDateTime& dt):
  name_(name),
  coord_(coord),
  count_(count),
  maxlevel_(maxlev),
  dt_(dt)
{
}
  
void PlaceData::clear()
{
  ocean_.clear();
  count_ = 0;
  coord_ = meteo::GeoPoint();
  name_ = QString();
}

void PlaceData::setIdentification(const QString& name, const meteo::GeoPoint& coord, uint count,
				  uint maxlev, const QDateTime& dt)
{
  ocean_.clear();
  name_ = name;
  coord_ = coord;
  count_ = count;
  maxlevel_ = maxlev;
  dt_ = dt;
}

void PlaceData::setData( TMeteoData& data) {
  ocean_.setData(data);
  count_ = ocean_.urovenList().size();
  data_ = data;
}

const PlaceData& operator>>(const PlaceData& data, QByteArray& out)
{
  QDataStream ds(&out, QIODevice::WriteOnly);
  ds << data.name_;
  ds << data.coord_;
  ds << data.count_;
  ds << data.maxlevel_;
  ds << data.dt_;
  ds << data.data_;

  return data;
}

PlaceData& operator<<(PlaceData& data, const QByteArray& ba)
{
  QDataStream ds(ba);
  ds >> data.name_;
  ds >> data.coord_;
  ds >> data.count_;
  ds >> data.maxlevel_;
  ds >> data.dt_;  
  ds >> data.data_;
  
  
  data.ocean_.clear();
  data.ocean_.setData(data.data_);

  return data;
}

QString& operator<<( QString& str, const PlaceData& pd )
{
  QString coordstr;
  coordstr << pd.coord_;
  QString urstr;
  auto urlist = pd.ocean_.urovenList();
  for ( auto it = urlist.begin(); it != urlist.end(); ++it ) {
    QString str;
    str << it.value();
    urstr += '\n' + str;
  }
  str += QString("Станция = %1. Координаты = %2. Количество = %3, Максимальная глубина = %4, Срок = %5. Метеодата = %6")
    .arg(pd.name_)
    .arg(coordstr)
    .arg(pd.count_)
    .arg(pd.maxlevel_)
    .arg(pd.dt_.toString(Qt::ISODate))
    .arg(urstr);
  return str;
}

TLog& operator<<( TLog& log, const PlaceData& pd )
{
  QString str;
  str << pd;
  return log << str;
}

}
