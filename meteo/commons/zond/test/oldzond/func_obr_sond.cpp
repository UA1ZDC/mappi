#include "func_obr_sond.h"

int Opr_H(TAeroDataAll Struc, TAeroDataAll *Struc1)
{
int i, j, k, k1;
float Tsr;
for(i=0; i<Struc.KolDan; i++)
   if((Struc.data[i].pok_kach[1]=='0')&&(Struc.data[i].pok_kach[2]=='0')) break;
if(i==Struc.KolDan) return 1;

for(i=0;i<Struc.KolDan-1;i++)
m1:  for(j=i+1;j<Struc.KolDan;j++)
    if(Struc.data[i].P==Struc.data[j].P) {
      if((Struc.data[i].pok_kach[1]!='0')&&(Struc.data[j].pok_kach[1]=='0'))
              Struc.data[i].H=Struc.data[j].H;
      if((Struc.data[i].pok_kach[2]!='0')&&(Struc.data[j].pok_kach[2]=='0'))
              Struc.data[i].T=Struc.data[j].T;
      if((Struc.data[i].pok_kach[3]!='0')&&(Struc.data[j].pok_kach[3]=='0'))
              Struc.data[i].D=Struc.data[j].D;
      if((Struc.data[i].pok_kach[4]!='0')&&(Struc.data[j].pok_kach[4]=='0'))
              Struc.data[i].dd=Struc.data[j].dd;
      if((Struc.data[i].pok_kach[5]!='0')&&(Struc.data[j].pok_kach[5]=='0'))
              Struc.data[i].ff=Struc.data[j].ff;
      for(k=j;k<Struc.KolDan-2;k++) Struc.data[k]=Struc.data[k+1];
      Struc.KolDan--;
      goto m1;
                                         }
//////////////////////////
for(i=0;i<Struc.KolDan;i++)
 if((Struc.data[i].pok_kach[6]=='1')) break;
if(i!=Struc.KolDan) Struc.data[i].H=Struc.Height;
/////////////////////////
for(i=0;i<Struc.KolDan;i++)
 if((Struc.data[i].pok_kach[1]=='0')&&(Struc.data[i].pok_kach[2]=='0')) break;

  k=i;
  for(j=i-1;j>=0;j--) {
      if(Struc.data[j].pok_kach[2]!='0') continue;
      Tsr=287.*(273.15+(Struc.data[k].T+Struc.data[j].T)/20.)/9.8;

      for(k1=k-1;k1>=j;k1--) {
           if(Struc.data[k1].pok_kach[1]=='0') continue;
           Struc.data[k1].H=(int)(Struc.data[k].H+Tsr*
                           log(double(Struc.data[k].P)/double(Struc.data[k1].P)));
           Struc.data[k1].pok_kach[1]='0';
                             }
      k=j;
                      }

while(i<Struc.KolDan-1) {
  k=i;
  for(j=k+1;j<Struc.KolDan;j++) {
      if(Struc.data[j].pok_kach[2]!='0') { if(j==Struc.KolDan-1) i=j;
                                           continue;
                                         }
      Tsr=287.*(273.15+(Struc.data[k].T+Struc.data[j].T)/20.)/9.8;
      if(Struc.data[j].pok_kach[1]=='0') {
        if(j==i+1) {i++;break;}
        i=j;
                                          }
      for(k1=k+1;k1<=j;k1++) {
           if(Struc.data[k1].pok_kach[1]=='0') continue;
           Struc.data[k1].H=(int)(Struc.data[k].H+Tsr*
                           log(double(Struc.data[k].P)/double(Struc.data[k1].P)));
           Struc.data[k1].pok_kach[1]='0';
                             }
      if(j==i) break;
      k=j;
                                }

                        }
*Struc1=Struc;
SondInterpolT(Struc1);
SondInterpolD(Struc1);
SondInterpolV(Struc1);
return 0;
}

