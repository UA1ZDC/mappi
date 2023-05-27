#ifndef METEO_COMMONS_UI_PLUGINS_RADARMAP_H
#define METEO_COMMONS_UI_PLUGINS_RADARMAP_H

#include <qpair.h>
#include <qmap.h>

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>
#include <meteo/commons/proto/map_radar.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>

namespace Ui {
class RadarMap;
}

class QAbstractButton;
class QMenu;
class QTreeWidgetItem;

namespace meteo {
namespace map {

class Layer;

class RadarMap : public MapWidget
{
  Q_OBJECT
  public:
    RadarMap( MapWindow* window );
    ~RadarMap();

    void setCurrentMap( const QString& map );

  protected:
    bool eventFilter( QObject* watched, QEvent* event );

  private:
    Ui::RadarMap* ui_;
    QMenu* menu_;
    QMap< QAbstractButton*, QPair< int, int > > kLayers;
    QMap<QString, proto::RadarColor> radartypes_;
    proto::RadarColor current_;

    QPair<int, int> currentLayer() const ;

    int currentIndex( const QString& title = QString() ) const ;

    QDateTime currentDateTime() const ;

    QTreeWidgetItem* currentItem() const ;

    void loadAvailableMaps();
    void loadResponseTree( surf::DataDescResponse* response );

    void loadRadar();
    bool hasLayer(QTreeWidgetItem* item);
    void updateItems();
    meteo::map::Layer* layerByItem(QTreeWidgetItem* item);

  private slots:
    void slotCurrentCmbChanged( const QString& name );
    void slotCurrentLayerChanged( QAbstractButton* btn );
    void slotDateChanged( const QDate& dt );
    void slotSelectionChanged();

    void slotAddRadarBtn();
    void slotDelRadarBtn();
    void slotDoubleClick();
    void slotCloseBtn();
    
    void slotContextMenu( const QPoint& pnt );
};

}
}

#endif
