#include "func_fillData.h"

void fillWeatherDataAll(TWeatherDataAll *struc)
{
 struc->s1.P=1013.2;
 struc->s1.P0=1014.0;
 struc->s1.T=12.3;
 struc->s1.Td=-0.8;
 struc->s1.Tend=11.2;
 struc->s1.R=2.3;

 struc->s1.V=96;
 struc->s1.TipTend=2;
 struc->s1.H=3;
 struc->s1.N=-1;
 struc->s1.w=13;
 struc->s1.W1=7;
 struc->s1.W2=6;
 struc->s1.Nh=3;
 struc->s1.Cl=2;
 struc->s1.Cm=6;
 struc->s1.Ch=7;
 struc->s1.dd=20;
 struc->s1.ff=33;
 struc->s1.Time=6;
 struc->s1.Hgor=1500;
 strcpy(struc->s1.pok_kach,"00000000000000000000");

 struc->s2.Tw=4.1;

 struc->s2.Ds=1;
 struc->s2.Vs=6;
 struc->s2.Pwa=25;
 struc->s2.Hwa=7;
 struc->s2.Pw=10;
 struc->s2.Hw=30;
 struc->s2.Dw1=22;
 struc->s2.Dw2=23;
 struc->s2.Pw1=20;
 struc->s2.Hw1=6;
 struc->s2.Pw2=15;
 struc->s2.Hw2=7;
 struc->s2.Is=1;
 struc->s2.Es=22;
 struc->s2.Rs=8;
 struc->s2.Ci=1;
 struc->s2.Si=4;
 struc->s2.Bi=5;
 struc->s2.Di=2;
 struc->s2.Zi=2;
 strcpy(struc->s2.pok_kach,"000000000000000000000");

 struc->s3.Tmax=0.2;
 struc->s3.Tmin=15.3;
 struc->s3.Tsr=8.1;
 struc->s3.R24=15.1;

 struc->s3.Tpochv=12;
 struc->s3.Ns=8;
 struc->s3.C=6;
 struc->s3.Hs=250;
 struc->s3.Es=2;
 struc->s3.S=15;
 struc->s3.E=3;
 struc->s3.Sp=12;
 struc->s3.sp=22;
 strcpy(struc->s3.pok_kach,"0000000000000");
 };

void fillWeatherDataAllBlank(TWeatherDataAll *struc)
{
 struc->s1.P=0.;
 struc->s1.P0=0.;
 struc->s1.T=0.;
 struc->s1.Td=0.;
 struc->s1.Tend=0.;
 struc->s1.R=0.;

 struc->s1.V=0;
 struc->s1.TipTend=0;
 struc->s1.H=0;
 struc->s1.N=0;
 struc->s1.w=0;
 struc->s1.W1=0;
 struc->s1.W2=0;
 struc->s1.Nh=0;
 struc->s1.Cl=0;
 struc->s1.Cm=0;
 struc->s1.Ch=0;
 struc->s1.dd=0;
 struc->s1.ff=0;
 struc->s1.Time=0;
 struc->s1.Hgor=0;
 strcpy(struc->s1.pok_kach,"111111111111111111111");

 struc->s2.Tw=0.;

 struc->s2.Ds=0;
 struc->s2.Vs=0;
 struc->s2.Pwa=0;
 struc->s2.Hwa=0;
 struc->s2.Pw=0;
 struc->s2.Hw=0;
 struc->s2.Dw1=0;
 struc->s2.Dw2=0;
 struc->s2.Pw1=0;
 struc->s2.Hw1=0;
 struc->s2.Pw2=0;
 struc->s2.Hw2=0;
 struc->s2.Is=0;
 struc->s2.Es=0;
 struc->s2.Rs=0;
 struc->s2.Ci=0;
 struc->s2.Si=0;
 struc->s2.Bi=0;
 struc->s2.Di=0;
 struc->s2.Zi=0;
 strcpy(struc->s2.pok_kach,"111111111111111111111");

 struc->s3.Tmax=0.;
 struc->s3.Tmin=0.;
 struc->s3.Tsr=0.;
 struc->s3.R24=0.;

 struc->s3.Tpochv=0;
 struc->s3.Ns=0;
 struc->s3.C=0;
 struc->s3.Hs=0;
 struc->s3.Es=0;
 struc->s3.S=0;
 struc->s3.E=0;
 struc->s3.Sp=0;
 struc->s3.sp=0;
 strcpy(struc->s3.pok_kach,"1111111111111");
 };

