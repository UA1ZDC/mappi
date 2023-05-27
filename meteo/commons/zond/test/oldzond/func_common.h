#ifndef func_common_h
#define func_common_h

#include <math.h>
//#include <time.h>
#include <stdio.h>
#include "stdlib.h"
//#include <qpointarray.h>
//#include <qfile.h>
//#include <qtextstream.h>
//#include <qtextcodec.h>
//#include <qdatetime.h>

#include "map_specification.h"
//#include "structures.h"
//#include "func_geogr.h"
//#include "types.h"
//#include "func_sunrise.h"
//#include "func_sun_moon.h"

#include<qstring.h>

#define NAPR  LINTOWIN

typedef unsigned char BYTE;

int ftoi_norm(float );

int max(int,int);
int min(int,int);
double max(double,double);
double min(double,double);
void SetBit1(int,BYTE *);
int GetBit1(int ,BYTE *);
double MPiToPi(double);
double M180To180(double);
double Ot0To360(double);
//void getCurrentTime(int *, int);
//int peresech(QPointArray *);
void sort2(double *, int *, int);
void sort1(double *,int);
int searh_inf(double, double *, int);

bool getMasFromString(QString ,double**,int*);
bool getMasFromString(QString ,int**,int*);
//void initPropertyXml(PropertyChooseData *,int, QString,QString);
bool linInterpol(double,double,double,double,double,double&);
int kolChiselVString(QString &);
bool getChiselIsString(QString &, double *&, int & );
bool getChiselIsString(QString &, int *&, int & );
bool getStringPoUppercase(QString &str, QString *&val, int &kol );
bool getStringChislPoUppercase(QString &str, QString *&s_val, int &kol_s, int *&i_val, int &kol_i );
bool getStringChislPoUppercase(QString &str, QString *&s_val, int &kol_s, double *&i_val, int &kol_i );
bool calcZnachPoIso(double *,int,double,double &, int);
bool integrTrapec(double *, double *, int, double &);
//int dist_plos(Screen pos,int x,int y);




#endif

