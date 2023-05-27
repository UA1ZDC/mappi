#ifndef METEO_COMMONS_SERVICES_CLIMATSAVER_AEROSAVER_H
#define METEO_COMMONS_SERVICES_CLIMATSAVER_AEROSAVER_H

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/complexmeteo.h>
#include <meteo/commons/proto/climat.pb.h>


namespace meteo {
  namespace rpc {
    class Channel;
  }
}

namespace zond {
  class Zond;
}

namespace meteo {
  namespace surf {
    class ValueReply;
    class FullValue;
    class TZondValueReply;
  }
}

namespace meteo {
  class Psql;
  class DbiEntry;
  class ConnectProp;
  
  namespace climat {

    class AeroSaver  {
    public:
      AeroSaver() = default;
      virtual ~AeroSaver();

      void updateAeroData(rpc::Channel* chan, const QString& station, sprinf::MeteostationType type,
			  const QDate& start, const QDate& end);


      QString dtCheckTableName();
      QDateTime first() { return _first; }
      QDateTime last() { return _last; }

      QString curStation() { return _curStation; }
      
    private:
      
      bool addAeroAccumulation(rpc::Channel* chan, const QString& station, sprinf::MeteostationType type, const QDateTime& dt);
      virtual surf::TZondValueReply* requestAeroData(rpc::Channel* chan, const QString& station,
						     const QDateTime& date, const QStringList& reqdescr);
      
      bool checkQuality(float value, int quality);


      int ddtoCode(float dd, float ff);

      
      QString createAeroJson(zond::Zond* zond, const QString& station,
			     sprinf::MeteostationType type);
      QString createAeroWindJson(zond::Zond* zond, const QString& station, sprinf::MeteostationType type);
      QString createAeroTropoJson(zond::Zond* zond, const QString& station,
				  sprinf::MeteostationType type, const QString& dt);
      QString createAeroLevelJson(zond::Zond& zond, const QString& station,
				  sprinf::MeteostationType type, const QString& dt);
      QString createAeroIsotermJson(zond::Zond* zond, const QString& station,
				    sprinf::MeteostationType type, const QString& dt);

      bool getOneZondData(const QString & station, const QString &dts, zond::Zond *znd);

      
    private:
      
      QDateTime _first;
      QDateTime _last;

      QString _curStation;
    };
    
  }
}


#endif
