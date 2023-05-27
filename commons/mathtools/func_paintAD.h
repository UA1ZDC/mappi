#ifndef FUNC_PAINTAD_H
#define FUNC_PAINTAD_H

#include <math.h>
//#include <alloc.h>
#include <stdio.h>

#include <qpainter.h>
#include <qpen.h>
#include <qstring.h>
#include <qfont.h>

#include "ad_specification.h"
#include "data_structures.h"
#include "func_common.h"
#include "func_fillData.h"
#include "tobjectvectorpolygon.h"


#define PARALLEL    0    // отрезки лежат на параллельных прямых
#define SAMELINE     1 // отрезки лежат на одной прямой
#define ONBOUNDS    2 // прямые пересекаются в конечных точках отрезков
#define INBOUNDS      3  // прямые пересекаются в   пределах отрезков
#define OUTBOUNDS  4     // прямые пересекаются вне пределов отрезков

struct CrossResultRec
{
int  type;  // тип пересечения
GeoPoint      pt;    // точка пересечения
};

/**
 * функция определяющая точки пересечения двух кривых заданных массивами точек
 * @param kr1 кривая 1
 * @param kr2 кривая 2
 * @param pointPeresech  координаты точек пересечения
 * @param numOtrPeresech  номера отрезков пересечения - номера нижних точкек отрезков
 * @param  znak  знак пересечения
 * @param  scale масштаб
 * @return
 */
double oprTpotSuch(double p,double t);
double oprTsuch(double p,double tet);
double oprTpotVlaz(double p,double t);
int oprTvlaz(double P1,double tet,double t1,double t2, GeoPoint *fl, int *kol);
int oprTochPeresech(GeoPoint *kr1,int,GeoPoint *kr2,int,GeoPoint *&pointPeresech, QPointArray *numOtrPeresech, int*,int*, double sc=1.);
int oprZnakIzm(GeoPoint p11,GeoPoint p12,GeoPoint p21,GeoPoint p22,GeoPoint,int,double);
CrossResultRec crossing(GeoPoint p11,GeoPoint p12,GeoPoint p21,GeoPoint p22);
bool findPeresechPoX(double x0,double x1,double y0,double y1,double znach,double *ret_val);
bool findPeresechPoY(double x0,double x1,double y0,double y1,double znach,double *ret_val);
double EP(double);
double Y(double);
bool INSET(int);
bool ADPts(int, TPointList **);
bool ADDataPts(int, TPointList **, TAeroDataAll *);
void calculateOnePointAD(int *, int *, PrjPar *, double);
bool ValidDouble(double, char);
bool ValidInt(double, char);
bool CloudDeficit(double, double, double *);
double ie2(double, double, double, double, double);
double PE(double);
bool CondensTrace(int, double, double, double *);
bool CondensCheck(int, double, double);
bool uk(TAeroDataAll *, int *, int*, double *, double *);
double Y2(double yy);
double Y1(double);
double oprHperesech(double *h,GeoPoint *kr,int num,double znach);
QString s_oprHperesech(double *h,GeoPoint *kr,int num,double znach);
bool resizeGp(GeoPoint *&array,int new_size);

#endif