void fillUroven(Uroven *struc)
{
  struc->P=8500;
  struc->H=1533;
  struc->T=-122;
  struc->D=42;
  struc->dd=320;
  struc->ff=37;
  strcpy(struc->pok_kach,"0000000");
}

void fillGRWUroven(GRWUroven *struc)
{
  struc->dd=320;
  struc->ff=37;
  struc->P=1013.5;
  struc->T=-0.2;
  struc->Td=-15.1;
  strcpy(struc->pok_kach,"00000");
}

void fillSeaDataAll(TSeaDataAll *struc)
{

  struc->kol_gr_3=1;
  struc->kol_gr_6=1;
  struc->kol_gr_7=1;
  struc->dd=320;
  struc->ff=37;
  struc->V=10;
  struc->TwTwTw=-1.1;
  struc->TT=-8;
  struc->SrTwTwTw=-1.0;
  struc->SrTTT=-6.9;
  struc->HwHw=9.4;
  struc->Pw=6;
  struc->Dw=1;
  struc->Me=4;
  struc->Le=5;
  struc->ii=35;
  struc->B=2;
  struc->b=3;
  struc->HnHn=4;
  struc->hehe=3;
  strcpy(struc->pok_kach,"00000000000000000");
}

void fillSea_gr_3(TSea_gr_3 *struc)
{
  struc->ah=9;
  struc->hshshs=11;
  struc->GtGt=12;
  struc->hshs=1;
  strcpy(struc->pok_kach,"0000");
}

void fillSea_gr_6(TSea_gr_6 *struc)
{
  struc->Mi=4;
  struc->C1=6;
  struc->C2=6;
  struc->Pit=5;
  strcpy(struc->pok_kach,"0000");
}

void fillSea_gr_7(TSea_gr_7 *struc)
{
  struc->A1A1=67;
  struc->A2A2=72;
  strcpy(struc->pok_kach,"00");
}

void konstrWeather(struct TWeatherDataAll *Sond)
{ int i;
  Sond->Tip=0;
  Sond->Index=0;
  Sond->Pok_Privyaz=1;
  strcpy(Sond->Name,"");
  Sond->KolDan=0;
  strcpy(Sond->pok_inf,"000");
  for(i=0; i<4; i++) Sond->Koord[i]=0;
  for(i=0; i<4; i++) Sond->Quadrat[i]=0;
  strcpy(Sond->Nanos,"");
// Weather
  Sond->s1.KolDan=0;
  Sond->s1.P=0; Sond->s1.P0=0; Sond->s1.T=0; Sond->s1.Td=0; Sond->s1.Tend=0;
  Sond->s1.V=0; Sond->s1.R=0;
  Sond->s1.TipTend=0; Sond->s1.H=0; Sond->s1.N=0;
  Sond->s1.w=0; Sond->s1.W1=0; Sond->s1.W2=0;
  Sond->s1.Nh=0; Sond->s1.Cl=0; Sond->s1.Cm=0; Sond->s1.Ch=0;
  Sond->s1.dd=0; Sond->s1.ff=0;
  strcpy(Sond->s1.pok_kach,"111111111111111111111");
// Water
  Sond->s2.KolDan=0;
  Sond->s2.Tw=0;
  Sond->s2.Ds=0; Sond->s2.Vs=0; Sond->s2.Pwa=0; Sond->s2.Hwa=0; Sond->s2.Pw=0;
  Sond->s2.Hw=0; Sond->s2.Dw1=0; Sond->s2.Dw2=0; Sond->s2.Pw1=0; Sond->s2.Hw1=0;
  Sond->s2.Pw2=0; Sond->s2.Hw2=0; Sond->s2.Is=0; Sond->s2.Es=0; Sond->s2.Rs=0;
  Sond->s2.Ci=0; Sond->s2.Si=0; Sond->s2.Bi=0; Sond->s2.Di=0; Sond->s2.Zi=0;
  strcpy(Sond->s2.pok_kach,"111111111111111111111");
// Utochnen

  Sond->s3.KolDan=0;
  Sond->s3.Tmax=0; Sond->s3.Tmin=0; Sond->s3.Tsr=0;
  Sond->s3.Tpochv=0; Sond->s3.R24=0;
  Sond->s3.Ns=0; Sond->s3.C=0; Sond->s3.Hs=0; Sond->s3.Es=0;
  Sond->s3.S=0; Sond->s3.E=0; Sond->s3.Sp=0; Sond->s3.sp=0;
  strcpy(Sond->s3.pok_kach,"1111111111111");
}

