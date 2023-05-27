#ifndef METEO_COMMONS_SERVICES_CLIMATDATA_CLIMATAIR_H
#define METEO_COMMONS_SERVICES_CLIMATDATA_CLIMATAIR_H

#include <meteo/commons/proto/climat.pb.h>

#include <meteo/commons/zond/turoven.h>

class NS_PGBase;

namespace meteo {
  namespace climat {

    class Control;

    class ClimatAeroService   
    {
    public:
      ClimatAeroService();

      bool getMonthValue(const meteo::climat::DataRequest* req,
			 meteo::climat::AeroLayerListReply* res);

      bool checkRequest(const meteo::climat::DataRequest* req, QDateTime* dtStart, QDateTime* dtEnd, QString* err);
      
    private :

    };
    
  }
}

#endif
