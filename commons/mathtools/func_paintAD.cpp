
#include "func_paintAD.h"

#include <commons/mathtools/mnmath.h>



double oprTpotSuch(double p,double t)
{
t=t+273.15;
return t*pow(1000./p,0.286)-273.15;
}

double oprTsuch(double p,double tet)
{
tet=tet+273.15;
return tet/pow(1000./p,0.286)-273.15;
}


double oprTpotVlaz(double p,double t)
{
double e, Sm1, Cp;

e=EP(t);
t=t+273.15;
Sm1 = (0.622*e/double(p));
Cp = 1004.7*(1.0-Sm1)+1846.0*Sm1;

return ((t+(((3.14821-0.00237*t)*1.e6)/Cp)*Sm1)*pow(1000./p,0.286))-273.15;
}

int oprTvlaz(double P1,double tet,double t1,double t2, GeoPoint *fl, int *kol_res)
{
 int kol2=0,kol=0,result=0;
 double t3, p2, Cp, e, Sm1,kap,Tet0,l,p,Tet1,Tet,t,ss,p0,p1;
 if (fl==NULL) return 1;

p2 = 155.0;
kap=0.286;
ss=tet+273.15;
kol=0;
for(t3=t1;t3>t2;t3--) 
    {
      p0=P1;
      p1=100.;
      e = EP(t3);
      t=t3+273.15;
      l=(3.14821-0.00237*t)*1.e6;
      Sm1 = (0.622*e/double(p0));
      Cp = 1004.7*(1.0-Sm1)+1846.0*Sm1;
      Tet0 = t +Sm1*l/Cp;
      Tet0*=pow(1000./p0,kap);
      Sm1 = (0.622*e/double(p1));
      Cp = 1004.7*(1.0-Sm1)+1846.0*Sm1;
      Tet = t +Sm1*l/Cp;
      Tet*=pow(1000./p1,kap);
      Tet1=Tet;
      if((Tet0-ss)*(Tet1-ss)>0.) continue;
      fl[kol2].fi=t3;
        while(fabs(Tet-ss)>1.e-6)
        {
         p=(p0+p1)*0.5;
         Sm1 = (0.622*e/double(p));
         Cp = 1004.7*(1.0-Sm1)+1846.0*Sm1;
         Tet = t +Sm1*l/Cp;
         Tet*=pow(1000./p,kap);
         if((Tet1-ss)*(Tet-ss)>0.) {p1=p; Tet1=Tet;} else {p0=p;Tet0=Tet;}
        }
     fl[kol2].la=p;
     kol2++;
     kol++;
     }
*kol_res=kol2;
return result;
}

/**определение высоты точки пересечения
 *
 * @param h массив высот точек
 * @param kr кривая высоту точки которой вычисляем
 * @param num номер нижней точки отрезка пересечения
 * @param znach значение в точке пересечения
 * @return
 */
double oprHperesech(double *h,GeoPoint *kr,int num,double znach)
{
double h1,h2,p1,p2;

    h1=h[num];
    h2=h[num+1];
    p1=kr[num].la;
    p2=kr[num+1].la;
    return h1+(h2-h1)*(p1-znach)/(p1-p2);
}

QString s_oprHperesech(double *h,GeoPoint *kr,int num,double znach)
{
double h1,h2,p1,p2;
QString wstr;
    h1=h[num];
    h2=h[num+1];
    p1=kr[num].la;
    p2=kr[num+1].la;
    wstr.sprintf("%d м",MnMath::ftoi_norm(h1+(h2-h1)*(p1-znach)/(p1-p2)));
    return wstr;
}

int oprTochPeresech (GeoPoint *kr1,int size_kr1,GeoPoint *kr2,int size_kr2,GeoPoint *&pointPeresech, QPointArray *numOtrPeresech, int *, int *kol_peresech, double )
{
int kol=0,i,j;
int kol1=0;
CrossResultRec rezult;
GeoPoint p11,p12,p21,p22;

kol1=size_kr1;
kol=size_kr2;
*kol_peresech=0;
for(i=0;i<kol1-2;i++)
{
//идем по кривой состояния
    p11.fi=kr1[i].fi;
    p11.la=kr1[i].la;//берем  i-й отрезок
    p12.fi=kr1[i+1].fi;
    p12.la=kr1[i+1].la;
    for(j=0;j<kol-2;j++)
          {
          //идем по кривой стратификации
                p21.fi=kr2[j].fi;          //берем  j-й отрезок
                p21.la=kr2[j].la;
                p22.fi=kr2[j+1].fi;
                p22.la=kr2[j+1].la;
                rezult=crossing( p11, p12, p21, p22); //проверяем на пересечение
                if(rezult.type==ONBOUNDS||rezult.type==INBOUNDS) //если внутри отрезка или на концах его
                {
                    (*kol_peresech)++;                            //количество пересечений
                    resizeGp(pointPeresech,(*kol_peresech));
                    pointPeresech[(*kol_peresech)-1].fi=rezult.pt.fi;
                    pointPeresech[(*kol_peresech)-1].la=rezult.pt.la;//добавляем очередную точку
                    numOtrPeresech->resize(*kol_peresech);                                         //пересечения в массив
                    numOtrPeresech->setPoint(*kol_peresech-1,i,j);                    //номера отрезков x- кривая состояния
                 }
          }
    }
return true;
}

int oprZnakIzm(GeoPoint p11,GeoPoint p12, // координаты первого отрезка
GeoPoint p21,GeoPoint p22,GeoPoint ,int ,double scaleX)//определяем знак пересечения
{
GeoPoint p_vs12,p_vs22;
CrossResultRec rezult;
        if(p12.la>=p22.la)
        {
        p_vs22.fi=40.*scaleX;
        p_vs22.la=p22.la;
        rezult=crossing(p11,p12,p21,p_vs22);
        if(rezult.type==ONBOUNDS||rezult.type==INBOUNDS) return -1;
        else return 1;
        }
        else
        {
        p_vs12.fi=40.*scaleX;
        p_vs12.la=p12.la;
        rezult=crossing(p11,p_vs12,p21,p22);
        if(rezult.type==ONBOUNDS||rezult.type==INBOUNDS) return 1;
        else return -1;
        }
}

