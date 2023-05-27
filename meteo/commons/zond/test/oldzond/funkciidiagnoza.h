#ifndef FUNKCIIDIAGNOZA_H
#define FUNKCIIDIAGNOZA_H

#include "func_common.h"
//#include "property_structures.h"
#include <qregexp.h>


bool oprTsm(double, double, double, double &);
double EP(double);
bool oprTet(double , double , double &);
bool oprTPoTet(double, double, double &);
bool oprTPoTPot(double P,double tet, double &res);
bool oprTPot(double, double, double &);
bool oprTPoSP(double,double, double &);
bool oprSPoTP(double,double, double &);
bool oprPkondens(double, double, double, double &, double &);
#endif
