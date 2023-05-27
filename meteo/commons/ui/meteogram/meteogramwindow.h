#ifndef METEO_COMMONS_UI_PLUGINS_METEOGRAM_WINDOW_H
#define METEO_COMMONS_UI_PLUGINS_METEOGRAM_WINDOW_H

#include <qdialog.h>

#include <commons/meteo_data/tmeteodescr.h>
#include <meteo/commons/ui/custom/preparewidget.h>
#include <meteo/commons/proto/surface.pb.h>
#include <meteo/commons/proto/meteogram.pb.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>

template<class T> class QList;
class TMeteoData;
class StationWidget;
namespace meteo {
  namespace map {
    class LayerGraph;
    class LayerGrid;
    class MapWindow;
    class Document;
    class Incut;
    class TraceAction;
  } // map
} // meteo

namespace meteo {
  namespace field {
    class DataRequest;
    class ValueDataReply;
  } // field
} // meteo

namespace meteo {
  namespace app {
    class MainWindow;
  } // appname
} // meteo

namespace meteo {

  class MeteogramWindow : public PrepareWidget
  {
    Q_OBJECT

  public:
    MeteogramWindow(map::MapWindow* parent = nullptr, map::Document* doc = nullptr, bool stw_load = true);
    virtual ~MeteogramWindow();
    bool isContainsMap() { return isContainsMap_; }
    void buildGraph(QList<TMeteoData> &mdList,
                    bool isBuildT, bool isBuildDewPoint, bool isBuildP, bool isBuildPQNH, bool isBuildU, bool isBuildWind);
    void setButtonsState(bool isBuildT, bool isBuildDewPoint, bool isBuildP, bool isBuildU, bool isBuildPQNH, bool isBuildWind);

    static map::MapWindow* createWindow(app::MainWindow*);
    static map::Document* createDocument(map::MapWindow*);
    QList<TMeteoData> loadSrcData(bool remot, const QString &index) const;
    void generateDoc(QList<TMeteoData> &mdList);
    virtual void closeEvent(QCloseEvent *event);

  signals:
    void docGenerated(const QString& title);

  public slots:
    void slotRun();
    void slotUpdateCenters();
    void slotLoadState();
    void slotSaveState() const;

  private slots:
    void slotCenterChanged(int idx);

  private:
    map::LayerGraph* addLayerT(const QList<TMeteoData>& mdList);
    map::LayerGraph* addLayerTd(const QList<TMeteoData>& mdList);
    map::LayerGraph* addLayerP(const QList<TMeteoData>& mdList);
    map::LayerGraph* addLayerPQNH(const QList<TMeteoData>& mdList);
    map::LayerGraph* addLayerU(const QList<TMeteoData>& mdList);
    map::LayerGraph* addLayerWind(const QList<TMeteoData>& mdList);


    QString setupTitle(const QString &station, const QString &stationIndex,
                       const QDateTime &dtStart, const QDateTime &dtEnd, meteo::GeoPoint stationCoord);

    void getLoadSrcDataRequest(surf::DataRequest* req, const QString &index) const;
    QList<TMeteoData> loadFieldData() const;
    void updateCenters(const field::DataRequest& request);

    int calcMin(float min, float max) const;
    int calcMax(float min, float max) const;

    //! Добавляет в список list элементы из списка from, исключая данных за один скрок
    void merge(QList<TMeteoData>* list, const QList<TMeteoData>& from) const;
    TMeteoData merge(const TMeteoData& to, const TMeteoData& from, const QList<descr_t>& descrs) const;

    QList<TMeteoData> toMeteoData(const field::ValueDataReply& resp) const;


  private:
    void askRemote();
    bool setIncut(meteo::map::Incut *,const QDateTime &dtStart, const QDateTime &dtEnd);
    bool addStWidget();

    bool isContainsMap_ = false;
    map::Document* document_;
    QList<map::LayerGraph*> layers_;


    // служебные
    map::MapWindow* mapwindow_;
    StationWidget* station_widget_;
    int lastSelectedCenter_;
  };

} // meteo

#endif // METEO_COMMONS_UI_PLUGINS_METEOGRAM_WINDOW_H
