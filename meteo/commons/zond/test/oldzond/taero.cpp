#include "taero.h"

namespace aero {

TAero::TAero()
{
//aero_vs=0;
is_data=false;
}


TAero::~TAero()
{
//if(aero_vs) {delete []aero_vs;aero_vs=0;}
}


QString TAero::getIndex()
{
if(!is_data) return "";
QString ws;
ws=ws.setNum(aeroDa.Index);
return ws;
}

QString TAero::getNameSt()
{
if(!is_data) return "";
return aeroDa.Name;
}

bool TAero::getUr(int ivalueLevel,Uroven &ur)
{

Uroven ur1,ur2;
bool b1=false,b2=false;
int vid=0;
int kolDan;
int j;
if(!is_data) return false;

ur.P=9999;
ur.H=9999;
ur.T=9999;
ur.D=9999;
ur.ff=9999;
ur.dd=9999;
strcpy(ur.pok_kach,"1111111");
kolDan=aeroDa.KolDan;
if(kolDan<1) return false;
if(kolDan>200) return false;

switch(ivalueLevel)
      {
        case 11000:
        case 25000:
        case 35000:
          if(ivalueLevel==11000) vid='1';
             else if(ivalueLevel==25000) vid='4';
                        else vid='5';
          for(j=0;j<kolDan;j++)
              if(aeroDa.data[j].pok_kach[6]==vid)
                  {
                    ur=aeroDa.data[j];
                     if(ur.pok_kach[4]=='0'&&ur.pok_kach[5]=='0')
                         preobrUVtoDF(ur.dd,ur.ff,ur.dd,ur.ff);
                    return true;
                  }
        return false;
        case 15000:
                for(j=0;j<kolDan;j++)
                  {
                    if(aeroDa.data[j].pok_kach[6]=='0'&&aeroDa.data[j].pok_kach[0]=='0'&&aeroDa.data[j].P==1000)
                        {
                          ur1=aeroDa.data[j];
                          b1=true;
                        }
                    if(aeroDa.data[j].pok_kach[6]=='0'&&aeroDa.data[j].pok_kach[0]=='0'&&aeroDa.data[j].P==500)
                        {
                          ur2=aeroDa.data[j];
                          b2=true;
                        }
                  }
              if(b1&&b2)
                  {
                    ur.P=15000; ur.pok_kach[0]='0'; ur.pok_kach[6]='7';
                      if(ur1.pok_kach[1]=='0'&&ur2.pok_kach[1]=='0') {ur.H=ur2.H-ur1.H; ur.pok_kach[1]='0';}
                      if(ur1.pok_kach[4]=='0'&&ur1.pok_kach[5]=='0'&&ur2.pok_kach[4]=='0'&&ur2.pok_kach[5]=='0')
                        {
                            ur.dd=ur2.dd-ur1.dd;
                            ur.ff=ur2.ff-ur1.ff;
                            preobrUVtoDF(ur.dd,ur.ff,ur.dd,ur.ff);
                            ur.pok_kach[4]='0';
                            ur.pok_kach[5]='0';
                        }
                    return true;
                  }
                  else  return false;

       default :
          for(j=0;j<kolDan;j++)
              if(aeroDa.data[j].pok_kach[6]=='0'&&aeroDa.data[j].pok_kach[0]=='0'&&aeroDa.data[j].P==ftoi_norm(ivalueLevel/10.))
                  {
                    ur=aeroDa.data[j];
                     if(ur.pok_kach[4]=='0'&&ur.pok_kach[5]=='0')
                         preobrUVtoDF(ur.dd,ur.ff,ur.dd,ur.ff);
                    return true;
                  }
          if(0<ivalueLevel&&ivalueLevel<10000)
             //if(!oprAeroParamFromP(aeroDb, ftoi_norm(ivalueLevel/10.), &ur))
              if(oprUrPoP(ftoi_norm(ivalueLevel/10.), ur))
                    {
                     if(ur.pok_kach[4]=='0'&&ur.pok_kach[5]=='0')
                         preobrUVtoDF(ur.dd,ur.ff,ur.dd,ur.ff);
                       return true;
                    }
          break;
      }
return false;
}

void TAero::preobrUVtoDF(int u, int v,int &dd, int &ff)
{
double rtog=180./3.1415926;
double dx,dy;

dx=u/100.;
dy=v/100.;
ff=ftoi_norm(sqrt(dx*dx+dy*dy));
dd=ftoi_norm(atan2(dy,dx)*rtog);
dd=ftoi_norm(Ot0To360(270.-dd));
}

bool TAero::oprUrPoP(double p, Uroven &ur)
{
TAeroDataAll struc;
int i,i_ur;
double gam,h,g=9.8065,R=287.,vs;
int kol;
double per=0.1, tK=273.15;
ur.P=9999;
ur.H=9999;
ur.T=9999;
ur.D=9999;
ur.ff=9999;
ur.dd=9999;
strcpy(ur.pok_kach,"1111117");
if(!is_data) return false;
struc=aeroDa;
kol=struc.KolDan-1;
if(kol>199) return false;
for(i=0;i<kol;i++)
      if(struc.data[i].pok_kach[0]=='0'&&struc.data[i+1].pok_kach[0]=='0'&&struc.data[i+1].P<p&&p<=struc.data[i].P) break;
if(i==kol) return false;
i_ur=i;
if(struc.data[i_ur].pok_kach[1]!='0'||struc.data[i_ur+1].pok_kach[1]!='0') return false;
if(struc.data[i_ur].pok_kach[2]!='0'||struc.data[i_ur+1].pok_kach[2]!='0') return false;
gam=-(struc.data[i_ur+1].T*per-struc.data[i_ur].T*per)/(struc.data[i_ur+1].H-struc.data[i_ur].H);
if(gam==0.)
   h=-log(p/(struc.data[i_ur].P))*R*(struc.data[i_ur].T*per+tK)/g;
else
   {
     vs=g/(gam*R);
     vs=exp(log(p/(struc.data[i_ur].P))/vs)*(struc.data[i_ur].T*per+tK);
     h=(struc.data[i_ur].T*per+tK-vs)/gam;
   }
ur.P=ftoi_norm(p); ur.pok_kach[0]='0';
ur.H=ftoi_norm(struc.data[i_ur].H+h); ur.pok_kach[1]='0';

if(linInterpol(struc.data[i_ur].H,struc.data[i_ur].T*per,struc.data[i_ur+1].H,struc.data[i_ur+1].T*per,(double)ur.H,vs))
 {ur.T=ftoi_norm(vs*10.); ur.pok_kach[2]='0';}
if(struc.data[i_ur].pok_kach[3]=='0'&&struc.data[i_ur+1].pok_kach[3]=='0')
    if(linInterpol(struc.data[i_ur].H,struc.data[i_ur].D*per,struc.data[i_ur+1].H,struc.data[i_ur+1].D*per,(double)ur.H,vs))
       {ur.D=ftoi_norm(vs*10.); ur.pok_kach[3]='0';}
if(struc.data[i_ur].pok_kach[4]=='0'&&struc.data[i_ur+1].pok_kach[4]=='0')
    if(linInterpol(struc.data[i_ur].H,struc.data[i_ur].dd*per,struc.data[i_ur+1].H,struc.data[i_ur+1].dd*per,(double)ur.H,vs))
       {ur.dd=ftoi_norm(vs*10.); ur.pok_kach[4]='0';}
if(struc.data[i_ur].pok_kach[5]=='0'&&struc.data[i_ur+1].pok_kach[5]=='0')
    if(linInterpol(struc.data[i_ur].H,struc.data[i_ur].ff*per,struc.data[i_ur+1].H,struc.data[i_ur+1].ff*per,(double)ur.H,vs))
       {ur.ff=ftoi_norm(vs*10.); ur.pok_kach[5]='0';}
return true;
}

bool TAero::oprUrPoH(double H, Uroven &ur)
{
TAeroDataAll struc;
int i,i_ur;
double gam,h,g=9.8065,R=287.,vs;
int kol;
double per=0.1, tK=273.15;
ur.P=9999;
ur.H=9999;
ur.T=9999;
ur.D=9999;
ur.ff=9999;
ur.dd=9999;
strcpy(ur.pok_kach,"1111117");
if(!is_data) return false;
struc=aeroDa;
kol=struc.KolDan-1;
if(kol>199) return false;
for(i=0;i<kol;i++)
      if(struc.data[i].pok_kach[1]=='0'&&struc.data[i+1].pok_kach[1]=='0'&&struc.data[i].H<=H&&H<struc.data[i+1].H) break;
if(i==kol) return false;
i_ur=i;

ur.H=ftoi_norm(H); ur.pok_kach[1]='0';
if(struc.data[i_ur].pok_kach[2]=='0'&&struc.data[i_ur+1].pok_kach[2]=='0')
    if(linInterpol(struc.data[i_ur].H,struc.data[i_ur].T*per,struc.data[i_ur+1].H,struc.data[i_ur+1].T*per,(double)ur.H,vs))
       {ur.T=ftoi_norm(vs*10.); ur.pok_kach[2]='0';}
if(struc.data[i_ur].pok_kach[0]=='0'&&struc.data[i_ur].pok_kach[2]=='0'&&
     struc.data[i_ur+1].pok_kach[2]=='0'&&ur.pok_kach[2]=='0')
   {
    h=ur.H-struc.data[i_ur].H;
    if(h==0.) ur.P=struc.data[i_ur].P;
        else
            {
              gam=-(struc.data[i_ur+1].T-struc.data[i_ur].T)*per/((struc.data[i_ur+1].H-struc.data[i_ur].H));
              if(gam==0.) ur.P=ftoi_norm(struc.data[i_ur].P*exp(-g*h/R/(struc.data[i_ur].T*per+tK)));
                  else
                  {
                     vs=pow((ur.T*per+tK)/(struc.data[i_ur].T*per+tK),g/(R*gam));
                     ur.P=ftoi_norm(struc.data[i_ur].P*vs);
                  }
            }
      ur.pok_kach[0]='0';
    }
if(linInterpol(struc.data[i_ur].H,struc.data[i_ur].T*per,struc.data[i_ur+1].H,struc.data[i_ur+1].T*per,(double)ur.H,vs))
 {ur.T=ftoi_norm(vs*10.); ur.pok_kach[2]='0';}
if(struc.data[i_ur].pok_kach[3]=='0'&&struc.data[i_ur+1].pok_kach[3]=='0')
    if(linInterpol(struc.data[i_ur].H,struc.data[i_ur].D*per,struc.data[i_ur+1].H,struc.data[i_ur+1].D*per,(double)ur.H,vs))
       {ur.D=ftoi_norm(vs*10.); ur.pok_kach[3]='0';}
if(struc.data[i_ur].pok_kach[4]=='0'&&struc.data[i_ur+1].pok_kach[4]=='0')
    if(linInterpol(struc.data[i_ur].H,struc.data[i_ur].dd*per,struc.data[i_ur+1].H,struc.data[i_ur+1].dd*per,(double)ur.H,vs))
       {ur.dd=ftoi_norm(vs*10.); ur.pok_kach[4]='0';}
if(struc.data[i_ur].pok_kach[5]=='0'&&struc.data[i_ur+1].pok_kach[5]=='0')
    if(linInterpol(struc.data[i_ur].H,struc.data[i_ur].ff*per,struc.data[i_ur+1].H,struc.data[i_ur+1].ff*per,(double)ur.H,vs))
       {ur.ff=ftoi_norm(vs*10.); ur.pok_kach[5]='0';}
return true;
}

bool TAero::oprPPoParam(char name, double znach, double **P, int &kol_P)
{
TAeroDataAll struc;
int i,ind,kol_ur,kol=0;
double  gam,h,g=9.8065,R=287.;
int kolXY,vs_i,vs_i1;
double per=0.1, tK=273.15;
double *vsX=0, *vsY=0, *vsT=0, *vsH=0;
bool ret_val=true;

if(!is_data) return false;
memcpy((void *)&struc,(void *)&aeroDa,sizeof(TAeroDataAll));
switch (name)
    {
      case 'H' : ind=1;  break;
      case 'T' : ind=2;  break;
      case 'D' : ind=3;  break;
      case 'd' : ind=4;  break;
      case 'f' : ind=5;  break;
      default : ret_val=false; goto m1;
     }
kolXY=0;
kol=struc.KolDan;
if(kol>199) {ret_val=false; goto m1;}

for(i=0;i<kol;i++)
      if(struc.data[i].pok_kach[ind]=='0'&&struc.data[i].pok_kach[0]=='0'
          &&struc.data[i].pok_kach[1]=='0'&&struc.data[i].pok_kach[2]=='0')  kolXY++;

if(kolXY==0)  {ret_val=false; goto m1;}
vsX= new double[kolXY];
if(!vsX) {ret_val=false; goto m1;}
vsY= new double[kolXY];
if(!vsY) {ret_val=false; goto m1;}
vsT= new double[kolXY];
if(!vsT) {ret_val=false; goto m1;}
vsH= new double[kolXY];
if(!vsH) {ret_val=false; goto m1;}

kolXY=0;
switch (name)
    {
      case 'H' :
                for(i=0;i<struc.KolDan;i++)
                    if(struc.data[i].pok_kach[ind]=='0'&&struc.data[i].pok_kach[1]=='0'&&struc.data[i].pok_kach[2]=='0')
                       {
                          vsX[kolXY]=struc.data[i].H; vsY[kolXY]=struc.data[i].P;
                          vsT[kolXY]=struc.data[i].T*per+tK; vsH[kolXY]=struc.data[i].H;
                          kolXY++;
                        }
                break;
      case 'T' :
                for(i=0;i<struc.KolDan;i++)
                    if(struc.data[i].pok_kach[ind]=='0'&&struc.data[i].pok_kach[0]=='0'
                        &&struc.data[i].pok_kach[1]=='0'&&struc.data[i].pok_kach[2]=='0')
                       {
                          vsX[kolXY]=struc.data[i].T*per; vsY[kolXY]=struc.data[i].P;
                          vsT[kolXY]=struc.data[i].T*per+tK; vsH[kolXY]=struc.data[i].H;
                          kolXY++;
                        }
                break;
      case 'D' :
                for(i=0;i<struc.KolDan;i++)
                    if(struc.data[i].pok_kach[ind]=='0'&&struc.data[i].pok_kach[0]=='0'
                        &&struc.data[i].pok_kach[1]=='0'&&struc.data[i].pok_kach[2]=='0')
                       {
                          vsX[kolXY]=struc.data[i].D*per; vsY[kolXY]=struc.data[i].P;
                          vsT[kolXY]=struc.data[i].T*per+tK; vsH[kolXY]=struc.data[i].H;
                          kolXY++;
                        }
                break;
      case 'd' :
                for(i=0;i<struc.KolDan;i++)
                    if(struc.data[i].pok_kach[4]=='0'&&struc.data[i].pok_kach[5]=='0'&&struc.data[i].pok_kach[0]=='0'
                        &&struc.data[i].pok_kach[1]=='0'&&struc.data[i].pok_kach[2]=='0')
                       {
                          preobrUVtoDF(struc.data[i].dd, struc.data[i].ff,vs_i,vs_i1);
                          vsX[kolXY]=vs_i;
                           vsY[kolXY]=struc.data[i].P;
                          vsT[kolXY]=struc.data[i].T*per+tK; vsH[kolXY]=struc.data[i].H;
                          kolXY++;
                        }
                break;
      case 'f' :
                for(i=0;i<struc.KolDan;i++)
                    if(struc.data[i].pok_kach[4]=='0'&&struc.data[i].pok_kach[5]=='0'&&struc.data[i].pok_kach[0]=='0'
                        &&struc.data[i].pok_kach[1]=='0'&&struc.data[i].pok_kach[2]=='0')
                       {
                          preobrUVtoDF(struc.data[i].dd, struc.data[i].ff,vs_i,vs_i1);
                          vsX[kolXY]=vs_i1;
                          vsY[kolXY]=struc.data[i].P;
                          vsT[kolXY]=struc.data[i].T*per+tK; vsH[kolXY]=struc.data[i].H;
                          kolXY++;
                        }
                break;
      default : ret_val=false; goto m1;
     }

kol_ur=0;
for(i=0;i<kolXY-1;i++)
if((vsX[i+1]<znach&&znach<=vsX[i])||(vsX[i+1]>znach&&znach>=vsX[i]))
kol_ur++;
if(kol_ur==0) {ret_val=false; goto m1;}
kol_P=kol_ur;

*P=NULL;
*P= new double[kol_ur];
if(!(*P)) {ret_val=false; goto m1;}

kol_ur=0;
for(i=0;i<kolXY-1;i++)
    if((vsX[i+1]<znach&&znach<=vsX[i])||(vsX[i+1]>znach&&znach>=vsX[i]))
       {
        if(!linInterpol(vsX[i],vsH[i],vsX[i+1],vsH[i+1],znach,h)) {ret_val=false; goto m1;}
         h-=vsH[i];
         gam=-(vsT[i+1]-vsT[i])/(vsH[i+1]-vsH[i]);
         if(gam==0.) (*P)[kol_ur]=vsY[i]*exp(-g*h/R/vsT[i]);
            else (*P)[kol_ur]=vsY[i]*pow((vsT[i]-gam*h)/vsT[i],g/R/gam);
        kol_ur++;
       }

m1:
if(vsX) {delete []vsX; vsX=0;}
if(vsY) {delete []vsY; vsY=0;}
if(vsT) {delete []vsT; vsT=0;}
if(vsH) {delete []vsH; vsH=0;}
if(!ret_val)  if(*P) {delete [](*P); *P=0; kol_P=0;}
return ret_val;
}

bool  TAero::oprSrParamPoPH(char name, double &znach, double &znach1,  double p1, double p2, bool type_p)
{
TAeroDataAll struc;
Uroven ur;
int i, ind, ind1, vs_i, vs_i1,kol=0;
int  kolXY;
double per=0.1;
double *vsX=0, *vsY=0, *vsZ=0;
bool ret_val=true;

znach=0.;
znach1=0.;
if(!is_data) return false;
if(type_p)  {if(p1<p2) return false;}
   else if(p2<p1) return false;
memcpy((void *)&struc,(void *)&aeroDa,sizeof(TAeroDataAll));
switch (name)
    {
      case 'P' : ind=0;  break;
      case 'H' : ind=1;  break;
      case 'T' : ind=2;  break;
      case 'D' : ind=3;  break;
      case 'd' : ind=4;  break;
      case 'f' : ind=5;  break;
      case 's' : ind=5;  break;
      default : ret_val=false; goto m1;
     }
if(type_p) ind1=0; else ind1=1;
kolXY=0;
kol=struc.KolDan;
if(kol>199) {ret_val=false; goto m1;}

for(i=0;i<struc.KolDan;i++)
      if(struc.data[i].pok_kach[ind]=='0' && struc.data[i].pok_kach[ind1]=='0'){
         if(type_p) {if(p1<=struc.data[i].P)  break; }
           else if(struc.data[i].H<=p1)  break;
      }
if(i==struc.KolDan) {ret_val=false; goto m1;}
kolXY++;
for(i=struc.KolDan-1;i>-1;i--)
      if(struc.data[i].pok_kach[ind]=='0'&&struc.data[i].pok_kach[ind1]=='0'){
         if(type_p) {if(struc.data[i].P<p2) break;}
            else if(p2<=struc.data[i].H)  break;
      }
if(i==-1) {ret_val=false; goto m1;}
kolXY++;
for(i=0;i<struc.KolDan;i++)
     if(struc.data[i].pok_kach[ind]=='0'&&struc.data[i].pok_kach[ind1]=='0'){
     if(type_p)  {if(p2<=struc.data[i].P&&struc.data[i].P<=p1)  kolXY++;}
        else if(p1<=struc.data[i].H&&struc.data[i].H<=p2)  kolXY++;
     }

if(kolXY==0)  {ret_val=false; goto m1;}
vsX= new double[kolXY];
if(!vsX) {ret_val=false; goto m1;}
vsY= new double[kolXY];
if(!vsY) {ret_val=false; goto m1;}
if(name=='s')
 {
   vsZ= new double[kolXY];
   if(!vsZ) {ret_val=false; goto m1;}
 }

if(type_p)  {if(!oprUrPoP(p1,ur))  {ret_val=false; goto m1;}}
 else if(!oprUrPoH(p1,ur))  {ret_val=false; goto m1;}
 kolXY=0;
 vsX[0]=p1;
 switch (name)
    {
      case 'P' :   if(ur.pok_kach[0]=='0') vsY[0]= ur.P;   break;
      case 'H' :   if(ur.pok_kach[1]=='0') vsY[0]= ur.H;   break;
      case 'T' :   if(ur.pok_kach[2]=='0') vsY[0]= ur.T;   break;
      case 'D' :  if(ur.pok_kach[3]=='0') vsY[0]= ur.D;   break;
      case 'd' :   if(ur.pok_kach[4]=='0'&&ur.pok_kach[5]=='0')
                         {
                            preobrUVtoDF(ur.dd, ur.ff,vs_i,vs_i1);
                            vsY[0]= vs_i;
                        }
                      break;
      case 'f' :   if(ur.pok_kach[4]=='0'&&ur.pok_kach[5]=='0')
                         {
                            preobrUVtoDF(ur.dd, ur.ff,vs_i,vs_i1);
                            vsY[0]= vs_i1;
                        }
                      break;
      case 's' :   if(ur.pok_kach[4]=='0'&&ur.pok_kach[5]=='0')
                         {
                            vsX[0]=ur.dd;
                            vsZ[0]= ur.ff;
                        }
                      break;
      default : ret_val=false; goto m1;
     }
kolXY++;
switch (name)
    {
      case 'P' :
                for(i=0;i<struc.KolDan;i++)
                   if(struc.data[i].pok_kach[ind]=='0'&&struc.data[i].pok_kach[ind1]=='0'){
                      if(type_p)
                       {
                          if(p2<=struc.data[i].P&&struc.data[i].P<=p1)
                           {vsX[kolXY]= struc.data[i].P;vsY[kolXY]= struc.data[i].P;  kolXY++;}
                       }
                           else if(p1<=struc.data[i].H&&struc.data[i].H<=p2)
                                      {vsX[kolXY]= struc.data[i].H;  vsY[kolXY]= struc.data[i].P;  kolXY++;}
		   }
                break;
      case 'H' :
                for(i=0;i<struc.KolDan;i++)
                   if(struc.data[i].pok_kach[ind]=='0'&&struc.data[i].pok_kach[ind1]=='0'){
                      if(type_p)
                       {
                          if(p2<=struc.data[i].P&&struc.data[i].P<=p1)
                           {vsX[kolXY]= struc.data[i].P;vsY[kolXY]= struc.data[i].H;  kolXY++;}
                       }
                           else if(p1<=struc.data[i].H&&struc.data[i].H<=p2)
                                      {vsX[kolXY]= struc.data[i].H;  vsY[kolXY]= struc.data[i].H;  kolXY++;}
		   }
                break;
      case 'T' :
                for(i=0;i<struc.KolDan;i++)
                   if(struc.data[i].pok_kach[ind]=='0'&&struc.data[i].pok_kach[ind1]=='0'){
                      if(type_p)
                       {
                          if(p2<=struc.data[i].P&&struc.data[i].P<=p1)
                           {vsX[kolXY]= struc.data[i].P;vsY[kolXY]= struc.data[i].T;  kolXY++;}
                       }
                           else if(p1<=struc.data[i].H&&struc.data[i].H<=p2)
                                      {vsX[kolXY]= struc.data[i].H;  vsY[kolXY]= struc.data[i].T;  kolXY++;}
		   }
                break;
      case 'D' :
                for(i=0;i<struc.KolDan;i++)
                   if(struc.data[i].pok_kach[ind]=='0'&&struc.data[i].pok_kach[ind1]=='0'){
                      if(type_p)
                       {
                          if(p2<=struc.data[i].P&&struc.data[i].P<=p1)
                           {vsX[kolXY]= struc.data[i].P;vsY[kolXY]= struc.data[i].D;  kolXY++;}
                       }
                           else if(p1<=struc.data[i].H&&struc.data[i].H<=p2)
                                      {vsX[kolXY]= struc.data[i].H;  vsY[kolXY]= struc.data[i].D;  kolXY++;}
		   }
                break;
      case 'd' :
                for(i=0;i<struc.KolDan;i++)
                   if(struc.data[i].pok_kach[4]=='0'&&struc.data[i].pok_kach[5]=='0'&&struc.data[i].pok_kach[ind1]=='0'){
                      {
                          preobrUVtoDF(struc.data[i].dd, struc.data[i].ff,vs_i,vs_i1);
                          vsY[kolXY]=vs_i;
                         if(type_p) {if(p2<=struc.data[i].P&&struc.data[i].P<=p1) {vsX[kolXY]= struc.data[i].P; kolXY++;}}
                            else if(p1<=struc.data[i].H&&struc.data[i].H<=p2) {vsX[kolXY]= struc.data[i].H; kolXY++;}
                      }
		   }
                break;
      case 'f' :
                for(i=0;i<struc.KolDan;i++)
                   if(struc.data[i].pok_kach[4]=='0'&&struc.data[i].pok_kach[5]=='0'&&struc.data[i].pok_kach[ind1]=='0'){
                      {
                          preobrUVtoDF(struc.data[i].dd, struc.data[i].ff,vs_i,vs_i1);
                          vsY[kolXY]=vs_i1;
                         if(type_p) {if(p2<=struc.data[i].P&&struc.data[i].P<=p1) {vsX[kolXY]= struc.data[i].P; kolXY++;}}
                            else if(p1<=struc.data[i].H&&struc.data[i].H<=p2) {vsX[kolXY]= struc.data[i].H; kolXY++;}
                      }
		   }
                break;
      case 's' :
                for(i=0;i<struc.KolDan;i++)
                   if(struc.data[i].pok_kach[4]=='0'&&struc.data[i].pok_kach[5]=='0'&&struc.data[i].pok_kach[ind1]=='0'){
                      {
                          preobrUVtoDF(struc.data[i].dd, struc.data[i].ff,vs_i,vs_i1);
                          vsY[kolXY]=vs_i;
                          vsZ[kolXY]=vs_i1;
                         if(type_p) {if(p2<=struc.data[i].P&&struc.data[i].P<=p1) {vsX[kolXY]= struc.data[i].P; kolXY++;}}
                            else if(p1<=struc.data[i].H&&struc.data[i].H<=p2) {vsX[kolXY]= struc.data[i].H; kolXY++;}
                      }
		   }
                break;
      default : ret_val=false; goto m1;
     }

if(type_p)  {if(!oprUrPoP(p2,ur))  {ret_val=false; goto m1;}}
 else if(!oprUrPoH(p2,ur))  {ret_val=false; goto m1;}
vsX[kolXY]=p2;
 switch (name)
    {
      case 'P' :   if(ur.pok_kach[0]=='0') vsY[kolXY]= ur.P;   break;
      case 'H' :   if(ur.pok_kach[1]=='0') vsY[kolXY]= ur.H;   break;
      case 'T' :   if(ur.pok_kach[2]=='0') vsY[kolXY]= ur.T;   break;
      case 'D' :  if(ur.pok_kach[3]=='0') vsY[kolXY]= ur.D;   break;
      case 'd' :   if(ur.pok_kach[4]=='0'&&ur.pok_kach[5]=='0')
                         {
                            preobrUVtoDF(ur.dd, ur.ff,vs_i,vs_i1);
                            vsY[kolXY]= vs_i;
                        }
                      break;
      case 'f' :   if(ur.pok_kach[4]=='0'&&ur.pok_kach[5]=='0')
                         {
                            preobrUVtoDF(ur.dd, ur.ff,vs_i,vs_i1);
                            vsY[kolXY]= vs_i1;
                        }
                      break;
      case 's' :   if(ur.pok_kach[4]=='0'&&ur.pok_kach[5]=='0')
                         {
                            vsX[kolXY]=ur.dd;
                            vsZ[kolXY]= ur.ff;
                        }
                      break;
      default : ret_val=false; goto m1;
     }
kolXY++;
if(kolXY<2) {ret_val=false; goto m1;}

if(!integrTrapec(vsY,vsX,kolXY,znach)) {ret_val=false; goto m1;}
znach/=(p2-p1);
if(name=='s')
   {
       if(!integrTrapec(vsZ,vsX,kolXY,znach1)) {ret_val=false; goto m1;}
       znach1/=(p2-p1);
   }
 switch (name)
    {
      case 'P' :
      case 'H' :
      case 'd' :
      case 'f' :  break;
      case 'T' :
      case 'D' :  znach*=per;  break;
      case 's' :
                      preobrUVtoDF(ftoi_norm(znach), ftoi_norm(znach1),vs_i,vs_i1);
                      znach=vs_i;
                      znach1=vs_i1;
                      break;
      default : ret_val=false; goto m1;
     }
ret_val=true;

m1:
if(vsX) {delete []vsX; vsX=0;}
if(vsY) {delete []vsY; vsY=0;}
if(vsZ) {delete []vsZ; vsZ=0;}
return ret_val;
}

bool TAero::oprTsost(double ival_lev,double &val)
{
double t_nach,td_nach,p_nach;
double p_kondens,tet,tpot,t_kondens,h_iv;

oprPprizInv(p_nach,t_nach,td_nach,h_iv);
if(ival_lev>p_nach) return false;
if(!::oprPkondens(t_nach,td_nach,p_nach,p_kondens,t_kondens)) return false;
if(!oprTet(p_nach,t_nach,tet)) return false;
if(!oprTPot(p_kondens,t_kondens,tpot)) return false;
if(p_kondens<ival_lev) return oprTPoTet(ival_lev,tet,val);
  else return oprTPoTPot(ival_lev,tpot,val);
}

bool TAero::oprPkondens(double &p_kondens,double &t_kondens)
{
double t_nach,td_nach,p_nach,h_nach;

oprPprizInv(p_nach,t_nach,td_nach,h_nach);
if(!::oprPkondens(t_nach,td_nach,p_nach,p_kondens,t_kondens)) return false;
 else return true;
}

bool TAero::getUrz(Uroven &ur)
{
TAeroDataAll struc;
int kol,i_nach=0;

if(!is_data) return false;
struc=aeroDa;
kol=struc.KolDan;
if(kol<2) return false;
if(kol>199) return false;

for(i_nach=0;i_nach<kol;i_nach++) if(struc.data[i_nach].pok_kach[6]=='1') break;
if(i_nach==kol) return false;
ur=struc.data[i_nach];
return true;
}

bool TAero::getPz(double &p)
{
p=0;
Uroven ur;
if(!getUrz(ur)) return false;
if(ur.pok_kach[0]!='0') return false;
p=ur.P;
return true;
}

bool TAero::getTz(double &t)
{
t=0;
Uroven ur;
if(!getUrz(ur)) return false;
if(ur.pok_kach[2]!='0') return false;
t=ur.T*.1;
return true;
}

bool TAero::oprPprizInv(double &p_inv,double &t_inv,double &td_inv,double &h_inv)
{
TAeroDataAll struc;
int i,kol,i_nach=0;
bool res=true;

if(!is_data) return false;
struc=aeroDa;
kol=struc.KolDan;
if(kol<2) return false;
if(kol>199) return false;

for(i_nach=0;i_nach<kol;i_nach++) if(struc.data[i_nach].pok_kach[6]=='1') break;
if(i_nach==kol) i_nach=0;
for(i=i_nach;i<kol-1;i++)
  {
   if(struc.data[i].pok_kach[2]=='0'&&struc.data[i+1].pok_kach[2]=='0'){
      if(struc.data[i+1].T<=struc.data[i].T) break;
        else continue;
   }
  }
if(i==i_nach) {i=0;res=false;}
if(i==kol-1) {i=0;res=false;}
if(struc.data[i].pok_kach[0]!='0'||struc.data[i].pok_kach[3]!='0') {i=1;res=false;}
t_inv=struc.data[i].T*0.1;
td_inv=t_inv-struc.data[i].D*0.1;
p_inv=struc.data[i].P;
h_inv=struc.data[i].H;
return res;
}

bool TAero::oprPsostRaznAll(double &p, double &deltaT, double ival_lev_end, double ival_lev_start, int step)
{
int i_lev;
int kol=0;
Uroven ur;
double p_kondens,t_kondens,t_vsSost,max_razn=-99999.,i_max_razn=0;

if(ival_lev_start==0.){
   if(!oprPkondens(p_kondens,t_kondens)) return false;
   ival_lev_start=p_kondens;
}


for(i_lev=ftoi_norm(ival_lev_start);i_lev>ftoi_norm(ival_lev_end)+1;i_lev-=step)
    {
      if(!oprTsost(i_lev,t_vsSost)) continue;
      if(!getUr(ftoi_norm(i_lev*10.),ur)) continue;
      if(ur.pok_kach[2]!='0') continue;
      if((t_vsSost-ur.T*.1)>max_razn) {i_max_razn=i_lev; max_razn=t_vsSost-ur.T*.1;}
      kol++;
    }
if(kol==0) return false;
p=(double)i_max_razn;
deltaT=max_razn;
return true;
}

bool TAero::getUrOsobyT(Uroven *&ur,int &kolUr)
{

int kolDan=0;
int i;
if(!is_data) return false;

kolUr=aeroDa.KolDan;
if(kolUr<1) return false;
ur=new Uroven[kolUr];
if(!ur) return false;
for(i=0;i<kolUr;i++)
{
if(aeroDa.data[i].pok_kach[6]=='2'&&aeroDa.data[i].pok_kach[2]=='0')
    {
       memcpy((void*)&ur[kolDan],(void*)&(aeroDa.data[i]),sizeof(Uroven));
       kolDan++;
   }
}
if(kolDan>0) {kolUr=kolDan; return true;}
return false;
}

bool TAero::getUrOsobyV(Uroven *&ur,int &kolUr)
{

int kolDan=0;
int i;
if(!is_data) return false;

kolUr=aeroDa.KolDan;
if(kolUr<1) return false;
ur=new Uroven[kolUr];
if(!ur) return false;
for(i=0;i<kolUr;i++)
{
if(aeroDa.data[i].pok_kach[6]=='3'&&aeroDa.data[i].pok_kach[4]=='0'
    &&aeroDa.data[i].pok_kach[5]=='0')
    {
       memcpy((void*)&ur[kolDan],(void*)&(aeroDa.data[i]),sizeof(Uroven));
       kolDan++;
   }
}
if(kolDan>0) {kolUr=kolDan; return true;}
return false;
}

/*void TAero::setPoTAeroDataBasa(TAeroDataBasa &aero_vs_vs)
{
TAeroDataAll aeroDa_vs; memcpy((void*)&aeroDb,(void*)&(aero_vs_vs),sizeof(TAeroDataBasa));
Preobr(&aeroDb, &aeroDa_vs);
Opr_H(aeroDa_vs,&aeroDa);
is_data=true;
}
*/
void TAero::setPoTAeroDataAll(TAeroDataAll &aero_vs_vs)
{
memcpy((void*)&aeroDa,(void*)&(aero_vs_vs),sizeof(TAeroDataAll));
is_data=true;
}

bool TAero::oprHmaxV( double &h, double &v, double ival_lev_end, double ival_lev_start, int step)
{
int i_lev;
int kol=0;
Uroven ur;
double max_v=-99999.,h_max_v=0;


for(i_lev=ftoi_norm(ival_lev_start);i_lev>ftoi_norm(ival_lev_end)+1;i_lev-=step)
    {
      if(!getUr(ftoi_norm(i_lev*10.),ur)) continue;
      if(ur.pok_kach[4]!='0'||ur.pok_kach[5]!='0') continue;
      if((ur.ff)>max_v) {max_v=ur.ff; h_max_v=ur.H;}
      kol++;
    }
if(kol==0) return false;
h=(double)h_max_v/1000.;
v=(double)(max_v*3.6);
return true;
}

bool TAero::oprIlina( double &p, double &v, double ival_lev_end, double ival_lev_start, int step)
{
int i_lev;
int kol=0;
Uroven ur;
double max_v=-99999.,p_max_v=0;

for(i_lev=ftoi_norm(ival_lev_start);i_lev>ftoi_norm(ival_lev_end)+1;i_lev-=step)
    {
      if(!getUr(ftoi_norm(i_lev*10.),ur)) continue;
      if(ur.pok_kach[4]!='0'||ur.pok_kach[5]!='0') continue;
      if((ur.ff)>max_v) {max_v=ur.ff; p_max_v=ur.P;}
      kol++;
    }
if(kol==0) return false;
p=(double)p_max_v;
v=(double)max_v;
return true;
}

bool TAero::oprH1(double &h1, double &h2, double ival_lev_end, double ival_lev_start, int step)
{
int i_lev;
int i_levl=0;
int kol=0;
double /*h,*/ v;
Uroven ur;
double max_v=-99999.;//,h_max_v=0;
double V_V,V_N,H_V,H_N;
for(i_lev=ftoi_norm(ival_lev_start);i_lev>ftoi_norm(ival_lev_end)+1;i_lev-=step)
    {
      if(!getUr(ftoi_norm(i_lev*10.),ur)) continue;
      if(ur.pok_kach[4]!='0'||ur.pok_kach[5]!='0') continue;
      if((ur.ff)>max_v) {max_v=ur.ff; /*h_max_v=ur.H;*/i_levl=i_lev;}
      kol++;
    }
if(kol==0) return false;

//h=(double)h_max_v;
v=(double)max_v;

for(i_lev=i_levl;i_lev<ftoi_norm(ival_lev_start )+1;i_lev+=step)
    {
      if(!getUr(ftoi_norm(i_lev*10.),ur)) continue;
      if(ur.pok_kach[4]!='0'||ur.pok_kach[5]!='0') continue;
      if((ur.ff)<v*0.8) {break;}
     }
if(ur.pok_kach[4]!='0'||ur.pok_kach[5]!='0') return false;

H_N=ur.H/1000.;
V_N=ur.ff;
if(getUr(ftoi_norm((i_lev-step)*10.),ur))
  {
  if(ur.pok_kach[4]!='0'||ur.pok_kach[5]!='0') h1=H_N;
  else
    {
     H_V=ur.H/1000.;
     V_V=ur.ff;
     h1=H_N+((0.8*v-V_N)/(V_V-V_N))*(H_V-H_N);
    }
  }

for(i_lev=i_levl;i_lev>ftoi_norm(ival_lev_end);i_lev-=step)
    {
      if(!getUr(ftoi_norm(i_lev*10.),ur)) continue;
      if(ur.pok_kach[4]!='0'||ur.pok_kach[5]!='0') continue;
      if((ur.ff)<v*0.8) {break;}
     }
if(ur.pok_kach[4]!='0'||ur.pok_kach[5]!='0') return false;

H_V=ur.H/1000.;
V_V=ur.ff;
if(getUr(ftoi_norm((i_lev+step)*10.),ur))
  {
  if(ur.pok_kach[4]!='0'||ur.pok_kach[5]!='0') h2=H_V;
  else
    {
     H_N=ur.H/1000.;
     V_N=ur.ff;
     h2=H_N+((0.8*v-V_N)/(V_V-V_N))*(H_V-H_N);
    }
}

return true;
}

// Определение границ КНС
// Входные данные : aeroDa
// Выходные данные :
// int kol - количество КНС
// double *p_start- массив нижних границ КНС
// double *p_end-массив верхних границ КНС
// Память для *p_start и *p_end выдляется в функции и должна очищаться снаружи
bool  TAero::oprGranKNS(int &kol, double *&p_kns)
{
TAeroDataAll struc;
Uroven ur;
double vsSost, t, h_vs;
int i;
bool ret_val=false;

p_kns=0;
if(!is_data) return false;
memcpy((void *)&struc,(void *)&aeroDa,sizeof(TAeroDataAll));
for(i=0;i<aeroDa.KolDan;i++) struc.data[i].pok_kach[2]='1';
for(i=0;i<aeroDa.KolDan;i++){
   if(aeroDa.data[i].pok_kach[0]=='0'&&aeroDa.data[i].pok_kach[1]=='0'&&aeroDa.data[i].pok_kach[2]=='0'){
    if(!oprTsost((double)aeroDa.data[i].P,vsSost)) continue;
      else {struc.data[i].T=ftoi_norm(vsSost*10.)-struc.data[i].T; struc.data[i].pok_kach[2]='0';}
   }
}

kol=0;
for(i=0;i<aeroDa.KolDan-1;i++){
   if(struc.data[i].pok_kach[2]=='0'&&struc.data[i+1].pok_kach[2]=='0'){
       if(struc.data[i].T*struc.data[i+1].T<=0){
         if(struc.data[i].T*struc.data[i+1].T<0) { kol++;struc.data[i].pok_kach[6]='8';}
             else  if((struc.data[i].T==0&&struc.data[i+1].T>0)||(struc.data[i].T>0&&struc.data[i+1].T==0))
                        {kol++; struc.data[i].pok_kach[6]='8';}
       }
   }
}
for(i=aeroDa.KolDan-2;i>-1;i--){
   if(struc.data[i].pok_kach[2]=='0'&&struc.data[i+1].pok_kach[2]=='0'){
        if(struc.data[i].T>=0){
          if(struc.data[i+1].T>0) { kol++;struc.data[i+1].pok_kach[6]='9'; break;}
             else break;
	}
   }
}

if(kol%2!=0) {ret_val=false; goto m1;}
if(kol==0) {ret_val=true; goto m1;}
p_kns=new double[kol];
if(p_kns==0) {ret_val=false; goto m1;}

kol=0;
for(i=0;i<aeroDa.KolDan-1;i++)
   {
      if(struc.data[i].pok_kach[6]=='8')
        {
           t=double(struc.data[i].T)/double(struc.data[i].T-struc.data[i+1].T);
           h_vs=(1.-t)*struc.data[i].H+t*struc.data[i+1].H;
           if(!oprUrPoH(ftoi_norm(h_vs),ur)) {ret_val=false; goto m1;}
           if(ur.pok_kach[0]!='0') {ret_val=false; goto m1;}
           p_kns[kol]=ur.P;
           kol++;
        }
      if(struc.data[i].pok_kach[6]=='9')
         {
            p_kns[kol]=struc.data[i].P;
             kol++;
         }
   }
ret_val=true;

m1:
if(!ret_val)
 {
   if(p_kns!=0) {delete []p_kns;p_kns=0;}
    kol=0;
 }
return ret_val;
}

bool  TAero::oprSkorDpKNS(int &kol, double *&w, double *&dp)
{
Uroven ur;
double *p_vs, *t_vs;
double *p_kns;
double step, t_sr;
int i,  j,  kol_vs, kol_kns;
bool ret_val=false;

kol=0;
w=0;
dp=0;
p_vs=0;
t_vs=0;
p_kns=0;
kol_vs=100;
if(!is_data) return false;

if(!oprGranKNS(kol_kns,p_kns) ) {ret_val=false; goto m1;}
if(kol_kns==0) {ret_val=true; goto m1;}
kol=kol_kns/2;
p_vs=new double[kol_vs];
if(p_vs==0) {ret_val=false; goto m1;}
t_vs=new double[kol_vs];
if(t_vs==0) {ret_val=false; goto m1;}
w=new double[kol];
if(w==0) {ret_val=false; goto m1;}
dp=new double[kol];
if(dp==0) {ret_val=false; goto m1;}

for(i=0;i<kol_kns;i+=2)
 {
    step=(p_kns[i+1]-p_kns[i])/kol_vs;
    if(step==0) {ret_val=false; goto m1;}
    for(j=0;j<kol_vs;j++)
     {
        p_vs[j]=p_kns[i]+step*j;
        if(!oprTsost(p_vs[j],t_vs[j])) {ret_val=false; goto m1;}
        if(!oprUrPoP(p_vs[j],ur)) {ret_val=false; goto m1;}
        if(ur.pok_kach[2]!='0') {ret_val=false; goto m1;}
        t_vs[j]=t_vs[j]-ur.T*0.1;
     }
     if(!integrTrapec(t_vs,p_vs,kol_vs,t_sr)) {ret_val=false; goto m1;}
     t_sr/=(p_kns[i+1]-p_kns[i]);
     w[i/2]=sqrt(287.*(t_sr+273.15)*log(p_kns[i]/p_kns[i+1]));
     dp[i/2]=p_kns[i]-p_kns[i+1];
 }
ret_val=true;

m1:
if(!ret_val)
{
   if(dp) {delete []dp;dp=0;}
   if(w) {delete []w;w=0;}
   kol=0;
 }
if(p_vs) {delete []p_vs;p_vs=0;}
if(t_vs) {delete []t_vs;t_vs=0;}
if(p_kns) {delete []p_kns;p_kns=0;}
return ret_val;
}

bool  TAero::oprSrKonvSkor(double &w_konv, double p1, double p2)
{
int i,  kol;
double  *w, *dp, m;
bool ret_val=false;

if(!is_data) return false;
if(p1<p2) return false;
w_konv=0.;
w=0;
dp=0;
if(!oprSkorDpKNS(kol,w,dp)) {ret_val=false; goto m1;}
m=0.;
for(i=0;i<kol;i++) m+=dp[i];
if(m<100.) {ret_val=true; goto m1;}

for(i=0;i<kol;i++) w_konv+=(w[i]*dp[i]);
w_konv=w_konv*0.3/(p1-p2);
ret_val=true;

m1:
if(dp) {delete []dp;dp=0;}
if(w) {delete []w;w=0; }
return ret_val;
}

bool  TAero::oprIntLivnOrl(double &intens)
{
int i;
Uroven ur;
double  w_konv, p1, p2, t_sr, t_vs, p_sr, q_850;
bool ret_val=false;

intens=0.;
if(!is_data) return false;

for(i=0;i<aeroDa.KolDan;i++)
   if(aeroDa.data[i].pok_kach[6]=='1'&&
       aeroDa.data[i].pok_kach[0]=='0'&&
       aeroDa.data[i].pok_kach[2]=='0')  break;
if(i==aeroDa.KolDan) {ret_val=false; goto m1;}
p1=aeroDa.data[i].P;
for(i=0;i<aeroDa.KolDan;i++)
   if(aeroDa.data[i].pok_kach[6]=='4'&&
       aeroDa.data[i].pok_kach[0]=='0'&&
       aeroDa.data[i].pok_kach[2]=='0')  break;
if(i==aeroDa.KolDan) {ret_val=false; goto m1;}
p2=aeroDa.data[i].P;
if(p1<p2) {ret_val=false; goto m1;}

if(!oprUrPoP(850.,ur))  {ret_val=false; goto m1;}
if(ur.pok_kach[2]!='0'||ur.pok_kach[3]!='0') {ret_val=false; goto m1;}
if(!oprSPoTP(850.,(ur.T-ur.D)*0.1,q_850)) {ret_val=false; goto m1;}

if(!oprSrParamPoPH('T', t_sr,t_vs,p1, p2,true)) {ret_val=false; goto m1;}
if(!oprSrKonvSkor(w_konv,p1,p2)) {ret_val=false; goto m1;}

p_sr=(p1+p2)*0.5;
intens=p_sr/(t_sr+273.15)/287.*q_850*w_konv*3600.;
if(intens<5.)  intens=0.;
ret_val=true;

m1:
return ret_val;
}

bool TAero::getTropo(Uroven &ur)
{
int kolDan=0;
int i;
if(!is_data) return false;
ur.P=9999;
ur.H=9999;
ur.T=9999;
ur.D=9999;
ur.ff=9999;
ur.dd=9999;
strcpy(ur.pok_kach,"1111111");
kolDan=aeroDa.KolDan;
if(kolDan<1) return false;
if(kolDan>200) return false;

for(i=0;i<kolDan;i++)
if(aeroDa.data[i].pok_kach[6]=='4')
     { memcpy((void*)&ur,(void*)&(aeroDa.data[i]),sizeof(Uroven)); return true;}

return false;
}

int TAero::oprZnakEnergy(double p)
{
double val;
Uroven ur;
double t_nach,td_nach,p_nach;
double h_iv;

oprPprizInv(p_nach,t_nach,td_nach,h_iv);
if(p>p_nach) p=p_nach-5.;
if(!oprTsost(p,val)) return 0;
if(!oprUrPoP(p, ur)) return 0;
if(ur.T*0.1==val) return 0;
if(ur.T*0.1<val) return 1;
 else return -1;
}


}