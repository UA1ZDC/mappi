#ifndef ATMOSPHYSICS_H
#define ATMOSPHYSICS_H

#include <math.h>
#include "physical_constants.h"
#include "mathematical_constants.h"




namespace MnMath
{

 struct IonosphereParams
 {
   double electronDensity;
   double  electronCollisionFrequency;
   const int ionTypesCount;   
   double* ionDensity;
   double* ionCollisionFrequency;
   double* ionMass;
   
   double permittivity;
   double conductivity;
  
   
   IonosphereParams(double elDens = 0., double elCollFreq = 0., int ionTCount =0):
                 electronDensity(elDens),
                 electronCollisionFrequency(elCollFreq),
		 ionTypesCount(ionTCount),
                 permittivity(0.),
                 conductivity(0.)
		 {
		   if(ionTCount == 0) return;
		   ionMass = new double[ionTCount]; 
		   ionDensity = new double[ionTCount];
		   ionCollisionFrequency = new double[ionTCount];
                 }
  ~IonosphereParams(){
   if(ionTypesCount == 0) return;
   delete []ionMass;
   delete []ionDensity;
   delete []ionCollisionFrequency;
  }
 };

 struct NeutrosphereParams
 {
 // [P]=hPa, [T]=K, [wetPartialPressure]=hPa (Bin, Datton, Radiomoteorology)
  double wetAirPressure;
  double temperature;
  double wetPartialPressure;

  double permittivity;
  double absorbtion;

  NeutrosphereParams(double wetAPress = 0., double temp = 0., double wetPartPress = 0.):wetAirPressure(wetAPress),
                                                                                        temperature(temp),
                                                                                        wetPartialPressure(wetPartPress),
											permittivity(0.),
											absorbtion(0.){} 
 };



double absolutMoisture(double wetPartialPressure, double temperature);
double wetPartialPressure1(double wetMassFraction, double wetAirPressure);
double wetPartialPressure2(double wetDensity, double temperature);
double wetDensity(double dryAirDensity, double wetMassFraction);
double dryAirPressure(double dryAirDensity, double temperature);
double wetAirPressure(double dryAirDensity, double temperature, double wetMassFraction);
double wetAirPressure(double dryAirPressure, double wetMassFraction);
 

 double f0ToNm(double f0);
 double NmTof0(double Nm);

 double WtoF107(double W);

 double average2ParticlesRelateVelocity(double particleMass1, double particleMass2, double temperature); 
 double averageParticleVelocity(double particleMass, double temperature);
 double averageParticleKineticEnergy(double temperature);
 double twoUnchargedParticlesCollisionFrequency(double particleRadius1, double particleRadius2, double particleMass1, double particleMass2, double temperature);
 
 double electronIonCollisionFrequency(double electronDensity, double electronTemperature); //if elecronTemp == ionTemp
 double electronIonCollisionFrequency(double electronDensity, double electronTemperature, double ionTemperature);  //if elecronTemp != ionTemp
 double electronIonCollisionFrequencyFast(double electronDensity, double electronTemperature); //if elecronTemp == ionTemp, not precise, but fast
 double electronO2CollisionFrequency(double neutralO2Density, double electronTemperature);
 double electronN2CollisionFrequency(double neutralN2Density, double electronTemperature);
 double electronOCollisionFrequency(double neutralODensity, double electronTemperature);
 double electronHCollisionFrequency(double neutralHDensity, double electronTemperature);
 double electronHeCollisionFrequency(double neutralHeDensity, double electronTemperature);
 double electronAnyNeutralCollisionFrequency(double anyNeutralRadius, double anyNeutralDensity, double electronTemperature);
 double electronNeutralCollisionFrequency(const double* collisionFrequencies, int countNeutrals);
 double electronCollisionFrequency(const double* collisionFrequencies, int countNeutrals, double electronIonCollisionFrequency);
 double ionElectronCollisionFrequency(double ionDensity, double electronDensity, double electronIonCollisionFrequency); 
 double ionIonCollisionFrequency(double ionMass, double ionsDensity, double ionTemperature);
 double ionNeutralCollisionFrequency(double ionMass, double neutralsDensity, double ionTemperature); //Average Frequency if ionMass == neutralMass, ionRadius = neutralRadius = averageParticleRadius 1.2E-8 cm
 double ionCollisionFrequency(double ionMass, double ionDensity, double neutralsDensity, double electronsDensity, double ionTemperature, double electronTemperature); //if neutralsDensity ==ionsDensity
 
}

#endif
