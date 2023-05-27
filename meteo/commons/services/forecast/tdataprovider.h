#ifndef METEO_COMMONS_FORECAST_TDATAPROVIDER_H
#define METEO_COMMONS_FORECAST_TDATAPROVIDER_H

#include <commons/geobasis/geopoint.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/proc_read/daemoncontrol.h>
#include <meteo/commons/global/global.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/services/sprinf/sprinfservice.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/forecast.pb.h>
#include <meteo/commons/proto/climat.pb.h>
#include <meteo/commons/zond/zond.h>

#include <qstring.h>
#include <qpair.h>
#include <commons/obanal/obanal_struct.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/zond/placedata.h>

struct fieldValue {
  fieldValue() {
    value = 0.;
    dx = 0.;
    dy = 0.;
    dxx = 0.;
    dyy = 0.;
    dxy = 0.;
    rk = 0.;
  }
  float value;
  float dx;
  float dy;
  float dxx;
  float dyy;
  float dxy;
  float rk;
};




struct StationData {
  StationData(double alat, double alon, double ah, const QString &an) {
    coord.setFi( alat);
    coord.setLa(alon);
    coord.setAlt( ah);
    name = an;
    index = QString::Null();
    station_type = meteo::sprinf::kStationUnk;
  }
  StationData(meteo::GeoPoint ac, const QString &an, QString in, int type) {
    coord = ac;
    name = an;
    index = in;
    station_type = meteo::sprinf::MeteostationType(type);
  }

  StationData() {
    index = QString::Null();
    coord = meteo::GeoPoint::Invalid;
    /*coord.setFi(0.);
    coord.setLa(0.);
    coord.setAlt(0.);*/
    name = QString::Null();
    station_type = meteo::sprinf::kStationUnk;
  }

  meteo::GeoPoint coord;
  QString name;
  QString index;
  meteo::sprinf::MeteostationType station_type;
//  meteo::GeoPoint st_coord;
};

struct Cloud{
  int h;
  int N;
  int type;
  bool empty=true;
};

struct StationValue {
  StationData station;
  QString descr;
  //float value;
  float value;
  int qual;
  Cloud CL;
  Cloud CM;
  Cloud CH;
  Cloud total;
};

enum dataSource {
  cur_station_src = 0,   // брать данные по текущей станции
  near_station_src = 1,   // брать данные по ближайшей станции
  near_aero_station_src = 2,   // брать данные по ближайшей станции
  cur_aero_station_src = 3,   // брать данные по текущей станции
  field_src      = 4,   // брать данные по полю
  station_field_src      = 5,// брать данные по ближайшей станции или по полю
  field_station_src      = 6// брать данные  по полю или по ближайшей станции
};

struct FMetaData{
  int level =-100;
  int type_level = -100;
  int srok = 0;
  QString field_id="";
  StationData station;
  dataSource data_source_type = cur_station_src;
  int distance_to_point = 0;
  QDateTime dt;
};

inline bool operator<(const FMetaData &e1, const FMetaData &e2)
{
    if (e1.srok != e2.srok)
        return e1.srok > e2.srok;
    return e1.distance_to_point > e2.distance_to_point;
}


class TDataProvider: public QObject
{
  Q_OBJECT
public:
  TDataProvider(const QDateTime&, bool is_service = false);
  ~TDataProvider();

  void setDate(const QDateTime &adt);
  //**Методы для работы с полями метеорологических величин**
  // bool getAdvectPoint(const meteo::field::AdvectDataRequest &request, meteo::GeoPoint* agp);

  bool getValue(const FMetaData &, const QString & , QMultiMap<int, FMetaData> *meta_list , QMap<int, float> *mres_list);


  const QDateTime& getDate() {return date_time_;}

  QString getSDate() {return date_time_.toString(Qt::ISODate);};

  void calcTableDailyRateTemp(const meteo::climat::SurfMonthAllValues& atemp,
                              const meteo::climat::SurfMonthAllValues& cloud,
                              QMap<int, QMap<int, float> >* table,
                              QMap<int, QMap<int, int> >* count);

  //параметр - срок прогноза:
  float getPrognozInterval(const QString& hours, QDateTime *dt_start, QDateTime *dt_end);

  int  getFieldList(const meteo::field::DataRequest &request, QList< meteo::field::DataDesc> *fieldlist );
  bool getFiedValuePoIndex(const meteo::GeoPoint &srcp, std::string fnum, fieldValue*);
  zond::Zond *getZond(const StationData& st, int srok = 0);
  zond::Zond * getProfileData(const StationData &, int srok = 0);

  Q_INVOKABLE bool getAdvectPoint(int level, int center, int srok, meteo::GeoPoint* agp);
  Q_INVOKABLE float getAdvectTrackLength();
  Q_INVOKABLE bool savePrognoz(const StationData&, const QString&, const QString& , const QString& , int, const QString&, const QString& , int, int);
  Q_INVOKABLE int getHourData() { return date_time_.time().hour(); }
  Q_INVOKABLE double getSutHod(const StationData& st, int month, int hour);
  Q_INVOKABLE int getMonth() {return date_time_.date().month();}
  Q_INVOKABLE float getSunRiseSet(const StationData&, int type);


  QRegExp  getRegexpByParam(const QString& param);
  int checkRoundDigits(const QString &param);

  void loadSettings();

private:
  bool getSynValue(FMetaData *meta, const QString& descr, QMultiMap<int, FMetaData> *meta_list, QMap<int, float> *mres_list);
  bool specialCaseValue(FMetaData *meta, const QString& descr, QMultiMap<int, FMetaData> *meta_list, QMap<int, float> *mres_list);
  bool getFiedValue(FMetaData *meta, const QString& aval, QMultiMap<int, FMetaData> *meta_list , QMap<int, float> *mres_list);
  zond::Zond *getAeroData(const StationData&, int srok);
 // bool getAeroValue(FMetaData *meta, const QString& aval, QMap<int, FMetaData> *meta_list, QMap<int, float> *mres_list);

  QString dataKey(const StationData& st, const QDateTime &dt, const QString& aval, int level = 0 , int type_level = 1);

  QDateTime date_time_;
  // StationData station_;
  StationData advectStation_;
  float advectTrackLength_=0.;

  QHash<QString, int> fields;
  meteodescr::TMeteoDescriptor* _d;


  QHash<QString , StationValue> value_hash_;
  QHash<QString , fieldValue> field_value_hash_;
  QHash<QString , zond::Zond*> zond_hash_;

//  meteo::rpc::Channel *field_ctrl_;
//  meteo::rpc::Channel *src_ctrl_;
//  meteo::rpc::Channel *forecast_ctrl_;

  //Settings:
  QHash<QString, int> round_map_;//! Массив для округлений (какие значения до какиз занков округлять)
  QHash<QString, QString> regexp_map_;//! Массив регулярных выражений для каждого типа параметра
  int max_dist_to_near_station_=20000; //!максимальное расстояние до станции при поиске данных
  bool set_only_best_=true;
  int field_search_interval_=3;
  int src_search_interval_=1;
  int max_hour_ = 48;
  bool is_service_ = false;
};

#endif // METEO_COMMONS_FORECAST_TDATAPROVIDER_H