void SondInterpolT(TAeroDataAll *Struc)
{ int i, i1, i2;
  float gam;
  for(i=0;i<Struc->KolDan-1;i++)
      if((Struc->data[i].pok_kach[1]=='0')&&(Struc->data[i].pok_kach[2]!='0')) {
      for(i1=i-1;i1>=0;i1--)
        if((Struc->data[i1].pok_kach[1]=='0')&&(Struc->data[i1].pok_kach[2]=='0')) break;
      if(i1<0) continue;
      for(i2=i+1;i2<Struc->KolDan;i2++)
        if((Struc->data[i2].pok_kach[1]=='0')&&(Struc->data[i2].pok_kach[2]=='0')) break;
      if(i2==Struc->KolDan) continue;
      gam=float(Struc->data[i2].T-Struc->data[i1].T)/float(Struc->data[i2].H-Struc->data[i1].H);
      Struc->data[i].T=ftoi_norm(Struc->data[i1].T+gam*float(Struc->data[i].H-Struc->data[i1].H));
      Struc->data[i].pok_kach[2]='0';

    }
}

void SondInterpolD(TAeroDataAll *Struc)
{ int i, i1, i2;
  float gam;
  for(i=0;i<Struc->KolDan-1;i++)
      if((Struc->data[i].pok_kach[1]=='0')&&(Struc->data[i].pok_kach[3]!='0')) {
      for(i1=i-1;i1>=0;i1--)
        if((Struc->data[i1].pok_kach[1]=='0')&&(Struc->data[i1].pok_kach[3]=='0')) break;
      if(i1<0) continue;
      for(i2=i+1;i2<Struc->KolDan;i2++)
        if((Struc->data[i2].pok_kach[1]=='0')&&(Struc->data[i2].pok_kach[3]=='0')) break;
      if(i2==Struc->KolDan) continue;
      gam=float(Struc->data[i2].D-Struc->data[i1].D)/float(Struc->data[i2].H-Struc->data[i1].H);
      Struc->data[i].D=ftoi_norm(Struc->data[i1].D+gam*float(Struc->data[i].H-Struc->data[i1].H));
      Struc->data[i].pok_kach[3]='0';
                                                                               }
}

void SondInterpolV(TAeroDataAll *Struc)
{ int i, i1, i2;
  float gam, radian=3.141592654/180., x, y;
      for(i1=0;i1<Struc->KolDan;i1++)
        if((Struc->data[i1].pok_kach[1]=='0')&&(Struc->data[i1].pok_kach[4]=='0')
            &&(Struc->data[i1].pok_kach[5]=='0')) {
      x=-Struc->data[i1].ff*sin((Struc->data[i1].dd)*radian);
      y=-Struc->data[i1].ff*cos((Struc->data[i1].dd)*radian);


      Struc->data[i1].dd=ftoi_norm(x*100);
      Struc->data[i1].ff=ftoi_norm(y*100);
                                                  }
   for(i=0;i<Struc->KolDan-1;i++)
      if((Struc->data[i].pok_kach[1]=='0')&&
         ((Struc->data[i].pok_kach[4]!='0')||(Struc->data[i].pok_kach[5]!='0'))) {
      for(i1=i-1;i1>=0;i1--)
        if((Struc->data[i1].pok_kach[1]=='0')&&(Struc->data[i1].pok_kach[4]=='0')&&(Struc->data[i1].pok_kach[5]=='0')) break;
      if(i1<0) continue;
      for(i2=i+1;i2<Struc->KolDan;i2++)
        if((Struc->data[i2].pok_kach[1]=='0')&&(Struc->data[i2].pok_kach[4]=='0')&&(Struc->data[i2].pok_kach[5]=='0')) break;
      if(i2==Struc->KolDan) continue;

      gam=float(Struc->data[i2].dd-Struc->data[i1].dd)/float(Struc->data[i2].H-Struc->data[i1].H);
      Struc->data[i].dd=ftoi_norm(Struc->data[i1].dd+gam*float(Struc->data[i].H-Struc->data[i1].H));
      gam=float(Struc->data[i2].ff-Struc->data[i1].ff)/float(Struc->data[i2].H-Struc->data[i1].H);
      Struc->data[i].ff=ftoi_norm(Struc->data[i1].ff+gam*float(Struc->data[i].H-Struc->data[i1].H));
      Struc->data[i].pok_kach[4]='0';
      Struc->data[i].pok_kach[5]='0';
  }
}

