#ifndef METEO_COMMONS_SERVICES_OBANAL_TFP_TEMP_FUNC_H
#define METEO_COMMONS_SERVICES_OBANAL_TFP_TEMP_FUNC_H

namespace meteo {
  double potentialTemperature(double t, double p);
  double equivalentPotentialTemperature(double t, double p, double q, bool flag = false);
  double saturatedEquivalentPotentialTemperature(double t, double p, bool flag = false);
  double saturatedEquivalentPotentialTemperatureWithPTerm(double t, double p, double pTerm, bool flag = false);
  
  bool   saturationLevel(double t, double p, double q, double *tSat, double *pSat, bool flag = false);
  double saturationSpecHumidity(double t, double p, bool flag = false);
  float  relToSpecHumidity(float relHum, double p, bool flag = false);
};


#endif
