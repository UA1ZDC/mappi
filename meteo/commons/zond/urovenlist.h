#ifndef TUROVENLIST_H
#define TUROVENLIST_H

#include "clouddata.h"

#include <meteo/commons/zond/turoven.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/geobasis/geovector.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <qlist.h>
#include <qvector.h>

namespace zond {
  const float KR_OSOBY_T_TROPO = 1.; //Критерий ОТ по температуре для тропосферы
  const float KR_OSOBY_T_STRATO = 2.; //Критерий ОТ по температуре для стратосферы
  const float KR_OSOBY_U = 15.;//Критерий ОТ по влажности
  const float KR_OSOBY_DD = 10.;//Критерий ОТ по направлению ветра
  const float KR_OSOBY_FF = 5.;//Критерий ОТ по скорости ветра


  const QVector<float> standartLevels =
      QVector<float>() << 200 << 250 << 300 << 400 << 500 << 600 << 700 << 850 << 925 << 950 << 1000;
  class Level{
  public:
    Level(float v):level(v){}
    Level():level(BAD_METEO_ELEMENT_VAL){}

    float p()const {return level;}

    Level& operator=( const float& src ) {
      level = src;
      return *this;
    }

    float operator-( const float& src )const{
      return level - src;
    }

    float operator+( const float& src )const{
      return level + src;
    }

    float level;
    bool operator==( const Level& src )const{
      return MnMath::isEqual(level,src.level);
    }
    operator float() const { return level; }
    // operator Level() { return Level(level); }
    friend QDataStream &operator<<(QDataStream &stream, const Level& z) {
      stream << z.level;
      return stream;
    }

    friend QDataStream &operator>>(QDataStream &stream, Level& z) {
      stream >> z.level;
      return stream;
    }

  };
  class CloudDkr{
  public:
    QMap<float, float> DkrP;
    QMap<float, float> DkrH; //запоняется внешними средствами

    CloudDkr();
    bool isCloud(float p, float d );

    bool getCloudDeficit(float H,float* Dkr) const;

  };


  class UrovenList  {
  public:

    UrovenList();
    ~UrovenList();


    bool setData(const meteo::surf::OneZondValueOnStation& );
    bool setDataNoPreobr(const meteo::surf::OneZondValueOnStation& );
    bool setDataNoPreobr(const TMeteoData& );
    bool setZDataNoPreobr(const TMeteoData& );
    void setData(int level_type, float level, const zond::Uroven & );
    bool setData(const TMeteoData& data);
    bool setDataPoH(const TMeteoData& data);


    bool contains(float val, int level_type)const;
    void insert(float level, const Uroven &val);
    void insertMulti(float level, const Uroven &val);
    int size();


    float value(int level_type, float level, ValueType urType, bool* res = nullptr);
    float valueDescr(int level_type, float level, int descr, bool* res = nullptr);

    void restoreUrList();
    void print() const;
    QString toString() const;


    void setValue(int level_type, float level, int descr, float value, int quality);
    void checkOsobyPoint();

    int countValid(ValueType) const;

    bool haveData() const {return isData_;}
    void UV2DF(bool onlyAbsent);
    void UV2DF();
    void DF2UV();
    bool getUrPoLevelType(int level_type, Uroven * aur)const;
    bool getUrPoLevelType(int level_type, QList<Uroven> * aur)const;
    bool getUrPoP( float aval, Uroven * aur)const;
    bool getUrPoH( float aval, Uroven * aur)const;
    bool getUrPoHH( float aval,Uroven * aur)const;

    const QMap<Level, Uroven> & urovenList() const {return urList_;}

    bool oprUrHTpoP( float aval, Uroven * aur)const;
    bool oprUrHTpoH( float aval,Uroven * aur)const;
    bool oprUrWindPoP( float aval,Uroven * aur)const;
    bool oprUrWindPoH( float aval,Uroven * aur)const;
    float oprHpoP( float p, float z_p = BAD_METEO_ELEMENT_VAL, float z_t = BAD_METEO_ELEMENT_VAL);
    bool p_tPoH( float h, float *p, float *t );
    bool oprPPoParam(ValueType urType, float value, QList<float> *P);
    bool oprHPoParam(ValueType urType, float value, QList<float>* H);



