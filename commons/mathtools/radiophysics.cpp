#include "radiophysics.h"

#include <stdio.h>

//////IONOSPHERE
/*!
 * \brief Расчет диэлектрической проницаемости ионосферы
 * \param frequency - частота сигнала, Гц
 * \param params - параметры ионосферы
 */
double MnMath::permittivityIonosphere(double frequency, const IonosphereParams& params)
{
 double omega = 2.*M_PI*frequency;
 double localConst = 4*M_PI*CGS_ELECTRON_CHARGE*CGS_ELECTRON_CHARGE/CGS_ELECTRON_MASS;
 double permitElec = 1. - localConst*params.electronDensity/(omega*omega+params.electronCollisionFrequency*params.electronCollisionFrequency); 
 double permitIons = 0.;
 for(int i = 0;i < params.ionTypesCount;i++ ){
  permitIons += 4*M_PI*CGS_ELECTRON_CHARGE*CGS_ELECTRON_CHARGE*params.ionDensity[i]/(params.ionMass[i]*omega*omega+params.ionMass[i]*params.ionCollisionFrequency[i]*params.ionCollisionFrequency[i]);
 }
 return permitElec-permitIons;
}

/*!
 * \brief Расчет удельной электропроводности ионосферы
 * \param frequency - частота сигнала, Гц
 * \param params - параметры ионосферы
 */
double MnMath::conductivityIonosphere(double frequency, const IonosphereParams& params)
{
 double omega = 2.*M_PI*frequency; 
 double localConst = CGS_ELECTRON_CHARGE*CGS_ELECTRON_CHARGE/CGS_ELECTRON_MASS;
 double conductElec = localConst*params.electronDensity*params.electronCollisionFrequency/(omega*omega+params.electronCollisionFrequency*params.electronCollisionFrequency);
 double conductIons = 0;
 for(int i = 0;i < params.ionTypesCount;i++ ){
  conductIons += CGS_ELECTRON_CHARGE*CGS_ELECTRON_CHARGE*params.ionDensity[i]*params.ionCollisionFrequency[i]/(params.ionMass[i]*omega*omega+params.ionMass[i]*params.ionCollisionFrequency[i]*params.ionCollisionFrequency[i]);
 }
 return conductElec+conductIons;
}

double MnMath::refractionIndexIonosphere(double frequency, const IonosphereParams& params)
{
 double permittivity = permittivityIonosphere(frequency, params);
 return permittivity*permittivity;
}

//Debay formulas of refraction index: [P]=hPa, [T]=K, [e]=hPa
double MnMath::refractionIndexTroposphere(const NeutrosphereParams& params)
{
 double N = 77.6*(params.wetAirPressure - 4810*params.wetPartialPressure/params.temperature)/params.temperature;
 return N*1E-6+1;
}

/*!Расчет диэлектрической проницаемости нейтральной атмосферы
 */
double MnMath::permittivityTroposphere(const NeutrosphereParams& params)
{
 return sqrt(refractionIndexTroposphere(params));
}




double MnMath::energyFluxeFromElectrIntensity(double Em,double Epsilon, double Mu, bool &ok ) //W/m2
{
 return Em*Em/waveResistance(Epsilon,Mu,ok);
}

double MnMath::energyFluxeFromMagnIntensity(double Hm, double Epsilon, double Mu, bool &ok) //W/m2
{
 return Hm*Hm*waveResistance(Epsilon,Mu,ok);
}

double MnMath::waveResistance(double Epsilon, double Mu, bool &ok)
{
 ok = (Epsilon>0)? true:false;
 return (ok)? sqrt(Mu/Epsilon)*VACUUM_WAVE_RESIST: 1 ;
}

double MnMath::powerLevel(double P0, double P1) //dB
{
 if(P1==0) return 0;
 if(P0>0&&P1>=0) return 10*log10(P1/P0);
 else  printf("ERROR: incorrect params in powerLevel()\n");
 return 0.;
}

double MnMath::intensityLevel(double Em0, double Em1) //dB
{
 if(Em1==0) return 0;
 if(Em0>0&&Em1>0) return 20*log10(Em1/Em0);
 else  printf("ERROR: incorrect params in intensityLevel()\n");
 return 0.;
}

