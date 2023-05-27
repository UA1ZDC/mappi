#ifndef LINALGEBRA_H
#define LINALGEBRA_H


#include <stdio.h>
#include <malloc.h>
#include <math.h>

#include <QVector>
#include <commons/geobasis/geodata.h>
#include "obanal_struct.h"



int Num_Elem(int i, int j);

void Basis(double *, double *, double *, double *, int );
void BasisDiff(double *, double *, double *, double *, int *, int  );
int Fac_Chol(double *, int );
void Resh_Chol(double *, double *, double *, int );
int Num_Elem(int , int );
int Diag_Inv(double *, double *, int  );
void Diag_Prod(double *, double *, double *, int );
double Scalar_Prod(double *, double *, int );
void DiagAny_Prod(double *, double *, double *, int , int );
void DiagAnyPol_Prod(double *, double *, double *, int , int );
void AnyDiag_Prod(double *, double *, double *, int , int );
void AnyPolDiag_Prod(double *, double *, double *, int , int );
void Any_Prod(double *, double *, double *, int , int , int );
void AnyAnyT_Prod(double *, double *, int , int );
void SymVec_Prod(double *, double *, double *, int );
int SymLVec_Prod(double *,double *, double *, int , int );
void BasAny_Prod(double *, double *, double *, int , int , int );
void BasTAny_Prod(double *, double *, double *, int , int , int );
int Matr_H(double *, double *, double *, int , int );
int Matr_B(double *, double *, double *, int , int );
void SymSym_Sum(double *, double *, double *, double , int );
void SymDiag_Sum(double *, double *, double *, double , int );
void Vec_Dif(double *, double *,  int );

////////////////////////////////////////////////////////////////////////

double Fun_Grin(double , double , double , double  );
double Fun_Grin1x(double , double , double , double );
double Fun_Grin1y(double , double , double , double );
double Fun_Grin2xy(double , double , double , double );
double Fun_Grin2x(double , double , double , double  );
double Fun_Grin2y(double , double , double , double );
void funMatrG(double *, int *, double *, int);
void funMatrG(double *, double *, int);
int reshGaus(double *, double*, double*, int,double);
//double abs(double);

#endif

