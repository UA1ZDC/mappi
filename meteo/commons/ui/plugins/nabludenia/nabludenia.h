#ifndef METEO_COMMON_UI_PLUGINS_NABLUDENIA_NABLUDENIA_H
#define METEO_COMMON_UI_PLUGINS_NABLUDENIA_NABLUDENIA_H

#include <qtreewidget.h>

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>
#include <meteo/commons/global/weatherloader.h>

#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/ui/map/view/mapview.h>

namespace Ui {
 class Nabludenia;
}

namespace meteo {
namespace puanson {
namespace proto {
  class Puanson;
}
}

namespace map {

class Puanson;

class Nabludenia : public MapWidget
{
  Q_OBJECT
  public:
    Nabludenia( MapWindow* window );
    ~Nabludenia();

    void setCurrentMap( const QString& title );
    void setCurrentPunch( const meteo::puanson::proto::Puanson& punch );

  protected:
    bool eventFilter( QObject* watched, QEvent* event );
    void closeEvent( QCloseEvent* event );

  private:
    void hideGribColumns();
    void showGribColumns();
    QString stepsToStr(int32_t,int32_t);

    Ui::Nabludenia* ui_;
    QMenu* menutermtype_;
    QMenu* switchHour_;
    proto::Map currentmap_;
    meteo::puanson::proto::Puanson currentpunch_;
    bool termsall_;

    QMap< QString, proto::Map > punchmaps_;
    QMap< QString, meteo::puanson::proto::Puanson > punchlibrary_;

    void turnSignals();
    void muteSignals();


    void setCurrentSource( proto::DataSource source );

    proto::DataSource currentSource() const;

    void loadAvailableData();
    void loadAvailableSurface();
    void loadAvailableField();
    void loadAvailableGrib();

    void updateTypeData( const QList<int>& types );

    meteo::surf::CountDataReply* sendSurfaceRequest();
    meteo::surf::GribDataAvailableReply* sendGribRequest();
    meteo::field::DataDescResponse* sendFieldRequest();

    int currentLevel() const;
    int currentTypeLevel() const;
    int currentTerm() const;

   // void addLayer( QTreeWidgetItem* item );
   // void removeLayer( QTreeWidgetItem* item );
    bool hasLayer( QTreeWidgetItem* item );
    bool hasLayer( Layer* layer );
    void updateItems();
    meteo::map::Layer* layerByItem( QTreeWidgetItem* item );

    void showPunch( const meteo::puanson::proto::Puanson& proto );

    void sendSelectPointsEvent( const QDateTime& srok ) const;
    void sendSelectPointsEvent( const QString& fieldId ) const;

  private slots:
    void slotDateTimeChanged();
    void slotTermChanged( int indx);
    void slotAddLayerBtn();
    void slotDoubleClicked(QTreeWidgetItem *, int);
    void slotRemoveLayer();
    void slotContextMenu(QPoint);

    void slotItemSelectionChanged( QTreeWidgetItem* item );
    void slotPunchmapChanged( const QString& title );
    void slotSourceChanged( int indx );

    void slotLevelChanged( int type_level, int level );

    void slotChangeSelection();

    void slotBtntermClicked();
    void slotTermStandard();
    void slotTermAll();
};

}
}

#endif
