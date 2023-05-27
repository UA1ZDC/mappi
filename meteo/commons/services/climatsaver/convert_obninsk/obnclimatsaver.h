#ifndef METEO_COMMONS_SERVICE_OBNCLIMATSAVER_H
#define METEO_COMMONS_SERVICE_OBNCLIMATSAVER_H

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/complexmeteo.h>
#include <meteo/commons/proto/climat.pb.h>

#include <meteo/commons/services/climatsaver/climatsaver.h>
#include <meteo/commons/services/climatsaver/convert_obninsk/obnsurfsaver.h>
#include <meteo/commons/services/climatsaver/convert_obninsk/obnaerosaver.h>

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

    class ObnClimatSaver : public ClimatSaver {
    public :
      ObnClimatSaver(const QDate& start, const QDate& end, const QString& station, std::shared_ptr<Psql> db);
      virtual ~ObnClimatSaver();

      bool updateClimatData(rpc::Channel* chan);
      bool updateClimatData(Psql* db, rpc::Channel* chan, const QString& str, sprinf::MeteostationType cltype);
      
    private:
      //параметры поиска в БД
      QDate _dtStart;
      QDate _dtEnd;
      QString _station;

      QMap<QString, QString> _oldStations;
      ClimatStation _curStation; //чтоб передать параметры станции для сохранения

      std::shared_ptr<Psql> _db;
    };
  }
}

#endif
