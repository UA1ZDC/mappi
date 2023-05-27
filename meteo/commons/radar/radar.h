#ifndef METEO_COMMONS_RADAR_RADAR_H
#define METEO_COMMONS_RADAR_RADAR_H

#include <qvector.h>
#include <qdatetime.h>

#include <commons/geobasis/geovector.h>

#include <meteo/commons/proto/surface_service.pb.h>

namespace meteo {

class Radar
{
  public:
    Radar( const GeoPoint& center, int cell_w, int cell_h, int row_s, int clmn_s, int empty );
    Radar(){}
    ~Radar();

    void setData( int descr, const QVector<double>& data, const GeoVector& coords );

    const GeoPoint& center() const { return center_; }
    int cellWidth() const { return cell_width_; }
    int cellHeight() const { return cell_height_; }

    int rowSize() const { return row_size_; }
    int columnSize() const { return clmn_size_; }

    double cellValue( int x, int y, bool* ok ) const ;
    bool hasValue( int x, int y ) const ;

    int descr() const { return descr_; }

    const QDateTime& datetime() const { return datetime_; }
    int station() const { return station_; }

    double direction() const { return direction_; }
    double speed() const { return speed_; }

    void setEmpty( int empty ) { empty_val_ = empty; }

    void setProto( const surf::OneMrlValueReply& proto );
    surf::OneMrlValueReply proto() const ;

    static Radar& fromProto( const surf::OneMrlValueReply& proto, Radar* r );

  private:
    GeoPoint center_;           //!< центр
    int cell_width_;            //!< ширина ячейки (м)
    int cell_height_;           //!< высота ячейки (м)
    int row_size_;              //!< количество строк
    int clmn_size_;             //!< количество столбцов
    QVector<double> data_;      //!< данные
    GeoVector coords_;          //!< координаты данных
    QVector<bool> hasval_;      //!< признак наличия значения в ячейке
    int descr_;                 //!< дескриптор данных
    QDateTime datetime_;        //!< срок данных
    int station_;               //!< индекс станции
    double direction_;          //!< направление перемещения
    double speed_;              //!< скорость перемещения
    int empty_val_;
};


}

#endif