CrossResultRec crossing(GeoPoint p11,GeoPoint p12, // координаты первого отрезка
                                              GeoPoint p21,GeoPoint p22)  // координаты второго отрезка
{
    CrossResultRec result;
    memset( (void*)&result, 0, sizeof(result) );
    // знаменатель
    double Z  = (p12.la-p11.la)*(p21.fi-p22.fi)-(p21.la-p22.la)*(p12.fi-p11.fi);
    // числитель 1
    double Ca = (p12.la-p11.la)*(p21.fi-p11.fi)-(p21.la-p11.la)*(p12.fi-p11.fi);
    // числитель 2
    double Cb = (p21.la-p11.la)*(p21.fi-p22.fi)-(p21.la-p22.la)*(p21.fi-p11.fi);

    // если числители и знаменатель = 0, прямые совпадают
    if( (Z < 1.e-9)&&(Z >-1.e-9)&&(Ca < 1.e-9)&&(Ca > -1.e-9)&&(Cb< 1.e-9)&&(Cb> -1.e-9) )
    {
    result.type = SAMELINE;
        return result;
    }
// если знаменатель = 0, прямые параллельны
    if( (Z < 1.e-9)&&(Z > -1.e-9) )
    {
    result.type = PARALLEL;
        return result;
    }
    double Ua = Ca/Z;
    double Ub = Cb/Z;
    result.pt.fi = p11.fi + (p12.fi - p11.fi) * Ub;
    result.pt.la = p11.la + (p12.la - p11.la) * Ub;
    // если 0<=Ua<=1 и 0<=Ub<=1, точка пересечения в пределах отрезков
    if( ( 0. <= Ua)&&(Ua <= 1.)&&( 0.  <= Ub)&&(Ub <= 1.) )
    {
        ( (Ua < 1.e-9)||(Ua == 1.)||(Ub < 1.e-9)||(Ub == 1.) ) ?
        result.type = ONBOUNDS :
        result.type = INBOUNDS;
    }
    // иначе точка пересечения за пределами отрезков
    else
    {
    result.type = OUTBOUNDS;
    }
    return result;
}


bool findPeresechPoX(double x0,double x1,double y0,double y1,double znach,double *ret_val)
{
double dy;
double dx;

if((x0-znach)*(x1-znach)<=0) return false;
dy=y0-y1;
dx=x0-x1;
if(dy<LITTLE_VALUE) {*ret_val=x0; return true;}
*ret_val=x0+(y0-znach)*dx/dy;
return true;
}

bool findPeresechPoY(double x0,double x1,double y0,double y1,double znach,double *ret_val)
{
double dy;
double dx;
//if((y0-znach)*(y1-znach)>=0.) return false;
dy=y1-y0;
dx=x1-x0;
if(fabs(dx)<LITTLE_VALUE) {*ret_val=y0; return true;}
*ret_val=y0+(znach-x0)*dy/dx;
return true;
}


bool INSET(int i)
{
 switch (i)
  {
   case  1: case  2: case  3: case  4: case  5: case  6:
   case 7: case 8: case 13: case 18: case 20: case 22: case 24: case 25:
   case 26: case 27: case 28: case 29: case 30: case 31: case 32: case 33:
   case 34: case 35: case 36: case 37: case 38: case 39: case 40: return true;
   default: return false;
  }
}

double Y(double yy)
{
 return exp (0.286*log(1000.0)) - exp (0.286*log(yy));
}

double Y2(double yy)
{
 return (104.4722*(7.3124-pow(yy, 0.286)));;
}

double Y1(double yy)
{
 return pow(-yy/104.4722+7.3124,1./0.286);
}




