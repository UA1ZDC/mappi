#ifndef METEO_COMMONS_UI_MAP_WIDGETS_WEATHER_TERMSELECT_H
#define METEO_COMMONS_UI_MAP_WIDGETS_WEATHER_TERMSELECT_H

#include <qdialog.h>
#include <qmap.h>
#include <qstring.h>
#include <qtreewidget.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/map_isoline.pb.h>
#include <meteo/commons/proto/map_radar.pb.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/widgets/mapwidget.h>

class QDate;
class QMenu;

namespace Ui {
class TermSelect;
}

namespace meteo {
namespace map {

class Document;
class MapView;

namespace weather {

class TermSelect : public MapWidget
{
  Q_OBJECT
  public:
    TermSelect( const QMap< QString, proto::Map>& types, MapWindow* window );
    ~TermSelect();

    void setCurrentMapType( const QString& type );
    void setCurrentMapType( const proto::Map& map );

    void keyReleaseEvent( QKeyEvent* e );

  protected:
    bool eventFilter( QObject* watched, QEvent* event );

  private:
    enum MapStatus {
      kAppliedFull      = 0,
      kAppliedPart      = 1,
      kNoApplied        = 2
    };
    Ui::TermSelect* ui_;
    QMap< QString, proto::Map > types_;
    proto::Map currentmap_;
    QMenu* menu_;
    QMap< std::string, QList< proto::WeatherLayer > > weatherMaps_;

    void loadTypesInList();
    void loadTermsForCurrentMap();

    void getAvailableDataForLayers();
    void getAvailableData(proto::WeatherLayer);
    void getSrc(proto::WeatherLayer wl, QDateTime dtbeg, QDateTime dtend, meteo::puanson::proto::Puanson punch);
    void getField(proto::WeatherLayer wl, QDateTime dtbeg, QDateTime dtend, meteo::puanson::proto::Puanson punch);
    void getRadar(proto::WeatherLayer wl, QDateTime dtbeg, QDateTime dtend, meteo::map::proto::RadarColor color);
    void getIso(proto::WeatherLayer wl, QDateTime dtbeg, QDateTime dtend, meteo::map::proto::FieldColor);
    void getSigwx(proto::WeatherLayer wl, QDateTime dtbeg, QDateTime dtend);

    MapStatus hasMap( QTreeWidgetItem* item ) const ;
    bool hasLayer( QTreeWidgetItem* item ) const ;

    void updateItems();

    static QString keyFromParameters( const QDateTime& dt, int center, int hour, int model, int level, int leveltype );
    static bool parametersFromKey( const QString& key, QDateTime* dt, int* center, int* hour, int* model, int* level, int* leveltype );

    static QString keyFromMap( const QDateTime& dt, int center, int hour, int model );
    static bool mapFromKey( const QString& key, QDateTime* dt, int* center, int* hour, int* model );

    static QString keyFromFieldDescription( const meteo::field::DataDesc& descr, bool* ok = 0 );
    static QString keyFromSurfaDescription( const meteo::surf::CountData& descr, bool* ok = 0 );

  private:
    QString makeMapInfoTitle(const proto::Map& );

  private slots:
    void slotCurrentMapIndexChanged( int indx );
    void slotDateChanged( const QDate& date );
    void slotItemSelectedChanged();
    void slotSpinSplineKoefChanged( int value );
    void slotHourChange(const QString& hour);
    void slotAddMap();
    void slotRmMap();
    void slotContextMenu( const QPoint& pos );
    void slotDblCl(QTreeWidgetItem *, int);
    void slotOk();
    void slotNo();
};

}
}
}

#endif
