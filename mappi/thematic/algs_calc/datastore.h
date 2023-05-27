#ifndef MAPPI_THEMATIC_ALGS_DATASTORE_H
#define MAPPI_THEMATIC_ALGS_DATASTORE_H

#include <mappi/proto/satellite.pb.h>
#include <mappi/proto/thematic.pb.h>

#include <mappi/global/streamheader.h>

#include <QMap>

class QDateTime;
class QString;


namespace mappi {
  namespace to {

    //! Интерфейс для доступа к данным ПО и сохранения ТО
    class DataStore {
    public:
      DataStore() {}
      virtual ~DataStore() {}

      virtual bool getFiles(const QDateTime& start, const QString& satname, mappi::conf::InstrumentType itype,
          const QList<std::string>& chAlias, QMap<std::string, QString>* chfiles) = 0;
      virtual bool getFiles(const QDateTime& start, const QString& satname, mappi::conf::InstrumentType itype,
          const QList<uint8_t>& chNum, QMap<uint8_t, QString>* chfiles) = 0;
      //      virtual bool saveThematic(const QDateTime& start, mappi::conf::ThemType) = 0;

      virtual bool save(const struct meteo::global::PoHeader& , conf::ThemType ,const std::string &themname,
                        const QString& ,const std::string &format) = 0;
      
    private:
      
    };

  }
}
 

#endif