void konstrShar(TSharDataAll *Sond)
{
 int i;
 Sond->Tip=0;
 Sond->Index=0;
 Sond->Pok_Privyaz=1;
 strcpy(Sond->Name,"");
 Sond->KolDan=0;
 strcpy(Sond->pok_inf,"0000");
 Sond->Tip_Sond=-1;
 for(i=0; i<4; i++) Sond->Koord[i]=0;
 for(i=0; i<2; i++) Sond->Quadrat[i]=0;
 for(i=0; i<200; i++)
  {
    Sond->data[i].KolDan=0;
    Sond->data[i].P=9999;
    Sond->data[i].H=9999;
    Sond->data[i].dd=9999;
    Sond->data[i].ff=9999;
    strcpy((char *)&Sond->data[i].pok_kach[0],"11118");
  }
}

void konstrSharBasa(TSharDataBasa *Sond)
{
 int i, j;
 Sond->Tip=0;
 Sond->Index=0;
 Sond->Pok_Privyaz=1;
 strcpy(Sond->Name,"");
 Sond->KolDan=0;
 strcpy(Sond->pok_inf,"0000");
 Sond->Tip_Sond=-1;
 for(i=0; i<4; i++) Sond->Koord[i]=0;
 for(i=0; i<2; i++) Sond->Quadrat[i]=0;
 for(j=0; j<4; j++)
   {
     Sond->s[j].KolDan=0; Sond->s[j].Tip_Gr=-1;
     for(i=0; i<100; i++)
      {
       Sond->s[j].data[i].KolDan=0;
       Sond->s[j].data[i].P=9999;
       Sond->s[j].data[i].H=9999;
       Sond->s[j].data[i].dd=9999;
       Sond->s[j].data[i].ff=9999;
       strcpy((char *)&Sond->s[j].data[i].pok_kach[0],"11118");
      }
   }
}

void konstrAero(TAeroDataAll *Sond)
{
 int i;
 Sond->Tip=0; // 0 -стационарная, 1 - судовая
 Sond->Index=0; // Для стационарных
 Sond->Pok_Privyaz=1; // 0 - привязана, 1 - не привязана (по координатам)
 strcpy(Sond->Name,"");
 Sond->KolDan=0;
 strcpy(Sond->pok_inf,"0000");
 for(i=0; i<4; i++) Sond->Koord[i]=0;
 for(i=0; i<4; i++) Sond->Quadrat[i]=0;
 for(i=0; i<200; i++)
   {
    Sond->data[i].KolDan=0;
    Sond->data[i].P=9999;
    Sond->data[i].H=9999;
    Sond->data[i].T=9999;
    Sond->data[i].D=9999;
    Sond->data[i].dd=9999;
    Sond->data[i].ff=9999;
    strcpy((char *)&Sond->data[i].pok_kach[0],"0000008");
   }
}


void konstrMeteo11(TMeteo11DataAll *Dan)
{
int i;
Dan->Index=9999;
bzero(Dan->Name,sizeof(Dan->Name));
Dan->P=9;
Dan->X=9999;
Dan->Y=9999;
Dan->Year=9999;
Dan->Month=99;
Dan->Day=99;
Dan->Hour=99;
Dan->Min=99;
Dan->H=999999;
Dan->dP0=999;
Dan->dT0=999;
for(i=0;i<19;i++)
 {
   Dan->data[i].PP=99999;
   Dan->data[i].TT=99999;
   Dan->data[i].dd=99999;
   Dan->data[i].ff=99999;
   strcpy(Dan->data[i].pok_kach,"1111");
 }
Dan->Ht=99999;
Dan->Hv=99999;
Dan->start_Level=0;
Dan->end_Level=0;
Dan->KolDan=0;
bzero(Dan->tlg,sizeof(Dan->tlg));
}

