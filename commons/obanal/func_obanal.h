#ifndef FUNC_OBANAL_H
#define FUNC_OBANAL_H

#define RAST_FROM_PROREJ 0.05

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include "haosmain.h"

#define SHIR_SKLEYKI 0.261799f//15.f*M_PIf / 180.0f


#define PREDEL_RADIUS 5.
#define KOL_STAN      20
#define KOL_STAN_MIN  3

#define TRUE_DATA 1//48
#define FALSE_DATA 0//49

namespace MnObanal{
  //int prepInterpolHaos(float *, float *, float *, int,
//                     float **, bool **, int *, int);
void sort2(float *, int *, int);
void sort2(double *, int *, int);
int searh_inf(float, const QVector<float> &, int);
int searh_inf(float a, float *mas, int n);
int searh_inf(double a, double *mas, int n);


int prorej(meteo::GeoData *adata, QVector<int> *num,  float rast);
int prorej_GRIBD(meteo::GeoData *adata,QVector<int> *num,  float rast);


int prepInterpolHaos(meteo::GeoData *adata, obanal::TField *rez,
          float predel_grad, QStringList* badData ,
          float rast = PREDEL_RADIUS, int kolstn=KOL_STAN,
          int kolstmin = KOL_STAN_MIN, float rstfrprorej=RAST_FROM_PROREJ, float shir_skl=SHIR_SKLEYKI);
float fun_skl(float fi, float a, float b,float);
int kontrol_gradient ( meteo::GeoData *adata, float rast, int kolstan, int kolstan_min,
           float predel_grad, QStringList* badData );

int prepInterpolOrder(meteo::GeoData *adata, obanal::TField *rezult_);

}

#endif