//------------------------------------------------------------
int GetMeteo11fromKN04(TAeroDataAll KN04, TMeteo11DataAll *Dan, int hMax)
{
  char Zag[19][3]={"02","04","08","12","16","20","24","30","40","50","60","80","10",
                   "12","14","18","22","26","30"};
  char sub_str[64], tlgm11[512];
  int i;
  int end_Level, vs_i;
  double dt;
  float vs_d;

  konstrMeteo11(Dan);

  Dan->Year  = KN04.Term[0];
  Dan->Month = KN04.Term[1];
  Dan->Day   = KN04.Term[2];
  Dan->Hour  = KN04.Term[3];
  Dan->Min   = KN04.Term[4];
  Dan->H     = KN04.Height;
  Dan->Index = KN04.Index;
  strcpy(Dan->Name, KN04.Name);

  strcpy(tlgm11,"METEO11");
  strcat(tlgm11,"-");

  sprintf(sub_str,"%2.2d%2d%1d",Dan->Day,Dan->Hour,Dan->Min);
  strcat(tlgm11,sub_str);
  strcat(tlgm11,"-");

// Мрдкфрювпке ЮЮЮЮ тр юэхрже хжвпзкк
  if(Dan->H >= 0) sprintf(sub_str,"%4.4d",Dan->H);
   else sprintf(sub_str,"5%3.3d",abs(Dan->H));
  strcat(tlgm11, sub_str);
  strcat(tlgm11, "-");

// Ртфеденепке dP0 к dT0
 for(i=0; i<KN04.KolDan; i++) if(KN04.data[i].pok_kach[6]=='1') break;
 if(i==KN04.KolDan) sprintf(sub_str,"/////");
  else
   {
    if(KN04.data[i].pok_kach[0]=='0')
      {
       Dan->dP0 = ftoi_norm(KN04.data[i].P*0.75 - 750.);
       sprintf(sub_str,"%3.3d",Dan->dP0);
      }
      else  sprintf(sub_str,"///");
    strcat(tlgm11, sub_str);
//dT0
    if(KN04.data[i].pok_kach[2]=='0') // Жеотефвжифв
     {
      if(KN04.data[i].pok_kach[3]=='0' && KN04.data[i].pok_kach[0]=='0')  //Децкзкж к двюнепке
         dt=def_dt(KN04.data[i].T/10., KN04.data[i].T/10.-KN04.data[i].D/10., (double)KN04.data[i].P);
        else if(KN04.data[i].pok_kach[3]=='0') dt=Table_5_1(KN04.data[0].T/10.);//Жеотефвжифв
                else dt=0.;
      vs_d = KN04.data[i].T/10. + dt - 15.9;
      Dan->dT0 = ftoi_norm(vs_d);
      if(Dan->dT0 >= 0) vs_i=Dan->dT0;
        else  vs_i=50+abs(Dan->dT0);
      sprintf(sub_str,"%2.2d",vs_i);
     }
      else  sprintf(sub_str,"//");
    strcat(tlgm11, sub_str);
   }

// Фвхаеж япваепкл твфвоежфрю пв хжвпдвфжпэй ифрюпуй
   if(calculLevelParam(KN04, Dan->data, &end_Level, true)!=0) return 1;

   if(hMax!=0) end_Level=def_end_Level(hMax);


// Явтрнпепке чбннежепу
  for(i=0; i<=end_Level; i++)
    {
      sprintf(sub_str,"-");
      strcat(tlgm11, sub_str);
      sprintf(sub_str,"%s",Zag[i]);
      strcat(tlgm11, sub_str);
      if(i<=12) //ъфиттв х ТТ
        {
          if(Dan->data[i].pok_kach[0] == '0') sprintf(sub_str,"%2.2d",Dan->data[i].PP);
            else   sprintf(sub_str,"//");
          strcat(tlgm11, sub_str);
        }
      sprintf(sub_str,"-"); // тфрчен оегди ТТ к ЖЖ
      strcat(tlgm11, sub_str);

      if(Dan->data[i].pok_kach[1]=='0') // TT
        {
          if(Dan->data[i].TT >= 0) vs_i=Dan->data[i].TT;
             else  vs_i=50+abs(Dan->data[i].TT);
          sprintf(sub_str,"%2.2d",vs_i);
        }
        else   sprintf(sub_str,"//");
      strcat(tlgm11, sub_str);

      if(Dan->data[i].pok_kach[2] == '0') sprintf(sub_str,"%2.2d",Dan->data[i].dd);
        else sprintf(sub_str,"//");
      strcat(tlgm11, sub_str);

      if(Dan->data[i].pok_kach[2] == '0') sprintf(sub_str,"%2.2d",Dan->data[i].ff);
        else   sprintf(sub_str,"//");
      strcat(tlgm11, sub_str);
    }
  strcat(tlgm11, "-");
  for(i=KN04.KolDan-1;i>-1;i--)
    if(KN04.data[i].pok_kach[1]=='0' && KN04.data[i].pok_kach[2]=='0') break;  // T & H
  Dan->Ht = ftoi_norm(KN04.data[i].H/1000);
  sprintf(sub_str,"%2.2d",Dan->Ht);
  strcat(tlgm11, sub_str);

  for(i=KN04.KolDan-1;i>-1;i--)
    if(KN04.data[i].pok_kach[1]=='0' && KN04.data[i].pok_kach[4]=='0' && KN04.data[i].pok_kach[5]=='0') // H & dd & ff
       break;
  Dan->Hv = ftoi_norm(KN04.data[i].H/1000);
  sprintf(sub_str,"%2.2d",Dan->Hv);
  strcat(tlgm11, sub_str);
  strcpy(Dan->tlg, tlgm11);

  Dan->start_Level = 0;
  Dan->end_Level = end_Level;
  Dan->KolDan = end_Level;

return 0;
}