void konstrLayer(TLayerDataAll *Dan)
{
int i;
Dan->Index=9999;
bzero(Dan->Name,sizeof(Dan->Name));
Dan->P=9;
Dan->X=9999;
Dan->Y=9999;
Dan->Year=9999;
Dan->Month=99;
Dan->Day=99;
Dan->Hour=99;
Dan->Min=99;
Dan->H=999999;
for(i=0;i<7;i++)
 {
   Dan->data[i].PP=99999;
   Dan->data[i].TT=99999;
   Dan->data[i].dd=99999;
   Dan->data[i].ff=99999;
   strcpy(Dan->data[i].pok_kach,"1111");
 }
Dan->start_Level=0;
Dan->end_Level=0;
Dan->KolDan=0;
bzero(Dan->tlg,sizeof(Dan->tlg));
}


TSharDataAll PreobrShar(TSharDataBasa Struc)
{
 TSharDataAll Param, SondCC;
 Uroven_SHAR dataVS;
 int i, j, k, k1, KolDanAA, KolDanBB, KolDanCC, KolDanDD, Num;
 konstrShar(&Param);
 ZagolovokShar(&Param,Struc);
 if(!Struc.KolDan) return Param;
 konstrShar(&SondCC);
 KolDanAA=Struc.s[0].KolDan;
 KolDanBB=Struc.s[1].KolDan;
 KolDanCC=Struc.s[2].KolDan;
 KolDanDD=Struc.s[3].KolDan;

 k=KolDanAA+KolDanBB;
 if(k!=0)
  {
   if(k>200) KolDanBB=200-KolDanAA;
   if(KolDanAA) {
                 for(i=0; i<KolDanAA; i++) Param.data[i]=Struc.s[0].data[i];
                 if(KolDanBB!=0) for(i=0; i<KolDanBB; i++) Param.data[KolDanAA+i]=Struc.s[1].data[i];
                }
      else for(i=0; i<KolDanBB; i++) Param.data[i]=Struc.s[1].data[i];
  }
 k=KolDanAA+KolDanBB;
 k1=KolDanCC+KolDanDD;
 if(k1!=0)
  {
   if(k1>200) KolDanDD=200-KolDanCC;
   if(KolDanCC) {
                    for(i=0; i<KolDanCC; i++)
                          SondCC.data[i]=Struc.s[2].data[i];
                    if(KolDanDD!=0) for(i=0; i<KolDanDD; i++)
                          SondCC.data[KolDanCC+i]=Struc.s[3].data[i];
                }
      else for(i=0; i<KolDanDD; i++) SondCC.data[i]=Struc.s[3].data[i];
  }
 k1=KolDanCC+KolDanDD;
 if(k1!=0) {
   if((k+k1)>200) k1=200-k;
   for(i=0; i<k1; i++) Param.data[k+i]=SondCC.data[i];
          }
 Num=Param.Tip_Sond;
 if(Num!=-1)
  {
    k+=k1;
    Param.KolDan=k;
    k1=k-1;
m1: for(i=0; i<k1; i++)
     {
      if((Param.data[i].pok_kach[4]=='8')||((Param.data[i].pok_kach[Num]!='0')&&
         (Param.data[i].pok_kach[Num]!='3'))) continue;
      for(j=i+1; j<k; j++)
       {
        if((Param.data[j].pok_kach[4]=='8')||((Param.data[j].pok_kach[Num]!='0')&&
           (Param.data[j].pok_kach[Num]!='3'))) continue;
        if(!Num) {if(Param.data[i].P>=Param.data[j].P) break;}
          else {if(Param.data[i].H<=Param.data[j].H) break;}
        dataVS=Param.data[i];
        Param.data[i]=Param.data[j];
        Param.data[j]=dataVS;
        goto m1;
       }
     }
  }
 return Param;
}

