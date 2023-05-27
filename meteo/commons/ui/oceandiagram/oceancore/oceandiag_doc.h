#ifndef METEO_COMMONS_UI_OCEANDIAGRAM_OCEANCORE_OCEANDIAG_DOC_H
#define METEO_COMMONS_UI_OCEANDIAGRAM_OCEANCORE_OCEANDIAG_DOC_H

#include <cross-commons/app/paths.h>

#include <commons/textproto/tprototext.h>

#include <meteo/commons/ocean/placedata.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/ui/map/event.h>
#include <meteo/commons/ui/map/puanson.h>

class TMeteoData;

namespace meteo {
class Projection;
class PlaceData;
class OceanDiag;
class GeoPoint;
} // meteo


namespace meteo {
namespace map {
class Layer;
class Document;
class MapScene;
class LayerGraph;
} // map
} // meteo

namespace meteo {
namespace odiag {

enum LayerType {
  ODIAG_LAYER_BLANK  =  1,//бланк диаграммы состояния моря
  ODIAG_LAYER_T  =  2,//кривая температуры
  ODIAG_LAYER_S  =  3,//кривая солености
  ODIAG_LAYER_C  =  4,//кривая скорости звука
  ODIAG_LAYER_CHANNEL = 5,//подводный звуковой канал
  ODIAG_LAYER_AXIS =  6,//Оси
  LAST_ODIAG_LAYER =  7
};

class OceanDiagDoc : public QObject
{
  Q_OBJECT
public:
  static bool logF2X_one( const Projection& proj, const GeoPoint& geoCoord, QPoint* meterCoord );
  static bool logX2F_one( const Projection& proj, const QPoint& meterCoord, GeoPoint* geoCoord );
  static bool logF2X_onef( const Projection& proj, const GeoPoint& geoCoord, QPointF* meterCoord );
  static bool logX2F_onef( const Projection& proj, const QPointF& meterCoord, GeoPoint* geoCoord );

  OceanDiagDoc(meteo::map::Document* doc, meteo::map::MapScene* scene);
  ~OceanDiagDoc();

  bool create(const ocean::PlaceData *profileData);
  bool createEmptyBlank();
  void setData(const ocean::PlaceData* aprofileData);

  void showNoData();
  void hideNoData();
  bool hasData();

  const QList<TMeteoData>& getMdList(const ocean::PlaceData* aprofileData);
    
public slots:
  void layerChanged(map::Layer* layer, int event);
  void repaintChannel(float);

signals:
  void PZK_Exist(float);

private:
  bool createGrid();
  bool addChannel(float xe0_=-999.);

  float StoT(float);
  float CtoT(float);

  void createLayerT();
  void createLayerS();
  void createLayerC();
  void createLayerGrid();

private:
  bool has_data_;
  map::Document* doc_;
  map::MapScene* scene_;
  const ocean::PlaceData* pd_;

  map::LayerGraph* layerT_;
  map::LayerGraph* layerS_;
  map::LayerGraph* layerC_;
  map::LayerGraph* layerGrid_;
  map::Layer* layerPZK_;
  QList<TMeteoData> mdList_;
};

} // odiag
} // meteo

#endif // METEO_COMMONS_UI_OCEANDIAGRAM_OCEANCORE_OCEANDIAG_DOC_H
