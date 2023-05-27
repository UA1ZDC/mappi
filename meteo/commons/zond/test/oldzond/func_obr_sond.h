#ifndef FUNC_OBR_SOND_H
#define FUNC_OBR_SOND_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "data_structures.h"
#include "func_common.h"
#include "func_fillData.h"
//#include "structures_p.h"
//#include "usvoen_param.h"

void SondInterpolT(TAeroDataAll *);
void SondInterpolD(TAeroDataAll *);
void SondInterpolV(TAeroDataAll *);
int Opr_H(TAeroDataAll, TAeroDataAll *);


char integr_Func_Value(double *, double *, int, double, double *);
int itegr_Func(double *, double *, int, double *, double *, char *, int);
double Table_5_1(double);
double def_dt(double, double, double);
int calculLevelParam(TAeroDataAll, Level *, int *, bool);

int GetMeteo11fromKN04(TAeroDataAll, TMeteo11DataAll *, int);
int GetLayerfromKN04(TAeroDataAll, TLayerDataAll *, int);
int def_end_Level(int);


int oprAeroParamFromP(TAeroDataBasa, int, Uroven *);






#endif