double MnMath::waveSpeed(double Epsilon, double Mu, bool& ok)
{
 return LIGHT_SPEED/refractionIndice(Epsilon,Mu,ok);
}

double MnMath::refractionIndice(double Epsilon, double Mu, bool& ok)
{
 ok = (Epsilon>0)? true:false;
 return  (ok)? sqrt(Epsilon*Mu): 1;
}

double MnMath::electrIntencityFromEnergyFluxe(double energyFlux, double Epsilon, double Mu, bool& ok) //V/m
{
 return sqrt(energyFlux*VACUUM_WAVE_RESIST*Mu/refractionIndice(Epsilon,Mu,ok));
}

double MnMath::magnIntencityFromEnergyFluxe(double energyFlux, double Epsilon, double Mu, bool& ok) //A/m
{
 return sqrt(energyFlux*refractionIndice(Epsilon,Mu,ok)/Mu*VACUUM_WAVE_RESIST);
}

double MnMath::energyFluxFromEmittedPower(double emitPower, double R) //W/m2
{
 return (R>0)? emitPower/(4*M_PI*R*R):0;
}

double MnMath::emittedPowerFromTrasmitPower(double transmitPower, double antennaAmplif, double antennaKPD)
{
 return transmitPower*antennaAmplif*antennaKPD;
}

////////IONOSPHERE :  Mu=1//////////////////////////////////////////////////////
double MnMath::electrIntensityAmplitudeForLinUniIonosphere(double frequency, double Em0, double teta,  double l, double eps, double sigm, double gradEps, double gradSigm, bool&ok ) //V/m
{
 // l - wave penetration path
 double z1=-eps/gradEps;         //reflection height free absorbtion
 double omega = 2.*M_PI*frequency;  //angular frequency
 double alpha = 4.*M_PI*sigm/omega;
 double beta = 4.*M_PI*gradSigm*z1/omega;
 double cosTeta2 = cos(teta)*cos(teta);  //square of wave penetration angle with gradient (axis z) of permittivity;
 double sinTet = sin(teta);
 double x = l*sinTet;
 double z = l*cos(teta);
 

//printf("alpha = %e\n",alpha);
//printf("beta = %e\n", beta);
//printf("ab = %e\n",1+alpha*beta*z/z1);
//printf("cosTet = %e\n", cosTeta2);

 double zr = z1*(eps*eps-alpha*beta)/(eps*eps+beta*beta)*cosTeta2; //reflection height
 double fi = atan(beta/eps);
 double gamma = atan(alpha/eps);
 double ro1 = sqrt(eps*eps+beta*beta);
 double ro2 = sqrt(eps*eps+alpha*alpha);
 double ro3 = sqrt(z1*z1*ro2*ro2/(ro1*ro1)*cosTeta2*cosTeta2-2*z*z1/(ro1*ro1)*(eps*eps-alpha*beta)*cosTeta2+z*z);
 double R = 2.*omega*sqrt(ro3*ro3*ro3*ro1/z1)/(3.*CGS_LIGHT_SPEED);
// double nu = 0.5*fi - 1.5*atan(z1*(alpha+beta)/(z1*(1-alpha*beta)-z*ro1*ro1));
 double nu = -fi -  1.5*atan((z1*alpha*cosTeta2+beta*z)/(eps*(z1*cosTeta2-z)));
 double nu0 = -0.5*(3.*gamma+2.*fi);
 double R0 = 2.*omega*z1*sqrt(ro2*ro2*ro2)/(3.*CGS_LIGHT_SPEED*ro1);
 double Hi = omega/CGS_LIGHT_SPEED*sqrt(ro2)*sin(teta)*sin(0.5*gamma); // absorbtion coefficient for direction x (where ionosphere is uniform)

// printf("nu=%e\n", nu);
// printf("R0=%e\n", R0);
// printf("ro3=%e\n", ro3);
// printf("Hi=%e\n", Hi);
// printf("Arg=%e\n", R0*sin(nu0)-R*sin(nu)-Hi*x);

 printf("zr=%e\n", zr);
 printf("z1=%e\n", z1);
 printf("z=%e\n", z);
 printf("x=%e\n", x);
 printf("teta=%e\n", teta*180./M_PI);

 ok = (zr>z || zr<0)? true:false; //Reflection conditions
 
 return (ok)? Em0*pow(R0/R,1/6)*exp(R0*sin(nu0)-R*sin(nu)-Hi*x):0; 
}

