#ifndef METEO_COMMONS_UI_STATIONLIST_STATIONLIST_H
#define METEO_COMMONS_UI_STATIONLIST_STATIONLIST_H

#include <qdialog.h>
#include <qdatetime.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/surface.pb.h>
#include <commons/geobasis/geopoint.h>

class QTreeWidgetItem;

namespace Ui {
class StationList;
}


namespace meteo {
namespace surf {
class StationsDataReply;
}

//! Список станций, по которым есть данные
class StationList : public QDialog {
  Q_OBJECT
  public:
    StationList(QWidget* parent,
            const QString& name,
            const QList<meteo::sprinf::MeteostationType>& stations_to_load);
    ~StationList();

    //! Параметры выбранной станции
    QString station() { return station_; }
    QString name()    { return name_; }
    meteo::GeoPoint coord() { return coord_; }

    void setPoint(const QString& station, const GeoPoint& gp, const QString& name);
    void setDataTypes(const QList<meteo::surf::DataType> &dtypes) { dtypes_ = dtypes; }
    void clearAndClose();
                
  public slots:
    void setDateTime(const QDateTime& dt);
    void setStation(const QString& station);
    void fill();
    void fill(const QList<meteo::surf::DataType>& types);
           
  private slots:
    void dtChanged();
    void recalcDistance();
    void changeCurrent(QTreeWidgetItem* item);
    
  signals:
    void dtChanged(const QDateTime& dt);
    void currentStationChanged();

  private:
    surf::StationsDataReply* requestStations(const QList<meteo::surf::DataType>& data_types);
    
  private:
    
    Ui::StationList* ui_;
    QTreeWidgetItem* current_item_; //!< Текущий выбранный item (построенный)
    QDateTime curDt_; //!< Дата/время построенного item
    QString station_; //!< Станция построенного item
    QString name_;    //!< Название построенного item
    meteo::GeoPoint coord_;  //!< Координаты построенного item
    
    QList<meteo::surf::DataType> dtypes_; //!< Типы данных для запроса

};


}

#endif