bool ADPts(int type, TPointList **apoint)
{
 int i,iX,iY,amount,kol=0;
 int X[12],Y[12];
 bool result=false;
 QPointArray *scrPt=NULL;
 TPointList *result_sub_object=NULL;

 int e1[] = {67,115,165,217,272,327,385,440,497,557,617,680,740,802,865};
 int e2[] = {92,138,193,242,300,355,412,470,527,587,650,710,770,832,895};
 double e3[] = {2.0,3.4,6.4,9.4,12.4,15.4,18.4,21.4,24.4,27.4,30.4,33.4,36.4,39.4};
 double e4[] = {0.7,5.5,9.15,12.3,15.05,19.6,23.35,26.7,29.65,32.35,33.5,34.7,36.95,38.0};
 double e5[] = {39.35,22.4,30.1,33.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35};
 double e6[] = {3.95,4.25,4.5,4.8,5.1,5.35,5.7,5.95,6.25,6.5,6.8,7.15,7.4,7.7,8.0,8.25,8.5,8.8,9.1,9.4,9.7,10.0,10.2,10.5};
 double e7[] = {1.2,1.7,2.22,2.73,3.21,3.70,4.17,4.65,5.1,5.86,6.26,6.66,7.05,7.42,7.82,8.2,8.51,8.85,9.40,9.72,10.06,10.38,10.70,11.02,11.34,
  11.66,11.98,12.9,13.45,14.0,14.5,15.05,15.55,16.05,16.5,16.95,17.4,17.88,18.3,18.7,19.1,20.0,20.37,20.75,21.12,21.5,21.9,22.3,22.65,23.0,23.7,24.05,
  24.4,24.78,25.1,25.42,25.75,26.1,26.4,27.0,27.3,27.6,27.9,28.2,28.5,28.8,29.1,29.4,29.9,30.15,30.45,30.7,30.98,31.25,31.52,31.78,32.03,32.55,32.8,
  33.05,33.28,33.75,34.0,34.25,34.45,34.92,35.15,35.37,35.60,35.82,36.05,36.27,36.5,36.72,37.16,37.37,37.58,37.79};
 double e8[] = {18.95,18.95,18.95,18.95,18.95,18.95,19.75,20.55,21.5,23.35,24.15,24.9,25.7,26.5,27.45,28.3,28.8,29.85,30.95,31.75,32.35,
  32.95,33.35,33.35,33.35,33.35,33.35,33.35,33.35,33.35,33.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,
  39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,
  39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,39.35,
  39.35,39.35,39.35,39.35,39.35};
 double Sm[] = {0.01,0.02,0.05,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0,
               1.2,1.4,1.6,1.8,2.0,2.5,3.0,3.5,4.0,4.5,5.0,6.0,7.0,8.0,9.0,10.0,
               12.0,14.0,16.0,18.0,20.0,25.0,30.0,35.0,40.0,45.0};

 double dm25 = 25.0;
 int m7    = 7;
 int m22   = 22;
 int m25   = 25;
 int m37   = 37;
 int m928  = 928;
 int m994  = 994;
 int m1009 = 1009;
 int m1023 = 1023;
 int press;

 int a3, x, y;
 double a2, temp, t3, p2, p3, Cp, Tp, e, Sm1;

 switch(type)
    {
     case AD_RAMKA:
                   X[0]=DX+m22+RVS;    Y[0]=DY+m25+RVS;
                   X[1]=DX+m22+RVS;    Y[1]=DY+m928+RVS;
                   X[2]=DX+m1009+RVS;  Y[2]=DY+m25+RVS;
                   X[3]=DX+m1009+RVS;  Y[3]=DY+m928+RVS;
                   X[4]=DX+m7+RVS;     Y[4]=DY+m25+RVS;  // верхняя граница шкалы слева
                   X[5]=DX+m37+RVS;    Y[5]=DY+m25+RVS;
                   X[6]=DX+m7+RVS;     Y[6]=DY+m928+RVS; // нижняя граница шкалы слева
                   X[7]=DX+m37+RVS;    Y[7]=DY+m928+RVS;
                   X[8]=DX+m994+RVS;   Y[8]=DY+m25+RVS;  // верхняя граница шкалы справа
                   X[9]=DX+m1023+RVS;  Y[9]=DY+m25+RVS;
                   X[10]=DX+m994+RVS;  Y[10]=DY+m928+RVS; // нижняя граница шкалы справа
                   X[11]=DX+m1023+RVS; Y[11]=DY+m928+RVS;

                  for(i=0;i<6;i++)
                  {
                   kol=0;
                   amount=2;
                   if(!(scrPt= new QPointArray(amount))) return false;
            			 iX=X[2*i];
				           iY=Y[2*i];
				           scrPt->setPoint(kol,iX,iY);
                   kol++;
            			 iX=X[2*i+1];
				           iY=Y[2*i+1];
				           scrPt->setPoint(kol,iX,iY);
                   kol++;

                   result_sub_object = new TPointList(*apoint);
			             if((result_sub_object)==NULL)
                    {
                     if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                     return false;
                    }
                    else (*apoint) = result_sub_object;
                   if(!((*apoint)->initPointList(scrPt,kol)))
                    {
                     if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                     return false;
                    }
                   result=true;
                   if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                  }

                  for(i=0;i<88;i++)
                  {
                   kol=0;
                   amount=2;
                   if(!(scrPt= new QPointArray(amount))) return false;
            			 if(i<15) {iX=X[4]; iY=DY+int(e1[i]*M1)+RVS;}
                     else if(i<30) {iX=X[2]; iY=DY+int(e1[i-15]*M1)+RVS;}
                            else if(i<45) {iX=DX+int(15.0*M1)+RVS; iY=DY+int(e2[i-30]*M1)+RVS;}
                                   else if(i<60) {iX=X[2]; iY=DY+int(e2[i-45]*M1)+RVS;}
                                          else if(i<74) {iX=DX+int(e3[i-60]*dm25)+RVS; iY=DY+int(0.7*dm25)+RVS;}
                                                  else if(i<88) {iX=DX+int(2.0*dm25)+RVS; iY=DY+int(e4[i-74]*dm25)+RVS;}
				           scrPt->setPoint(kol,iX,iY);
                   kol++;
            			 if(i<15) {iX=X[0]; iY=DY+int(e1[i]*M1)+RVS;}
                     else if(i<30) {iX=X[9]; iY=DY+int(e1[i-15]*M1)+RVS;}
                            else if(i<45) {iX=X[0]; iY=DY+int(e2[i-30]*M1)+RVS;}
                                   else if(i<60) {iX=DX+int(1016.0*M1)+RVS; iY=DY+int(e2[i-45]*M1)+RVS;}
                                          else if(i<74) {iX=DX+int(e3[i-60]*dm25)+RVS; iY=DY+int(38.0*dm25)+RVS;}
                                                  else if(i<88) {iX=DX+int(e5[i-74]*dm25)+RVS; iY=DY+int(e4[i-74]*dm25)+RVS;}
				           scrPt->setPoint(kol,iX,iY);
                   kol++;

                      result_sub_object = new TPointList(*apoint);
                      if((result_sub_object)==NULL)
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                        else (*apoint) = result_sub_object;
                      if(!((*apoint)->initPointList(scrPt,kol)))
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                      result=true;
                      if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                  }
                  break;
     case AD_PRESS:
                  for(i=0;i<98;i++)
                  {
                   kol=0;
                   amount=2;
                   if(!(scrPt= new QPointArray(amount))) return false;
            			 iX=DX+int(2.0*dm25)+RVS; iY=DY+int(e7[i]*dm25)+RVS;
				           scrPt->setPoint(kol,iX,iY);
                   kol++;

                   iX=DX+int(e8[i]*dm25)+RVS; iY=DY+int(e7[i]*dm25)+RVS;
				           scrPt->setPoint(kol,iX,iY);
                   kol++;

                      result_sub_object = new TPointList(*apoint);
                      if((result_sub_object)==NULL)
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                        else (*apoint) = result_sub_object;
                      if(!((*apoint)->initPointList(scrPt,kol)))
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                      result=true;
                      if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                  }
                  break;
     case AD_TEMPER:
                  for(i=0;i<121;i++)
                  {
                   kol=0;
                   amount=2;
                   if(!(scrPt= new QPointArray(amount))) return false;
            			 if(i<52) {iX=DX+int((e3[1]+(i+1)*0.3)*dm25)+RVS; iY=DY+int(0.7*dm25)+RVS;}
                     else if(i<76) {iX=DX+int((e3[6]+0.3+(i-51)*0.6)*dm25)+RVS; iY=DY+int(e6[i-52]*dm25)+RVS;}
                            else if(i<100) {iX=DX+int((e3[6]+(i-74)*0.6)*dm25)+RVS; iY=DY+int(0.7*dm25)+RVS;}
                                   else if(i<111) {iX=DX+int((e3[11]+(i-100)*0.6)*dm25)+RVS; iY=DY+int(0.7*dm25)+RVS;}
                                          else if(i<121) {iX=DX+int((e3[11]+0.3+(i-111)*0.6)*dm25)+RVS; iY=DY+int(15.0*dm25)+RVS;}
				           scrPt->setPoint(kol,iX,iY);
                   kol++;
            			 if(i<52) {iX=DX+int((e3[1]+(i+1)*0.3)*dm25)+RVS; iY=DY+int(38.0*dm25)+RVS;}
                     else if(i<76) {iX=DX+int((e3[6]+0.3+(i-51)*0.6)*dm25)+RVS; iY=DY+int(38.0*dm25)+RVS;}
                            else if(i<100) {iX=DX+int((e3[6]+(i-74)*0.6)*dm25)+RVS; iY=DY+int(38.0*dm25)+RVS;}
                                   else if(i<111) {iX=DX+int((e3[11]+(i-100)*0.6)*dm25)+RVS; iY=DY+int(38.0*dm25)+RVS;}
                                          else if(i<121) {iX=DX+int((e3[11]+0.3+(i-111)*0.6)*dm25)+RVS; iY=DY+int(38.0*dm25)+RVS;}
				           scrPt->setPoint(kol,iX,iY);
                   kol++;

                      result_sub_object = new TPointList(*apoint);
                      if((result_sub_object)==NULL)
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                        else (*apoint) = result_sub_object;
                      if(!((*apoint)->initPointList(scrPt,kol)))
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                      result=true;
                      if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                  }
                  break;
     case AD_STAND:
                  for(i=0;i<3;i++)
                  {
                   kol=0;
                   amount=2;
                   if(!(scrPt= new QPointArray(amount))) return false;
            			 if(i<1) {iX=DX+int(10.4*25.)+RVS; iY=DY+int(0.7*25.)+RVS;}
                     else if(i<2) {iX=DX+int(10.4*25.)+RVS; iY=DY+int(11.*25.)+RVS;}
                            else if(i<3) {iX=DX+int(27.35*25.)+RVS; iY=DY+int(15.*25.)+RVS;}
				           scrPt->setPoint(kol,iX,iY);
                   kol++;
            			 if(i<1) {iX=DX+int(10.4*25.)+RVS; iY=DY+int(11.0*25.)+RVS;}
                     else if(i<2) {iX=DX+int(31.9*25.)+RVS; iY=DY+int(37.2*25.)+RVS;}
                            else if(i<3) {iX=DX+int(27.35*25.)+RVS; iY=DY+int(38.0*25.)+RVS;}
				           scrPt->setPoint(kol,iX,iY);
                   kol++;

                      result_sub_object = new TPointList(*apoint);
                      if((result_sub_object)==NULL)
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                        else (*apoint) = result_sub_object;
                      if(!((*apoint)->initPointList(scrPt,kol)))
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                      result=true;
                      if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                  }
                  break;
     case AD_SUKHADIAB:
                  for(i=0;i<43;i++)
                  {
                   kol=0;
                   amount=2;
                   if(!(scrPt= new QPointArray(amount))) return false;
            			 if(i<22)
                    {
                     iX=DX+int(e3[1]*dm25)+RVS;
               	     press = MnMath::ftoi_norm(1000.0*exp(-3.497*log((203.15+i*5)/193.15)));
                     iY = DY+MnMath::ftoi_norm(36.9*25.0-(exp(0.286*log(1000.0))-exp(0.286*log(press)))*260.0)+RVS;
                    }
                     else if(i<35)
                            {
                             iX=DX+int(e3[1]*dm25)+RVS;
                             press = MnMath::ftoi_norm(1000.0*exp(-3.497*log((313.15+(i-22)*5)/193.15)));
                             iY = DY+MnMath::ftoi_norm(36.9*25.0-(exp(0.286*log(1000.0))-exp(0.286*log(press)))*260.0)+RVS;
                            }
                            else
                               {
                                temp=(378.15+(i-35)*5)*pow(0.1,0.286)-273.15;
                                iX=DX+int((e3[1]+(temp+80.)*0.3)*dm25)+RVS;
                                iY=DY+int(0.7*dm25)+RVS;
                               }
				           scrPt->setPoint(kol,iX,iY);
                   kol++;
            			 if(i<22)
                    {
                      temp=(203.15+i*5)*pow(1.05,0.286)-273.15;
                      iX=DX+int((e3[1]+(temp+80.)*0.3)*dm25)+RVS;
                      iY=DY+int(38.0*dm25)+RVS;
                    }
                     else if(i<35)
                           {iX=DX+int(e3[13]*dm25)+RVS;
                            press = MnMath::ftoi_norm(1000.0*exp(-3.497*log((313.15+(i-22)*5)/313.15)));
                            iY = DY+MnMath::ftoi_norm(36.9*25.0-(exp(0.286*log(1000.0))-exp(0.286*log(press)))*260.0)+RVS;
                           }
                           else
                              {
                               iX=DX+int(e3[13]*dm25)+RVS;
                               press = MnMath::ftoi_norm(1000.0*exp(-3.497*log((378.15+(i-35)*5)/313.15)));
                               iY = DY+MnMath::ftoi_norm(36.9*25.0-(exp(0.286*log(1000.0))-exp(0.286*log(press)))*260.0)+RVS;
                              }
				           scrPt->setPoint(kol,iX,iY);
                   kol++;

                      result_sub_object = new TPointList(*apoint);
                      if((result_sub_object)==NULL)
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                        else (*apoint) = result_sub_object;
                      if(!((*apoint)->initPointList(scrPt,kol)))
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                      result=true;
                      if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                  }
                  break;
     case AD_VLAZHADIAB:
                   p2 = 105.0;
                   a2 = 425.0;
                   for(i=0;i<29;i++)
                     {
                      a3=0;
                      kol=0;
                      amount=2;
                      if(!(scrPt= new QPointArray(amount))) return false;
                      p2 -= 5.0;
                      t3 = 40.0;
                      p3 = 1000.;
                      a2+= (i+1);
                      do
                       {
                        t3 -= 0.3; e = EP(t3);
                        Sm1 = (0.622*e/double(p3-180.0+a3));
                        Cp = 1004.7*(1.0-Sm1)+1846.0*Sm1;
                        Tp = t3 + 273.15 +Sm1*2501000.0/Cp;
                        p3 = 1000.0*exp(3.5*log(Tp/(273.15+p2)));
                        if((p3<1030.)&&(p3>100.))
                        	{
                           a3 += 1;
                           x = DX+int(27.35*25.0+t3*7.5)+RVS;
                           y = DY+int(36.9*25.0-(exp(0.286*log(1000.0))-exp(0.286*log(p3)))*260.0)+RVS;
                           kol++;
                           scrPt->resize(kol);
                           scrPt->setPoint(kol-1,x,y);
                    		  }
                       }
                   	  while (t3 >-60.0);
                      result_sub_object = new TPointList(*apoint);
                      if((result_sub_object)==NULL)
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                        else (*apoint) = result_sub_object;
                      if(!((*apoint)->initPointList(scrPt,kol)))
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                      result=true;
                      if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                  }
                   break;
     case AD_ISOGR:
                   for(i=0;i<39;i++)
                     {
                      t3 = 40.3;
                      kol=0;
                      amount=2;
                      if(!(scrPt= new QPointArray(amount))) return false;
                      do
                       {
                   	    t3 -= 0.3;
                   	    e = EP(t3);
                  	    p3 = 622.0*e/Sm[i];
                        if((p3<1050.)&&(p3>200.))
                        	{
                           x = DX+MnMath::ftoi_norm(27.35*25.0+t3*7.5)+RVS;
                           y = DY+MnMath::ftoi_norm(36.9*25.0-(exp(0.286*log(1000.0))-exp(0.286*log(p3)))*260.0)+RVS;
                           kol++;
                           scrPt->resize(kol);
                           scrPt->setPoint(kol-1,x,y);
                    		  }
                       }
                   	  while (t3 >-70.0);
                      result_sub_object = new TPointList(*apoint);
                      if((result_sub_object)==NULL)
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                        else (*apoint) = result_sub_object;
                      if(!((*apoint)->initPointList(scrPt,kol)))
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                      result=true;
                      if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                  }
                   break;
		 default:
                           break;
	  }

  if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
  return result;
}

