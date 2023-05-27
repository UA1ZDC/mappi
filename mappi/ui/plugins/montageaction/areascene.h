#ifndef AREASCENE_H
#define AREASCENE_H

#include <qgraphicsscene.h>
#include <qgraphicsview.h>

#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>
#include <sat-commons/satellite/satviewpoint.h>
#include <mappi/schedule/session.h>
#include <mappi/ui/satlayer/satlayer.h>
#include <mappi/ui/pos/posgrid.h>

namespace meteo {
  namespace map {
    class Document;
    class Layer;

class AreaScene: public QGraphicsScene
{
  Q_OBJECT
public:
  AreaScene(QGraphicsView* parent);
  ~AreaScene();
  void setSession(const ::mappi::schedule::Session& session, const ::mappi::conf::Instrument& instrument, const MnSat::TLEParams& tle);
  void resize(const QSize& size);
protected:
  void drawBackground(QPainter *painter, const QRectF &rect);
  void drawItems(QPainter *painter, int numItems, QGraphicsItem *items[], const QStyleOptionGraphicsItem options[], QWidget *widget);
  void drawForeground(QPainter *painter, const QRectF &rect);
private:
  bool setupDoc();
  void resizeDocument(const QSize& size);
  void redraw();
  void drawSatelliteArea();

  meteo::map::Document*    document_;            //!< Документ - карта
  meteo::map::SatLayer*    layer_;               //!< Слой со спутником и станцией
  PosGrid*                 grid_;
  meteo::GeoPointPb        center_;              //!< Координаты станции приема
  float                    nadirAngle_;          //!< Максимальный угол от надира
  meteo::GeoVector         satelliteAreaCoord_;  //!< Координаты области снимка
  QString                  satelliteAreaUuid_;   //!< Зона снимка

};

  } // map
} // meteo

#endif // AREASCENE_H
