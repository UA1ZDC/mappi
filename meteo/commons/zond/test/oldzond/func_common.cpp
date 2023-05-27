#include "func_common.h"
#include <qstringlist.h>
#include <qregexp.h>
////////////////////////////////////////////////////////////////



int ftoi_norm(float d)
{ if(d<0) return int(d-0.5); else return int(d+0.5);
}
//------------------------------------------------------------
////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
void SetBit1(int ibit,BYTE *b1)
{
	switch(ibit)
	{
		case 0: { (*b1)=(*b1)|1;break; }
		case 1: { (*b1)=(*b1)|2;break; }
		case 2: { (*b1)=(*b1)|4;break; }
		case 3: { (*b1)=(*b1)|8;break; }
		case 4: { (*b1)=(*b1)|16;break; }
		case 5: { (*b1)=(*b1)|32;break; }
		case 6: { (*b1)=(*b1)|64;break; }
		case 7: { (*b1)=(*b1)|128; }
	}
}

int GetBit1(int ibit,BYTE *b1)
{
	BYTE b;
	switch(ibit)
	{
		case 0: { b=(*b1)&1;break; }
		case 1: { b=(*b1)&2;break; }
		case 2: { b=(*b1)&4;break; }
		case 3: { b=(*b1)&8;break; }
		case 4: { b=(*b1)&16;break; }
		case 5: { b=(*b1)&32;break; }
		case 6: { b=(*b1)&64;break; }
		case 7: { b=(*b1)&128; }
	}
	if(b) return 1;
		else return 0;
}

int max(int i1,int i2)
{
	return ((i1>i2)?i1:i2);
}

int min(int i1,int i2)
{
	return ((i1<i2)?i1:i2);
}

double min(double i1,double i2)
{
	return ((i1<i2)?i1:i2);
}

double max(double i1,double i2)
{
	return ((i1>i2)?i1:i2);
}

double MPiToPi(double d)
{
while(d<=-Pi) d+=Pix2;
while(d>Pi) d-=Pix2;
return d;
}

double M180To180(double d)
{
while(d<=-180.) d+=360.;
while(d>180.) d-=360.;
return d;
}

double Ot0To360(double d)
{
while(d<-0.) d+=360.;
while(d>=360.) d-=360.;
return d;
}




void sort2(double *mas, int *num, int n)
{
int predel=7;
int i,j,k,n1=n-1;
double a, temp;
int nvs, ntemp;

if(n<predel)
  {
   for(j=1;j<n;j++)
     {
      a=mas[j];
      nvs=num[j];
      for(i=j-1;i>-1;i--)
        {
         if(mas[i]<=a) break;
         mas[i+1]=mas[i];
         num[i+1]=num[i];
        }
      mas[i+1]=a;
      num[i+1]=nvs;
     }
   return;
  }

k=n>>1;
temp=mas[k];
ntemp=num[k];
mas[k]=mas[1];
num[k]=num[1];
mas[1]=temp;
num[1]=ntemp;
if(mas[0]>mas[n1])
  {
   temp=mas[n1];
   ntemp=num[n1];
   mas[n1]=mas[0];
   num[n1]=num[0];
   mas[0]=temp;
   num[0]=ntemp;
  }
if(mas[1]>mas[n1])
  {
   temp=mas[n1];
   ntemp=num[n1];
   mas[n1]=mas[1];
   num[n1]=num[1];
   mas[1]=temp;
   num[1]=ntemp;
  }
if(mas[0]>mas[1])
  {
   temp=mas[1];
   ntemp=num[1];
   mas[1]=mas[0];
   num[1]=num[0];
   mas[0]=temp;
   num[0]=ntemp;
  }
i=2;
j=n-2;
a=mas[1];
nvs=num[1];
for(;;)
  {
   while((mas[i]<=a)&&(i<n1)) i++;
   while((mas[j]>a)&&(j>1)) j--;
   if(j<=i) break;
   temp=mas[i];
   ntemp=num[i];
   mas[i]=mas[j];
   num[i]=num[j];
   mas[j]=temp;
   num[j]=ntemp;
  }
mas[1]=mas[j];
num[1]=num[j];
mas[j]=a;
num[j]=nvs;

if((n-i)>j)
  {
   if(j>1) sort2(mas,num,j);
   if((n-i)>1) sort2(&mas[i],&num[i],n-i);
  }
 else
   {
    if((n-i)>1) sort2(&mas[i],&num[i],n-i);
    if(j>1) sort2(mas,num,j);
   }
}

