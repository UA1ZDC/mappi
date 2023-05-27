#ifndef TAERO_H
#define TAERO_H

//#include <qtextcodec.h>
//#include <qobject.h>
//#include <qdatetime.h>
//#include <qapplication.h>
//#include <qcursor.h>

//#include <property_structures.h>
//#include <func_isoline.h>
//#include <structures.h> //TAeroDataAll, TAeroDataBasa

//#include <func_geogr.h>
//#include <func_common.h>
//#include <interpolHaos.h>
//#include <data_types.h>
#include "func_obr_sond.h"     //Opr_H()
#include "funkciidiagnoza.h"

#define PREDEL_RAST_KN04 2.

namespace aero {

 struct Uroven
 {
  int KolDan;
  int P, H, T, D, dd, ff;
  char pok_kach[8]; 
 };

struct TAeroDataAll
 {
  int Tip;
  long Index;
  int Height;
  int Pok_Privyaz;
  char Name[31];
  int KolDan;
  char pok_inf[5];
  int Term[5];
  int Koord[4];
  int Quadrat[2];
  Uroven data[200];
 };

class TAero
{
public:
    TAero();

    ~TAero();


     bool getUr(int,Uroven &);
     bool getTropo(Uroven &);

     bool getUrz(Uroven &);
     bool getPz(double &);
     bool getTz(double &);
     void preobrUVtoDF(int, int, int &, int &);
     bool oprUrPoP(double, Uroven &);
     bool oprUrPoH(double, Uroven &);

     bool oprPPoParam(char, double, double **, int &);
     bool oprPkondens(double &,double &);
     bool oprTsost(double,double &);
     bool oprPsostRaznAll(double &, double &, double ival_lev_end=100., double ival_lev_start=0., int step=10);
     bool getUrOsobyT(Uroven *&ur,int &kolUr);
     bool getUrOsobyV(Uroven *&ur,int &kolUr);
     bool oprPprizInv(double &,double &,double &,double &);

//     void setPoTAeroDataBasa(TAeroDataBasa &aero_vs);
     void setPoTAeroDataAll(TAeroDataAll &aero_vs);
     bool oprHmaxV(double &, double &, double ival_lev_end=100., double ival_lev_start=1000., int step=10);
     bool oprIlina(double &, double &, double ival_lev_end=100., double ival_lev_start=1000., int step=10);
     bool oprH1(double &, double &, double ival_lev_end=100., double ival_lev_start=1000., int step=10);

     bool oprGranKNS(int &, double *&);
     bool  oprSkorDpKNS(int &, double *&, double *&);
     bool  oprSrKonvSkor(double &, double, double);
     bool oprSrParamPoPH(char, double &, double &, double, double, bool);
     bool  oprIntLivnOrl(double &);
     int  oprZnakEnergy(double);
     QString getIndex();
     QString getNameSt();



//     TAeroDataBasa *aero_vs;
//     TAeroDataBasa aeroDb;
     TAeroDataAll aeroDa;
     bool is_data;
};

}
#endif
