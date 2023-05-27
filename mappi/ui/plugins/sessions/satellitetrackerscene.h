#ifndef MAPPI_UI_PLUGIN_SESSION_SATELLITETRACKERSCENE_H
#define MAPPI_UI_PLUGIN_SESSION_SATELLITETRACKERSCENE_H

#include <memory>

#include <qgraphicsscene.h>
#include <qgraphicsview.h>
#include <qmap.h>

#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>

#include <sat-commons/satellite/satellite.h>

#include <mappi/proto/schedule.pb.h>

class Satellite;

namespace meteo {
  namespace map {
    class Document;
    class Layer;
  } // map
} // meteo

namespace mappi {

  namespace conf {
    class Reception;
    class Instrument;
  }
  namespace schedule {
    class Session;
  }

  class SatelliteTrackerScene : public QGraphicsScene
  {
    Q_OBJECT
    struct RcvWndInf {
      meteo::GeoVector   circle;
      QString            areaUuid_;
    };

  public:
    SatelliteTrackerScene(QGraphicsView* parent);
    ~SatelliteTrackerScene();

    enum MapType {
		  kOrbita = 1, kReceiver = 2
    };

    void resize(const QSize& size);

    void changeMapMode(MapType mode);

    void setConf(const mappi::conf::Reception& conf);

    void setSession(std::shared_ptr<schedule::Session> session, const conf::Instrument& instrument,
                    int timeoffset, const MnSat::STLEParams &);

  protected:
    void drawBackground(QPainter *painter, const QRectF &rect);
    void drawForeground(QPainter *painter, const QRectF &rect);

  private slots:
    void slotTimeout();

  private:
    void resizeDocument(const QSize& size);
    bool setupMap(MapType mode);
    void redraw();

    void createReceiverLabel(const QString& receiverName, const meteo::GeoPoint& pos);
    void createSatelliteLabel(const QString& satelliteName);
    void createSatelliteInformation(std::shared_ptr<schedule::Session> session);

    void drawReceiverArea();
    void drawSatelliteTrajectory();
    void drawSatelliteArea();
  private:
    QTimer*                  updateTimer_;         //!< Таймер обновления карты

    meteo::map::Document*    document_;            //!< Документ - карта
    meteo::map::Layer*       layer_;               //!< Слой со спутником и станцией

    MapType                  mapMode_;             //!< Режим отображения карты 1 - Орбита, 2 - место приема
    Satellite*               satellite_;           //!< Интерфейс доступа к информации о спутнике

    QString                  receiverName_;        //!< Имя станции приема
    meteo::GeoPoint          receiverCoords_;      //!< Координаты станции приема
    QMap<float, RcvWndInf*>  receiverAreaInfo_;    //!< Координаты зоны приема станции для разных углов

  float                    nadirAngle_;          //!< Максимальный угол от надира
  meteo::GeoVector         satelliteTrajectory_; //!< Траектория спутника
  meteo::GeoVector         satelliteAreaCoord_;  //!< Координаты области снимка
  float                    satelliteHeight_;     //!< Высота спутника

  QGraphicsItemGroup*      receiverAreas_;       //!< Зона покрытия станциии приема

  QGraphicsItemGroup*      satelliteLabel_;      //!< Метка спутника
  QGraphicsPixmapItem*     satelliteIcon_;       //!< Иконка спутника
  QGraphicsTextItem*       satelliteText_;       //!< Название спутника
  QGraphicsPolygonItem*    satelliteDirection_;  //!< Вектор направления движения спутника

  // UUID объектов
  QString                  receiverLabelUuid_;  //!< Название станции приема
  QString                  receiverPixUuid_;    //!< Иконки станции приёма

  QString                  satelliteLabelUuid_; //!< Название спутника
  QString                  satellitePixUuid_;   //!< Иконка спутника
  QString                  satelliteTrajectoryUuid_; //!< Траектория спутника
  QString                  satelliteAreaUuid_;   //!< Зона снимка

  int timeOffset_ = 0;
  
private:
  static float calcRadius(float angle, float height);
  static meteo::GeoVector getCircle(const meteo::GeoPoint& center, float radius);
};

} // mappi

#endif // MAPPI_UI_PLUGIN_SESSION_SATELLITETRACKERSCENE_H