void sort1(double *mas,int n)
{
int predel=7;
int i,j,k,n1=n-1;
double a, temp;

if(n<predel)
  {
   for(j=1;j<n;j++)
     {
      a=mas[j];
      for(i=j-1;i>-1;i--)
        {
         if(mas[i]<=a) break;
         mas[i+1]=mas[i];
        }
      mas[i+1]=a;
     }
   return;
  }

k=n>>1;
temp=mas[k];
mas[k]=mas[1];
mas[1]=temp;
if(mas[0]>mas[n1])
  {
   temp=mas[n1];
   mas[n1]=mas[0];
   mas[0]=temp;
  }
if(mas[1]>mas[n1])
  {
   temp=mas[n1];
   mas[n1]=mas[1];
   mas[1]=temp;
  }
if(mas[0]>mas[1])
  {
   temp=mas[1];
   mas[1]=mas[0];
   mas[0]=temp;
  }
i=2;
j=n-2;
a=mas[1];
for(;;)
  {
   while((mas[i]<=a)&&(i<n1)) i++;
   while((mas[j]>a)&&(j>1)) j--;
   if(j<=i) break;
   temp=mas[i];
   mas[i]=mas[j];
   mas[j]=temp;
  }
mas[1]=mas[j];
mas[j]=a;

if((n-i)>j)
  {
   if(j>1) sort1(mas,j);
   if((n-i)>1) sort1(&mas[i],n-i);
  }
 else
   {
    if((n-i)>1) sort1(&mas[i],n-i);
    if(j>1) sort1(mas,j);
   }
}

int searh_inf(double a, double *mas, int n)
{
int niz=0, verh=n-1, vs;

if(n<2) return -3;
if(a<mas[niz]) return -1;
if(a==mas[niz]) return niz;
if(a>mas[verh]) return -2;
if(a==mas[verh]) return n-2;

while((verh-niz)!=1)
  {
   vs=niz+((verh-niz)>>1);
   if(a<mas[vs]) verh=vs; else niz=vs;
  }
return niz;
}


bool getMasFromString(QString str, double **data, int *kol)
{
QStringList listDep;
bool ret_val=true;
int i=0;

*data=0;
*kol=0;
if(str.isNull()) {ret_val=false; goto m1;}
listDep.clear();
listDep=str.split(QRegExp("[;]"));
*kol=listDep.count();
if(*kol<=0) {ret_val=false; goto m1;}
*data=new double[*kol];
if(!(*data)) {ret_val=false; goto m1;}
while(!listDep[i].isNull())
      {
          (*data)[i]=listDep[i].toDouble(&ret_val);
           if(!ret_val) goto m1;
          i++;
      }
m1:
if(!ret_val)
    {
    *kol=0;
     if(*data) {delete [](*data);*data=0;}
     }
return ret_val;
}

bool getMasFromString(QString str, int **data, int *kol)
{
QStringList listDep;
bool ret_val=true;
int i=0;

*data=0;
*kol=0;
if(str.isNull()) {ret_val=false; goto m1;}
listDep.clear();
listDep=str.split(QRegExp("[;]"));
*kol=listDep.count();
if(*kol<=0) {ret_val=false; goto m1;}
*data=new int[*kol];
if(!(*data)) {ret_val=false; goto m1;}
while(!listDep[i].isNull())
      {
          (*data)[i]=listDep[i].toInt(&ret_val);
           if(!ret_val) goto m1;
          i++;
      }
m1:
if(!ret_val)
    {
    *kol=0;
     if(*data) {delete [](*data);*data=0;}
     }
return ret_val;
}


bool linInterpol(double x1,double y1,double x2,double y2,double x,double &y)
{
double dx=x2-x1,dy=y2-y1;
y=0.;
if(dx==0) return false;
if((x1<=x&&x<x2)||(x1<x&&x<=x2)||(x2<=x&&x<x1)||(x2<x&&x<=x1))
   {
      y=y1+dy/dx*(x-x1);
      return true;
  }
 else return false;
}

