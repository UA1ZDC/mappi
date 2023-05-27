#include "func_obanal.h"
#include "interpolorder.h"
#include "tfield.h"

#include <cross-commons/debug/tlog.h>

#include <QtDebug>

#include <cross-commons/funcs/mn_errdefs.h>

#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wmisleading-indentation"

namespace MnObanal{

int prorej_GRIBD(meteo::GeoData *adata,QVector<int> *num,  float rast)
// int prorej_GRIBD(GeoPoint *pos, int *num, int n, int *kol_num)
{
  int n = adata->size();
  float b, c, A, r, rd, acsv;
  int i, j, k, kk,  jk, jk1;
  int imin, imax, jmin, jmax;
  int ret_val=0;
  QVector<int> nomer;

  QVector<float> fi(n), la(n);
  QVector<char> pok_kach(n);
  int ii[180][360], maxkol; //ii масив числа стнций в квадратах
  float gtor= MnMath::RAD_PER_DEGf;
  float rtog = MnMath::DEG_PER_RADf;
  //float rast=0.05;

  num->fill(-1);
  pok_kach.fill('0');

  for(i=0; i<n; i++)
  {
    fi[i]=90.f - adata->at(i).point.fiDeg();
    la[i]=180.f+adata->at(i).point.laDeg();
  }
  int kol_num = 0;

  //--------------------------------------------------------------------------
  for(i=0; i<180; i++) for(j=0; j<360; j++) ii[i][j]=0;
  for(i=0; i<n; i++) {
    j=int(fi[i]);
    if(j<0) j=0;
    else if(j>179) j=179;
    k=int(la[i]);
    if(k<0) k=0;
    else if(k>359) k=359;
    (ii[j][k])++;
  }
  maxkol=0;
  for(i=0; i<180; i++)
    for(j=0; j<360; j++)
      if(maxkol<ii[i][j]) {maxkol=ii[i][j]; }


  nomer.resize(180*360*maxkol);

  for(i=0; i<180; i++) for(j=0; j<360; j++) for(k=0; k<maxkol; k++)
    nomer[(i*360+j)*maxkol+k]=-1;
  //--------------------------------------------------------------------------
  for(i=0; i<180; i++) for(j=0; j<360; j++) ii[i][j]=0;
  for(i=0; i<n; i++)
  {
    j=int(fi[i]);
    if(j<0) j=0;
    if(j>179) j=179;
    k=int(la[i]);
    if(k<0) k=0;
    if(k>359) k=359;
    nomer[(j*360+k)*maxkol+ii[j][k]]=i;
    (ii[j][k])++;
  }
  //--------------------------------------------------------------------------
  for(i=0; i<n; i++) {
    if(pok_kach[i]!='0') continue;
    imin=(int)floor(fi[i]-rast);
    imax=(int)ceil(fi[i]+rast);
    if(imin<0) {
      imin=0;
      jmin=0;
      jmax=360;
    }
    else if(imax>180) {
      imax=180;
      jmin=0;
      jmax=360;
    }
    else {
      rd=rast/sin(fi[i]*gtor);
      jmin=(int)floor(la[i]-rd);
      jmax=(int)ceil(la[i]+rd);
    }
    //----------------------------
    for(int ik = imin; ik < imax; ik++){
      for(jk1=jmin; jk1<jmax; jk1++) {
        if(jk1<0) {
          jk=360+jk1;
        }
        else {
          if(jk1>359){
            jk=jk1-360;
          }
          else {
            jk=jk1;
          }
        }

        for(kk=0; kk<maxkol; kk++) {
          k=nomer[(ik*360+jk)*maxkol+kk];
          if(k==-1) {
            break;
          }
          if(k==i){
            continue;
          }
          b=fi[i]*gtor;
          c=fi[k]*gtor;
          A=(la[k]-la[i])*gtor;
          acsv=cosf(b)*cosf(c)+sinf(b)*sinf(c)*cosf(A);
          if(acsv>1) {
            acsv=1.;
          }
          else {
            if(acsv<-1){
              acsv=-1.;
            }
          }
          r=acosf(acsv)*rtog;
          if(r< MnMath::deg2rad( rast) ) {
            pok_kach[k]='1';
          }
        }

      }
    }
    //--------------------------
    (*num)[kol_num]=i;
    kol_num++;
  }
  num->resize(kol_num);

  return ret_val;
}



/**
   * Интерполирует значения fun в точках с координатами fi la (хаотическая сетка) в узлы регулярной сетки с типом type_net
   * значения type_net:
   *  1 через 5 градусов по широте и долготе
   *  2  через 2.5 градуса по широте и долготе
   *  3 через 1.25 градуса по широте и долготе
   *  4 через 1 градус по широте и долготе
   *  10 через 2.983 градуса по широте и долготе
   *  100 через 1 градус по широте и долготе для данных c особым кол-вом координат по долготе
   *  -1 10x10 точек по широте и долготе
   *  -2 100x100 точек по широте и долготе
   *  -3 1000x1000 точек по широте и долготе
   * @param fi широта
   * @param la долгота
   * @param fun значение
   * @param n количество точек
   * @param rez результат инткрполяции - значения в узлах регулярной сетки
   * @param quality маска (true - значение в узле рассчитано)
   * @param kol_rez количество рассчитанных значений
   * @param type_net тип сетки
   * @param startfi
   * @param endfi
   * @param startla
   * @param endla
   * @return 0, если все нормально
   */

int prepInterpolHaos (meteo::GeoData *adata, obanal::TField *rez,
                      float predel_grad, QStringList* badData,
                      float rast , int kolstn, int kolstmin , float rstfrprorej, float shir_skl)

{
  int n = adata->size();
  if ( !rez || adata->size() < 4 ) return ERR_NODATA_OBANAL;

  QVector<int> num_p(n,-1);
  adata->sortByLa();

  if(0.f < predel_grad){
    MnObanal::kontrol_gradient ( adata ,rast, kolstn, kolstmin, predel_grad, badData );
  }
  adata->sortByFi();
  //   debug_log << QObject::tr("------------------------ До прореживания") << adata->size()<<predel_grad;
  int ret_val=0;
  int is_sever=true;
  int is_ug=true;
  float startfi = rez->getRegPar().start.fi();
  float endfi = rez->getRegPar().end.fi();
  //float startla = rez->getReg().start_la;
  //////////////////////////////////////////////////////////////////
  obanal::TField rezult_sever = *rez;// = new obanal::TField();
  RegionParam sever_param;
  sever_param = rez->getRegPar();
  if ( startfi >= -shir_skl ) {
    sever_param.start.setFi( startfi);
  } else {
    sever_param.start.setFi( -shir_skl);
  }
  rezult_sever.setNet (sever_param/*rez->getRegPar() */,rez->typeNet());
  rezult_sever.setMasks(false);
  rezult_sever.setNetMask (  sever_param);
  meteo::GeoData src_sever;

  MnObanal::prorej(adata, &num_p,rstfrprorej);
  meteo::GeoData new_data;
  for(int i=0;i<num_p.size();i++) {
    if(num_p[i] != -1 &&adata->at(num_p[i]).mask == TRUE_DATA)
    {
      new_data.append(adata->at(num_p[i]));
    }
  }

  for ( int i = 0; i < new_data.size();++i  ) {
    if ( new_data.at(i).point.fi() >=  -shir_skl && TRUE_DATA == new_data.at(i).mask) {
      src_sever.append ( new_data.at(i) );
    }
  }
  // rez->setSrcPointCount(new_data.size());
  rez->setSrcPointCount(n);
  rezult_sever.setSrcPointCount(src_sever.size());

  if ( src_sever.size() > 4 ) {
    if ( src_sever.size() < 3000 ) {
      ret_val = obanal::interpolHaos( src_sever, &rezult_sever );
    } else {
      ret_val = obanal::interpolHaosRazbien( src_sever, &rezult_sever );
    }
    if ( ret_val != ERR_NOERR ) {
      is_sever = false;//ничего нет в северном полушарии
      // info_log << QObject::tr("ничего нет в северном полушарии");
    }
  } else {
    is_sever = false;//ничего нет в северном полушарии
    // info_log << QObject::tr("ничего нет в северном полушарии");
  }

  ///////////////////////////////////////////////////////////////////
  //ЮЖНОЕ ПОЛУШАРИЕ

  obanal::TField rezult_ug = *rez;
  RegionParam ug_param ;
  ug_param = rez->getRegPar();

  if ( endfi <= shir_skl ) {
    ug_param.end.setFi(endfi);
  } else {
    ug_param.end.setFi(shir_skl);
  }
  rezult_ug.setNet ( sever_param,rez->typeNet());
  rezult_ug.setMasks(false);
  rezult_ug.setNetMask (sever_param);
  //  rezult_ug.setNetMask ( rez->netMask() );


  meteo::GeoData src_ug;

  for ( int i = 0; i < new_data.size(); ++i  ) {
    if ( new_data.at(i).point.fi() <=  shir_skl && TRUE_DATA == new_data.at(i).mask) {
      meteo::MData vs =  new_data.at(i) ;
      vs.point.setFi(-vs.point.fi());
      src_ug.append ( vs );
    }
  }
  //    debug_log << "Данных  в южном полушарии "<<src_ug.size();
  rezult_ug.setSrcPointCount(src_ug.size());
  if(src_ug.size() > 4){
    if ( src_ug.size() < 3000 ) {
      ret_val = obanal::interpolHaos( src_ug, &rezult_ug );
    } else {
      ret_val = obanal::interpolHaosRazbien( src_ug, &rezult_ug);
    }
    if ( ret_val != ERR_NOERR ) {
      is_ug = false;//ничего нет в северном полушарии
      //  info_log << QObject::tr("ничего нет в южном полушарии")<<ret_val;
    }
  } else {
    is_ug = false;//ничего нет в северном полушарии
    // info_log << QObject::tr("ничего нет в южном полушарии");
  }

  ret_val = ERR_NOERR;
  if(!is_ug&&!is_sever){ //интерполировать неполучилось
    //debug_log << "интерполировать неполучилось";
    return ERR_NODATA_OBANAL;
  }

  float step_fi;
  if(!rezult_sever.stepFi(&step_fi)){
    error_log << QObject::tr("Не определен тип сетки для склеивания");
    return ERR_OBANAL;
  }
  int end_n = MnMath::ftoi_norm((M_PI_2f+ shir_skl) / step_fi);
  int kol_la = rez->kolLa();
  //  int kol_fi = rez->kolFi();
  if(is_sever&& !is_ug) { //
    *rez = rezult_sever;
    //debug_log <<"только северное полушарие";
    return ret_val;
  }
  int start_n = MnMath::ftoi_norm((M_PI_2f- shir_skl) / step_fi);
  //   debug_log<< "rezult_sever.kolFi()"<< rezult_sever.kolFi()<<"rezult_sever.kolla()"<<rezult_sever.kolLa();
  //   debug_log<< "start_n"<<start_n<<"end_n"<<end_n<<"kol_la"<<kol_la<<"kol_fi"<<kol_fi;

  if(is_sever) {  //есть северное
    for(int i= 0 ; i < rezult_sever.kolFi(); i++){
      for(int j=0; j < rezult_sever.kolLa(); j++){
        if( (rez->kolFi() > (i + start_n - 1)) &&  TRUE_DATA ==rezult_sever.getMask( i, j))
        {
          float vs2 = rezult_sever.getData(i, j);
          rez->setData(rez->num( i+start_n-1, j ),vs2,true);
        }
      }
    }
  }

  if(is_ug ) {  //есть южное
    for(int i= 0 ; i < rezult_ug.kolFi(); i++){
      for(int j=0; j < rezult_ug.kolLa(); j++){
        if( (rez->kolFi() > i) && TRUE_DATA ==rezult_ug.getMask( i, j))
        {
          float vs2 = rezult_ug.getData(rezult_ug.kolFi() - i-1, j);
          rez->setData(rez->num( i, j ),vs2,true);
        }
      }
    }
  }
  if( !is_sever) {  //только южное
    // debug_log <<"только южное полушарие";
    return ret_val;
  }
  //склеиваем результаты интерполяции
  //    debug_log <<"                                                            оба полушария";

  for(int i= start_n; i < end_n; i++){
    float fi = rez->getFi(i);
    int k = i-start_n+1;
    int kk = rezult_ug.kolFi() - i-1;
    //debug_log<<fi*180./3.1415926<<rezult_sever.getFi(i-start_n+1)*180./3.1415926 <<rezult_ug.getFi(rezult_ug.kolFi() - i-1)*180./3.1415926 ;
    for(int j=0; j < kol_la; j++){
      if((i < rezult_sever.kolFi() &&  TRUE_DATA == rezult_sever.getMask(i, j))
         &&  ( i < rezult_ug.kolFi() &&  TRUE_DATA ==rezult_ug.getMask(i, j)))
      {
        float vs2 = rezult_sever.getData( k, j);
        float vs1 = rezult_ug.getData(kk, j);
        rez->setData(rez->num( i, j ),fun_skl(fi,vs1,vs2,shir_skl),true);
      }
    }
  }

  return ret_val;
}


float fun_skl(float fi, float a, float b,float shir_skl)
{
  float t=(fi+shir_skl)/(2.f*shir_skl);
  float f=1.f-(3.f-2.f*t)*t*t;
  return a*f+(1.f-f)*b;
}

void sort2(float *mas, int *num, int n)
{
  int predel=7;
  int i,j,k,n1=n-1;
  float a, temp;
  int nvs, ntemp;

  if(n<predel)
  {
    for(j=1;j<n;j++)
    {
      a=mas[j];
      nvs=num[j];
      for(i=j-1;i>-1;i--)
      {
        if(mas[i]<a) break;
        if(MnMath::isEqual(mas[i],a)) break;
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
    while(((mas[i]<a)||MnMath::isEqual(mas[i],a)) &&(i<n1)) i++;
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
        if(mas[i]<a) break;
        if(MnMath::isEqual(mas[i],a)) break;
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
    while(((mas[i]<a)||MnMath::isEqual(mas[i],a)) &&(i<n1)) i++;
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

int searh_inf(float a, float *mas, int n)
{
  int niz=0, verh=n-1, vs;

  if(n<2) return -3;
  if(a<mas[niz]) return -1;
  if(MnMath::isEqual( a,mas[niz])) return niz;
  if(a>mas[verh]) return -2;
  if(MnMath::isEqual(a,mas[verh])) return n-2;

  while((verh-niz)!=1)
  {
    vs=niz+((verh-niz)>>1);
    if(a<mas[vs]) verh=vs; else niz=vs;
  }
  return niz;
}

int searh_inf(double a, double *mas, int n)
{
  int niz=0, verh=n-1, vs;

  if(n<2) return -3;
  if(a<mas[niz]) return -1;
  if(MnMath::isEqual( a,mas[niz])) return niz;
  if(a>mas[verh]) return -2;
  if(MnMath::isEqual(a,mas[verh])) return n-2;

  while((verh-niz)!=1)
  {
    vs=niz+((verh-niz)>>1);
    if(a<mas[vs]) verh=vs; else niz=vs;
  }
  return niz;
}

int searh_inf(float a, const QVector<float> &mas, int n)
{
  int niz=0, verh=n-1, vs;
  if(n<2){

    return -3;
  }
  if(a<mas[niz]) {
    // debug_log<<a<<mas<<n;
    return -1;
  }
  if(MnMath::isEqual(a,mas[niz])){
    //  debug_log<<a<<mas<<n;
    return niz;
  }
  if(a>mas[verh]) {
    // debug_log<<a<<mas<<n;
    return -2;
  }
  if(MnMath::isEqual(a,mas[verh])) return n-2;

  while((verh-niz)!=1)
  {
    vs=niz+((verh-niz)>>1);
    if(a<mas[vs]) verh=vs; else niz=vs;
  }
  return niz;
}



/////////////////////////////////////////////////////////////////////

int prorej(meteo::GeoData *adata, QVector<int> *num, float rast)
{
  float b, c, a, r, rd, acsv;
  int i, j, k, kk, ik, jk, jk1;
  int imin, imax, jmin, jmax;
  int ii[180][360]; //ii масив числа стнций в квадратах
  //float rtog=180./3.1415926;
  //float gtor=3.1415926/180.;
  float rast_1 = MnMath::deg2rad(rast);
  int n = adata->size();
  int kkk=0;
  //--------------------------------------------------------------------------
  bzero(ii,180*360*sizeof(int));
  QVector<float> fi(n), la(n);

  for(i=0; i<n; i++)
  {
    fi[i]=90.f - adata->at(i).point.fiDeg();
    float vs=int(adata->at(i).point.laDeg()/360.f);
    la[i]=adata->at(i).point.laDeg()-vs*360.f;
    if(la[i]<0) la[i]+=360.f;
  }

  int maxkol=0;
  for(i=0; i<n; i++) {
    if(TRUE_DATA != adata->at(i).mask){
      continue;
    }
    j = int(fi[i]);
    if( j < 0 ) j = 0;
    if(j>179) j=179;
    k=int(la[i]);
    if(k<0) k=0;
    if(k>359) k=359;
    (ii[j][k])++;
    if(maxkol<ii[j][k]) {
      maxkol=ii[j][k];
    }
  }

  QVector<int> nomer(180*360*maxkol,-1);

  bzero(ii,180*360*sizeof(int));

  for(i=0; i<n; i++)
  {
    if(TRUE_DATA != adata->at(i).mask){
      continue;
    }
    j=int(fi[i]);
    if(j<0) j=0;
    if(j>179) j=179;
    k=int(la[i]);
    if(k<0) k=0;
    if(k>359) k=359;
    nomer[(j*360+k)*maxkol+ii[j][k]]=i;
    (ii[j][k])++;
  }


  //--------------------------------------------------------------------------
  for(i=0; i<n; i++) {
    if(TRUE_DATA != adata->at(i).mask
       && 1.f > adata->at(i).w){
      continue;
    }
    imin=(int)floor(fi[i]-rast);
    imax=(int)ceil(fi[i]+rast);
    if(imin<0) {
      imin=0;
      jmin=0;
      jmax=360;
    }
    else if(imax>180) {
      imax=180;
      jmin=0;
      jmax=360;
    }
    else {
      rd=rast/sinf(MnMath::deg2rad(fi[i]));
      jmin=(int)floor(la[i]-rd);
      jmax=(int)ceil(la[i]+rd);
    }
    //----------------------------
    for(ik=imin; ik<imax; ik++){
      for(jk1=jmin; jk1<jmax; jk1++) {
        if(jk1<0) jk=360+jk1;
        else if(jk1>359) jk=jk1-360;
        else jk=jk1;
        for(kk=0; kk<maxkol; kk++) {
          k=nomer[(ik*360+jk)*maxkol+kk];
          if(k==-1) break;
          if(k==i) continue;
          b=MnMath::deg2rad(fi[i]);
          c=MnMath::deg2rad(fi[k]);
          a=MnMath::deg2rad(la[k]-la[i]);
          acsv=cosf(b)*cosf(c)+sinf(b)*sinf(c)*cosf(a);
          if(acsv>1.f) acsv=1;
          else if(acsv<-1.f) acsv=-1;
          r= acosf(acsv);
          if(r<rast_1) {
            // if (FALSE_DATA != adata->at(i).mask) {
            (*adata)[k].mask = FALSE_DATA;
            // }
          }
        }
      }
    }
    (*num)[kkk]=i;
    kkk++;
  }

  return 0;
}


/**
   * реализация алгоритма объективного контроля "градиентный контроль"
   * @param rast радиус корреляции
   * @param kolstan кол -во станций в радиусе
   * @param kolstan_min минимальное кол-во станций
   * @param predel_grad максимально допустимый градиент величины fun
   * @return 0, if ok
   */
int kontrol_gradient ( meteo::GeoData *vdata,
                       float rast, int kolstan, int kolstan_min, float predel_grad,
                       QStringList* badData) {

  badData->clear();
  QVector<uint> count_stations ( 180*360,0 ); //матрица числа станций в квадратах
  ++kolstan;
  ++kolstan_min;
  int g=0;
  meteo::GeoData adata ;
  adata = *vdata;
  adata.to360();

  uint maxkol=0;
  QVectorIterator<meteo::MData> it ( adata );
  while ( it.hasNext() ) {
    meteo::MData vs = it.next();
    if ( vs.mask != TRUE_DATA)
      continue;
    int j=int ( vs.point.latDeg());
    if ( j<0 ) j=0;
    if ( j>179 ) j=179;
    int k=int ( vs.point.lonDeg());
    if ( k<0 ) k=0;
    if ( k>359 ) k=359;
    ++ ( count_stations[j*360+k] );
    if ( maxkol < count_stations[j*360+k] )
      maxkol = count_stations[j*360+k];
  }

  QVector<int> nomer ( 180*360*maxkol,-1 );
  QVector<int> nn ( kolstan );
  QVector<double> rr ( kolstan );

  count_stations.fill ( 0 );

  it.toFront();
  int i = -1;
  while ( it.hasNext() ) {
    ++i;
    meteo::MData vs = it.next();
    if ( vs.mask != TRUE_DATA )
      continue;
    int j=int (  vs.point.latDeg() );
    if ( j<0 ) j=0;
    if ( j>179 ) j=179;
    int k=int (  vs.point.lonDeg() );
    if ( k<0 ) k=0;
    if ( k>359 ) k=359;
    nomer[ ( j*360+k ) *maxkol+count_stations[j*360+k]] = i;
    ++ ( count_stations[j*360+k] );
  }
  //--------------------------------------------------------------------------
  int imin=0, imax=0, jmin=0, jmax=0;
  int ds = adata.size();

  for(int i=0; i< ds; ++i) {

    meteo::MData vs = adata.at(i);
    double vs_lat = vs.point.lat();
    double vs_lon = vs.point.lon();
    double vs_latG = vs.point.latDeg();
    double vs_lonG = vs.point.lonDeg();

    if ( vs.mask != TRUE_DATA )
      continue;
    rr.fill ( rast );
    nn.fill ( -1 );
    imin= ( int ) floor ( vs_latG - rast );
    imax= ( int ) ceil ( vs_latG + rast );
    if ( imin<0 ) {
      imin=0;
      jmin=0;
      jmax=360;
    } else if ( imax>180 ) {
      imax=180;
      jmin=0;
      jmax=360;
    } else {
      double rd=rast/sin (  vs_lat );
      jmin= ( int ) floor (  vs_lonG - rd );
      jmax= ( int ) ceil (  vs_lonG + rd );
    }
    //----------------------------
    int jk =0;
    double b =  vs_lat;
    double cosb = cos ( b );
    double sinb = sin (b);
    for (int ik=imin; ik<imax; ++ik )
      for (int jk1=jmin; jk1<jmax; ++jk1 ) {
        if ( jk1<0 ) jk=360+jk1;
        else if ( jk1>359 ) jk = jk1-360;
        else jk=jk1;
        for (uint kk=0; kk<maxkol; ++kk) {
          int k=nomer[ ( ik*360+jk ) *maxkol+kk];
          if ( k == -1 ) break;
          double c =  adata.at ( k ).point.lat();
          double A = ( adata.at ( k ).point.lon() - vs_lon );
          double acsv=cosb *cos ( c ) +sinb *sin( c ) *cos ( A );
          if ( acsv>1 ) acsv=1.;
          else if ( acsv<-1 ) acsv=-1.;
          double r = MnMath::rad2deg(acos ( acsv )) ;
          //float r = acosf ( acsv ) ;
          // if( r < 0.001) r = 0.01;
          //debug_log <<"  r "<<  r <<acsv<<"A"<<A<<"b"<<b<<"c"<<c;

          int l = 0;
          for (l=0; l<kolstan; ++l ) if ( r<rr[l] ) break;
          if ( l == kolstan ) continue;
          //  debug_log <<"r "<<r;

          for (int m=kolstan-1; m>l; m-- ) {
            rr[m]=rr[m-1];
            nn[m]=nn[m-1];
          }
          rr[l]=r;

          nn[l]=k;
        }
      }
    //--------------------------
    int kkk=0;
    for (int  l=0; l<kolstan; ++l )
      if ( rr[l] < rast )
        ++kkk;
    if ( kkk < kolstan_min ){
      //debug_log<<vs.data<< vs.point.toString();
      continue;
    }
    double sum=0.;
    int sum_i=0;
    int kol_kkk=0;
    for ( int k=1; k<kkk; ++k ) {
      if( rr[k] < 1e-3){
        continue;
      }
      kol_kkk++;
      double gradient = fabs ( adata.at ( nn[k] ).data - vs.data ) /(rr[k]);//fabs(fun[nn[k]]-fun[i])/rr[k];
      sum += gradient;
      if ( gradient > predel_grad ) {
        ++sum_i;
      }
    }
    sum/=float ( kol_kkk );
    if ( ( sum_i > ( kolstan_min-1 ) ) || ( sum > predel_grad ) ) {
      (*vdata)[i].mask = FALSE_DATA;
      badData->append((*vdata).at(i).id);
      g++;
    }
  }
  return g;
}


int prepInterpolOrder( meteo::GeoData *adata, obanal::TField *rezult_)
{
  //    debug_log << rez->getRegPar().start.toString() << rez->getRegPar().end.toString();
  QVector<int> num_p(adata->size(),-1);
  MnObanal::prorej_GRIBD(adata, &num_p, RAST_FROM_PROREJ);

  meteo::GeoData new_data;
  for(int i=0;i<num_p.size();i++) {
    if(num_p[i] != -1 &&adata->at(num_p[i]).mask == TRUE_DATA)
    {
      //debug_log<<gdata.at(num_p[i]).point.fiDeg()<<gdata.at(num_p[i]).point.laDeg()<<gdata.at(num_p[i]).data;
      if( (adata->at(num_p[i]).point.fiDeg()<-89.f) || (adata->at(num_p[i]).point.fiDeg()>89.f)) continue;
      new_data.append(adata->at(num_p[i]));
    }
  }

  rezult_->setSrcPointCount(new_data.size());

  int ret_val=0;

  int n = new_data.size();
  if(n<5) { return ERR_NODATA_OBANAL;}
  new_data.sortByFi();
  //////////////////////////////////////////////////////////////////
  int i = 1;
  for(; i < n; i ++) if(new_data[i-1].point.fi() > new_data[i].point.fi()) break;
  if(i != n) {
    error_log << QObject::tr("Ошибка при сортировке по широте!");
    return ERR_OBANAL;
  }

  int kol_fi_fact = 0;
  float vs = new_data[0].point.fi();
  for(i=1;i<n;i++){
    //debug_log << new_data[i].point.fi()<<vs;
    if(!MnMath::isEqual( new_data[i].point.fi(), vs)) {
      vs = new_data[i].point.fi();
      kol_fi_fact++;
    }
    //kol_fi_fact++;
  }
  kol_fi_fact++;

  if(kol_fi_fact<3) {
    error_log << QObject::tr("Ошибка анализа сетки!");
    return ERR_OBANAL;
  }

  QVector<float> netfi_fact(kol_fi_fact,0.);
  QVector<int> kol_point_fact(kol_fi_fact,0);
  QVector<float> rez_vs(kol_fi_fact*rezult_->kolLa());
  kol_fi_fact = 0;

  vs = new_data[0].point.fi();
  netfi_fact[0]=vs;
  for(i=0;i<n;i++){
    if( !MnMath::isEqual( new_data[i].point.fi(), vs))
    {
      kol_fi_fact++;
      vs = new_data[i].point.fi();
      netfi_fact[kol_fi_fact] = vs;
      kol_point_fact[kol_fi_fact] ++;
    }
    else kol_point_fact[kol_fi_fact] ++;
  }
  kol_fi_fact++;

 // QVector<float> netla;
  //QVector<float> netfi;
 // QVector<float>  rezult(rezult_->kolFi()*rezult_->kolLa());
  QVector<float> la;
  QVector<float> fi;
  QVector<float> fun;

  for(int i = 0; i < new_data.size(); ++i){
    la.append(new_data.at(i).point.la());
    fi.append(new_data.at(i).point.fi());
    fun.append(new_data.at(i).data);
    //debug_log <<i<<"fi "<<new_data.at(i).point.fiDeg()<<"la "<<new_data.at(i).point.laDeg()<<new_data.at(i).data;
  }
  ret_val = InterpolLong( rezult_->netLa(), rez_vs.data(), la.data(), fun.data(),
                          kol_point_fact.data(),kol_fi_fact,rezult_->kolLa());
  if(ret_val!=0) return ret_val;
  ret_val = InterpolLat(rezult_->netFi(),rezult_,netfi_fact.data(),rez_vs.data(),
                        rezult_->kolFi(),kol_fi_fact,rezult_->kolLa());
  if(ret_val!=0) return ret_val;

  return ret_val;
}




}

#pragma GCC diagnostic pop





