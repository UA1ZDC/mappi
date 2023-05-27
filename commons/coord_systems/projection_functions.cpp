// projection_functions.cpp : Defines the entry point for the console application.
//


#include "projection_functions.h"


namespace meteo
{

double clamp(double value, double min, double max )
{

 if(value<min) return min;
 if(value>max) return max;
// printf("%f %f\n",res, value);
 return value;
}


//!функция расчета эксцентриситета эллипса
double func_ellipsE(double a, double b)
{
	return sqrt(1.-b*b/a/a);
}

double func_ellipsN(double a,double b, double Fi)
{
	double e;
	e=func_ellipsE(a,b);
	return a/sqrt(1.-e*e*sin(Fi)*sin(Fi));
}


//!коэффициенты t1,t2,t3,t4,t5,tau
double func_t1(double fi0,double la0, double fi, double la)
{
	return sin(fi)*cos(fi0)-cos(fi)*sin(fi0)*cos(la-la0);
}
double func_t2(double fi0,double fi)
{
	return sin(fi)*sin(fi0)*(sin(fi)+sin(fi0))+(sin(fi)-sin(fi0))*(3*sin(fi)*sin(fi)-1);
}
double func_t3(double fi0,double fi)
{
    return sin(fi)*sin(fi)-0.5*sin(fi0)*(sin(fi)-sin(fi0));
}
double func_t4(double la0, double fi, double la)
{
    return cos(fi)*sin(la-la0);
}
double func_t5(double fi0,double la0, double fi, double la)
{
    return sin(fi)*sin(fi0)+cos(fi)*cos(fi0)*cos(la-la0);
}
double func_tau(double fi0,double fi)
{
	return sin(fi)-sin(fi0);
}

double func_sinzcosa(double a,double b,double fi0,double la0,double fi,double la)
{
 double t1,t2,t3,tau,e;
 t1=func_t1(fi0,la0,fi,la);
 t2=func_t2(fi0,fi);
 t3=func_t3(fi0,fi);
 tau=func_tau(fi0,fi);
 e=func_ellipsE(a,b);

 return t1+e*e*tau*((t1*sin(fi)-cos(fi0))+0.5*e*e*(t1*t2-2*t3*cos(fi0)));
}
double func_sinzsina(double a,double b,double fi0,double la0,double fi,double la)
{
	double t2,t4,tau,e;
	t2=func_t2(fi0,fi);
	t4=func_t4(la0,fi,la);
	tau=func_tau(fi0,fi);
    e=func_ellipsE(a,b);

	return t4*(1.+e*e*tau*(sin(fi)+0.5*e*e*t2));
}
double func_cosz(double a,double b,double fi0,double la0,double fi,double la)
{
	double t2,t3,t5,tau,e;
    t2=func_t2(fi0,fi);
    t3=func_t3(fi0,fi);
    t5=func_t5(fi0,la0,fi,la);
	tau=func_tau(fi0,fi);
    e=func_ellipsE(a,b);

	return t5+e*e*tau*((t5*sin(fi)-sin(fi0))+0.5*e*e*(t2*t5-2*t3*sin(fi0)));
}
/////////////////////////////////////////////////////////////////////


double calcAzimooth_ellips(double a, double b, double Fi0, double La0, double Fi, double La)
{
 double sc,c, az;
 sc=func_sinzcosa(a,b,Fi0,La0,Fi,La);
 c=func_cosz(a,b,Fi0,La0,Fi,La);
 if(c>=1.) return 0.;
 
 az=sc/sqrt(1.-c*c);
 az=acos(clamp(az,-1.0, 1.0));
 
 if(La<La0) az=2.*M_PI-az;
 
//  printf("fi0=%f la0=%f\n", Fi0*180/M_PI, La0*180/M_PI);
//  printf("fi=%f la=%f\n", Fi*180/M_PI, La*180/M_PI);
//  printf("az=%f\n", az*180/M_PI);
 return az;

 
}


double calcAzimooth_sphere(double Fi0, double La0, double Fi, double La)
{
 double c,sc, az;
 sc=func_t1(Fi0,La0,Fi,La);

 c=func_t5(Fi0,La0,Fi,La);

 if(c>=1.) return 0.;
 
 az=sc/sqrt(1.-c*c);
 az=acos(clamp(az,-1.0,1.0));

 if(La<La0) az=2.*M_PI-az;
 
 return az;
 
}




//!
//!Функция пересчета из географических координат Fi La в прямоугольные координаты проекции x, y
//!a, b - большая, малая полуоси эллипсойда Красовского
//!Fi0, La0 - географические координаты точки касания эллипсойда плоскостью (координаты МРЛ)
//!Fi,La - географические координаты заданной точки
//!x,y - прямоугольные координаты точки на карте, центр координат в точке касания эллипсойда, ось OX направлена на север, ось OY направлена на восток 
//!m - масштабный коэффициент
//!
void FLtoXY_ellips(double a, double b, double Fi0,double La0, double Fi, double La, double& x, double& y, double &m)
{
 double N0,k,sc,ss,c,e;
 k=1;
 N0=func_ellipsN(a,b,Fi0);
 sc=func_sinzcosa(a,b,Fi0,La0,Fi,La);
 ss=func_sinzsina(a,b,Fi0,La0,Fi,La);
 c=func_cosz(a,b,Fi0,La0,Fi,La);
 e=func_ellipsE(a,b);
 x=2*N0*k*sc/(1+c);
 y=2*N0*k*ss/(1+c);
 m=k*2*(1+0.5*e*e*pow(ss*cos(Fi0)+sin(Fi0)*(c-1),2))/(1+c);
}


//!
//!Функция пересчета из географических координат Fi La в прямоугольные координаты проекции x, y
//!R - радиус Земли
//!Fi0, La0 - географические координаты точки касания Земного шара плоскостью (координаты МРЛ)
//!Fi,La - географические координаты заданной точки
//!x,y - прямоугольные координаты точки на карте, центр координат в точке касания Земного шара, ось OX направлена на север, ось OY направлена на восток 
//!m - масштабный коэффициент
//!

void FLtoXY_sphere(double R, double Fi0,double La0, double Fi, double La, double& x, double& y, double &m)
{
 double k,sc,ss,c;
 k=1;
 sc=func_t1(Fi0,La0,Fi,La);
 ss=func_t4(La0,Fi,La);
 c=func_t5(Fi0,La0,Fi,La);

 x=2*R*k*sc/(1+c);
 y=2*R*k*ss/(1+c);
 m=k*2/(1+c);
}

//!
//!Функция пересчета из географических координат Fi La H (Height) в прямоугольные координаты x, y, z топоцентрической СК 
//!R - радиус Земли
//!Fi0, La0 - географические координаты точки касания Земного шара плоскостью (координаты МРЛ)
//!alpha - азимут направления оси OX
//!Fi,La - географические координаты заданной точки
//!x,y,z - прямоугольные координаты точки в топоцентрической СК, центр координат в точке  Fi0, La0, ось OX направлена под углом alpha от направления на север, ось OZ направлена по нормали к поверхности из центра Земли, ось OY дополняет СК до правой 
//!m - масштабный коэффициент
//!

void FLHtoXYZ_sphere(double R, double Fi0,double La0, double alpha, double Fi, double La, double H, double& x, double& y,double& z)
{
 double sc,ss,c,X1,Y1;

 sc=func_t1(Fi0,La0,Fi,La);
 ss=func_t4(La0,Fi,La);
 c=func_t5(Fi0,La0,Fi,La);

 X1=(R+H)*sc; ///topocenter - NORD
 Y1=(R+H)*ss; 
 z=(R+H)*c-R;

 x=sin(alpha)*Y1+cos(alpha)*X1;
 y=-cos(alpha)*Y1+sin(alpha)*X1;

}

//!
//!Функция пересчета из сферических координат azimooth, tet(elevation), D (Distance) топоцентрической СК в географические координаты Fi La H (Height)
//!R - радиус Земли
//!Fi0, La0 - географические координаты точки касания Земного шара плоскостью (координаты МРЛ)
//!Fi,La - географические координаты искомой точки
//!azimooth,tet,D - сферические координаты (азимут, угол места, дальность) заданной точки в топоцентрической СК, центр координат в точке  Fi0, La0
//!


void AEDtoFLH_sphere(double R, double Fi0,double La0, double azimooth, double tet, double D, double& Fi, double& La, double& H)
{
 double fi1,dla;

 H=sqrt(D*D+R*R+2*R*D*sin(tet))-R;
 fi1=asin(clamp(D*cos(tet)/(H+R),-1.0,1.0));
// c=cos(fi1);
 dla=atan(sin(fi1)*sin(azimooth)/(cos(fi1)*cos(Fi0)-sin(fi1)*cos(azimooth)*sin(Fi0)));

 if(dla==0)
   Fi=acos(clamp((cos(fi1)/sin(Fi0)-sin(fi1)*cos(azimooth)/cos(Fi0))/(1/tan(Fi0)+tan(Fi0)/cos(dla)),-1.0,1.0));
 else
   Fi=acos(clamp(sin(fi1)*sin(azimooth)/sin(dla),-1.0,1.0));
//   Fi=acos(fabs(sin(fi1)*sin(alpha)/sin(dla)));

// if(sin(alpha)>=0)
  La=dla+La0;
// else
//  La=-dla+La0;

}



//!
//!Функция пересчета из прямоугольных координат x,y,z в сферические координаты azimooth, tet(elevation), D (Distance) топоцентрической СК
//!R - радиус Земли
//!alpha - азимут направления оси OX
//!azimooth,tet,D - сферические координаты (азимут, угол места, дальность) точки в топоцентрической СК
//!X,Y,Z - прямоугольные координаты точки в топоцентрической СК, центр координат в точке  Fi0, La0, ось OX направлена под углом alpha от направления на север, ось OZ направлена по нормали к поверхности из центра Земли, ось OY дополняет СК до правой 
//!

void XYZtoAED_sphere(double alpha, double X, double Y, double Z,  double& az, double& tet, double& D )
{
 double X1,Y1;
 
    tet=atan2(Z,sqrt(X*X+Y*Y));   
    X1=cos(alpha)*X+sin(alpha)*Y;
    Y1=sin(alpha)*X-cos(alpha)*Y;
    az=atan2(Y1,X1);
    D=sqrt(X*X+Y*Y+Z*Z);

}


};