//------------------------------------------------------------
int GetLayerfromKN04(TAeroDataAll KN04, TLayerDataAll *Dan, int hMax)
{
  char sub_str[64], tlgm11[512];
  int i,des;
  int end_Level;
//  double dt;
//  float vs_d;

  konstrLayer(Dan);

  Dan->Year  = KN04.Term[0];
  Dan->Month = KN04.Term[1];
  Dan->Day   = KN04.Term[2];
  Dan->Hour  = KN04.Term[3];
  Dan->Min   = KN04.Term[4];
  Dan->H     = KN04.Height;
  Dan->Index = KN04.Index;
  strcpy(Dan->Name, KN04.Name);

  strcpy(tlgm11,"SLOY ");

  sprintf(sub_str,"%5.5ld %2.2d%2.2d0 ",Dan->Index, Dan->Day,Dan->Hour); // 0 - ржнкакженыпву зкцфв ъфиттэ
  strcat(tlgm11,sub_str);

// Фвхаеж япваепкл твфвоежфрю пв хжвпдвфжпэй ифрюпуй
   if(calculLevelParam(KN04, Dan->data, &end_Level, false)!=0) return 1;

   if(hMax!=0) end_Level=def_end_Level(hMax);

// Явтрнпепке чбннежепу CНРЛ
for(i=0; i<=end_Level; i++)
    {

      sprintf(sub_str,"%1.1d",i+1);
      strcat(tlgm11, sub_str);

/*      sprintf(sub_str,"%s",Zag[i]);
      strcat(tlgm11, sub_str);
      if(i<=12) //ъфиттв х ТТ
        {
         if(Dan->data[i].pok_kach[0] == '0') sprintf(sub_str,"%2.2d",Dan->data[i].PP);
            else   sprintf(sub_str,"//");
          strcat(tlgm11, sub_str);
        }
      sprintf(sub_str,"-"); // тфрчен оегди ТТ к ЖЖ
      strcat(tlgm11, sub_str);

      if(Dan->data[i].pok_kach[1]=='0') // TT
        {
          if(Dan->data[i].TT >= 0) vs_i=Dan->data[i].TT;
             else  vs_i=50+abs(Dan->data[i].TT);
          sprintf(sub_str,"%2.2d",vs_i);
        }
        else   sprintf(sub_str,"//");
      strcat(tlgm11, sub_str);
 */


//      if(Dan->data[i].pok_kach[2] == '0') sprintf(sub_str,"%1.1i",Dan->data[i].dd);
        if(Dan->data[i].pok_kach[2] == '0')
         {
           des=Dan->data[i].dd/10;
           sprintf(sub_str,"%2.2d",des);
          }
        else sprintf(sub_str,"//");
      strcat(tlgm11, sub_str);

      if(Dan->data[i].pok_kach[2] == '0') sprintf(sub_str,"%2.2d ",Dan->data[i].ff);
        else   sprintf(sub_str,"// ");
      strcat(tlgm11, sub_str);
      strcpy(Dan->tlg, tlgm11);
    }
/*  strcat(tlgm11, "-");
  for(i=KN04.KolDan-1;i>-1;i--)
    if(KN04.data[i].pok_kach[1]=='0' && KN04.data[i].pok_kach[2]=='0') break;  // T & H
  Dan->Ht = ftoi_norm(KN04.data[i].H/1000);
  sprintf(sub_str,"%2.2d",Dan->Ht);
  strcat(tlgm11, sub_str);

  for(i=KN04.KolDan-1;i>-1;i--)
    if(KN04.data[i].pok_kach[1]=='0' && KN04.data[i].pok_kach[4]=='0' && KN04.data[i].pok_kach[5]=='0') // H & dd & ff
       break;
  Dan->Hv = ftoi_norm(KN04.data[i].H/1000);
  sprintf(sub_str,"%2.2d",Dan->Hv);
  strcat(tlgm11, sub_str);
  strcpy(Dan->tlg, tlgm11);
*/

// Явтрнпепке чбннежепу
/*for(i=0; i<=end_Level; i++)
    {
      sprintf(sub_str,"-");
      strcat(tlgm11, sub_str);
      sprintf(sub_str,"%s",Zag[i]);
      strcat(tlgm11, sub_str);
      if(i<=12) //ъфиттв х ТТ
        {
          if(Dan->data[i].pok_kach[0] == '0') sprintf(sub_str,"%2.2d",Dan->data[i].PP);
            else   sprintf(sub_str,"//");
          strcat(tlgm11, sub_str);
        }
      sprintf(sub_str,"-"); // тфрчен оегди ТТ к ЖЖ
      strcat(tlgm11, sub_str);

      if(Dan->data[i].pok_kach[1]=='0') // TT
        {
          if(Dan->data[i].TT >= 0) vs_i=Dan->data[i].TT;
             else  vs_i=50+abs(Dan->data[i].TT);
          sprintf(sub_str,"%2.2d",vs_i);
        }
        else   sprintf(sub_str,"//");
      strcat(tlgm11, sub_str);

      if(Dan->data[i].pok_kach[2] == '0') sprintf(sub_str,"%2.2d",Dan->data[i].dd);
        else sprintf(sub_str,"//");
      strcat(tlgm11, sub_str);

      if(Dan->data[i].pok_kach[2] == '0') sprintf(sub_str,"%2.2d",Dan->data[i].ff);
        else   sprintf(sub_str,"//");
      strcat(tlgm11, sub_str);
    }
  strcat(tlgm11, "-");
  for(i=KN04.KolDan-1;i>-1;i--)
    if(KN04.data[i].pok_kach[1]=='0' && KN04.data[i].pok_kach[2]=='0') break;  // T & H
  Dan->Ht = ftoi_norm(KN04.data[i].H/1000);
  sprintf(sub_str,"%2.2d",Dan->Ht);
  strcat(tlgm11, sub_str);

  for(i=KN04.KolDan-1;i>-1;i--)
    if(KN04.data[i].pok_kach[1]=='0' && KN04.data[i].pok_kach[4]=='0' && KN04.data[i].pok_kach[5]=='0') // H & dd & ff
       break;
  Dan->Hv = ftoi_norm(KN04.data[i].H/1000);
  sprintf(sub_str,"%2.2d",Dan->Hv);
  strcat(tlgm11, sub_str);
  strcpy(Dan->tlg, tlgm11);
*/
  Dan->start_Level = 0;
  Dan->end_Level = end_Level;
  Dan->KolDan = end_Level;

return 0;
}

