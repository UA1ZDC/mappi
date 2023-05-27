#include "temp_func.h"

#include <cmath>

const double cTMELT  = 273.16;
const double cR3LES  = 17.502;
const double cR3IES  = 22.587;
const double cR4LES  = 32.19;
const double cR4IES  = -0.7;
const double cRD     = 287.05;
//specific heat of air on constant volume
const double cRV     = 461.51;
//specific heat of air on constant pressure
const double cRP     = 1005.46;
const double cKAPPA  = cRD/cRP;
const double cEP     = cRD/cRV;
const double cCI     = 2710.;
const double cRESTT  = 611.21;
const double cR2ES   = cRESTT * cRD / cRV;
const double cVTMPC1 = cRV / cRD - 1.;
const double RAD     = 180. / (2. * asin(1));

const double cEPSILON = 0.621981;


// в коде используется flag, определяется так:
//yyyymmdd.f, f=fraction of hour
//bool flag = (yyyymmdd.f <  19830421.);
//везде по умолчанию false


// t temperature in K
// p pressure in Pa
// q specific humidity in kg/kg
double meteo::equivalentPotentialTemperature(double t, double p, double q, bool flag /*= false*/)
{
  double pSat, tSat;
  if (saturationLevel(t, p, q, &tSat, &pSat, flag)) {
    double qSat = saturationSpecHumidity(tSat, pSat, flag);
    return tSat * pow(100000./pSat, cKAPPA)*exp(qSat*cCI/tSat);
  }
  
  return -1.;
}


// t temperature in K
// p pressure in Pa
double meteo::potentialTemperature(double t, double p)
{
    return t*pow(100000./p, cKAPPA);
}


// As used in Pott.cc
// t temperature in K
// p pressure in Pa
double meteo::saturatedEquivalentPotentialTemperature(double t, double p, bool flag /*=false*/)
{
    double qSat = saturationSpecHumidity(t, p, flag);
    return t * pow(100000./p, cKAPPA)*exp(qSat*cCI/t);
}


// As used in Pott.cc
// t temperature in K
// p pressure in Pa
// pTerm precomputed potentialTemperaturePressureTerm
double meteo::saturatedEquivalentPotentialTemperatureWithPTerm(double t, double p, double pTerm, bool flag /*=false*/)
{
    double qSat = saturationSpecHumidity(t, p, flag);
    return t * pTerm*exp(qSat*cCI/t);
}

// t temperature in K
// p pressure in Pa
// q specific humidity in kg/kg
bool meteo::saturationLevel(double t, double p, double q, double *tSat, double *pSat, bool flag /*= false*/)
{
    int count = 0;
    const int maxIter = 20;

    if (q < 0.000002 && p > 10000.)
        q=0.000002;

    double pSatLimit=p*0.2;
    *pSat = p*0.98;
    *tSat = t;

    while ( count < maxIter ) {
        *tSat = t*pow(*pSat/p, cKAPPA) ;
        double qSat = saturationSpecHumidity(*tSat, *pSat, flag);
        double tSatTop = t*pow((*pSat-100.)/p, cKAPPA);
        double qSatTop = saturationSpecHumidity(tSatTop, (*pSat-100.),flag);
        double ratio =(qSat-q)/(qSat-qSatTop);
        //dp = p - pSat;
        if (fabs(ratio) < 1.) {
            return true;
        }
        *pSat -= ratio*100;
        if (*pSat < pSatLimit) {
            *pSat += ratio*100;
            return true;
        }

        count++;
    }

    //error
    return false;
}

// t temperature in K
// p pressure in Pa
double meteo::saturationSpecHumidity(double t, double p, bool flag /*= false*/)
{
    double A = 0.;
    double B = 0.;

    if (t >= cTMELT || flag) {
        A = 17.269;
        B = 35.86;
    } else {
        A = 21.875;
        B = 7.66;
    }

    double v = 610.78*exp(A*(t-cTMELT)/(t-B));
    return (flag)?
                (cEP*v/p):
                ((cEP*v)/(p - (1.-cEP)*v));
}



float meteo::relToSpecHumidity(float relHum, double p, bool flag /*= false*/)
{
  return relHum * saturationSpecHumidity(relHum, p, flag) * 0.01;  
}