    //!< определение параметров на уровне станции методом поиска по уровням используется в oprHPoP()
    bool getUrz(Uroven *); //!< вместо getUr(11000, ur)
    bool getPz(float *);
    bool getTz(float *);

    bool getUrMaxWind(Uroven *);//!< вместо getUr(35000, ur)
    bool getUr5001000(Uroven *)const; //!< вместо getUr(15000, ur)
    bool getTropo(Uroven *); //!< вместо getUr(25000, ur)
    bool getTropo(QList<Uroven> *);
    bool getCloudDkrH(CloudDkr *Dkr) const;

    bool interpolDF();

    zond::Uroven getUrPoKey(float key)const;
    bool isEmpty() const;
    void clear();

    void preobr(bool onlyAbsent );
    void preobr();
    void preobr_for_wrap() { preobr(); }

    void setDateTime(const QDateTime& adt) {dateTime_ = adt;}
    void setDateTimeAnalyse(const QDateTime& adt){dateTimeAnalyse_ = adt;}

    void setStIndex(const QString& ast){stIndex_= ast;}
    void setCoord(const meteo::GeoPoint& agp){coord_= agp;}
    void addCenterName(const QString& ast);

    QDateTime dateTime()const {return dateTime_;}
    QDateTime dateTimeAnalyse()const {return dateTimeAnalyse_;}
    QString stIndex()const{return stIndex_;}
    QStringList centersName()const{return center_;}
    meteo::GeoPoint coord()const{return coord_;}

    zond::CloudData cloud(){return cloud_;}
    zond::CloudData* cloudP(){return &cloud_;}
    void setCloud(const zond::CloudData& cld){ cloud_ = cld;}

    const QList<Uroven> &getUrOsobyT() {return urList_T_;}
    const QList<Uroven> &getUrOsobyV() {return urList_V_;}
    bool getPprizInvLow(Uroven * ur);
    bool oprH1Fast(float *h1, float *h2);
    bool getPprizInvHi_P(Uroven *ur);
    bool getPprizInvHi_H(Uroven * ur);
    bool getPprizInvHi(Uroven *ur);


    int sizeofBuffer()const;
    friend QDataStream &operator<<(QDataStream &stream, const UrovenList& z);
    friend QDataStream &operator>>(QDataStream &, UrovenList& z);
    friend const UrovenList& operator>>(const UrovenList& data, QByteArray& out);
    friend UrovenList& operator<<(UrovenList& data, const QByteArray& ba);

    // Оператор для извлечения данных  в прото сообщении
    QList <meteo::surf::MeteoDataProto> toMeteoDataProto();

    QList<meteo::surf::Uroven> toProtoUroven() const;
    TMeteoData toMeteoData() const;

  protected:
    bool getPNaURPoMeanSeaLevel(float *p,const Uroven&);
    void setHeightValue(int level_type,float level,  int descr, float value, int quality);
    void setIsoBarValue(int level_type,float level,  int descr, float value, int quality);
    void setTropoValue(int descr, float value, int quality);
    void setUrZValue(int descr, float value, int quality);
    void setMaxWindValue(int descr, float value, int quality);
    void setUrHValue(int level, int descr, float value, int quality);

    void fromProtoUroven(const meteo::surf::Uroven& res, zond::Uroven *ur_);
    bool oprD();
    bool oprH();
    void resetlevelType();

    bool isData_; //!< Загружены или нет данные
    QMap<Level, zond::Uroven> urList_; //!< данные по уровням
    QMap<int, zond::Uroven> urHList_; //!< данные по высотам
    zond::Uroven urZ_;//!< данные у земли
    zond::Uroven urZinv_;//!< приземная инверсия
    zond::Uroven urMaxWind_;//!< данные на уровне максимального ветра
    QList<zond::Uroven> urTropo_;//!< данные на уровне тропопаузы
    QList<zond::Uroven> urList_T_; //!< данные по особым точкам по температуре
    QList<zond::Uroven> urList_V_; //!< данные по особым точкам по ветру
    zond::CloudData cloud_;

    ValueType keyType_;

    QDateTime dateTime_;         //на какую дату прогноз
    QDateTime dateTimeAnalyse_; //когда проводили анализ
    QString stIndex_;
    QStringList center_;
    meteo::GeoPoint coord_;
  };
}

#endif
