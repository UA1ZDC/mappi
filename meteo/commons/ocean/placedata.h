#ifndef METEO_COMMONS_OCEAN_PLACEDATA_H
#define METEO_COMMONS_OCEAN_PLACEDATA_H

#include <commons/geobasis/geopoint.h>
#include <commons/meteo_data/meteo_data.h>

#include <meteo/commons/ocean/ocean.h>

namespace ocean {
  
class PlaceData
{
  public:
    PlaceData();
    PlaceData(const QString& name, const meteo::GeoPoint& coord, uint count, uint maxlev, const QDateTime& dt);

    void clear();


    QString name() const {return name_;}
    const meteo::GeoPoint& coord() const { return coord_; }
    uint count() const { return count_; }
    uint maxlevel() const { return maxlevel_; }
    QDateTime dt() const { return dt_; }
    const Ocean& ocean() const { return ocean_; }

    void setIdentification(const QString& name, const meteo::GeoPoint& coord, uint count, uint maxlev, const QDateTime& dt);
    void setData( TMeteoData& data);
    TMeteoData data() const { return data_; }

    // bool setData(const meteo::surf::OneOceanValueOnPoint &ocean);
    // bool setRegion(const meteo::GeoPoint& p1, const meteo::GeoPoint& p2);
  //  bool getMeteoData(float pur, TMeteoData *md)const;

  friend const PlaceData& operator>>(const PlaceData& data, QByteArray& out);
  friend PlaceData& operator<<(PlaceData& data, const QByteArray& ba);

  friend QString& operator<<( QString& str, const PlaceData& gc );
  friend TLog& operator<<( TLog& log, const PlaceData& gc );


  private:
    QString name_;
    meteo::GeoPoint coord_;
    uint count_ = 0;
    uint maxlevel_ = 0;
    QDateTime dt_;
    Ocean ocean_;

    TMeteoData data_;
};

const PlaceData& operator>>(const PlaceData& data, QByteArray& out);
PlaceData& operator<<(PlaceData& data, const QByteArray& ba);

}

#endif