double MnMath::electrIntensityAmplitudeUniIonosphere(double frequency, double Em0, double l, double eps, double sigm, bool& ok) //V/m
{
 //l - path;
 double omega = 2.*M_PI*frequency;
 double hi = sqrt(-eps/2+sqrt(eps*eps/4+4*M_PI*M_PI*sigm*sigm/(omega*omega))); //absorbtion coefficient for penetration direction l (ionosphere is uniform)
 ok = (eps>0)? true:false; // reflection conditions
 return (ok)? Em0*exp(-omega*hi/CGS_LIGHT_SPEED*l):0;
}

//Van Fleck formulas of absorbtion in troposphere: [P]=hPa, [T]=K, [partialPressure]=hPa (Bin, Datton, Radiomoteorology)
double MnMath::troposphereOxigenVapourAbsorbtionIndex(double frequency, const NeutrosphereParams& params) //dB/km
{
 double absMoisture = absolutMoisture(params.wetPartialPressure, params.temperature); //g/m3
 double waveLength = CGS_LIGHT_SPEED/frequency;//sm
 double nu1 = 0.018*(params.wetAirPressure/1013.25)*pow(293./params.temperature,3./4.);// cm-1
 double nu2 = 0.049*(params.wetAirPressure/1013.25)*pow(300./params.temperature,3./4.);// cm-1;
 double nu3 = 0.087*(params.wetAirPressure/1013.25)*sqrt(318./params.temperature)*(1+0.0046*absMoisture);
 double nu4 = 0.087*(params.wetAirPressure/1013.25)*sqrt(318./params.temperature)*(1+0.0046*absMoisture);
 double absorbOxigen = 0.34/pow(waveLength,2.)*(params.wetAirPressure/1013.25)*pow(293./params.temperature,2.)*(nu1/(1./(waveLength*waveLength)+nu1*nu1)+nu2/(pow(2.+1./waveLength,2.)+nu2*nu2)+nu2/(pow(2.-1./waveLength,2.)+nu2*nu2));
 double absorbVapour1 = 0.0318/pow(waveLength,2.)*pow(293./params.temperature,3./2.)*exp(-644./params.temperature)*(nu3/(pow(1./waveLength-1./1.35,2.)+nu3*nu3)+nu3/(pow(1./waveLength+1./1.35,2.)+nu3*nu3))*absMoisture;
 double absorbVapour2 = 0.05/pow(waveLength,2.)*(293./params.temperature)*nu4*absMoisture;
 return absorbOxigen + absorbVapour1 + absorbVapour2;
}

double MnMath::phaseVelocityUniMedium(double refractionIndex)
{
 return CGS_LIGHT_SPEED*refractionIndex;
}

double MnMath::groupVelocityUniIonosphere(double refractionIndex)
{
 return CGS_LIGHT_SPEED/refractionIndex;
}

double MnMath::phaseDelayUniMedium(double refractionIndex, double path)
{
 return path/phaseVelocityUniMedium(refractionIndex);
}

double MnMath::groupDelayUniIonosphere(double refractionIndex, double path)
{
 return path/groupVelocityUniIonosphere(refractionIndex);
}

double MnMath::averagePhaseDelayLinUniMedium(double refractionIndex1, double refractionIndex2, double path)
{
  return 2.*path/(phaseVelocityUniMedium(refractionIndex1)+phaseVelocityUniMedium(refractionIndex2));
}

double MnMath::averageGroupDelayLinUniIonosphere(double refractionIndex1, double refractionIndex2, double path)
{
  return 2.*path/(groupVelocityUniIonosphere(refractionIndex1)+groupVelocityUniIonosphere(refractionIndex2));
}




