#ifndef METEO_COMMONS_SERVICES_CLIMATSAVER_SURFSAVER_H
#define METEO_COMMONS_SERVICES_CLIMATSAVER_SURFSAVER_H

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/complexmeteo.h>
#include <meteo/commons/proto/climat.pb.h>


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

    class SurfSaver  {
    public:
      SurfSaver() = default;
      virtual ~SurfSaver();

      void updateSurfData(rpc::Channel* chan, const QString& station, sprinf::MeteostationType type,
			  const QDate& start, const QDate& end);

      
      QString dtCheckTableName();
      QDateTime first() { return _first; }
      QDateTime last() { return _last; }

    private:

     
      bool addSurf(rpc::Channel* chan, const QString& station, sprinf::MeteostationType type,
		   const QDateTime& date, bool setR);
      bool addSurfAccumulation(ComplexMeteo* md, const QString& station, sprinf::MeteostationType type);
      bool addSurfHour(ComplexMeteo* md, const QString& station, sprinf::MeteostationType type);
      bool addSurfDay(ComplexMeteo* md, const QString& station, sprinf::MeteostationType type);
      
      bool updateDbSurf();
      bool updateSurfDayMax();
      bool updateSurfDayBitmask();
      bool updateSurfDecade();
      bool updateSurfMonthAvg();
      bool updateSurfMonthDays();
      bool updateSurfMonthSum();

      virtual bool requestSurfData(rpc::Channel* chan, const QString& station, sprinf::MeteostationType type,
				   const QDateTime& date, bool setR, ComplexMeteo* md);
      
      bool updWvko(int ww, float Ef, int* Wvko);
      bool setCLMH(int clmh, float* res);
      //bool hasCL(surf::ValueReply* surf);
      bool setdd(const TMeteoParam& ffp, float dd, int* ddCode);
      int ddtoCode(float dd, float ff);

      QString createSurfAccumJson(ComplexMeteo* md, const QString& station, sprinf::MeteostationType type);
      QString createSurfHourJson(ComplexMeteo* md, const QString& station, sprinf::MeteostationType type);
      QString createDayJson(ComplexMeteo* md, const QString& station, sprinf::MeteostationType type);


      bool checkQuality(float value, int quality);

     
    private:
      
      QDateTime _first;
      QDateTime _last;

      bool _isRmain = true; //!< true - осадки передаются за основной срок
      
    };
    
  }
}

#endif