void calculateOnePointAD(int *x, int *y, PrjPar *prj_par, double dRVS)
{
 int iX,iY,xx,yy;
 xx=*x;
 yy=*y;
 if (prj_par->print_param==PRINT_PARAM_NONE)
	{
	iX=int(xx>>prj_par->Par1)-MnMath::ftoi_norm(dRVS);
	iY=int(yy>>prj_par->Par1)-MnMath::ftoi_norm(dRVS);
	}
  else
	 {
	  iX=int((xx-RVS)*prj_par->print_param);
	  iY=int((yy-RVS)*prj_par->print_param);
	 }

 *x=iX-prj_par->Par5;
 *y=iY-prj_par->Par6;
}

bool ADDataPts(int type, TPointList **apoint, TAeroDataAll *Struc)
{
 int i,ii,i_start,j,A1,iX,iY,amount,kol=0;
 int Pb,Pt,P_B,P_E;
 int EndIndex;
 double P0, P1, T0, T1, Td0, s0, s1, Step, Err, E, Cp, Tp, P2;
 double Tb,Tt,T_B,T_E;
 bool result=false;
 bool AchieveTropo;
 QPointArray *scrPt=NULL;
 TPointList *result_sub_object=NULL;

 switch(type)
    {
     case AD_STRAT:
                   kol=0;
                   amount=2;
                   if(!(scrPt= new QPointArray(amount))) return false;
                   for(i=0; i<99; i++)
                      {
                       if(ValidInt(Struc->data[i].P,'p') && ValidDouble((Struc->data[i].T/10.),'t'))
                        {
                           iX = DX+MnMath::ftoi_norm(27.35*25.+(Struc->data[i].T/10.)*7.5)+RVS;
                           iY = DY+MnMath::ftoi_norm(36.9*25.-Y(Struc->data[i].P)*260.)+RVS;
                           kol++;
                           scrPt->resize(kol);
                           scrPt->setPoint(kol-1,iX,iY);
                        }
                      }
                      result_sub_object = new TPointList(*apoint);
                      if((result_sub_object)==NULL)
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                        else (*apoint) = result_sub_object;
                      if(!((*apoint)->initPointList(scrPt,kol)))
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                      result=true;
                      if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                  break;
     case AD_TROS:
                   kol=0;
                   amount=2;
                   if(!(scrPt= new QPointArray(amount))) return false;
                   for(i=0; i<99; i++)
                      {
                       if(ValidInt(Struc->data[i].P,'p') && ValidDouble((Struc->data[i].T/10.),'t') && ValidDouble(Struc->data[i].D/10,'d'))
                        {
                           iX = DX+MnMath::ftoi_norm(27.35*25.+(Struc->data[i].T/10.-Struc->data[i].D/10.)*7.5)+RVS;
                           iY = DY+MnMath::ftoi_norm(36.9*25.-Y(Struc->data[i].P)*260.)+RVS;
                           kol++;
                           scrPt->resize(kol);
                           scrPt->setPoint(kol-1,iX,iY);
                        }
                      }
                      result_sub_object = new TPointList(*apoint);
                      if((result_sub_object)==NULL)
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                        else (*apoint) = result_sub_object;
                      if(!((*apoint)->initPointList(scrPt,kol)))
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                      result=true;
                      if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                  break;
     case AD_0:
     case AD_10:
     case AD_20:
                  switch(type)
                   {
                    case AD_0: Pt=0; break;
                    case AD_10: Pt=-10; break;
                    case AD_20: Pt=-20; break;
                   }
                   kol=0;
                   amount=2;
                   if(!(scrPt= new QPointArray(amount))) return false;

                   for(i=0; i<99; i++)
                    {
                     Pb = 99999;
                     if(ValidInt(Struc->data[i].P,'p') && ValidDouble(Struc->data[i].T/10.,'t') )
                      {
                       P0 = Struc->data[i].P;
	                     T0 = Struc->data[i].T/10;
                       for(ii=i+1;ii<100;ii++)
                        {
    		                 if(ValidInt(Struc->data[ii].P,'p') && ValidDouble(Struc->data[ii].T/10.,'t') )
    		                  {
                           P1 = Struc->data[ii].P;
		                       T1 = Struc->data[ii].T/10.;
		                       ii = 100;
		                      }
    		                }
	                     if(((T0>=Pt)&&(T1<Pt)) || ((T0<=Pt)&&(T1>Pt)) )
                       Pb = MnMath::ftoi_norm(P1+(P0-P1)*(double(Pt)-T1)/(T0-T1));
	                  }

                   if(ValidInt(Pb,'p'))
                        {
                           iX = DX+MnMath::ftoi_norm(27.35*25.+(Pt*7.5)+2.)+RVS;
                           iY = DY+MnMath::ftoi_norm(36.9*25.-Y(Pb)*260.)+RVS;
                           kol++;
                           scrPt->resize(kol);
                           scrPt->setPoint(kol-1,iX,iY);
                           iX = DX+MnMath::ftoi_norm(27.35*25.+(Pt*7.5)+200.)+RVS;
                           iY = DY+MnMath::ftoi_norm(36.9*25.-Y(Pb)*260.)+RVS;
                           kol++;
                           scrPt->resize(kol);
                           scrPt->setPoint(kol-1,iX,iY);
                           break;
                        }
                    }
                      result_sub_object = new TPointList(*apoint);
                      if((result_sub_object)==NULL)
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                        else (*apoint) = result_sub_object;
                      if(!((*apoint)->initPointList(scrPt,kol)))
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                      result=true;
                      if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                  break;
     case AD_KRSOST:
                   kol=0;
                   amount=2;
                   if(!(scrPt= new QPointArray(amount))) return false;
                   for (i=0;i<100;i++)
	                 if(ValidInt(Struc->data[i].P,'p')&&ValidDouble(Struc->data[i].T/10.,'t'))
    	              {
	                   i_start = i;
	                   break;
	                  }
                   for (i=i_start;i<99;i++)
                	  {
	                   for (j=i+1;j<100;j++)
                	     if(ValidInt(Struc->data[j].P,'p') && ValidDouble(Struc->data[j].T/10.,'t')) break;
                	   if(Struc->data[i].T/10.>Struc->data[j].T/10.) {i_start = i; break;}
                       else i=j-1;
                    }
                   P0 = Struc->data[i_start].P;
                   T0 = Struc->data[i_start].T/10.;
                   Td0 = Struc->data[i_start].T/10.-Struc->data[i_start].D/10.;
                   s0 = EP(Td0)/P0;
                   Step = 0.5;
                   Err = 0.00000001;
                   i = 0;
                   P1 = P0;
                   do
                     {
                      P1-=Step;
	                    T1 = exp(log(T0+273.15)+0.286*(log(P1)-log(P0)))-273.15;
                      s1 = EP(T1)/P1;
                      i++;
                     }
                   while((i<10000)&&((s1-s0)>Err));
                   iX = DX+MnMath::ftoi_norm(27.35*25.+T0*7.5)+RVS;
                   iY = DY+MnMath::ftoi_norm(36.9*25.-Y(P0)*260.)+RVS;
                   kol++;
                   scrPt->resize(kol);
                   scrPt->setPoint(kol-1,iX,iY);
                   iX = DX+MnMath::ftoi_norm(27.35*25.+T1*7.5)+RVS;
                   iY = DY+MnMath::ftoi_norm(36.9*25.-Y(P1)*260.)+RVS;
                   kol++;
                   scrPt->resize(kol);
                   scrPt->setPoint(kol-1,iX,iY);

                   E = EP(T1);
                   s1 = (0.622*E/(P1-180.0));
                   Cp = 1004.7*(1.0-s1)+1846.0*s1;
                   Tp = (T1+273.15)+((2501000.0/Cp)*s1);
                   P2 = Tp*exp(0.286*log(1000.0/P1));
                   A1 = 0;
                   do
                	  {
                	   T1-=0.3;
                 	   A1+=1;
                	   E=EP(T1);
                     s1=((0.622*E)/(P1-180.0+A1));
                     Cp = 1004.7*(1.0-s1)+1846.0*s1;
	                   Tp = (T1+273.15)+((2501000.0/Cp)*s1);
	                   P1 = 1000.0*exp(3.5*log(Tp/(P2)));
                     iX = DX+int(27.35*25.0+T1*7.5)+RVS;
                     iY = DY+int(36.9*25.0-Y(P1)*260.0)+RVS;
                     kol++;
                     scrPt->resize(kol);
                     scrPt->setPoint(kol-1,iX,iY);
	                  }
                   while ( (T1>-60.0)&&(P1>0.0) );

                   result_sub_object = new TPointList(*apoint);
                   if((result_sub_object)==NULL)
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                        else (*apoint) = result_sub_object;
                   if(!((*apoint)->initPointList(scrPt,kol)))
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                   result=true;
                   if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                   break;
     case AD_UK:
                   kol=0;
                   amount=2;
                   if(!(scrPt= new QPointArray(amount))) return false;
                   for (i=0;i<100;i++)
	                 if(ValidInt(Struc->data[i].P,'p')&&ValidDouble(Struc->data[i].T/10.,'t'))
    	              {
	                   i_start = i;
	                   break;
	                  }
                   for (i=i_start;i<99;i++)
                	  {
	                   for (j=i+1;j<100;j++)
                	     if(ValidInt(Struc->data[j].P,'p') && ValidDouble(Struc->data[j].T/10.,'t')) break;
                	   if(Struc->data[i].T/10.>Struc->data[j].T/10.) {i_start = i; break;}
                       else i=j-1;
                    }
                   P0 = Struc->data[i_start].P;
                   T0 = Struc->data[i_start].T/10.;
                   Td0 = Struc->data[i_start].T/10.-Struc->data[i_start].D/10.;
                   s0 = EP(Td0)/P0;
                   Step = 0.5;
                   Err = 0.00000001;
                   i = 0;
                   P1 = P0;
                   do
                     {
                      P1-=Step;
	                    T1 = exp(log(T0+273.15)+0.286*(log(P1)-log(P0)))-273.15;
                      s1 = EP(T1)/P1;
                      i++;
                     }
                   while((i<10000)&&((s1-s0)>Err));
                   iX = DX+MnMath::ftoi_norm(27.35*25.+T1*7.5)+RVS;
                   iY = DY+MnMath::ftoi_norm(36.9*25.-Y(P1)*260.)+RVS;
                   kol++;
                   scrPt->resize(kol);
                   scrPt->setPoint(kol-1,iX,iY);
                   iX = DX+MnMath::ftoi_norm(27.35*25.+T1*7.5+100.)+RVS;
                   iY = DY+MnMath::ftoi_norm(36.9*25.-Y(P1)*260.)+RVS;
                   kol++;
                   scrPt->resize(kol);
                   scrPt->setPoint(kol-1,iX,iY);

                   result_sub_object = new TPointList(*apoint);
                   if((result_sub_object)==NULL)
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                        else (*apoint) = result_sub_object;
                   if(!((*apoint)->initPointList(scrPt,kol)))
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                   result=true;
                   if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                   break;
     case AD_INV:
                   kol=0;
                   amount=2;
                   if(!(scrPt= new QPointArray(amount))) return false;

                   P_B = 9999;
                   P_E = 9999;
                   AchieveTropo = false;

                   for (i=0;i<99;i++)
                 	   {
                      if(ValidInt(Struc->data[i].P,'p') && ValidDouble(Struc->data[i].T/10.,'t'))
                	      {
                         if((Struc->data[i].P<350)&&(Struc->data[i].P!=9999))
                          {
    		                   P_B = 9999;
		                       P_E = 9999;
				                   continue;
                          }
                         Pb = Struc->data[i].P;
	                       Tb = Struc->data[i].T/10.;
                         if(Struc->data[i].H == 8888)	AchieveTropo = true;
                         for(ii=i+1;ii<100;ii++)
    		                  {
                           if(ValidInt(Struc->data[ii].P,'p') && ValidDouble(Struc->data[ii].T/10.,'t'))
                            {
		    		                 Tt = Struc->data[ii].T/10.;
                             ii = 100;
		    		                }
     		                  }
	                       if((Tt>=Tb) && !ValidInt(P_B,'p') && !AchieveTropo )
                          {
				                   P_B = Pb;
				                   T_B = Tb;
			                    }
	                       if((Tt<Tb) && ValidInt(P_B,'p'))
                          {
				                   P_E = Pb;
				                   T_E = Tb;
				                  }
                         if(ValidInt(P_B,'p') && ValidInt(P_E,'p'))
                          {
                           iX = DX+MnMath::ftoi_norm(27.35*25.+T_E*7.5+2)+RVS;
                           iY = DY+MnMath::ftoi_norm(36.9*25.-Y(P_E)*260.)+RVS;
                           kol++;
                           scrPt->resize(kol);
                           scrPt->setPoint(kol-1,iX,iY);
                           iX = DX+MnMath::ftoi_norm(27.35*25.+T_E*7.5+200)+RVS;
                           iY = DY+MnMath::ftoi_norm(36.9*25.-Y(P_E)*260.)+RVS;
                           kol++;
                           scrPt->resize(kol);
                           scrPt->setPoint(kol-1,iX,iY);
                           iX = DX+MnMath::ftoi_norm(27.35*25.+T_B*7.5+200)+RVS;
                           iY = DY+MnMath::ftoi_norm(36.9*25.-Y(P_B)*260.)+RVS;
                           kol++;
                           scrPt->resize(kol);
                           scrPt->setPoint(kol-1,iX,iY);
                           iX = DX+MnMath::ftoi_norm(27.35*25.+T_B*7.5+2)+RVS;
                           iY = DY+MnMath::ftoi_norm(36.9*25.-Y(P_B)*260.)+RVS;
                           kol++;
                           scrPt->resize(kol);
                           scrPt->setPoint(kol-1,iX,iY);

                           P_B = 9999;
			                     P_E = 9999;
    		                  }
    		                }
                     }

                   result_sub_object = new TPointList(*apoint);
                   if((result_sub_object)==NULL)
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                        else (*apoint) = result_sub_object;
                   if(!((*apoint)->initPointList(scrPt,kol)))
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                   result=true;
                   if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                   break;
     case AD_TROPO_P:
                   kol=0;
                   amount=2;
                   if(!(scrPt= new QPointArray(amount))) return false;

                   P_B = 9999; P_E = 9999;

                   for (i=99;i>0;i--)
                     if (ValidInt(Struc->data[i].P,'p')) {EndIndex = i; break;}
                   for (i=0;i<99;i++)
                 	   {
	                    if(ValidInt(Struc->data[i].P,'p')&&ValidDouble(Struc->data[i].T/10.,'t')&&(Struc->data[i].pok_kach[6]=='4'))
                	      {//if in KN01 exist group 8888
                	       Pb = Struc->data[i].P;
                	       Tb = Struc->data[i].T/10.;
                 	       P_B = Pb;
                	       T_B = Tb;
                         iX = DX+MnMath::ftoi_norm(27.35*25.+T_B*7.5)+RVS;
                         iY = DY+MnMath::ftoi_norm(36.9*25.-Y(P_B)*260.)+RVS;
                         kol++;
                         scrPt->resize(kol);
                         scrPt->setPoint(kol-1,iX,iY);

                	       Pt = Struc->data[i].P;
	                       Tt = Struc->data[i].T/10;
	                       for(ii=i+1;ii<100;ii++)
    		                   {
    		                    if(ValidInt(Struc->data[ii].P,'p') && ValidDouble(Struc->data[ii].T/10.,'t') )
                              {
    		                       if(Struc->data[ii].T/10<Tt)
							                  {
							                   P_E = Pt;
							                   T_E = Tt;
							                   break;
							                  }
    		                        else
        		                      {
        		                       if(ii==EndIndex)
			    		                       {
					                            P_E = Struc->data[ii].P;
					                            T_E = Struc->data[ii].T/10.;
					                            kol++;
                                      iX = DX+MnMath::ftoi_norm(27.35*25.+T_E*7.5)+RVS;
                                      iY = DY+MnMath::ftoi_norm(36.9*25.-Y(P_E)*260.)+RVS;
                                      scrPt->resize(kol);
                                      scrPt->setPoint(kol-1,iX,iY);
                                      break;
                                     }
        		                         else
					                              {
					                               Pt = Struc->data[ii].P;
					                               Tt = Struc->data[ii].T/10.;
                                         kol++;
                                         iX = DX+MnMath::ftoi_norm(27.35*25.+Tt*7.5)+RVS;
                                         iY = DY+MnMath::ftoi_norm(36.9*25.-Y(Pt)*260.)+RVS;
                                         scrPt->resize(kol);
                                         scrPt->setPoint(kol-1,iX,iY);
                        					      }
        		                      }
    		                      }
    		                   }
                         kol=kol+3;
                         scrPt->resize(kol);
                         iX = DX+MnMath::ftoi_norm(27.35*25.+T_E*7.5)+RVS;
                         iY = DY+MnMath::ftoi_norm(36.9*25.-Y(P_E)*260.)+RVS;
                         scrPt->setPoint(kol-3,iX,iY);
                         iX = DX+MnMath::ftoi_norm(27.35*25.+T_E*7.5+200)+RVS;
                         iY = DY+MnMath::ftoi_norm(36.9*25.-Y(P_E)*260.)+RVS;
                         scrPt->setPoint(kol-2,iX,iY);
                         iX = DX+MnMath::ftoi_norm(27.35*25.+T_E*7.5+200)+RVS;
                         iY = DY+MnMath::ftoi_norm(36.9*25.-Y(P_B)*260.)+RVS;
                         scrPt->setPoint(kol-1,iX,iY);
    		                }
                     }
                   result_sub_object = new TPointList(*apoint);
                   if((result_sub_object)==NULL)
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                        else (*apoint) = result_sub_object;
                   if(!((*apoint)->initPointList(scrPt,kol)))
                       {
                        if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                        return false;
                       }
                   result=true;
                   if(scrPt!=NULL) { delete scrPt; scrPt=NULL;}
                   break;
		 default:
                  break;
	  }
 return true;
}

