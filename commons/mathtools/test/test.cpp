#include <stdio.h>
//#include <commons/mathtools/mathematical_constants.h>
#include <commons/mathtools/mnmath.h>
#include <radiophysics.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

using namespace MnMath;

void testAbsorbtion()
{
bool ok1,ok2;
double E1,E2, dB1, dB2;
double Em0=1.0; //V/m
double frequency = 160E+6; //Hz
double teta = 0./180.*M_PI;
double l = 100000.*50.; //cm


//double x = l*cos(teta);
//double z = l*sin(teta);


IonosphereParams params, params1;

double ionDensityArray[] = {0., 0., 0., 0.};
double ionCollisionFrequencyArray[] = {0., 0., 0., 0.};
double ionMassArray[] = {1., 1., 1., 1.};

params.electronDensity = 1E+3; //cm-3
params.electronCollisionFrequency = 1E+7; // Hz (1E+8 -- 1E+4)
params.ionDensity = ionDensityArray; //cm-3
params.ionCollisionFrequency = ionCollisionFrequencyArray; //Hz
params.ionMass = ionMassArray; //g
//params.ionTypesCount = 4;

params1.electronDensity = 1E+6; //cm-3
params1.electronCollisionFrequency = 1E+4; // Hz (1E+8 -- 1E+4)
params1.ionDensity = ionDensityArray; //cm-3
params1.ionCollisionFrequency = ionCollisionFrequencyArray; //Hz
params1.ionMass = ionMassArray; //g
//params1.ionTypesCount = 4;


double eps = permittivityIonosphere(frequency, params); //CGSE
double eps1 = permittivityIonosphere(frequency, params1); //CGSE
double sigm = conductivityIonosphere(frequency, params); //CGSE
double sigm1 = conductivityIonosphere(frequency, params1); //CGSE
double gradEps = (eps1-eps)/5000000; //cm-1
double gradSigm =  (sigm1-sigm)/5000000; //с-1/cm-1


printf("epsilon=%e\n", eps);
printf("sigma=%e c-1\n", sigm);
printf("epsilon_1=%e\n", eps1);
printf("sigma_1=%e c-1\n", sigm1);

printf("gradEps=%e cm-1\n", gradEps);
printf("gradSigm=%e c-1/cm-1 \n", gradSigm);



E1 = electrIntensityAmplitudeForLinUniIonosphere(frequency, Em0, teta, l, eps, sigm, gradEps, gradSigm, ok1 ); //V/m
E2 = electrIntensityAmplitudeUniIonosphere(frequency, Em0, l, eps, sigm, ok2); //V/m

dB1 = intensityLevel(Em0,E1);
dB2 = intensityLevel(Em0,E2);


 if(ok1)
  { 
    printf("E1=%e\n", E1);
    printf("Absorbtion1=%e\n", dB1);
  }
 if(ok2) 
  {
    printf("E2=%e\n", E2);
    printf("Absorbtion2=%e\n", dB2);
  }
 return ;

 
}


void testDelay()
{
 double frequency = 160E+6; //Hz
 double teta = 0./180.*M_PI;
 double l = 100000.*250.; //cm
 bool ok;

IonosphereParams params, params1;

double ionDensityArray[] = {0., 0., 0., 0.};
double ionCollisionFrequencyArray[] = {0., 0., 0., 0.};
double ionMassArray[] = {1., 1., 1., 1.};

params.electronDensity = 1E+3; //cm-3
params.electronCollisionFrequency = 1E+7; // Hz (1E+8 -- 1E+4)
params.ionDensity = ionDensityArray; //cm-3
params.ionCollisionFrequency = ionCollisionFrequencyArray; //Hz
params.ionMass = ionMassArray; //g
//params.ionTypesCount = 4;

params1.electronDensity = 1E+6; //cm-3
params1.electronCollisionFrequency = 1E+4; // Hz (1E+8 -- 1E+4)
params1.ionDensity = ionDensityArray; //cm-3
params1.ionCollisionFrequency = ionCollisionFrequencyArray; //Hz
params1.ionMass = ionMassArray; //g
//params1.ionTypesCount = 4;




 double eps = permittivityIonosphere(frequency, params); //CGSE
 double eps1 = permittivityIonosphere(frequency, params1); //CGSE
 // double sigm = conductivityIonosphere(frequency, params); //CGSE
 // double sigm1 = conductivityIonosphere(frequency, params1); //CGSE

 double groupDelay = averageGroupDelayLinUniIonosphere(eps*eps, eps1*eps1, l);

 printf("groupDelay = %5.3f msec\n", groupDelay*1E+3);

 double groupDelayPrecise = groupDelayLinUniIonosphere(frequency, teta,  l, params, params1, ok); //CGSE //sec
 double phaseDelayPrecise = phaseDelayLinUniIonosphere(frequency, teta,  l, params, params1, ok); //CGSE //sec

 printf("groupDelayPrecise = %5.3f msec\n", groupDelayPrecise*1E+3);
 printf("phaseDelayPrecise = %5.3f msec\n", phaseDelayPrecise*1E+3);

 return;
}

void testUnlinearEquations()
{
 double x1, x2, x3, x4;
 bool ok1,ok2;

//unlinear equations 
 if(squareEquation(2.,3.,1.,x1,x2)) printf("squareEquation: x1 = %e , x2 = %e\n", x1, x2);
 if(cubeEquation(1.,0.,-4.,0.,x1,x2,x3)) printf("cubeEquation: x1 = %e , x2 = %e, x3 = %e\n", x1, x2, x3);
 else printf("cubeEquation: x1 = %e , x2 = Im, x3 = Im\n", x1);
 fourPowerEquation(1.,100.,2.,2.,-1., x1,x2,x3,x4, ok1, ok2); // 1, 3, 3, -1 ,-6  real roots: 1, -2   
 if(ok1) printf("fourPowerEquation: x1 = %e, x2 = %e\n", x1, x2);
 if(ok2) printf("fourPowerEquation: x3 = %e, x4 = %e\n", x3, x4);
 if(ok1) printf("fourPowerEquation control = %e\n", 1.*x1*x1*x1*x1+100.*x1*x1*x1+2.*x1*x1+2.*x1-1.);
 if(ok2) printf("fourPowerEquation control = %e\n", 1.*x3*x3*x3*x3+100.*x3*x3*x3+2.*x3*x3+2.*x3-1.);
}

void psihr1(float T, float U, float* D)
{
  float vs = log(0.01*U)/17.5043 + T/(241.2+T);
  float Td = 241.2 * (vs) / (1-vs);
  *D = T - Td;
}

void testHumidity()
{
  float T[10] = {-0.5, -0.5, -0.5, 21, 21, 0.6, 0.6, 0.6, 21.1, 21.1};
  float U[10] = {50, 48, 46, 35, 24, 50, 49, 47, 35, 25};
  float Td[10]={-9.65006, -10.1673, -10.7041, 4.95169, -0.346333, -8.63228, -8.89081, -9.42227, 5.03983, 0.30064};

  float D, D1;
  float U_res;

  for (int i=0; i<10; i++) {
    U2D(T[i], U[i], &D);
    psihr1(T[i], U[i], &D1);
    U_res = Td2U(T[i], Td[i]);
    debug_log << T[i] << U[i] << D << D1 << fabs(D-D1) << U_res << fabs(U_res - U[i]);
  }

}


int main()
{ 
  TAPPLICATION_NAME( "test" );
  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale());

  testAbsorbtion();
  
  testDelay();
 
  testUnlinearEquations();

  testHumidity();

 return 0;
}