TAeroDataAll Preobr(TAeroDataBasa Struc)
{
 TAeroDataAll Param, SondCC;
 Uroven dataVS;
 int i, j, k, k1, KolDanAA, KolDanBB, KolDanCC, KolDanDD;

 konstrAero(&Param);
 Zagolovok(&Param,Struc);
 if(!Struc.KolDan) return Param;
 konstrAero(&SondCC);

 KolDanAA=Struc.s[0].KolDan;
 KolDanBB=Struc.s[1].KolDan;
 KolDanCC=Struc.s[2].KolDan;
 KolDanDD=Struc.s[3].KolDan;

 k=KolDanAA+KolDanBB;
 if(k!=0)
  {
   if(k>200) KolDanBB=200-KolDanAA;
   if(KolDanAA) {
                 for(i=0; i<KolDanAA; i++) Param.data[i]=Struc.s[0].data[i];
                 if(KolDanBB!=0) for(i=0; i<KolDanBB; i++) Param.data[KolDanAA+i]=Struc.s[1].data[i];
                }
     else for(i=0; i<KolDanBB; i++) Param.data[i]=Struc.s[1].data[i];
  }
 k=KolDanAA+KolDanBB;

 k1=KolDanCC+KolDanDD;
 if(k1!=0)
  {
   if(k1>200) KolDanDD=200-KolDanCC;
   if(KolDanCC) {
                 for(i=0; i<KolDanCC; i++) SondCC.data[i]=Struc.s[2].data[i];
                 if(KolDanDD!=0) for(i=0; i<KolDanDD; i++) SondCC.data[KolDanCC+i]=Struc.s[3].data[i];
                }
     else for(i=0; i<KolDanDD; i++) Param.data[i]=Struc.s[3].data[i];
  }
 k1=KolDanCC+KolDanDD;

 if(k1!=0)
  {
   if((k+k1)>200) k1=200-k;
   for(i=0; i<k1; i++) Param.data[k+i]=SondCC.data[i];
  }
 k+=k1;
 Param.KolDan=k;
 k1=k-1;
m1:  for(i=0; i<k1; i++)
       {
        if((Param.data[i].pok_kach[6]=='8')|| ((Param.data[i].pok_kach[0]!='0')&&
           (Param.data[i].pok_kach[0]!='3'))) continue;
        for(j=i+1; j<k; j++)
          {
           if((Param.data[j].pok_kach[6]=='8')||((Param.data[j].pok_kach[0]!='0')&&
              (Param.data[j].pok_kach[0]!='3'))) continue;
           if(Param.data[i].P>=Param.data[j].P) break;
           dataVS=Param.data[i];
           Param.data[i]=Param.data[j];
           Param.data[j]=dataVS;
           goto m1;
          }
       }
 return Param;
}

bool PreobrShar1(TSharDataAll Struc_all, TSharDataBasa *Struc)
{
 int i, j, k, k1;

 j=0;
 k=0;

 for(i=0; i<200; i++)
    if(Struc_all.data[i].P<1000 || Struc_all.data[i].pok_kach[4]=='8') break;
 if(i==200) return false;
 k=i;
 for(i=k; i<200; i++)
    if(Struc_all.data[i].pok_kach[4]=='8') break;
 k1=i;

 for(i=0; i<k; i++)
  {
  if(Struc_all.data[i].pok_kach[4]=='1')
    {
     Struc->s[0].data[j]=Struc_all.data[i];
     j++;
    }
  if(Struc_all.data[i].pok_kach[4]=='0')
    {
     Struc->s[0].data[j]=Struc_all.data[i];
     j++;
    }
  if(Struc_all.data[i].pok_kach[4]=='5')
    {
     Struc->s[0].data[j]=Struc_all.data[i];
     j++;
    }
  }
 Struc->s[0].KolDan=j;

 j=0;
 for(i=k; i<k1; i++)
  {
  if(Struc_all.data[i].pok_kach[4]=='0')
    {
     Struc->s[2].data[j]=Struc_all.data[i];
     j++;
    }
  if(Struc_all.data[i].pok_kach[4]=='5')
    {
     Struc->s[2].data[j]=Struc_all.data[i];
     j++;
    }
  }
 Struc->s[2].KolDan=j;

 j=0;
 for(i=0; i<k; i++)
  {
  if(Struc_all.data[i].pok_kach[4]=='1')
    {
     Struc->s[1].data[j]=Struc_all.data[i];
     j++;
    }
  if(Struc_all.data[i].pok_kach[4]=='3')
    {
     Struc->s[1].data[j]=Struc_all.data[i];
     j++;
    }
  }
 Struc->s[1].KolDan=j;

 j=0;
 for(i=k; i<k1; i++)
  {
  if(Struc_all.data[i].pok_kach[4]=='3')
    {
     Struc->s[3].data[j]=Struc_all.data[i];
     j++;
    }
  }
 Struc->s[3].KolDan=j;

 return true;
}