//------------------------------------------------------------
int calculLevelParam(TAeroDataAll KN04, Level *Data, int *en_Level, bool meteo11)
{
int i;
double *param=NULL; // Твфвоежф
double *param1=NULL; // Твфвоежф
double *h=NULL;
int count;
double dt;
double mas_h[19]={0.2, 0.4, 0.8, 1.2, 1.6, 2.0, 2.4, 3.0, 4.0, 5.0, 6.0, 8.0,10.0, 12.0, 14.0, 18.0, 22.0, 26.0, 30.0};
double mas_h1[7]={1.5, 3.0, 6.0, 12.0, 18.0, 24.0, 30.0};
double T_stand[19]={15.27, 14.63, 13.37, 12.10, 10.84, 9.57, 8.31, 6.41, 3.24, 0.08, -3.08, -9.41, -15.72, -21.4, -25.7,
                    -31.43, -35.08, -37.61, -39.46};
double R_stand[13]={1.19, 1.18, 1.16, 1.14, 1.12, 1.09, 1.07, 1.04, 0.99, 0.95, 0.9, 0.82, 0.75};
double rez_integr[19], rez_integr1[19], dvs;
char pok_kach[20];
char pok_kach1[8];
int count_rez, ivs;
int start, vs_i=0;

pok_kach[19]=0;
pok_kach1[7]=0;

for(i=0; i<KN04.KolDan; i++) if(KN04.data[i].pok_kach[6]=='1') break;
if(i==KN04.KolDan) return 1; // Пеж Яеонк
start = i;

for(i=start;i<KN04.KolDan;i++)
  if(KN04.data[i].pok_kach[1]=='0') KN04.data[i].H-=KN04.Height;

param = new double[KN04.KolDan];
if(param==NULL) goto m1;
param1 = new double[KN04.KolDan];
if(param1==NULL) goto m1;
h = new double[KN04.KolDan];
if(h==NULL) goto m1;

if (meteo11)
 {
  count=0;
  for(i=start; i<KN04.KolDan; i++)
   {
    if(KN04.data[i].pok_kach[1]!='0') continue; // Юэхржв
    if(KN04.data[i].pok_kach[2]!='0') continue;// Жеотефвжифв

    if(KN04.data[i].pok_kach[3]=='0' && KN04.data[i].pok_kach[0]=='0')  //Децкзкж к двюнепке
       dt = def_dt(KN04.data[i].T/10., KN04.data[i].T/10.-KN04.data[i].D/10.,  (double)KN04.data[i].P);
     else dt = Table_5_1(KN04.data[i].T/10.);
    param[count] = KN04.data[i].T/10. + dt;
    h[count] = KN04.data[i].H;
    count++;
   }
  strcpy(pok_kach,"1111111111111111111"); // 19

  count_rez=itegr_Func(h, param, count, mas_h, rez_integr, pok_kach, 19);
  vs_i = count_rez;

  for(i=0; i<=count_rez; i++)
    if(pok_kach[i]=='0')
      {
        ivs=ftoi_norm(rez_integr[i]-T_stand[i]);
        Data[i].TT=ivs;
        Data[i].pok_kach[1]=pok_kach[i];
      }

  count=0;
  for(i=start; i<KN04.KolDan; i++)
   {
    if(KN04.data[i].pok_kach[0]!='0') continue; // Двюнепке
    if(KN04.data[i].pok_kach[1]!='0') continue; // Юэхржв
    if(KN04.data[i].pok_kach[2]!='0') continue; // Жеотефвжифв дну тнржпрхжк

    if(KN04.data[i].pok_kach[3]=='0')  //Децкзкж
       dt = def_dt(KN04.data[i].T/10., KN04.data[i].T/10.-KN04.data[i].D/10., (double)KN04.data[i].P);
      else dt = Table_5_1(KN04.data[i].T/10.);
     param[count] =  (KN04.data[i].P*100.)/(287.*(KN04.data[i].T/10.+dt+273.15));
     h[count] = KN04.data[i].H;
     count++;
   }
  strcpy(pok_kach,"1111111111111111111"); // 19

  count_rez=itegr_Func(h, param, count, mas_h, rez_integr, pok_kach, 19);
  if(count_rez > vs_i) vs_i=count_rez;

  for(i=0; i<13; i++)
   if(pok_kach[i]=='0')
      {
        ivs=ftoi_norm((rez_integr[i]-R_stand[i])/R_stand[i]*100.);
        Data[i].PP=ivs;
        Data[i].pok_kach[0]=pok_kach[i];
      }

 } // end if(meteo11)

count=0;
for(i=start; i<KN04.KolDan; i++)
   {
    if(KN04.data[i].pok_kach[1]!='0') continue; // Юэхржв
    if(KN04.data[i].pok_kach[4]!='0') continue; // Юежеф (ярп.)
    if(KN04.data[i].pok_kach[5]!='0') continue; // Юежеф (оеф.)

    param[count] = KN04.data[i].dd/100.;
    param1[count] = KN04.data[i].ff/100.;
    h[count] = KN04.data[i].H;
    count++;
   }
strcpy(pok_kach,"1111111111111111111"); // 19
strcpy(pok_kach1,"1111111"); // 7

if(meteo11) count_rez=itegr_Func(h, param, count, mas_h, rez_integr, pok_kach, 19);
  else      count_rez=itegr_Func(h, param, count, mas_h1, rez_integr, pok_kach1, 7);

if(count_rez > vs_i) vs_i=count_rez;

if(meteo11) count_rez=itegr_Func(h, param1, count, mas_h, rez_integr1, pok_kach, 19);
  else      count_rez=itegr_Func(h, param1, count, mas_h1, rez_integr1, pok_kach1, 7);
if(count_rez > vs_i) vs_i=count_rez;
(*en_Level=vs_i);

if(meteo11)
{
  for(i=0; i<=count_rez; i++)
   if(pok_kach[i]=='0') // && meteo11
//   if(pok_kach1[i]=='0')
      {
        Data[i].ff=ftoi_norm(sqrt(rez_integr[i]*rez_integr[i]+rez_integr1[i]*rez_integr1[i])) ;
        if(Data[i].ff == 0) Data[i].dd=0;
          else
            {
              dvs=atan2(rez_integr1[i],rez_integr[i])*180./3.1415926 + 180.;
              /*if(meteo11) */
              Data[i].dd=ftoi_norm((M180To180(dvs)+180.)/6.);
            }
        Data[i].pok_kach[2]=pok_kach[i];
        Data[i].pok_kach[3]=pok_kach[i];
      }
}
else
{
  for(i=0; i<=count_rez; i++)
   if(pok_kach1[i]=='0')
      {
        Data[i].ff=ftoi_norm(sqrt(rez_integr[i]*rez_integr[i]+rez_integr1[i]*rez_integr1[i])) ;
        if(Data[i].ff == 0) Data[i].dd=0;
          else
            {
              dvs=atan2(rez_integr1[i],rez_integr[i])*180./3.1415926 + 180.;
              /*if(meteo11) */
              Data[i].dd=ftoi_norm((M180To180(dvs)+180.));  // денкжы пв 6
            }
        Data[i].pok_kach[2]=pok_kach1[i];  //хюрл pok_kach1
        Data[i].pok_kach[3]=pok_kach1[i]; // хюрл pok_kach1
      }
}

m1:
if(param != NULL) { delete []param; param=NULL; }
if(param1 != NULL) { delete []param1; param1=NULL; }
if(h != NULL)  { delete []h;  h=NULL; }

return 0;
}