double MnMath::phaseLinUniIonosphere(double frequency, double teta,  double l, const IonosphereParams& params0, const IonosphereParams& params1, bool& ok) //CGSE //Rad
{
  double cosTeta2 = cos(teta)*cos(teta);  //square of wave penetration angle with gradient (axis z) of permittivity;
  double sinTet = sin(teta);
  double x = l*sinTet;
  double z = l*cos(teta);

  double eps0 = permittivityIonosphere(frequency, params0); //CGSE
  double eps1 = permittivityIonosphere(frequency, params1); //CGSE
  double sigm0 = conductivityIonosphere(frequency, params0); //CGSE
  double sigm1 = conductivityIonosphere(frequency, params1); //CGSE
  double gradEps = (z != 0.)?  (eps1-eps0)/z:0.; //cm-1
  double gradSigm = (z != 0.)?  (sigm1-sigm0)/z:0.; //с-1/cm-1

 
   // l - wave penetration path
  double z1=-eps0/gradEps;         //reflection height free absorbtion
  double omega = 2.*M_PI*frequency;  //angular frequency
  double alpha = 4.*M_PI*sigm0/omega;
  double beta = 4.*M_PI*gradSigm*z1/omega;
 

 //printf("alpha = %e\n",alpha);
 //printf("beta = %e\n", beta);
//printf("ab = %e\n",1+alpha*beta*z/z1);
//printf("cosTet = %e\n", cosTeta2);

  double zr = z1*(eps0*eps0-alpha*beta)/(eps0*eps0+beta*beta)*cosTeta2; //reflection height
  double fi = atan(beta/eps0);
  double gamma = atan(alpha/eps0);
  double ro1 = sqrt(eps0*eps0+beta*beta);
  double ro2 = sqrt(eps0*eps0+alpha*alpha);
  double ro3 = sqrt(z1*z1*ro2*ro2/(ro1*ro1)*cosTeta2*cosTeta2-2*z*z1/(ro1*ro1)*(eps0*eps0-alpha*beta)*cosTeta2+z*z);
  double R = 2*omega*sqrt(ro3*ro3*ro3*ro1/z1)/(3*CGS_LIGHT_SPEED);
// double nu = 0.5*fi - 1.5*atan(z1*(alpha+beta)/(z1*(1-alpha*beta)-z*ro1*ro1));
  double nu = -fi -  1.5*atan((z1*alpha*cosTeta2+beta*z)/(eps0*(z1*cosTeta2-z)));
  double k = omega/CGS_LIGHT_SPEED*sqrt(ro2)*sin(teta)*cos(0.5*gamma); // absorbtion coefficient for direction x (where ionosphere is uniform)

// printf("nu=%e\n", nu);
// printf("R0=%e\n", R0);
// printf("ro3=%e\n", ro3);
// printf("Hi=%e\n", Hi);
// printf("Arg=%e\n", R0*sin(nu0)-R*sin(nu)-Hi*x);

  printf("zr=%e\n", zr);
  printf("z1=%e\n", z1);
  printf("z=%e\n", z);
  printf("x=%e\n", x);
  printf("teta=%e\n", teta*180./M_PI); 

  ok = (zr>z || zr<0.)? true:false; //Reflection conditions
  return (ok)? R*cos(nu)-nu/6.-k*x:0.; 
}



double MnMath::groupDelayLinUniIonosphere(double frequency, double teta,  double l, const IonosphereParams& params0, const IonosphereParams& params1, bool& ok) //CGSE 
{
 double dFrequency = 100.; //Hz
 double phase1 = phaseLinUniIonosphere(frequency, teta, l, params0, params1, ok); //CGSE //Rad
 double phase2 = phaseLinUniIonosphere(frequency+dFrequency, teta,  l, params0, params1, ok); //CGSE //Rad
 double dPhase = phase2-phase1;
 printf("dPhase =%f\n",dPhase);
 double dOmega = 2.*M_PI*dFrequency;
 return (ok)? dPhase/dOmega:0.;
}

double MnMath::phaseDelayLinUniIonosphere(double frequency, double teta,  double l, const IonosphereParams& params0, const IonosphereParams& params1, bool& ok) //CGSE //Rad
{
 double phase = phaseLinUniIonosphere(frequency, teta, l, params0, params1, ok); //CGSE //Rad
 double omega = 2.*M_PI*frequency;
 return (ok)? phase/omega:0.;
}


