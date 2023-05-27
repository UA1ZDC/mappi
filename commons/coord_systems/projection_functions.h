# ifndef PROJECTION_FUNCTIONS_H
# define PROJECTION_FUNCTIONS_H



#include <stdio.h>
#include <math.h>
#include <commons/mathtools/mathematical_constants.h>



namespace meteo {

double clamp(double, double, double);

inline double toRad(double angle){return angle*M_PI/180.;}
inline double toGrad(double angle){return angle*180./M_PI;}

double func_ellipsE(double, double);

double func_ellipsN(double,double,double);

double func_t1(double,double,double,double);
double func_t2(double,double);
double func_t3(double,double);
double func_t4(double,double,double);
double func_t5(double,double,double,double);
double func_tau(double,double);
double func_sinzcosa(double,double,double,double,double,double);
double func_sinzsina(double,double,double,double,double,double);
double func_cosz(double,double,double,double,double,double);



double calcAzimooth_ellips(double, double, double, double, double, double);
double calcAzimooth_sphere(double, double, double, double);

void FLtoXY_ellips(double , double , double ,double , double , double , double& , double& , double &);
void FLtoXY_sphere(double , double ,double , double , double , double& , double& , double &);
void FLHtoXYZ_sphere(double , double, double, double, double, double, double, double&, double&, double&);
void AEDtoFLH_sphere(double, double,double, double, double, double, double&, double&, double&);
void XYZtoAED_sphere(double, double, double, double, double&, double&, double&);



//
//int test();

};

# endif