//------------------------------------------------------------
double def_dt(double T0, double Td0, double P0)
{
double e;
e=6.1*pow(10,7.45*Td0/(235.+Td0));
return (0.378*(T0+273.15)*e/P0);
}
//------------------------------------------------------------
double Table_5_1(double T)
{
if(T < -20.) return 0.;
if(T > 50. ) return 7.4;

if(T<=50.  && T >45.)  return 7.4;
if(T<=45.  && T >40.)  return 5.8;
if(T<=40.  && T >35.)  return 4.4;
if(T<=35.  && T >30.)  return 3.3;
if(T<=30.  && T >25.)  return 2.4;
if(T<=25.  && T >20.)  return 1.8;
if(T<=20.  && T >15.)  return 1.3;
if(T<=15.  && T >10.)  return 0.9;
if(T<=10.  && T >5. )  return 0.6;
if(T<=5.   && T >0. )  return 0.5;
if(T<=0.   && T >-5.)  return 0.3;
if(T<=-5.  && T >-10.) return 0.2;
if(T<=-10. && T >-15.) return 0.1;
if(T<=-15. && T >-20.) return 0.0;

return 0.;
}

//------------------------------------------------------------
int itegr_Func(double *h, double *param, int count, double *mas_h, double *rez_integr, char *pok_kach, int count_rez)
/*
Ципмзку itegr_Func трниавеж ю мваехжюе кхйрдпэй двппэй:
  h - имвявжены пв оввхкю япваепекл вфъиоежпв
  param - имвявжены пв овххкю япваепкл ципмзкк
  count - мрнкаехжюр шнеоепжрю ю овххкювй h к param
к тфркяюрдкж кпжеъфкфрювпке трниаеппрл ципмзкк х юэакхнепкео хфедпкй япваепкл
пв кпжефювнвй рж 0 др mas_h[i], i=0(1)count_rez.
Феяиныжвжэ троеьвбжху ю овххкю rez_integr.
ципмзку юряюфвьвеж акхнр хнрею, дну мржрфэй тфркяюрдкнххы юэакхнепку.
Дну ртфеденепку мваехжюв шнеоепжрю овххкюв rez_integr ююедеп овххкю pok_kach
жрл ге фвяоефпрхжк, ажр к rez_integr.
Ехнк pok_kach[i]="0", жр япваепкб оргпр дрюефужы
*/
{

int i;
double vs;
int ret_val=0;

if(count==1) return ret_val;

for(i=0; i<count_rez; i++)
 {
   pok_kach[i] = integr_Func_Value(h, param, count, mas_h[i]*1000.,&vs);
   if(pok_kach[i] == '0')
     {
       rez_integr[i]=vs;
       ret_val=i;
     }
 }

return ret_val;
}