int kolChiselVString(QString &str)
{
int kol=0;
bool ok;
QString wstr;
QStringList listDep;

listDep.clear();
listDep=str.split(QRegExp("[,; ]"));
  while(!listDep.empty())
      {
        wstr= listDep.last();
        wstr.toDouble(&ok);
        if(ok) kol++;
        listDep.pop_back();
      }
return kol;
}

bool getChiselIsString(QString &str, double *&val, int &kol )
{
kol=0;
bool ok;
QString wstr;
QStringList listDep;
double vs;

kol=kolChiselVString(str);
if(kol<1) return false;
if (!val) val=new double [kol];
if (!val) return false;
 kol=0;
listDep.clear();
listDep=str.split(QRegExp("[,; ]"));
  while(!listDep.empty())
      {
        wstr= listDep.last();
        vs=wstr.toDouble(&ok);
        if(ok) {val[kol]=vs; kol++;}
        listDep.pop_back();
      }
return true;
}
bool getChiselIsString(QString &str, int *&val, int &kol )
{
kol=0;
bool ok;
QString wstr;
QStringList listDep;
int vs;

kol=kolChiselVString(str);
if(kol<1) return false;
if (!val) val=new int [kol];
 if (!val) return false;
 kol=0;
listDep.clear();
listDep=str.split(QRegExp("[,; ]"));
  while(!listDep.empty())
      {
        wstr= listDep.last();
        vs=wstr.toInt(&ok);
        if(ok) {val[kol]=vs; kol++;}
        listDep.pop_back();
      }
return true;
}

bool getStringChislPoUppercase(QString &str, QString *&s_val, int &kol_s, int *&i_val, int &kol_i )
{
int i;
bool ret1=true,ret2=true;
QString wstr;

ret1=getChiselIsString(str, i_val,kol_i);
for(i=0;i<kol_i;i++)
   str.replace(wstr.setNum(i_val[i]),"");
ret2=getStringPoUppercase(str, s_val, kol_s );
return (ret1||ret2);
}

bool getStringChislPoUppercase(QString &str, QString *&s_val, int &kol_s, double *&i_val, int &kol )
{
int i;
bool ret1=true,ret2=true;
QString wstr;

ret1=getChiselIsString(str, i_val,kol);
for(i=0;i<kol;i++)
   str.replace(wstr.setNum(i_val[i]),"");
ret2=getStringPoUppercase(str, s_val, kol_s );
return (ret1||ret2);
}

bool getStringPoUppercase(QString &str, QString *&val, int &kol )
{
kol=0;
QStringList listDep;
QString wstr;

listDep.clear();
listDep=str.split(QRegExp("[êãõëåîçûýúèÿüöäìïòá÷ùæñþóíéôøâà³]"));
kol=listDep.count();
val=new QString[kol];
if(!val) return false;
kol=0;
while(!listDep.empty())
      {

        val[kol]=listDep.last();
        wstr=val[kol];
        val[kol]=str[str.indexOf(val[kol])-1]+wstr;
        listDep.pop_back();
        kol++;
      }
return true;
}



bool calcZnachPoIso(double *iso, int kol, double koord, double &value, int type)
{
int i;
if(kol<2) return false;

for(i=0;i<kol-1;i++)
  if(type==0) {if((iso[i*2]<=koord&&koord<iso[(i+1)*2])||(iso[i*2]>=koord&&koord>iso[(i+1)*2])) break;}
      else {if((iso[i*2+1]<=koord&&koord<iso[(i+1)*2+1])||(iso[i*2+1]>=koord&&koord>iso[(i+1)*2+1])) break;}
if(i==(kol-1)) return false;
if(type==0) return linInterpol(iso[i*2],iso[i*2+1],iso[(i+1)*2],iso[(i+1)*2+1],koord,value);
     else return linInterpol(iso[i*2+1],iso[i*2],iso[(i+1)*2+1],iso[(i+1)*2],koord,value);
}

bool integrTrapec(double *par, double *net, int kol, double &rez)
{
int i;

if(kol==0) return false;
rez=0.;
for(i=0;i<kol-1;i++)
  rez+=((par[i]+par[i+1])*0.5*(net[i+1]-net[i]));

return true;
}
