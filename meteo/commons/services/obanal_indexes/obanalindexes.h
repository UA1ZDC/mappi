#ifndef METEO_COMMONS_SERVICES_OBANALINDEXES_OBANALINDEXES_H
#define METEO_COMMONS_SERVICES_OBANALINDEXES_OBANALINDEXES_H

#include <qdatetime.h>
#include<qmap.h>

class TObanalDb;

namespace obanal {
  class TField;
}

namespace meteo {
  namespace field {
    class DataDescResponse;
    class DataRequest;
    class DataDesc;
  }
}

namespace meteo {
  class TObanalService;
  
  class ObanalIndexes {
  public:
    struct FieldsId {
      std::string t850;
      std::string t500;
      std::string D850;
      std::string D700;
      std::string f850;
      std::string f500;
      std::string d850;
      std::string d500;
      FieldsId() {}
      FieldsId(const std::string& at850): t850(at850) {}
    };
    struct FieldsKey {
      int hour = 0;
      int model = 0;
      int center = 0;
      int net_type = 0;
      FieldsKey() {}     
      FieldsKey(int ahour, int amodel, int acenter, int anet):
	hour(ahour), model(amodel), center(acenter), net_type(anet) {}
      bool operator<(const FieldsKey& key) const {
	if (hour < key.hour) {
	  return true;
	} else if (model < key.model) {
	  return true;
	} else if (center < key.center) {
	  return true;
	} else if (net_type < key.net_type) {
	  return true;
	}
	return false;
      }	
    };
        
    ObanalIndexes();
    ~ObanalIndexes();

    bool run(const QDateTime& dt, const QStringList& descrname);
    bool run(const QDateTime& dt, const QString& descrname);

    bool runField(const QDateTime& dt);
    
  private:
    bool getAvailable(const QDateTime& dt, QMap< FieldsKey, FieldsId >* fileid);
    meteo::field::DataDescResponse* requestAvailable(meteo::field::DataRequest* req,
						     const QString& descrname, int level);
    bool getField(const std::string &id, obanal::TField *fd, meteo::field::DataDesc* fdescr = nullptr);
    bool saveField(obanal::TField* fd, const meteo::field::DataDesc& fdescr);
    
    obanal::TField* calcSweat(const obanal::TField& atd850, const obanal::TField& att,
			      const std::string& if850, const std::string& if500,
			      const std::string& id850, const std::string& id500);
  private:
    meteo::TObanalService* _oservice = nullptr;
    TObanalDb* _db = nullptr;
  };
}

#endif