//------------------------------------------------------------
char integr_Func_Value(double *h, double *param, int count, double h_integr, double *rez)
{
char ret_val='1';
double sum=0., dvs;
int ivs, i;

ivs=searh_inf(h_integr, h, count);
if(ivs<0)
 {
   ret_val='1';
   *rez=0.;
 }
 else
   {
     dvs=(param[ivs+1] - param[ivs])/(h[ivs+1]-h[ivs])*(h_integr-h[ivs])+param[ivs];
     for(i=0; i<ivs; i++) sum = sum + (param[i+1]+param[i])*(h[i+1]-h[i]);
     sum=sum+(param[ivs]+dvs)*(h_integr-h[ivs]);
     *rez=sum/h_integr*0.5;
     ret_val='0';
   }

return ret_val;
}

int def_end_Level(int hMax)
{
 int Level[20] ={0,200,400,800,1200,1600,2000,2400,3000,4000,5000,6000,8000,10000,
                12000,14000,18000,22000,26000,30000};
 int i;

 for(i=0;i<19;i++) if(hMax > Level[i] && hMax <= Level[i+1]) return i;

 return 18;

/* int Level1[8] ={0,1500,3000,6000,12000,18000,24000,30000};
 int i;

 for(i=0;i<7;i++) if(hMax > Level1[i] && hMax <= Level1[i+1]) return i;

 return 6;  */
}

