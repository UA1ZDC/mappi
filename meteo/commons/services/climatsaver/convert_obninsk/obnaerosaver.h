#ifndef METEO_COMMONS_SERVICES_CLIMATSAVER_OBNAEROSAVER_H
#define METEO_COMMONS_SERVICES_CLIMATSAVER_OBNAEROSAVER_H

#include <meteo/commons/services/climatsaver/aerosaver.h>

namespace meteo {
  namespace climat {
    
    class ObnAeroSaver : public AeroSaver {
    public:
      ObnAeroSaver(std::shared_ptr<Psql> db);
      virtual ~ObnAeroSaver();

      const ClimatStation& currentStation() { return _curStation; }
    private:

      void fillStationObn(const QString& station, sprinf::MeteostationType type, const DbiEntry& doc);
      
      surf::TZondValueReply* requestAeroData(rpc::Channel* , const QString& station,
					     const QDateTime& date, const QStringList& reqdescr);
    
      bool getOneZondData(const QString & station, const QString &dts, zond::Zond *znd);

    private:

      std::shared_ptr<Psql> _db;      
      QMap<QString, QString> _oldStations;
      ClimatStation _curStation;      
    };
    
  }
}


#endif
