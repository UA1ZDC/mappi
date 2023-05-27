#ifndef MAPPI_THEMATIC_ALGS_DATASERVICESTORE_H
#define MAPPI_THEMATIC_ALGS_DATASERVICESTORE_H

#include <mappi/thematic/algs/datastore.h>

namespace mappi {
  namespace to {
    //! Доступ к данным ПО и сохранения ТО через сервис (работа с БД)
    class DataServiceStore : public DataStore {
    public:
      DataServiceStore();
      virtual ~DataServiceStore();

      virtual bool getFiles(const QDateTime& start, const QString& satname, mappi::conf::InstrumentType itype,
			    const QList<std::string>& chAlias, QMap<std::string, QString>* chfiles);
      virtual bool getFiles(const QDateTime& start, const QString& satname, mappi::conf::InstrumentType itype,
			    const QList<uint8_t>& chNum, QMap<uint8_t, QString>* chfiles);
      //      virtual bool saveThematic(const QDateTime& start, mappi::conf::ThemType);

      virtual bool save(const struct meteo::global::PoHeader& header, conf::ThemType type, const QString& fileName);
      
    private:
      
    };

  }
}
 


#endif
