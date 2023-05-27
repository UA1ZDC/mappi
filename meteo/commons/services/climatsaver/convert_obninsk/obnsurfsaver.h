#ifndef METEO_COMMONS_SERVICES_CLIMATSAVER_OBNSURFSAVER_H
#define METEO_COMMONS_SERVICES_CLIMATSAVER_OBNSURFSAVER_H

#include <meteo/commons/services/climatsaver/surfsaver.h>

namespace meteo {
  namespace climat {

    class ObnSurfSaver : public SurfSaver  {
    public:
      ObnSurfSaver(std::shared_ptr<Psql> db);
      virtual ~ObnSurfSaver();

      void setStrQuery(const QString& squery) { _squery = squery; }
      const ClimatStation& currentStation() { return _curStation; }
      
    private:
      virtual bool requestSurfData(rpc::Channel* chan, const QString& station, sprinf::MeteostationType type,
				   const QDateTime& date, bool setR, ComplexMeteo* md);

      void fillStationObn(const QString& station, sprinf::MeteostationType type, const DbiEntry& doc);
      
    private:
      std::shared_ptr<Psql> _db;
      QString _squery;      
      ClimatStation _curStation;
    };
  }
}


#endif