bool ValidDouble(double v, char s)
{
 double vmin, vmax;
 switch (s)
  {
   case 't': {vmin = -99.0; vmax = 99.0;} break;
   case 'd': {vmin = 0.0; vmax = 99.0;}
  }
 if ( (v>=vmin)&&(v<=vmax) ) {return true;} else return false;
}

bool ValidInt(double v, char s)
{
 int vmin, vmax;
 switch (s)
  {
   case 'p': {vmin = 100; vmax = 1500;} break;
   case 'h': {vmin = -1000; vmax = 20000;} break;
   case 'a': {vmin = 0; vmax = 360;} break;
   case 'f': {vmin = 0; vmax = 99;}
  }
 if ( (v>=vmin)&&(v<=vmax) ) {return true;} else return false;
}

bool CloudDeficit(double p, double d, double *d_m)
{
 double dmax = 1.5;
 if (p >  925)  dmax = 1.6;
 if ((p <= 825)&&(p > 775)) dmax = 1.7;
 if ((p <= 775)&&(p > 725)) dmax = 1.8;
 if ((p <= 725)&&(p > 675)) dmax = 1.9;
 if ((p <= 675)&&(p > 625)) dmax = 2.0;
 if ((p <= 625)&&(p > 575)) dmax = 2.2;
 if ((p <= 575)&&(p > 525)) dmax = 2.4;
 if ((p <= 525)&&(p > 450)) dmax = 2.5;
 if ((p <= 450)&&(p > 350)) dmax = 2.8;
 if ((p <= 350)&&(p > 250)) dmax = 3.2;
 if (p <= 250)  dmax = 4.0;
 *d_m = dmax;

 if(d<=dmax) return true;
   else return false;
}

