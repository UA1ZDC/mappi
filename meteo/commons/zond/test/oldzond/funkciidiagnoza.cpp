#include "funkciidiagnoza.h"



bool oprTsm(double p_nach,double p_kon, double t_nach, double &val)
{
if(!oprTPot(p_nach,t_nach,val)) return false;
if(!oprTPoTPot(p_kon,val,val)) return false;
return true;
}

bool oprTPot(double p, double t, double &val)
{
double s,cp,l;
if(t<-60.||t>500.) return false;
if(p<100.||p>1100.) return false;

s=0.622*EP(t)/p;
cp = 1004.7*(1.0-s)+1846.0*s;
l=(3.14821-0.00237*(t+273.15))*1.e6;
val=t+273.15+s*l/cp;
val*=pow(1000./p,0.286);
val-=273.15;
return true;
}

bool oprTet(double p, double t, double &val)
{
if(t<-60.||t>500.) return false;
if(p<100.||p>1100.) return false;

val=t+273.15;
val*=pow(1000./p,0.286);
val-=273.15;
return true;
}

double EP(double t)
{
return   exp ((10.79574*(1.0-273.16/(t+273.15))-
         5.02800*log((t+273.15)/273.16)*0.4343+
         0.000150475*(1.0-((t+273.15)/273.16-1.0))+
         0.0000001745*(58823.38332*(1.0-273.16/(t+273.15))-1.0)+
         0.78614)*log(10.0));
}

bool oprSPoTP(double p,double t, double &res)
{
 if(t<-60.||t>50.) return false;
 if(p<100.||p>1100.) return false;
res=0.622*EP(t)/p;
return true;
}

bool oprTPoSP(double p,double s, double &res)
{
double t,t1,t2,s1,s2,ss;
if(s<0.) return false;
if(p<100.||p>1100.) return false;
t1=-60.;
t2=50.;

if(!oprSPoTP(p,t1,s1)) return false;
if(!oprSPoTP(p,t2,s2)) return false;
ss=s2;
if((s1-s)*(s2-s)>0.) return false;
t=t1;
while(fabs(s-ss)>1.e-6)
  {
    t=(t1+t2)*0.5;
    if(!oprSPoTP(p,t,ss)) return false;
    if((s-ss)*(s1-ss)>0.) t2=t; else t1=t;
  }
res=t;
return true;
}

bool oprTPoTet(double p,double tet, double &res)
{
 if(tet<-60.||tet>350.) return false;
 if(p<100.||p>1100.) return false;

res=(tet+273.15)/pow(1000./p,.286)-273.15;
return true;
}

bool oprTPoTPot(double P,double tet, double &res)
{
 double t, Cp, Sm1,kap,l,Tet,ss;
 double t1,t2,tt,tt1,tet0,tet1;

t1=-200.+273.15;
t2=200.+273.15;
kap=0.286;
ss=tet+273.15;
res=0;

for(t=t1;t<t2;t++)
    {
      tt=t-1.;
      l=(3.14821-0.00237*t)*1.e6;
      Sm1=0.622*EP(t-273.15)/P;
      Cp=1004.7*(1.0-Sm1)+1846.0*Sm1;
      tet0=t +Sm1*l/Cp;
      tet0*=pow(1000./P,kap);
      l=(3.14821-0.00237*tt)*1.e6;
      Sm1=0.622*EP(tt-273.15)/P;
      Cp=1004.7*(1.0-Sm1)+1846.0*Sm1;
      tet1=tt+Sm1*l/Cp;
      tet1*=pow(1000./P,kap);
      if((tet0-ss)*(tet1-ss)>0.) continue;
      Tet=tet0;
      while(fabs(Tet-ss)>1.e-6)
        {
         tt1=(tt+t)*0.5;
         l=(3.14821-0.00237*tt1)*1.e6;
         Sm1 = (0.622*EP(tt1-273.15)/P);
         Cp = 1004.7*(1.0-Sm1)+1846.0*Sm1;
         Tet = tt1 +Sm1*l/Cp;
         Tet*=pow(1000./P,kap);
         if((tet0-ss)*(Tet-ss)>0.) {t=tt1; tet0=Tet;} else {tt=tt1;}
        }
     res=tt1-273.15;
     break;
     }
if(t2<=t) return false;
else return true;
}

bool oprPkondens(double t, double td, double p, double &res_p, double &res_t)
{
double tet,p_vs,s;
double tTet,tPot,tTet1,tPot1,tTet2,tPot2,p1,p2;

res_p=0.;
res_t=0.;
if(!oprTet(p,t,tet)) return false;
if(!oprSPoTP(p,td,s)) return false;

p1=1000.;
p2=100.;
if(!oprTPoTet(p1,tet,tTet1)) return false;
if(!oprTPoTet(p2,tet,tTet2)) return false;
if(!oprTPoSP(p1,s,tPot1)) return false;
if(!oprTPoSP(p2,s,tPot2)) return false;
if((tTet1-tPot1)*(tTet2-tPot2)>0.) return false;
tTet=tTet1;
tPot=tPot1;
while(fabs(tTet-tPot)>1.e-6)
  {
    p_vs=(p1+p2)*0.5;
    if(!oprTPoTet(p_vs,tet,tTet)) return false;
    if(!oprTPoSP(p_vs,s,tPot)) return false;
    if((tTet1-tPot1)*(tTet-tPot)>0.) p1=p_vs; else p2=p_vs;
  }
res_p=p_vs;
res_t=tTet;
return true;
}

