#ifndef METEO_COMMONS_UI_MAP_AERODOCUMENT_H
#define METEO_COMMONS_UI_MAP_AERODOCUMENT_H

#include "document.h"

#include <meteo/commons/zond/placedata.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/ui/map/axis/layeraxis.h>

namespace meteo {
namespace map {

enum LayerType {
  kAeroBlank1           = 1,            //!< бланк аэрологической диаграммы
  kAeroBlank2           = 2,            //!< бланк аэрологической диаграммы
  kAeroBlank3           = 3,            //!< бланк аэрологической диаграммы
  kAeroT                = 4,            //!< кривая стратификации
  kAeroTd               = 5,            //!< кривая точек росы
  kAeroSost             = 6,            //!< кривая состояния
  kAeroWindIso          = 7,            //!< Ветер по высотам (основные точки)
  kAeroWindAdd          = 8,            //!< Ветер по высотам (дополнительные точки)
  kAeroKondens          = 9,            //!< уровень конденсации
  kAeroKonvek           = 10,           //!< уровень конвекции
  kAeroKNS              = 11,            //!< конвективно-неустойчивый слой
  kAeroInvers           = 12,           //!< слои инверсии
  kAeroTropo            = 13,           //!< Тропопауза
  kAeroOblak            = 14,           //!< облачные слои
  kAeroObled            = 15,           //!< слои обледенения
  kAeroBolt             = 16,           //!< слои болтанки
  kAeroTrace            = 17,           //!< слои конденсационных следов
  kAeroAxis             = 18,           //!< оси по станциям
  kAeroLines            = 19,           //!< сухие, влажные адиабаты, линии удельной влажности
  kAeroSaturIce         = 20,           //!< температура насыщения относительно льда
  kAeroAltAxis          = 21,           //!< шкала высоты
  kAeroLastLayer        = 22
};

class AeroDocument : public Document
{
  public:
    AeroDocument( const GeoPoint& mapcenter,
              const GeoPoint& doc_center,
              proto::DocumentType type,
              ProjectionType proj_type );
    AeroDocument( const proto::Document& doc );
    AeroDocument();
    ~AeroDocument();

    enum {
      Type = kAero
    };

    int type() const { return Type; }

    bool init();
    
    Document* stub( const GeoPoint& center, const QSize& docsize );

    const zond::PlaceData& data() const { return data_; }
    void setData(const zond::PlaceData &profileData);
    void showNoData();    
    void hideNoData();
    bool create( const zond::PlaceData& data );
    bool createEmptyBlank();

  private:
    zond::PlaceData data_;
    puanson::proto::Puanson ddff_;

 private:
    AeroDocument( Document* doc, const GeoPoint& center, const QSize& docsize );

  private:
    bool addMain();
    bool addTemperature(const GeoVector& gv );
    bool addTemperatureR(const GeoVector& gv );
    bool addTempSaturIce(const GeoVector& gv ); //!< температура насыщения относительно льда
 
    bool addSost();                             //!< кривая состояния
    bool addInvers();                           //!< слои инверсии
    bool addKondens( float* pkond );           //!< уровень конденсации
    bool addKonvek( float pkond );             //!< уровень конвекции
    bool addKNS();                              //!< конвективно-неустойчивый слой
    bool addObled();                            //!< слои обледенения  
    bool addOblak();                            //!< облачные слои
    bool addBoltan();                           //!< слои болтанки
    bool addTropo( float* Ptropo );
    void addTropo(const zond::Uroven& ur_tropo, Layer* l );
    bool addTrace(float Ptropo);               //!< слои конденсационных следов

    bool addTempLayers();
    bool addTempMax();
    bool addWind(const zond::Uroven& ur );

    bool addEnergy();//энергия неустойчивости

  private:
    bool addBlank();
    bool addSetka();
    bool addSA81();
    bool addSA81Line();
    bool addSA81BlankText();
    void addSA81BlankText(int* value, int size, float Tstart, float Tstep, 
                          float P, const meteo::Property& prop);
    bool addBlankLines();
    bool addSuhAdiab();
    bool addVlazhAdiab();
    bool addUdelVlazh();
    void createAxis();

    bool addLinePodpis(const GeoVector& line, meteo::map::Layer* layer );

    bool addLine(const GeoVector& skelet,const ::meteo::Property& prop, LayerType layerType);
    map::GeoPolygon* addLine(map::Layer* layer, const GeoVector& skelet, int width, const QColor& color);

    bool addText(const GeoVector& skelet,const QString& text, const meteo::Property& prop, LayerType type );
    meteo::map::GeoText* addText( Layer* layer, const QString& text, const GeoPoint& pnt, int fontSize, const QColor& color );

    bool prepLayer( LayerType type );

    void drawSectionPhenomen(
        float p1,
        float p2,
        float t,
        const QColor& clr,
        const QString& pixmap,
        Layer* layer,
        Qt::PenStyle lineStyle = Qt::SolidLine );

  private:
    static bool logF2X_one( const Projection& proj, const GeoPoint& geoCoord, QPoint* meterCoord );
    static bool logX2F_one( const Projection& proj, const QPoint& meterCoord, GeoPoint* geoCoord );
    static bool logF2X_onef( const Projection& proj, const GeoPoint& geoCoord, QPointF* meterCoord );
    static bool logX2F_onef( const Projection& proj, const QPointF& meterCoord, GeoPoint* geoCoord );
};

}
}

#endif
