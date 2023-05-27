#ifndef RADIOPHYSICS_H
#define RADIOPHYSICS_H

#include <math.h>
#include <stdio.h>

#include "mathematical_constants.h"
#include "physical_constants.h"
#include "atmosphysics.h"

namespace MnMath
{

 struct PlainRectCoords
 {
  double x;
  double y;
  PlainRectCoords()
  { 
   x=0.;
   y=0.;
  }
 };

 struct PlainAtmosElectroParams
 {
  double Epsilon;
  double Sigma;
  double gradEps;
  double gradSigma;
  PlainAtmosElectroParams()
  { 
   Epsilon =   0.;
   Sigma =     0.;
   gradEps =   0.;
   gradSigma = 0.;
  }

 };

 struct WavesParams
 {
  double frequency;
  double electrAmplitude;
  double magnAmplitude;
  double energFluxe;
  double energy;
  double direction;
  WavesParams()
  { 
   frequency=0;
   electrAmplitude=0;
   magnAmplitude=0;
   energFluxe=0;
   energy=0;
  }
 };

 double permittivityIonosphere(double frequency, const IonosphereParams& params);
 double conductivityIonosphere(double frequency, const IonosphereParams& params);
 double refractionIndexIonosphere(double frequency, const IonosphereParams& params);

 double refractionIndexTroposphere(const NeutrosphereParams& params);
 double permittivityTroposphere(const NeutrosphereParams& params);

 
 double energyFluxeFromElectrIntensity(double Em,double Epsilon, double Mu, bool &ok); //W/m2
 double energyFluxeFromMagnIntensity(double Hm, double Epsilon, double Mu, bool &ok); //W/m2
 double waveResistance(double Epsilon, double Mu, bool &ok);
 double powerLevel(double P0, double P1); //dB
 double intensityLevel(double Em0, double Em1); //dB
 double waveSpeed(double Epsilon, double Mu, bool& ok);
 double refractionIndice(double Epsilon, double Mu, bool& ok);
 double electrIntencityFromEnergyFluxe(double energyFlux, double Epsilon, double Mu, bool& ok); //V/m
 double magnIntencityFromEnergyFluxe(double energyFlux, double Epsilon, double Mu, bool& ok); //A/m
 double energyFluxFromEmittedPower(double emitPower, double R); //W/m2
 double emittedPowerFromTrasmitPower(double transmitPower, double anntennAmplif, double antennKPD);

 ////////IONOSPHERE :  Mu=1//////////////////////////////////////////////////////
 double electrIntensityAmplitudeForLinUniIonosphere(double frequency, double Em0, double teta, double l, double eps, double sigm, double gradEps, double gradSigm, bool&ok ); //V/m
 double electrIntensityAmplitudeUniIonosphere(double frequency, double Em0,  double l, double eps, double sigm, bool& ok); //V/m 

 double groupVelocityUniIonosphere(double refractionIndex);//sm/sec
 double groupDelayUniIonosphere(double refractionIndex, double path);
 double averageGroupDelayLinUniIonosphere(double refractionIndex1, double refractionIndex2, double path); 

 ////////TROPOSPHERE: Sigma = 0; Mu = 1; 
 //Van Fleck formulas of absorbtion in troposphere: [P]=hPa, [T]=K, [partialPressure]=hPa (Bin, Datton, Radiomoteorology)
 double troposphereOxigenVapourAbsorbtionIndex(double frequency, const NeutrosphereParams& params); //dB/km 

 double phaseVelocityUniMedium(double refractionIndex);//sm/sec
 double phaseDelayUniMedium(double refractionIndex, double path);
 double averagePhaseDelayLinUniMedium(double refractionIndex1, double refractionIndex2, double path); 

 double phaseLinUniIonosphere(double frequency, double teta,  double l, const IonosphereParams& params0, const IonosphereParams& params1, bool& ok); //CGSE /Rad

 double groupDelayLinUniIonosphere(double frequency, double teta,  double l, const IonosphereParams& params0, const IonosphereParams& params1, bool& ok); //CGSE 
 double phaseDelayLinUniIonosphere(double frequency, double teta,  double l, const IonosphereParams& params0, const IonosphereParams& params1, bool& ok); //CGSE 
}

#endif