bool Preobr1(TAeroDataAll Struc_all, TAeroDataBasa *Struc)
{
 int i, j, k, k1;

 j=0;
 k=0;

 for(i=0; i<200; i++)
    if(Struc_all.data[i].P<1000 || Struc_all.data[i].pok_kach[6]=='8') break;
 if(i==200) return false;
 k=i;
 for(i=k; i<200; i++)
    if(Struc_all.data[i].pok_kach[6]=='8') break;
 k1=i;

 for(i=0; i<k; i++)
  {
  if(Struc_all.data[i].pok_kach[6]=='1')
    {
     Struc->s[0].data[j]=Struc_all.data[i];
     j++;
    }
  if(Struc_all.data[i].pok_kach[6]=='0')
    {
     Struc->s[0].data[j]=Struc_all.data[i];
     j++;
    }
  if(Struc_all.data[i].pok_kach[6]=='4')
    {
     Struc->s[0].data[j]=Struc_all.data[i];
     j++;
    }
  if(Struc_all.data[i].pok_kach[6]=='5')
    {
     Struc->s[0].data[j]=Struc_all.data[i];
     j++;
    }
  }
 Struc->s[0].KolDan=j;

 j=0;
 for(i=k; i<k1; i++)
  {
  if(Struc_all.data[i].pok_kach[6]=='0')
    {
     Struc->s[2].data[j]=Struc_all.data[i];
     j++;
    }
  if(Struc_all.data[i].pok_kach[6]=='4')
    {
     Struc->s[2].data[j]=Struc_all.data[i];
     j++;
    }
  if(Struc_all.data[i].pok_kach[6]=='5')
    {
     Struc->s[2].data[j]=Struc_all.data[i];
     j++;
    }
  }
 Struc->s[2].KolDan=j;

 j=0;
 for(i=0; i<k; i++)
  {
  if(Struc_all.data[i].pok_kach[6]=='1')
    {
     Struc->s[1].data[j]=Struc_all.data[i];
     j++;
    }
  if(Struc_all.data[i].pok_kach[6]=='2')
    {
     Struc->s[1].data[j]=Struc_all.data[i];
     j++;
    }
  if(Struc_all.data[i].pok_kach[6]=='3')
    {
     Struc->s[1].data[j]=Struc_all.data[i];
     j++;
    }
  }
 Struc->s[1].KolDan=j;

 j=0;
 for(i=k; i<k1; i++)
  {
  if(Struc_all.data[i].pok_kach[6]=='2')
    {
     Struc->s[3].data[j]=Struc_all.data[i];
     j++;
    }
  if(Struc_all.data[i].pok_kach[6]=='3')
    {
     Struc->s[3].data[j]=Struc_all.data[i];
     j++;
    }
  }
 Struc->s[3].KolDan=j;

 return true;
}

void ZagolovokShar(TSharDataAll *Sond, TSharDataBasa Struc)
{
 int i;
 Sond->Tip=Struc.Tip;
 Sond->Index=Struc.Index;
 Sond->Pok_Privyaz=Struc.Pok_Privyaz;
 strcpy(Sond->Name,Struc.Name);
 strcpy(Sond->pok_inf,Struc.pok_inf);
 Sond->Tip_Sond=Struc.Tip_Sond;
 for(i=0; i<4; i++) Sond->Koord[i]=Struc.Koord[i];
 for(i=0; i<2; i++) Sond->Quadrat[i]=Struc.Quadrat[i];
}

