#ifndef METEO_COMMONS_UI_VERTICALCUT_CORE_VERTICALCUT_DOC_H
#define METEO_COMMONS_UI_VERTICALCUT_CORE_VERTICALCUT_DOC_H

#include <qmargins.h>

#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>
#include <commons/meteo_data/meteo_data.h>
#include <meteo/commons/zond/zond.h>
#include <meteo/commons/zond/placedata.h>
#include <meteo/commons/ui/map/document.h>

#include <meteo/commons/ui/map/scale.h>
#include <meteo/commons/ui/map/vprofile.h>


class TMeteoData;

namespace obanal {
class TField;
} // obanal



namespace meteo {
namespace map {
class Layer;
class LayerIso;
class LayerVProfile;
class LayerProfile;
class MapScene;
class MapWindow;



//!
class VProfileDoc : public Document
{
public:
  static const QVector<float> kIsoLevels;
  static const int kBlankWidth;

  VProfileDoc( const GeoPoint& mapcenter,
            const GeoPoint& doc_center,
            proto::DocumentType type,
            ProjectionType proj_type );
  VProfileDoc( const proto::Document& doc );
  VProfileDoc();
  virtual ~VProfileDoc();
  bool init();

  enum {
    Type = kVProfile
  };

  int type() const { return Type; }

  bool create(const QList<zond::PlaceData> &profilesData, int type);
  bool createForBulletin(const QList<zond::PlaceData>& profiles, const proto::Map &mapProto);
  void clear();

  QMargins paddings() const { return paddings_; }

  void setRamka(const QMargins& p, float maxP, float maxH = -1);

private:
  void createAxis(int type);
  bool addTropo();
  bool addMaxWind();

  bool addObled(); // слои обледенения
  bool addOblak(); // облака заливка
  bool addOblakSloi(); // облачные слои
  bool addBoltan(); // слои болтанки
  bool addTrace();  // слои конденсационных следов
  bool addTemperature();
  bool addTemperatureR();
  bool addH();
  bool addWindff();
  bool addWinddd();
  bool addNanoska();
  bool addWind();
  void addEchelon();

  bool prepLayer(int type, const QString&);
  bool prepLayerEasy(int type, const QString&);
  bool prepWindLayers();

  void showNoData();

  //! Вычисляет положение на бланке для каждого профиля из data на основе расстояния между координатами. Аргумент length
  //! задаёт ширину бланка.
  void calcDistanceByCoord(int length) ;
  //! Вычисляет положение на бланке для каждого профиля из data на основе времени формирования данных. Аргумент length
  //! задаёт ширину бланка.
  void calcDistanceByDt( int length) ;

  bool fillLayer(LayerIso* layer, float min, float max, float step, const QColor& color);
  void fillScaleOptions(ScaleOptions* opt, obanal::TField* f, float maxH, float x) const;

private:
  float getPByH(float x, float h) const;
  static bool cutLinearF2X_one( const Projection& proj, const GeoPoint& geoCoord, QPoint* meterCoord );
  static bool cutLinearX2F_one( const Projection& proj, const QPoint& meterCoord, GeoPoint* geoCoord );

  static bool cutLinearF2X_onef( const Projection& proj, const GeoPoint& geoCoord, QPointF* meterCoord );
  static bool cutLinearX2F_onef( const Projection& proj, const QPointF& meterCoord, GeoPoint* geoCoord );
  static bool isInRange(float value, float begin, float end, bool properBegin = false, bool properEnd = false);

  //!
  static QVector<float> yByValue(const obanal::TField* field, float x, float value, bool* ok = nullptr);


private:
  // атрибуты
  QMargins paddings_;

  // данные
  QList<zond::PlaceData> profilesData_;

  // служебные
  QMap<int,Layer*> dataLayerRef_;
  QMap<int,LayerIso*> isoLayers_;
  LayerVProfile *nanoskaLayer_ = nullptr;
  LayerProfile* layerProfiles_ = nullptr;

  
  VProfile vcut_;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_VERTICALCUT_CORE_VERTICALCUT_DOC_H
