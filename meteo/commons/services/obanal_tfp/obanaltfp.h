#ifndef METEO_COMMONS_SERVICES_OBANALINDEXES_OBANALTFP_H
#define METEO_COMMONS_SERVICES_OBANALINDEXES_OBANALTFP_H

#include <qdatetime.h>
#include <qmap.h>
#include <memory>



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

  class ObanalTfp {
  public:
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

    
    ObanalTfp();
    ~ObanalTfp();
    
    bool runField(const QDateTime& dt);
    
  private:
    bool getAvailable(const QDateTime& dt, const QString& descrname, int level, int net_type, QMap< FieldsKey, std::string >* fileid);
    meteo::field::DataDescResponse* requestAvailable(meteo::field::DataRequest* req,
						     const QString& descrname, int level);
    bool getField(const std::string &id, obanal::TField *fd, meteo::field::DataDesc* fdescr = nullptr);
    bool saveField(obanal::TField* fd, const meteo::field::DataDesc& fdescr);

    bool runField_Renard_Clark(const QDateTime& dt, int net_type);
    bool runField_Renard_ClarkQpot(const QDateTime& dt, int net_type);
    bool runField_HuberPock_Kress(const QDateTime& dt, int net_type);
    bool runField_JapanAgency(const QDateTime& dt, int net_type);
    bool runField_Hewson(const QDateTime& dt, int net_type);
    bool runField_Hewson6(const QDateTime& dt, int net_type);


    bool runField_Renard_ClarkKm(const QDateTime& dt, int net_type);
    bool runField_JapanAgencyKm(const QDateTime& dt, int net_type);
    bool runField_HuberPock_KressKm(const QDateTime& dt, int net_type);
   
    std::unique_ptr<obanal::TField> calcTFP(const obanal::TField& field);
    std::unique_ptr<obanal::TField> calcTFPKm(const obanal::TField& field);
    std::unique_ptr<obanal::TField> calcTFP_Hewson(const obanal::TField& field);
    std::unique_ptr<obanal::TField> calcTFP_Hewson6(const obanal::TField& field);
    bool toQe(obanal::TField* t850, const obanal::TField& q850, bool isSpecific);

    std::unique_ptr<obanal::TField> calcHewsomM2(const obanal::TField& field);
    bool runField_Hewson6_m1(const QDateTime& dt, int net_type);
    bool runField_Hewson6_m2(const QDateTime& dt, int net_type);

    std::unique_ptr<obanal::TField> calcZte(const obanal::TField& h850, const obanal::TField& h500,
			    const obanal::TField& T850, const obanal::TField& T700,
			    const obanal::TField& T500);
    std::unique_ptr<obanal::TField> calcTpot(const obanal::TField& T, float level);
    void deriveMeanAxis(float fi_x, float la_x, float fi_a, float la_a,
		               float fi_b, float la_b, float fi_c, float la_c,
		               float fi_d, float la_d, float *betta, float *length);


  private:
    meteo::TObanalService* _oservice = nullptr;
    TObanalDb* _db = nullptr;
    
  };
  
}


#endif