void Zagolovok(TAeroDataAll *Sond, TAeroDataBasa Struc)
{
 int i;
 Sond->Tip=Struc.Tip;
 Sond->Index=Struc.Index;
 Sond->Height=Struc.Height;
 Sond->Pok_Privyaz=Struc.Pok_Privyaz;
 strcpy(Sond->Name,Struc.Name);
 strcpy(Sond->pok_inf,Struc.pok_inf);
 for(i=0; i<4; i++) Sond->Koord[i]=Struc.Koord[i];
 for(i=0; i<4; i++) Sond->Quadrat[i]=Struc.Quadrat[i];
 for(i=0; i<5; i++) Sond->Term[i]=Struc.Term[i];
}

int ZamenaKN01(TWeatherDataAll *Struc1, TWeatherDataAll Struc0)
{ int i;
  i=0;
  if(Struc1->s1.KolDan<Struc0.s1.KolDan)
          {Struc1->pok_inf[0]='1'; Struc1->s1=Struc0.s1; i=1;}
  if(Struc1->s2.KolDan<Struc0.s2.KolDan)
          {Struc1->pok_inf[1]='1'; Struc1->s2=Struc0.s2; i=1;}
  if(Struc1->s3.KolDan<Struc0.s3.KolDan)
          {Struc1->pok_inf[2]='1'; Struc1->s3=Struc0.s3; i=1;}
  if(i) Struc1->KolDan=Struc1->s1.KolDan+Struc1->s2.KolDan+Struc1->s3.KolDan;
  return i;
}

int ZamenaShar(TSharDataBasa *Struc1, TSharDataBasa Struc0)
{
 int i;
 i=0;
 if(Struc1->Tip_Sond!=Struc0.Tip_Sond) Struc1->Tip_Sond=-1;
 if(Struc1->s[0].KolDan<Struc0.s[0].KolDan)
         {Struc1->pok_inf[0]='1'; Struc1->s[0]=Struc0.s[0]; i=1;}
 if(Struc1->s[1].KolDan<Struc0.s[1].KolDan)
         {Struc1->pok_inf[1]='1'; Struc1->s[1]=Struc0.s[1]; i=1;}
 if(Struc1->s[2].KolDan<Struc0.s[2].KolDan)
         {Struc1->pok_inf[2]='1'; Struc1->s[2]=Struc0.s[2]; i=1;}
 if(Struc1->s[3].KolDan<Struc0.s[3].KolDan)
         {Struc1->pok_inf[3]='1'; Struc1->s[3]=Struc0.s[3]; i=1;}
 if(i) Struc1->KolDan=Struc1->s[0].KolDan+Struc1->s[1].KolDan+
                      Struc1->s[2].KolDan+Struc1->s[3].KolDan;
 return i;
}

int ZamenaKN04(TAeroDataBasa *Struc1, TAeroDataBasa Struc0)
{
 int i;
 i=0;
 if(Struc1->s[0].KolDan<Struc0.s[0].KolDan)
   {Struc1->pok_inf[0]='1'; Struc1->s[0]=Struc0.s[0]; i=1;}
 if(Struc1->s[1].KolDan<Struc0.s[1].KolDan)
   {Struc1->pok_inf[1]='1'; Struc1->s[1]=Struc0.s[1]; i=1;}
 if(Struc1->s[2].KolDan<Struc0.s[2].KolDan)
   {Struc1->pok_inf[2]='1'; Struc1->s[2]=Struc0.s[2]; i=1;}
 if(Struc1->s[3].KolDan<Struc0.s[3].KolDan)
   {Struc1->pok_inf[3]='1'; Struc1->s[3]=Struc0.s[3]; i=1;}
 if(i) Struc1->KolDan=Struc1->s[0].KolDan+Struc1->s[1].KolDan+Struc1->s[2].KolDan+Struc1->s[3].KolDan;
 return i;
}

