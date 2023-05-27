#ifndef TRadarDataService_H
#define TRadarDataService_H

#include <commons/meteo_data/meteo_data.h>
#include <meteo/commons/proto/surface_service.pb.h>

//#include <sql/nspgbase/ns_pgbase.h>
#include <commons/geobasis/geodata.h>

namespace meteo {
  namespace surf {

    const int KVADRATS_DSCR = 5999;
    const int HEIGHT_DSCR = 7002;
    const int level_DSCR = 7006;
    const int KOLX_DSCR = 30021;
    const int KOLY_DSCR = 30022;
    const int dX_DSCR = 5033;
    const int dY_DSCR = 6033;
    const int isabsh_DSCR = 8001;
    const int proj_DSCR = 29001;
    const int STATION_DSCR = 1018;
    const int SYNOP_DIRECTION = 19005;
    const int SYNOP_SPEED = 19006;


class Control;

class TRadarDataService
{
  public:
    TRadarDataService(  );

    void getAllMrlData(const meteo::surf::DataRequest* req,
                       meteo::surf::ManyMrlValueReply* res);
    bool getAvailableRadar( const meteo::surf::DataRequest* req,
                            meteo::surf::DataDescResponse* res );
 private :
   bool toProto(int descr, const TMeteoData &md, meteo::surf::OneMrlValueReply* res);

  void setMeteoDataData(meteo::surf::OneMrlValueReply* r_par) ;
};

}
}

#endif
