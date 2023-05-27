#ifndef METEO_COMMONS_SERVICE_CLIMATSAVER_H
#define METEO_COMMONS_SERVICE_CLIMATSAVER_H

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/complexmeteo.h>
#include <meteo/commons/proto/climat.pb.h>

#include <meteo/commons/services/climatsaver/surfsaver.h>
#include <meteo/commons/services/climatsaver/aerosaver.h>

#include <QDateTime>

namespace meteo {
  namespace rpc {
    class Channel;
  }
}


namespace meteo {
  class Psql;
  class DbiEntry;
  class ConnectProp;
  
  namespace climat {

    class ClimatSaver {
    public :
      ClimatSaver(const QDate& start, const QDate& end);
      virtual ~ClimatSaver();

      virtual bool updateClimatData(rpc::Channel* chan);
      virtual bool updateClimatData(rpc::Channel* chan, const QString& station,
				    sprinf::MeteostationType type, sprinf::MeteostationType cltype,
				    QDateTime* first, QDateTime* last);

      SurfSaver* getSurf() { return _surf.get(); }
      AeroSaver* getAero() { return _aero.get(); }

    protected:

      ClimatSaver(const QDate& start, const QDate& end,
		  std::unique_ptr<SurfSaver> surf,
		  std::unique_ptr<AeroSaver> aero);

      
      // bool updateClimatDataObn(rpc::Channel* chan);
      // bool updateClimatDataObn(Psql* db, rpc::Channel* chan, const QString& str, sprinf::MeteostationType cltype);
    protected:

      // void init( const ConnectProp& prop );
      
      QDate findDtStart(const QString& query);
      // void updateSurfData(rpc::Channel* chan, const QString& station, sprinf::MeteostationType type,
      // 			  const QDate& start, QDateTime* first, QDateTime* last);
      // void updateAeroData(rpc::Channel* chan, const QString& station, sprinf::MeteostationType type,
      // 			  const QDate& start, QDateTime* first, QDateTime* last);

      // bool addSurf(rpc::Channel* chan, const QString& station, sprinf::MeteostationType type,
      // 		   const QDateTime& date, bool setR);
      // bool addSurfAccumulation(ComplexMeteo* md, const QString& station, sprinf::MeteostationType type);
      // bool addSurfHour(ComplexMeteo* md, const QString& station, sprinf::MeteostationType type);
      // bool addSurfDay(ComplexMeteo* md, const QString& station, sprinf::MeteostationType type);

      // bool updateDbSurf();
      // bool updateSurfDayMax();
      // bool updateSurfDayBitmask();
      // bool updateSurfDecade();
      // bool updateSurfMonthAvg();
      // bool updateSurfMonthDays();
      // bool updateSurfMonthSum();


      // bool addAeroAccumulation(rpc::Channel* chan, const QString& station, sprinf::MeteostationType type, const QDateTime& dt);


      // bool requestSurfData(rpc::Channel* chan, const QString& station, sprinf::MeteostationType type,
      // 			   const QDateTime& date, bool setR, ComplexMeteo* md);
      // surf::TZondValueReply* requestAeroData(rpc::Channel* chan, const QString& station,
      // 					    const QDateTime& date, const QStringList& reqdescr);
      
      // bool checkQuality(float value, int quality);
      
      // bool updWvko(int ww, float Ef, int* Wvko);
      // bool setCLMH(int clmh, float* res);
      // //bool hasCL(surf::ValueReply* surf);
      // bool setdd(const TMeteoParam& ffp, float dd, int* ddCode);
      // int ddtoCode(float dd, float ff);

      
      // QString createAeroJson(zond::Zond* zond, const QString& station,
      // 			     sprinf::MeteostationType type);
      // QString createAeroWindJson(zond::Zond* zond, const QString& station, sprinf::MeteostationType type);
      // QString createAeroTropoJson(zond::Zond* zond, const QString& station,
      // 				  sprinf::MeteostationType type, const QString& dt);
      // QString createAeroLevelJson(zond::Zond& zond, const QString& station,
      // 				  sprinf::MeteostationType type, const QString& dt);
      // QString createAeroIsotermJson(zond::Zond* zond, const QString& station,
      // 				    sprinf::MeteostationType type, const QString& dt);
      // QString createSurfAccumJson(ComplexMeteo* md, const QString& station, sprinf::MeteostationType type);
      // QString createSurfHourJson(ComplexMeteo* md, const QString& station, sprinf::MeteostationType type);
      // QString createDayJson(ComplexMeteo* md, const QString& station, sprinf::MeteostationType type);
      
      QDate findDtStart(const QString& station, sprinf::MeteostationType type, const QString& collection);
      void  updateStation(const ClimatStation& station, const QDateTime& first, const QDateTime& last);

      // bool getOneZondData(const QString & station, const QString &dts, zond::Zond *znd);

      
      // surf::TZondValueReply* requestAeroDataObn(rpc::Channel* , const QString& station,
      // 						const QDateTime& date, const QStringList& reqdescr);
      // bool requestSurfDataObn(rpc::Channel* chan, const QString& station, sprinf::MeteostationType type,
      // 			      const QDateTime& date, bool setR, ComplexMeteo* md);

      //void fillStationObn(const QString& station, sprinf::MeteostationType type, const DbiEntry& doc);
      
    private:
      QDate _dtStart;
      QDate _dtEnd;

      std::unique_ptr<SurfSaver> _surf;
      std::unique_ptr<AeroSaver> _aero;
      


      // QMap<QString, QString> _oldStations;
      // ClimatStation _curStation;
      // QString _curTable;
      
      // NoSql* _db = nullptr;
    };
  }
}

#endif