void konstrAeroBasa(TAeroDataBasa *Sond)
{ int i, j;
   Sond->Tip=0;
   Sond->Index=0;
   Sond->Pok_Privyaz=1;
   strcpy(Sond->Name,"");
   Sond->KolDan=0;
   strcpy(Sond->pok_inf,"0000");
   for(i=0; i<4; i++) Sond->Koord[i]=0;
   for(i=0; i<4; i++) Sond->Quadrat[i]=0;
   for(j=0; j<4; j++) {Sond->s[j].KolDan=0;
     for(i=0; i<100; i++) {
        Sond->s[j].data[i].KolDan=0;
        Sond->s[j].data[i].P=9999;
        Sond->s[j].data[i].H=9999;
        Sond->s[j].data[i].T=9999;
        Sond->s[j].data[i].D=9999;
        Sond->s[j].data[i].dd=9999;
        Sond->s[j].data[i].ff=9999;
        strcpy((char *)&Sond->s[j].data[i].pok_kach[0],"0000008");
                       }
                      }
}
void Preobr(TAeroDataBasa *Struc, TAeroDataAll *Param)
{
 int i, j, k, k1, KolDanAA, KolDanBB, KolDanCC, KolDanDD;
 TAeroDataAll SondCC;
 Uroven dataVS;

 konstrAero(Param);
 Zagolovok(Param,*Struc);
 if(!Struc->KolDan) return;

 konstrAero(&SondCC);
 KolDanAA=Struc->s[0].KolDan;
 KolDanBB=Struc->s[1].KolDan;
 KolDanCC=Struc->s[2].KolDan;
 KolDanDD=Struc->s[3].KolDan;
if(KolDanAA>200||KolDanBB>200||KolDanCC>200||KolDanDD>200) return;
 for(i=0;i<KolDanAA;i++)
  {
	 if(Struc->s[0].data[i].pok_kach[6]=='1') continue;
	 if(Struc->s[0].data[i].pok_kach[1]!='0') continue;
  }

 for (i=0;i<KolDanCC;i++)
  {
	 if(Struc->s[3].data[i].pok_kach[6]=='1') continue;
	 if(Struc->s[3].data[i].pok_kach[1]!='0') continue;
  }

 k=KolDanAA+KolDanBB;
 if(k!=0)
  {
   if(k>200) KolDanBB=200-KolDanAA;
   if(KolDanAA)
	  {
     for(i=0; i<KolDanAA; i++) Param->data[i]=Struc->s[0].data[i];
     if(KolDanBB!=0)
	      for(i=0; i<KolDanBB; i++) Param->data[KolDanAA+i]=Struc->s[1].data[i];
    }
    else
    	for(i=0; i<KolDanBB; i++) Param->data[i]=Struc->s[1].data[i];
  }

 k=KolDanAA+KolDanBB;
 k1=KolDanCC+KolDanDD;
 if(k1!=0)
  {
   if(k1>200) KolDanDD=200-KolDanCC;
   if(KolDanCC)
    {
     for(i=0; i<KolDanCC; i++) SondCC.data[i]=Struc->s[2].data[i];
     if(KolDanDD!=0)
	      for(i=0; i<KolDanDD; i++) SondCC.data[KolDanCC+i]=Struc->s[3].data[i];
    }
    else
    	for(i=0; i<KolDanDD; i++) SondCC.data[i]=Struc->s[3].data[i];
    }

 k1=KolDanCC+KolDanDD;
 if(k1!=0)
  {
   if((k+k1)>200) k1=200-k;
   for(i=0; i<k1; i++) Param->data[k+i]=SondCC.data[i];
  }

 k+=k1;
 Param->KolDan=k;
 k1=k-1;
m1: for(i=0; i<k1; i++)
	   {
      if((Param->data[i].pok_kach[6]=='8')||((Param->data[i].pok_kach[0]!='0')&&(Param->data[i].pok_kach[0]!='3')))
  	    continue;
      for(j=i+1; j<k; j++)
	      {
         if((Param->data[j].pok_kach[6]=='8')||((Param->data[j].pok_kach[0]!='0')&&(Param->data[j].pok_kach[0]!='3')))
		       continue;
         if(Param->data[i].P>=Param->data[j].P) break;
         dataVS=Param->data[i];
         Param->data[i]=Param->data[j];
         Param->data[j]=dataVS;
         goto m1;
        }
     }

 for(i=0;i<Param->KolDan;i++)
  {
   Param->data[i].P=ftoi_norm(Param->data[i].P/10.);
  }
}