int oprAeroParamFromP(TAeroDataBasa RecordKN04,int P,Uroven *uroven)
{
int i;
TAeroDataAll  Struc_vs, Struc_itog;

//1. дПВБЧМЕОЙЕ ОПЧПК ПУПВПК ФПЮЛЙ РП ЧЕФТХ
RecordKN04.s[1].data[RecordKN04.s[1].KolDan].P=P*10;
strcpy(RecordKN04.s[1].data[RecordKN04.s[1].KolDan].pok_kach,"0111113");
RecordKN04.s[1].data[RecordKN04.s[1].KolDan].KolDan++;
RecordKN04.s[1].KolDan++;
// рТПЧЕТЛБ РТЙОБДМЕЦОПУФЙ т ДЙБРБЪПОХ ЙЪНЕОЕОЙС ДБЧМЕОЙС Ч RecordKN04

//2. рТЕПВТБЪПЧБОЙЕ УФТХЛФХТЩ TAeroDataBasa Ч УФТХЛФХТХ TAeroDataBasaAll
konstrAero(&Struc_vs);
konstrAero(&Struc_itog);
Preobr(&RecordKN04,&Struc_vs);
Opr_H(Struc_vs, &Struc_itog);

//3. жПТНЙТПЧБОЙЕ ЧЩИПДОПК УФТХЛФХТЩ uroven ЙЪ УФТХЛФХТЩ TAeroDataBasaAll
for(i=0;i<Struc_itog.KolDan;i++)
  if(Struc_itog.data[i].P==P) break;
if((i==Struc_itog.KolDan)&&(Struc_itog.data[i].P!=P)) return -1;

uroven->KolDan=0;
uroven->P=Struc_itog.data[i].P;
if(Struc_itog.data[i].pok_kach[0]!='1') uroven->KolDan++;

uroven->H=Struc_itog.data[i].H;
if(Struc_itog.data[i].pok_kach[1]!='1') uroven->KolDan++;

uroven->T=Struc_itog.data[i].T;
if(Struc_itog.data[i].pok_kach[2]!='1') uroven->KolDan++;

uroven->D=Struc_itog.data[i].D;
if(Struc_itog.data[i].pok_kach[3]!='1') uroven->KolDan++;

uroven->dd=Struc_itog.data[i].dd;
if(Struc_itog.data[i].pok_kach[4]!='1') uroven->KolDan++;

uroven->ff=Struc_itog.data[i].ff;
if(Struc_itog.data[i].pok_kach[5]!='1') uroven->KolDan++;

strcpy(uroven->pok_kach,Struc_itog.data[i].pok_kach);

return 0;
}


