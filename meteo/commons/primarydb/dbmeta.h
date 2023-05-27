#ifndef METEO_COMMONS_PRIMARYDB_DbMeta_H
#define METEO_COMMONS_PRIMARYDB_DbMeta_H

#include <meteo/commons/proto/sprinf.pb.h>
#include <commons/geobasis/geopoint.h>
#include <commons/meteo_data/meteo_data.h>

#include <QSharedPointer>



namespace meteo {

  struct StationInfo {
    QString station;                     //!< станция
    int type = BAD_METEO_ELEMENT_VAL;    //!< тип станции
    GeoPoint coord;                      //!< координаты
    int index = BAD_METEO_ELEMENT_VAL;   //!< индекс
    QString icao;                        //!< ИКАО
    QString call_sign;                   //!< позывной
    int buoy = BAD_METEO_ELEMENT_VAL;    //!< номер буя
    QString name;                        //!< название международное
    QString name_ru;                     //!< название на русском
    int country = BAD_METEO_ELEMENT_VAL; //!< номер страны
    QString country_en;  //!< название страны международное
    QString country_ru;  //!< название страны на русском
    QString satellite;   //!< название спутника
    QString instrument;    //!< название прибора

    bool operator<(const StationInfo& key) const {
      if (type < key.type) return true;
      if (type == key.type && station < key.station) return true;
      return false;
    }
    QString toString() const ;
  };

  namespace anc {

    struct StationKey {
      QString station;
      meteo::sprinf::MeteostationType type;
      StationKey(const QString& astation, meteo::sprinf::MeteostationType atype):station(astation), type(atype)
      {}

      bool operator<(const StationKey& key) const {
        if (type < key.type) return true;
        if (type == key.type && station < key.station) return true;
        return false;
      }
    };

    class DbMeta {
    public:
      DbMeta();
      virtual ~DbMeta();

      bool checkStations();
      bool fillTypes();

      const StationInfo* stationInfo(int category, const QString& station);
      const StationInfo* aerodromeInfo(const QString& stationRu);
      meteo::sprinf::MeteostationType stationType(int category);

      int bufrcode(const QString& text);
      bool fillStations();
      bool loadBufrCode();

      bool loadEsimo();
      descr_t esimoDescr(const QString& name) { return _esimoDescr.value(name, BAD_DESCRIPTOR_NUM); }

      
    protected:

      QMap<int, meteo::sprinf::MeteostationType> _types; //!< тип данных (минимальная граница), тип станции
      QMap<StationKey, StationInfo> _info;
      QMap<QString, StationInfo> _aerodrRu; //отечественные аэродромы по названию

      QMap<QString, int> _bufrcode; //!< Для явлений

      QMap<QString, descr_t> _esimoDescr; //!< имена ЕСИМО в дескриптор
    };
  }
}

#endif
