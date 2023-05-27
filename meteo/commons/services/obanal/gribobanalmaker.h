#ifndef METEO_C_SERVICES_OBANAL_GRIBOBANAL_H
#define METEO_C_SERVICES_OBANAL_GRIBOBANAL_H

#include <qobject.h>
#include <meteo/commons/services/obanal/tobanalservice.h>
#include <meteo/commons/rpc/rpc.h>

#define AUTOOBANAL_FINISHED           -11
#define AUTOOBANAL_CONNECTION_REFUSED -10
#define AUTOOBANAL_NOREPLY            - 9
#define AUTOOBANAL_NODATA             - 8
#define AUTOOBANAL_ERROR              - 7
#define AUTOOBANAL_NOINFO             - 2


class GribObanalMaker
{
public:
  GribObanalMaker();
  ~GribObanalMaker();
  int makeAutoGribObanal( meteo::TObanalService* service );
  int makeCustomObanal(meteo::TObanalService* service , const meteo::surf::GribDataRequest &request);
private:
  void init();

private:
  bool need_calc_forecast_accuracy_;
  meteo::rpc::Channel* ctrl_;
};

#endif
