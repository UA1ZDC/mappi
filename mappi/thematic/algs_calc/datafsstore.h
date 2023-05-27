#ifndef MAPPI_THEMATIC_ALGS_DATAFSSTORE_H
#define MAPPI_THEMATIC_ALGS_DATAFSSTORE_H

#include <mappi/thematic/algs_calc/datastore.h>

namespace mappi { namespace to {
  //  namespace to {
    //! Доступ к данным ПО в ФС и сохранения ТО в ФС
    class DataFsStore : public DataStore {
    public:
      DataFsStore();
      virtual ~DataFsStore();

      
      bool getFiles(const QDateTime& start, const QString& satname, mappi::conf::InstrumentType itype,
        const QList<std::string>& chAlias, QMap<std::string, QString>* chfiles);
    
      
      bool getFiles(const QDateTime& , const QString& , mappi::conf::InstrumentType ,
          const QList<uint8_t>& , QMap<uint8_t, QString>* )
			{ static_assert("Not realised", ""); return false; }
      //      bool saveThematic(const QDateTime& start, mappi::conf::ThemType);
      
      bool save(const struct meteo::global::PoHeader&, conf::ThemType, const std::string &themname, const QString& ,const std::string &);

    private:
      QString _dirname;
      
    };

  //}
} }
 


#endif
