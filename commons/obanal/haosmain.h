#ifndef INTERPOLHAOS_MAIN_H
#define INTERPOLHAOS_MAIN_H

#include <stdio.h>
#include <malloc.h>
#include <math.h>

//#include "func_vspom.h"
#include "linalgebra.h"
#include "tfield.h"


int test(meteo::GeoData* gd);

namespace obanal{

  struct  parCub
 {
  double x0;
  double y0;
  double pX;
  double pY;
  double Ed_pX;
  double Ed_pY;
 };

 int interpolHaosOnePoint(int n, double *x, double *y, double *z, double xx,double yy, float *rezult);

 int interpolHaos(const meteo::GeoData &src_data, TField *rezult);
 int interpolHaosRazbien(const meteo::GeoData &src_data, TField *rezult);


 int Coeff_NoIterat(double *x, double *z, double *w, double *d, int n,
                    double alfa, int kolw);

 void geogrToPlos(float fi, float la, double *xy);

 void geogrToPlos(const meteo::GeoPoint &xy, double *);
 void geogrToPlos(const QVector<meteo::GeoPoint> &src_data, double *);
 int preobrKoordToPlos(double *, int n, parCub *p );

 float valPointInt(double *xy, int n, double *netxy, const QVector<double> &coef);

 float valPointInt(double *xy, int n, double *netxy,
                    const QVector<double> &coef,   const parCub &p);
 double fun_skl_xy(double ddx, double dx, double a, double b);
 double ValPoint_Int_No_Preobr(double *xy, double x, double y, double *d, int n);

 bool getPointXY(const QVector<meteo::GeoPoint>& points, int numppr,
                 QVector<float> * rx,QVector<float> * ry, int *min, int *max,QVector<int> *kol);
 int searchY(const QMap<float, float> &xi,
             float x0,float y0,float x1, float *y1, int numppr, float step);
 int searchX(const QMap<float, float> &xi,
             float x0,float y0, float *x1, float y1, int numppr, float step);
 int searchXY(const QMap<float, float> &xi,
              float x0,float y0,float *x1,float *y1, int numppr, float step);
 int kolInKvadrat(const QMap<float, float> &xi,
                    float x0,float y0,float x1,float y1 );


}
#endif