double ie2(double a, double a1, double a2, double b1, double b2)
{
 if(a1==a2) {return b2;}
   else {return (b1*(a2-a)+b2*(a-a1))/(a2-a1);}
}

double PE(double e)
{
 int i; double t = 0.0;
 double TT[]={-100.,-95.,-90.,-85.,-80.,-75.,-70.,-65.,-60.,-55.,-50.,-45.,-40.,-35.,-30.,-25.,-20.,-15.,-10.,-5.,0.,5.,10.,15.,20.,25.,30.,35.,40.,45.,50.,55.,60.,65.,70.,75.,80.,85.,90.,95.,100.};
 double EE[]={0.00003471,0.00009051,0.00022315,0.00052248,0.00116659,0.00249304,0.00511623,0.01011300,0.01930615,0.03568326,0.06399660,0.11159821,0.18957147,0.31422640,0.50902995,0.80704278,
             1.25393225,1.91162474,2.86265061,4.21522164,6.10906353,8.72200658,12.27731537,17.05171520,23.38404936,31.68447815,42.44410870,56.24492531,73.76987332,95.81293650,
             123.28903877,157.24359626,198.86154487,249.47567204,310.57408925,383.80669248,470.99047297,574.11355718,695.33787487,837.00037499,1001.61273129};
 if (e<0.00003472) t=-100.; if (e>1001.61273128) t=100.;
 for (i=0;i<40;i++) if ((e>=EE[i])&&(e<EE[i+1])){t=TT[i]+(TT[i+1]-TT[i])*(e-EE[i])/(EE[i+1]-EE[i]);break;}
 return t;
}

