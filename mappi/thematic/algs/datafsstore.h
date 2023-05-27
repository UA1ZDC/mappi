#ifndef MAPPI_THEMATIC_ALGS_DATAFSSTORE_H
#define MAPPI_THEMATIC_ALGS_DATAFSSTORE_H

#include <mappi/thematic/algs/datastore.h>

namespace mappi::to {
  //  namespace to {
    //! Доступ к данным ПО в ФС и сохранения ТО в ФС
    class DataFsStore : public DataStore {
    public:
      DataFsStore();
      virtual ~DataFsStore();

      
      bool getFiles(const QDateTime& start, const QString& satname, mappi::conf::InstrumentType itype,
		    const QList<std::string>& chAlias, QMap<std::string, QString>* chfiles);
    
      
      bool getFiles(const QDateTime& start, const QString& satname, mappi::conf::InstrumentType itype,
			    const QList<uint8_t>& chNum, QMap<uint8_t, QString>* chfiles)
			{ static_assert("Not realised", ""); return false; }
      //      bool saveThematic(const QDateTime& start, mappi::conf::ThemType);
      
      bool save(const struct meteo::global::PoHeader& , conf::ThemType , const QString& );
     
    private:
      QString _dirname;
      
    };

  //}
}
 


#endif