bool CondensTrace(int r, double p, double t, double *t_m)
{
 bool Result = false;
 double t2, e;
 double Sm[] = {0.15,0.1};
 e = double(p)*Sm[r]/622.0;
 t2 = PE(e);
 *t_m = t2;
 if (t2>=t) Result = true;
 return Result;
}

bool CondensCheck(int r, double p, double t)
{
 bool Result = false;
 double p2, e;
 double Sm[] = {0.15,0.1};
 e = EP(t);
 p2 = e*622.0/Sm[r];
 if (p>=p2) Result = true;
 return Result;
}

bool uk(TAeroDataAll *Struc, int *x, int *y, double *p, double *t)
{
 int i,i_start,j;
 double P0, P1, T0, T1, Td0, s0, s1, Step, Err;
 for (i=0;i<100;i++)
   if(ValidInt(Struc->data[i].P,'p')&&ValidDouble(Struc->data[i].T/10.,'t'))
    {
     i_start = i;
     break;
    }
 for (i=i_start;i<99;i++)
  {
	 for (j=i+1;j<100;j++)
     if(ValidInt(Struc->data[j].P,'p') && ValidDouble(Struc->data[j].T/10.,'t')) break;
   if(Struc->data[i].T/10.>Struc->data[j].T/10.) {i_start = i; break;}
     else i=j-1;
  }
 P0 = Struc->data[i_start].P;
 T0 = Struc->data[i_start].T/10.;
 Td0 = Struc->data[i_start].T/10.-Struc->data[i_start].D/10.;
 s0 = EP(Td0)/P0;
 Step = 0.5;
 Err = 0.00000001;
 i = 0;
 P1 = P0;
 do
  {
   P1-=Step;
	 T1 = exp(log(T0+273.15)+0.286*(log(P1)-log(P0)))-273.15;
   s1 = EP(T1)/P1;
   i++;
  }
 while((i<10000)&&((s1-s0)>Err));
 *x = DX+MnMath::ftoi_norm(27.35*25.+T1*7.5)+RVS;
 *y = DY+MnMath::ftoi_norm(36.9*25.-Y(P1)*260.)+RVS;
 *p=P1;
 *t=T1;

 return true;
}

bool resizeGp(GeoPoint *&array,int new_size)
{
GeoPoint *newArray=NULL;
newArray=new GeoPoint[new_size];
if(newArray==NULL) return false;
if(array!=NULL)
{for(int i=0;i<new_size;i++)
{
newArray[i].fi=array[i].fi;
newArray[i].la=array[i].la;
}
delete []array;}
array=newArray;
return true;
}

