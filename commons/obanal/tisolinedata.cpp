#include "tisolinedata.h"
#include <cross-commons/debug/tlog.h>


//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wmisleading-indentation"


using namespace TGis;

inline bool isEqualHARD(float a, float b ){
  return fabs(a-b)<1.e-6;
}

TIsoLineData::TIsoLineData(const obanal::TField* afield)
  :field_(afield)
{
  _kolyn2 = 0;
  _kolyn3 = 0;
}


TIsoLineData::~TIsoLineData(){
  
}

#include <malloc.h>

int TIsoLineData::makeAll(int iso_min, int iso_max, int step,meteo::GeoVector *isoline){
  
  int i,j, k, l, num, kolx1=field_->kolFi()-1, koly1=field_->kolLa()-1;
  int Kol_Num=10000, Kol_Point, Kol_Num_Predel=3;
  float *argf, *iso,  vs;
  char *indf = nullptr;
  bool *mask = nullptr;
  int size_fi = field_->kolFi();
  int size_la = field_->kolLa();
  
  indf= new char [kolx1*koly1*4];
//  indf=(char *)calloc(kolx1*koly1*4, sizeof(char));
   mask= new bool[field_->kolData()];
  //mask=(bool *)calloc(field_->kolData(), sizeof(bool));
  argf=new float [kolx1*koly1*4];
  //argf=(float *)calloc(kolx1*koly1*4, sizeof(float));
m1:
  iso= new float[Kol_Num*2];
  //iso=(float *)calloc(Kol_Num*2, sizeof(float));
  const bool *mask1 = field_->mask();
  const bool *mask2 = field_->netMask();
  for(int i=0, is = field_->kolData(); i< is;++i ){
      mask[i] = (mask1[i]&&mask2[i]);
    }

  
  for(num = iso_min; num < iso_max; num+=step)
    {
      vs=num;
      Iso_Vspom(vs,field_->netFi(),field_->netLa(),field_->data(),mask,indf,argf,size_fi,size_la);
      for(i=0; i<kolx1; i++)
        for(j=0; j<koly1; j++)
          for(k=0; k<4; k++)
            {
              Kol_Point=0;
              if(indf[Num3_Elem(i,j,k,koly1,4)])
                if(field_->typeNet() != GENERAL_NET) {//если на сфере
                    Kol_Point=One_Iso_Sphere(i,j,k,field_->netFi(),field_->netLa(),indf,argf,iso,size_fi,size_la,Kol_Num);
                  }
                else{
                    Kol_Point=One_Iso(i,j,k,field_->netFi(),field_->netLa(),indf,argf,iso,size_fi,size_la,Kol_Num);
                  }
              else continue;
              if(Kol_Point==Kol_Num)
                {
                  Kol_Num*=2;
                  delete[] iso;
                  goto m1;
                }
              if(Kol_Point<Kol_Num_Predel) continue;

     //         isoline->resize(Kol_Point);

              for(l=0;l<Kol_Point;l++)
                {
                  isoline->append(meteo::GeoPoint(iso[l*2],iso[l*2+1]));
                }
            }
    }
  delete []indf;
  delete []argf;
  delete []iso;
  delete []mask;
  
  return 0;
}


int TIsoLineData::makeOne1( float val, QVector<meteo::GeoVector> *isolines){
  if(field_->typeNet() == GENERAL_NET) {//если на сфере
      return makeOne(val, isolines);
  }
  if(val < field_->min_val()|| val > field_->max_val()){
    return ERR_NODATA;
  }


  int i,j, k, l,  kolx1=field_->kolFi()-1, koly1=field_->kolLa()-1;
  int Kol_Num=10000, Kol_Point, Kol_Num_Predel=3;
  float *argf = nullptr, *iso = nullptr;
  char *indf = nullptr;
  int size_fi = field_->kolFi();
  int size_la = field_->kolLa();
  bool *mask = nullptr;

  indf = new char [kolx1*koly1*4];
  //indf=(char *)calloc(kolx1*koly1*4, sizeof(char));
  mask = new bool[field_->kolData()];
  //mask=(bool *)calloc(field_->kolData(), sizeof(bool));

  argf = new float[kolx1*koly1*4];
  //argf= (float *)calloc(kolx1*koly1*4, sizeof(float));
  m1:
  iso = new float [Kol_Num*2];

  //iso=(float *)calloc(Kol_Num*2, sizeof(float));

  const bool *mask1 = field_->mask();
  const bool *mask2 = field_->netMask();
  for(int i=0, is = field_->kolData(); i< is;++i ){
      mask[i] = (mask1[i]&&mask2[i]);
    }

  Iso_Vspom(val,field_->netFi(),field_->netLa(),field_->data(),mask,indf,argf,size_fi,size_la);
  for(i=0; i<kolx1; i++)
    for(j=0; j<koly1; j++)
      for(k=0; k<4; k++)
        {
          Kol_Point=0;
          if(indf[Num3_Elem(i,j,k,koly1,4)]){
              if(field_->typeNet() != GENERAL_NET) {//если на сфере
                  Kol_Point=One_Iso_Sphere(i,j,k,field_->netFi(),field_->netLa(),indf,argf,iso,size_fi,size_la,Kol_Num);
                }
              else{
                  Kol_Point=One_Iso(i,j,k,field_->netFi(),field_->netLa(),indf,argf,iso,size_fi,size_la,Kol_Num);
                }
            }
          else {
              continue;
            }
          if(Kol_Point==Kol_Num)
            {
              Kol_Num*=2;
              delete []iso;
              goto m1;
            }
          if(Kol_Point<Kol_Num_Predel) continue;

          meteo::GeoVector isoline(Kol_Point);

          for(l=0;l<Kol_Point;l++)
            {
              isoline[l].setFi(iso[l*2]);
              isoline[l].setLa(iso[l*2+1]);
            }
          isolines->append(isoline);
        }
  
  delete []indf;
  delete []argf;
  delete []iso;
  delete []mask;
  return 0;
}

int TIsoLineData::makeOne(const float& val, QVector<meteo::GeoVector> *isolines) {
  if(!isolines)
    return ERR_NODATA;
  if(nullptr == field_ || field_->isEmpty()
     || 2 > field_->kolFi()
     || 2 > field_->kolLa()
     || val < field_->min_val()
     || val > field_->max_val()){
      return ERR_NODATA;
    }
  
  int ret_val = ERR_NOERR;
  if( ERR_NOERR != (ret_val = prepIso(val))){
      return ret_val;
    }
  const char *indf = indf_.data();
  meteo::GeoVector iso_vector;
  int kolx1= field_->kolFi() - 1;
  int koly1= field_->kolLa() - 1;

  for(int i=0; i<kolx1; i++){
      for(int j=0; j<koly1; j++){
          for(int k=0; k<4; k++)
            {
              if(!indf[n3e(i,j,k)])
                continue;
              iso_vector.clear();
              if(field_->typeNet() != GENERAL_NET) //если на сфере
                oneIsoSphere(i, j, k, iso_vector);
              else
                oneIso(i, j, k, iso_vector);
              //qDebug()<<"iso "<< iso_vector.size();
              if(iso_vector.size() < min_point_in_iso) continue;
              isolines->append(iso_vector);
            }
        }
    }
  return ERR_NOERR;
}


int TIsoLineData::prepIso(float iso){
  
  if(nullptr == field_) {
      return -1;
    }
  
  int kolx = field_->kolFi();
  int koly= field_->kolLa();
  int kolx1= kolx - 1;
  int koly1= koly - 1;
  int kolx2 = kolx - 2;
  int koly2 = koly - 2;
  
  _kolyn3 = koly -1 ;
  
  float dx=0,dy=0;
  field_->stepFi(&dx);
  field_->stepLa(&dy);
  int nvs = (kolx-1)*(koly-1)*4;
  indf_.resize(nvs);
  argf_.resize(nvs);
  indf_.fill(0);
  argf_.fill(0);
  char * indf = &indf_[0];
  float * argf = &argf_[0];

  int a, b;
  for (int j=0; j < koly1; ++j) {// Слева  и справа
      int j1=j+1;
      float f=0.;
      if(!field_->getData( 0, j,&f )){
          continue;
        }

      if (isEqualHARD(iso,  f ))
        a=1;
      else a=0;
      float f1=0. ;
      if(!field_->getData( 0, j1,&f1 )){
          continue;
        }
      if (isEqualHARD(iso,f1))
        b=1;
      else b=0;
      bool m= field_->getMask(0, j );
      bool m1= field_->getMask(0, j1 );
      if (m && m1 ) {// Слева
          if ((( a || f < iso) && ( b || iso < f1)) ||
              (( a || iso < f )&&( b || iso > f1 ))) {
              if (b) {
                  indf[n3e(0,j,3)] = 1;
                  argf[n3e(0,j,3)] = field_->getLa(j1);
                }
              else if (!a) {
                  indf[n3e(0,j,3)] = 1;
                  argf[n3e(0,j,3)] = field_->getLa(j)+ dy*(iso - f)/(f1 - f);
                }
            }
        }
      if(!field_->getData( kolx1, j ,&f)){
          continue;
        }
      if (isEqualHARD(iso, f )) a=1;
      else a=0;
      if(!field_->getData( kolx1, j1 ,&f1)){
          continue;
        }
      if (isEqualHARD(iso,f1 )) b=1;
      else b=0;
      m= field_->getMask(kolx1, j);
      m1= field_->getMask(kolx1, j1);

      if( m && m1 ) {// Справа
          if (((a || f < iso )&&( b || iso < f1)) ||
              ((a || iso < f ) && ( b || iso > f1))) {
              if(a) {
                  indf[n3e(kolx2,j,1)] = 1;
                  argf[n3e(kolx2,j,1)] = field_->getLa(j);
                }
              else if (!b) {
                  indf[n3e(kolx2,j,1)] = 1;
                  argf[n3e(kolx2,j,1)] = field_->getLa(j) + dy*( iso- f )/(f1 - f);
                }
            }
        }
      if (j<koly2) {// Справа по горизонтали
          if(!field_->getData( kolx2, j ,&f)){
              continue;
          }
          if (isEqualHARD(iso, f )) a=1;
          else a=0;
          if(!field_->getData( kolx2, j1 ,&f1)){
              continue;
          }
          float f11;
          if(!field_->getData( kolx1, j1 ,&f11)){
              continue;
          }

          bool m= field_->getMask(kolx2, j1);
          bool m1= field_->getMask(kolx1, j1);

          if ( m && m1 ) {
              if ((( a || f1< iso )&&( b || iso < f11 ))||
                  (( a || iso < f1 )&&( b || iso > f11))) {
                  if (b) {
                      indf[n3e(kolx2,j,2)] = 1;
                      argf[n3e(kolx2,j,2)] = field_->getFi(kolx1);
                    }
                  else if (!a) {
                      indf[n3e(kolx2,j,2)] = 1;
                      indf[n3e(kolx2,j1,0)] = 1;
                      argf[n3e(kolx2,j,2)] = field_->getFi(kolx2) + dx*( iso - f1 )/(f11 - f1);
                      argf[n3e(kolx2,j1,0)]=argf[n3e(kolx2,j,2)];
                    }
                  if (a) {
                      indf[n3e(kolx2,j+1,0)] = 1;
                      argf[n3e(kolx2,j+1,0)] = field_->getFi(kolx2);
                    }
                }
            }
        }
    }
  //***************************************************************
  for (int i = 0; i < kolx1; ++i) { // Снизу и сверху
      int i1=i+1;

      float f=0 ;
      if(!field_->getData( i,0 ,&f)){
          continue;
        }

      if (isEqualHARD(iso, f )) a=1;
      else a=0;
      float f1=0;
      if(!field_->getData( i1,0 ,&f1)){
          continue;
        }

      if (isEqualHARD(iso, f1 )) b=1;
      else b=0;

      bool m= field_->getMask(i, 0);
      bool m1= field_->getMask(i1, 0);

      if ( m && m1 ) {// Снизу
          if (((a || f <iso)&&( b || iso < f1))||
              (( a || iso < f ) && ( b || iso > f1))) {
              if (a) {
                  indf[n3e(i,0,0)] = 1;
                  argf[n3e(i,0,0)] = field_->getFi(i);
                }
              else if (!b) {
                  indf[n3e(i,0,0)] = 1;
                  argf[n3e(i,0,0)] = field_->getFi(i) + dx*( iso - f )/( f1 - f );
                }
            }
        }
      if(!field_->getData( i, koly1,&f)){
          continue;
        }

      if (isEqualHARD(iso, f )) a=1;
      else a=0;
      if(!field_->getData( i1, koly1,&f1)){
          continue;
        }
      if (isEqualHARD(iso, f1 )) b=1;
      else b=0;

      m= field_->getMask(i, koly1);
      m1= field_->getMask(i1, koly1);


      if ( m && m1 ) {// Сверху
          if (((a || f < iso)&&(b || iso < f1))||
              (( a || iso < f)&&(b || iso > f1))) {
              if (b) {
                  indf[n3e(i,koly2,2)] = 1;
                  argf[n3e(i,koly2,2)] = field_->getFi(i1);
                }
              else if (!a) {
                  indf[n3e(i,koly2,2)] = 1;
                  argf[n3e(i,koly2,2)] = field_->getFi(i) + dx*(iso-f)/(f1 - f);
                }
            }
        }
      if (i<kolx2) {// Сверху по вертикали

          if(!field_->getData( i1, koly2,&f)){
              continue;
            }

          if (isEqualHARD(iso, f )) a=1;
          else a=0;
          if(!field_->getData( i1, koly1,&f1)){
              continue;
            }

          m= field_->getMask(i1,koly2 );
          m1= field_->getMask(i1,koly1 );

          if ( m && m1 ) {
              if ((( a || f < iso )&&( b || iso < f1 ))||
                  ((a || iso < f)&&( b || iso > f1)))
                {
                  if (a) {
                      indf[n3e(i,koly2,1)] = 1;
                      argf[n3e(i,koly2,1)] = field_->getLa(koly2);
                    }
                  else if (!b)
                    {
                      indf[n3e(i,koly2,1)] = 1;
                      indf[n3e(i1,koly2,3)] = 1;
                      argf[n3e(i,koly2,1)] = field_->getLa(koly2) + dy*(iso - f)/ (f1 - f);
                      argf[n3e(i1,koly2,3)] = argf[n3e(i,koly2,1)];
                    }
                  if (b) {
                      indf[n3e(i1,koly2,3)] = 1;
                      argf[n3e(i1,koly2,3)] = field_->getLa(koly1);
                    }
                }
            }
        }
    }
  //***************************************************************
  for (int i=0; i<kolx2; i++) {// Внутренность
      int i1=i+1;
      for (int j=0; j<koly2; j++) {
          // По вертикали
          int  j1=j+1;

          float f =0;
          if(!field_->getData( i1, j,&f)){
              continue;
            }

          if (isEqualHARD(iso, f )) a=1;
          else a=0;
          float f1=0;
          if(!field_->getData( i1, j1,&f1)){
              continue;
            }
          if (isEqualHARD(iso, f1 )) b = 1;
          else b = 0;

          bool m= field_->getMask(i1,j);
          bool m1= field_->getMask(i1,j1);

          if ( m && m1 ) {
              if (((a || f < iso )&&(b || iso < f1))||
                  (( a || iso < f )&&(b || iso > f1)))
                {
                  if (a) {
                      indf[n3e(i,j,1)]= 1;
                      argf[n3e(i,j,1)]= field_->getLa(j);
                    }
                  else if (!b)
                    {
                      indf[n3e(i,j,1)] = 1;
                      indf[n3e(i1,j,3)] = 1;
                      argf[n3e(i,j,1)] = field_->getLa(j) + dy*(iso-f)/(f1 - f);
                      argf[n3e(i1,j,3)] =argf[n3e(i,j,1)];
                    }
                  if (b) {
                      indf[n3e(i1,j,3)] = 1;
                      argf[n3e(i1,j,3)] = field_->getLa(j1);
                    }
                }
            }
          // По горизонтали
          if(!field_->getData( i, j1,&f)){
              continue;
            }
          if (isEqualHARD(iso, f )) a=1;
          else a=0;
          m= field_->getMask(i,j1);

          if ( m && m1 ) {
              if (((a || f < iso )&&( b || iso < f1))||
                  ((a || iso < f)&&( b || iso>f1)))
                {
                  if (b) {
                      indf[n3e(i,j,2)] = 1;
                      argf[n3e(i,j,2)] = field_->getFi(i1);
                    }
                  else if (!a)
                    {
                      indf[n3e(i,j,2)] = 1;
                      indf[n3e(i,j1,0)] = 1;
                      argf[n3e(i,j,2)]= field_->getFi(i) + dx*(iso - f)/(f1 - f);
                      argf[n3e(i,j1,0)] = argf[n3e(i,j,2)];
                    }
                  if (a) {
                      indf[n3e(i,j1,0)] = 1;
                      argf[n3e(i,j1,0)] = field_->getFi(i);
                    }
                }
            }
        }
    }
  return ERR_NOERR;
}

int TIsoLineData::oneIso(int j, int k, int l, meteo::GeoVector &ret_vector)
{
  int flag=1, flag1=1, flagn1=1, flagn2=1, flagn3=1;
  int  j0=j, k0=k, l0=l, l1 = 0, sum;
  int kk;
  int kolx = field_->kolFi();
  int koly = field_->kolLa();
  int kolx2 = kolx - 2;
  int koly2 = koly - 2;
  
  char *indf = &indf_[0];
  float *argf = &argf_[0];

  QList<meteo::GeoPoint> iso;
  iso.reserve(10000);
  int i_obhod[3]={-1,1,2};
  // Проверяем, есть ли куда идти
  switch (l) {
    case 0:
      kk=k-1;
//      if (fabs(field_->getFi(j) - argf[n3e(j,k,l)]) < eps) {
      if (isEqualHARD(field_->getFi(j),  (argf[n3e(j,k,l)])) ) {
          if ( k > 0)
            {
              sum=0; // Нижний квадрат
              for (int i=0; i<4; i++) sum += indf[n3e(j,kk,i)];
              if ((sum==0) || (sum%2)) {
                  indf[n3e(j,kk,3)]=0;
                  flagn1=0;
                }
              else {
                  j--;
                  k=kk;
                  l=1;
                  break;
                }
            }
          else flagn1=0;
          if (j>0)
            {
              if (k>0)
                {
                  sum=0; // Левый нижний квадрат
                  for (int i=0; i<4; i++) sum+=indf[n3e(j-1,kk,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j-1,kk,2)]=0;
                      flagn2=0;
                    }
                  else {
                      j--;
                      k=kk+1;
                      break;
                    }
                }
              else flagn2=0;
              sum=0; // Левый квадрат
              for (int i=0; i<4; i++) sum+=indf[n3e(j-1,k,i)];
              if ((sum==0)||(sum%2)) {
                  indf[n3e(j-1,k,1)]=0;
                  flagn3=0;
                }
              else {
                  l=3;
                  break;
                }
            }
          else {
              flagn2=0;
              flagn3=0;
            }
        }
      else if (k==0) {
          flag1=0;
          break;
        }
      if (!(flagn1||flagn2||flagn3)) flag1=0;
      break;
    case 1:
      if (isEqualHARD( field_->getLa(k),  argf[n3e(j,k,l)]))
        {
          kk=k-1;
          if (k>0)
            {
              sum=0; // Нижний квадрат
              for (int i=0; i<4; i++) sum+=indf[n3e(j,kk,i)];
              if ((sum==0)||(sum%2)) {
                  indf[n3e(j,kk,2)]=0;
                  flagn1=0;
                }
              else {
                  l=0;
                  k=kk+1;
                  break;
                }
            }
          else flagn1=0;
          if (j<kolx2)
            {
              if (k>0)
                {
                  sum=0; // Правый нижний квадрат
                  for (int i=0; i<4; i++) sum+=indf[n3e(j+1,kk,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j+1,kk,3)]=0;
                      flagn2=0;
                    }
                  else {
                      k=kk;
                      break;
                    }
                }
              else flagn2=0;
              sum=0; // Правый квадрат
              for (int i=0; i<4; i++) sum+=indf[n3e(j+1,k,i)];
              if ((sum==0)||(sum%2)) {
                  indf[n3e(j+1,k,0)]=0;
                  flagn3=0;
                }
              else {
                  k--;
                  j++;
                  l=2;
                  break;
                }
            }
          else {
              flagn2=0;
              flagn3=0;
            }
        }
      else if (j==kolx2) {
          flag1=0;
          break;
        }
      if (!(flagn1||flagn2||flagn3)) flag1=0;
      break;
    case 2:
      kk=k+1;
      if (isEqualHARD( field_->getFi(j+1), argf[n3e(j,k,l)]))
        {
          if (k<koly2)
            {
              sum=0; // Верхний квадрат
              for (int i=0; i<4; i++) sum+=indf[n3e(j,kk,i)];
              if ((sum==0)||(sum%2)) {
                  indf[n3e(j,kk,1)]=0;
                  flagn1=0;
                }
              else {
                  j++;
                  k=kk;
                  l=3;
                  break;
                }
            }
          else flagn1=0;
          if (j<kolx2)
            {
              if (k<koly2)
                {
                  sum=0; // Правый верхний квадрат
                  for (int i=0; i<4; i++) sum+=indf[n3e(j+1,kk,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j+1,kk,0)]=0;
                      flagn2=0;
                    }
                  else {
                      j++;
                      k=kk-1;
                      break;
                    }
                }
              else flagn2=0;
              sum=0; // Правый квадрат
              for (int i=0; i<4; i++) sum+=indf[n3e(j+1,k,i)];
              if ((sum==0)||(sum%2)) {
                  indf[n3e(j+1,k,3)]=0;
                  flagn3=0;
                }
              else {
                  l=1;
                  break;
                }
            }
          else {
              flagn2=0;
              flagn3=0;
            }
        }
      else if (k==koly2) {
          flag1=0;
          break;
        }
      if (!(flagn1||flagn2||flagn3)) flag1=0;
      break;
    case 3:
      if (isEqualHARD(field_->getLa(k+1), argf[n3e(j,k,l)] ))
        {
          kk=k+1;
          if (k<koly2)
            {
              sum=0; // Верхний квадрат
              for (int i=0; i<4; i++) sum+=indf[n3e(j,kk,i)];
              if ((sum==0)||(sum%2)) {
                  indf[n3e(j,kk,0)]=0;
                  flagn1=0;
                }
              else {
                  l=2;
                  k=kk-1;
                  break;
                }
            }
          else flagn1=0;
          if (j>0)
            {
              if (k<koly2)
                {
                  sum=0; // Левый верхний квадрат
                  for (int i=0; i<4; i++) sum+=indf[n3e(j-1,kk,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j-1,kk,1)]=0;
                      flagn2=0;
                    }
                  else {
                      k=kk;
                      l=3;
                      break;
                    }
                }
              else flagn2=0;
              sum=0; // Правый квадрат
              for (int i=0; i<4; i++) sum+=indf[n3e(j-1,k,i)];
              if ((sum==0)||(sum%2)) {
                  indf[n3e(j-1,k,2)]=0;
                  flagn3=0;
                }
              else {
                  k++;
                  j--;
                  l=0;
                  break;
                }
            }
          else {
              flagn2=0;
              flagn3=0;
            }
        }
      else if (j==0) flag1=0;
      if (!(flagn1||flagn2||flagn3)) flag1=0;
      break;
    default :
      break;
    }
  if (flag1==0) return 0; // Некуда идти из начальной точки
  switch (l) {
    case 0:
      k--;
      l=2;
      iso.append(meteo::GeoPoint(argf[n3e(j,k,l)],field_->getLa(k+1), 0.,meteo::LA_GENERAL));
      break;
    case 1:
      j++;
      l=3;
      iso.append(meteo::GeoPoint(field_->getFi(j), argf[n3e(j,k,l)],0., meteo::LA_GENERAL));
      break;
    case 2:
      k++;
      l=0;
      iso.append(meteo::GeoPoint(argf[n3e(j,k,l)], field_->getLa(k),0., meteo::LA_GENERAL));
      break;
    case 3:
      j--;
      l=1;
      iso.append(meteo::GeoPoint( field_->getFi(j+1),argf[n3e(j,k,l)],0., meteo::LA_GENERAL));
      break;
    default :
      break;
    }
  indf[n3e(j,k,l)]=0;
  // Ищем следующую точку в этом квадрате (справа, слева, напротив, если смотреть
  //         внутрь квадрата)
  for (;;) {
      int i;
      for (i=0; i<3; i++) {
          l1=l+i_obhod[i];
          if (l1>3) l1-=4;
          else if (l1==-1) l1=3;
          if (indf[n3e(j,k,l1)]==1) break;
        }
      if (i==3) {//Если точка не найдена
          if (flag==0)
            break;//Если изолиния построена до конца
          else { // Меняем начало и конец изолинии
              int num1 = iso.size()/2;
              for (i=0; i<num1; i++) {
                  iso.swap(i,iso.size() - i-1);
                }
              flag=0;
              j=j0;
              k=k0;
              l=l0;
              indf[n3e(j,k,l)]=0;
              continue;
            }
        }
      // Если точка найдена
      l=l1;
      switch (l) {
        case 0:
          iso.append(meteo::GeoPoint(argf[n3e(j,k,l)],field_->getLa(k), 0.,meteo::LA_GENERAL));
          break;
        case 1:
          iso.append(meteo::GeoPoint(field_->getFi(j+1),argf[n3e(j,k,l)],0., meteo::LA_GENERAL));
          break;
        case 2:
          iso.append(meteo::GeoPoint(argf[n3e(j,k,l)],field_->getLa(k+1), 0.,meteo::LA_GENERAL));
          break;
        case 3:
          iso.append(meteo::GeoPoint(field_->getFi(j),argf[n3e(j,k,l)], 0.,meteo::LA_GENERAL));
          break;
        default :
          break;
        }
      indf[n3e(j,k,l)]=0;
      if (iso.size() == max_point_in_iso) {
          ret_vector = ret_vector.fromList(iso);;
          return iso.size(); // Получено предельное количество точек
        }
      if ((j==j0)&&(k==k0)&&(l==l0)) break; // Изолиния замкнулась
      // Проверяем, есть ли куда идти
      flagn1=1;
      flagn2=1;
      flagn3=1;
      switch (l) {
        case 0:
          kk=k-1;
          if (isEqualHARD(field_->getFi(j) ,  argf[n3e(j,k,l)])) {
              if (k>0)
                {
                  sum=0; // Нижний квадрат
                  for (i=0; i<4; i++) sum+=indf[n3e(j,kk,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j,kk,3)]=0;
                      flagn1=0;
                    }
                  else {
                      j--;
                      k=kk;
                      l=1;
                      break;
                    }
                }
              else flagn1=0;
              if (j>0)
                {
                  if (k>0)
                    {
                      sum=0; // Левый нижний квадрат
                      for (i=0; i<4; i++) sum+=indf[n3e(j-1,kk,i)];
                      if ((sum==0)||(sum%2)) {
                          indf[n3e(j-1,kk,2)]=0;
                          flagn2=0;
                        }
                      else {
                          j--;
                          k=kk+1;
                          break;
                        }
                    }
                  else flagn2=0;

                  sum=0; // Левый квадрат
                  for (i=0; i<4; i++) sum+=indf[n3e(j-1,k,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j-1,k,1)]=0;
                      flagn3=0;
                    }
                  else {
                      l=3;
                      break;
                    }
                }
              else {
                  flagn2=0;
                  flagn3=0;
                }
            }
          else if (k==0) {
              flag1=0;
              break;
            }
          if (!(flagn1||flagn2||flagn3)) flag1=0;
          break;
        case 1:
          if (isEqualHARD(field_->getLa(k),argf[n3e(j,k,l)]))
            {
              kk=k-1;
              if (k>0)
                {
                  sum=0; // Нижний квадрат
                  for (i=0; i<4; i++) sum+=indf[n3e(j,kk,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j,kk,2)]=0;
                      flagn1=0;
                    }
                  else {
                      l=0;
                      k=kk+1;
                      break;
                    }
                }
              else flagn1=0;
              if (j<kolx2)
                {
                  if (k>0)
                    {
                      sum=0; // Правый нижний квадрат
                      for (i=0; i<4; i++) sum+=indf[n3e(j+1,kk,i)];
                      if ((sum==0)||(sum%2)) {
                          indf[n3e(j+1,kk,3)]=0;
                          flagn2=0;
                        }
                      else {
                          k=kk;
                          break;
                        }
                    }
                  else flagn2=0;

                  sum=0; // Правый квадрат
                  for (i=0; i<4; i++) sum+=indf[n3e(j+1,k,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j+1,k,0)]=0;
                      flagn3=0;
                    }
                  else {
                      k--;
                      j++;
                      l=2;
                      break;
                    }
                }
              else {
                  flagn2=0;
                  flagn3=0;
                }
            }
          else if (j==kolx2) {
              flag1=0;
              break;
            }
          if (!(flagn1||flagn2||flagn3)) flag1=0;
          break;
        case 2:
          kk=k+1;
          if (isEqualHARD(field_->getFi(j+1), argf[n3e(j,k,l)]))
            {
              if (k<koly2)
                {
                  sum=0; // Верхний квадрат
                  for (i=0; i<4; i++) sum+=indf[n3e(j,kk,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j,kk,1)]=0;
                      flagn1=0;
                    }
                  else {
                      j++;
                      k=kk;
                      l=3;
                      break;
                    }
                }
              else flagn1=0;
              if (j<kolx2)
                {
                  if (k<koly2)
                    {
                      sum=0; // Правый верхний квадрат
                      for (i=0; i<4; i++) sum+=indf[n3e(j+1,kk,i)];
                      if ((sum==0)||(sum%2)) {
                          indf[n3e(j+1,kk,0)]=0;
                          flagn2=0;
                        }
                      else {
                          j++;
                          k=kk-1;
                          break;
                        }
                    }
                  else flagn2=0;

                  sum=0; // Правый квадрат
                  for (i=0; i<4; i++) sum+=indf[n3e(j+1,k,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j+1,k,3)]=0;
                      flagn3=0;
                    }
                  else {
                      l=1;
                      break;
                    }
                }
              else {
                  flagn2=0;
                  flagn3=0;
                }
            }
          else if (k==koly2) {
              flag1=0;
              break;
            }
          if (!(flagn1||flagn2||flagn3)) flag1=0;
          break;
        case 3:
          if (isEqualHARD(field_->getLa(k+1), argf[n3e(j,k,l)]))
            {
              kk=k+1;
              if (k<koly2)
                {
                  sum=0; // Верхний квадрат
                  for (i=0; i<4; i++) sum+=indf[n3e(j,kk,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j,kk,0)]=0;
                      flagn1=0;
                    }
                  else {
                      l=2;
                      k=kk-1;
                      break;
                    }
                }
              else flagn1=0;
              if (j>0)
                {
                  if (k<koly2)
                    {
                      sum=0; // Левый верхний квадрат
                      for (i=0; i<4; i++) sum+=indf[n3e(j-1,kk,i)];
                      if ((sum==0)||(sum%2)) {
                          indf[n3e(j-1,kk,1)]=0;
                          flagn2=0;
                        }
                      else {
                          k=kk;
                          l=3;
                          break;
                        }
                    }
                  else flagn2=0;
                  sum=0; // Правый квадрат
                  for (i=0; i<4; i++) sum+=indf[n3e(j-1,k,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j-1,k,2)]=0;
                      flagn3=0;
                    }
                  else {
                      k++;
                      j--;
                      l=0;
                      break;
                    }
                }
              else {
                  flagn2=0;
                  flagn3=0;
                }
            }
          else if (j==0) flag1=0;
          if (!(flagn1||flagn2||flagn3)) flag1=0;
          break;
        default :
          break;
        }
      // Проверяем, не на границе ли области мы находимся
      if (flag1==0)
        // Находимся на границе области
        if (flag==0) break;//Если изолиния построена до конца
        else { // Меняем начало и конец изолинии
            int num1 = iso.size()/2;
            for (i=0; i<num1; i++) {
                iso.swap(i,iso.size() - i-1);
              }
            flag=0;
            j=j0;
            k=k0;
            l=l0;
            indf[n3e(j,k,l)]=0;
            flag1=1;
            continue;
          }
      // Находимся не на границе области
      else {
          // Переходим в соседний квадрат
          switch (l) {
            case 0:
              k--;
              l=2;
              break;
            case 1:
              j++;
              l=3;
              break;
            case 2:
              k++;
              l=0;
              break;
            case 3:
              j--;
              l=1;
              break;
            default :
              break;
            }
          indf[n3e(j,k,l)]=0;
        }
    }
  ret_vector = ret_vector.fromList(iso);
  return iso.size();
}

int TIsoLineData::oneIsoSphere(int j, int k, int l, meteo::GeoVector &ret_vector)
{
  int flag=1, flag1=1, flagn1=1, flagn2=1, flagn3=1;
  int  j0=j, k0=k, l0=l, l1 = 0, sum;
  int kk;
  int kolx = field_->kolFi();
  int koly = field_->kolLa();
  int kolx2 = kolx - 2;
  int koly2 = koly - 2;
  
  char * indf = &indf_[0];
  float * argf = &argf_[0];

  QList<meteo::GeoPoint> iso;
  
  int i_obhod[3]={-1,1,2};
  // Проверяем, есть ли куда идти
  switch (l) {
    case 0:
      kk=k-1;
      if (kk<0) kk=koly2;
      if (isEqualHARD(field_->getFi(j), argf[n3e(j,k,l)])) {
          sum=0; // Нижний квадрат
          for (int i = 0; i < 4; i++)
            sum+=indf[n3e(j,kk,i)];
          if ((sum==0)||(sum%2)) {
              indf[n3e(j,kk,3)]=0;
              flagn1=0;
            }
          else {
              j--;
              k=kk;
              l=1;
              break;
            }
          if (j>0) {
              sum=0; // Левый нижний квадрат
              for (int i=0; i<4; i++) sum+=indf[n3e(j-1,kk,i)];
              if ((sum==0)||(sum%2)) {
                  indf[n3e(j-1,kk,2)]=0;
                  flagn2=0;
                }
              else {
                  j--;
                  k=kk+1;
                  break;
                }

              sum=0; // Левый квадрат
              for (int i=0; i<4; i++) sum+=indf[n3e(j-1,k,i)];
              if ((sum==0)||(sum%2)) {
                  indf[n3e(j-1,k,1)]=0;
                  flagn3=0;
                }
              else {
                  l=3;
                  break;
                }
            }
          else {
              flagn2=0;
              flagn3=0;
            }
        }
      else k=kk+1;
      if (!(flagn1||flagn2||flagn3)) flag1=0;
      break;
    case 1:
      if (isEqualHARD(field_->getLa(k), argf[n3e(j,k,l)])) {
          kk=k-1;
          if (kk<0) kk=koly2;
          sum=0; // Нижний квадрат
          for (int i=0; i<4; i++) sum+=indf[n3e(j,kk,i)];
          if ((sum==0)||(sum%2)) {
              indf[n3e(j,kk,2)]=0;
              flagn1=0;
            }
          else {
              l=0;
              k=kk+1;
              break;
            }
          if (j<kolx2) {
              sum=0; // Правый нижний квадрат
              for (int i=0; i<4; i++) sum+=indf[n3e(j+1,kk,i)];
              if ((sum==0)||(sum%2)) {
                  indf[n3e(j+1,kk,3)]=0;
                  flagn2=0;
                }
              else {
                  k=kk;
                  break;
                }

              sum=0; // Правый квадрат
              for (int i=0; i<4; i++) sum+=indf[n3e(j+1,k,i)];
              if ((sum==0)||(sum%2)) {
                  indf[n3e(j+1,k,0)]=0;
                  flagn3=0;
                }
              else {
                  k--;
                  j++;
                  l=2;
                  break;
                }
            }
          else {
              flagn2=0;
              flagn3=0;
            }
        }
      else if (j==kolx2) flag1=0;
      if (!(flagn1||flagn2||flagn3)) flag1=0;
      break;
    case 2:
      kk=k+1;
      if (kk>koly2) kk=0;
      if (isEqualHARD(field_->getFi(j+1), argf[n3e(j,k,l)])) {
          sum=0; // Верхний квадрат
          for (int i=0; i<4; i++) sum+=indf[n3e(j,kk,i)];
          if ((sum==0)||(sum%2)) {
              indf[n3e(j,kk,1)]=0;
              flagn1=0;
            }
          else {
              j++;
              k=kk;
              l=3;
              break;
            }
          if (j<kolx2) {
              sum=0; // Правый верхний квадрат
              for (int i=0; i<4; i++) sum+=indf[n3e(j+1,kk,i)];
              if ((sum==0)||(sum%2)) {
                  indf[n3e(j+1,kk,0)]=0;
                  flagn2=0;
                }
              else {
                  j++;
                  k=kk-1;
                  break;
                }

              sum=0; // Правый квадрат
              for (int i=0; i<4; i++) sum+=indf[n3e(j+1,k,i)];
              if ((sum==0)||(sum%2)) {
                  indf[n3e(j+1,k,3)]=0;
                  flagn3=0;
                }
              else {
                  l=1;
                  break;
                }
            }
          else {
              flagn2=0;
              flagn3=0;
            }
        }
      else k=kk-1;
      if (!(flagn1||flagn2||flagn3)) flag1=0;
      break;
    case 3:
      if (isEqualHARD(field_->getLa(k+1), argf[n3e(j,k,l)])) {
          kk=k+1;
          if (kk>koly2) kk=0;
          sum=0; // Верхний квадрат
          for (int i=0; i<4; i++) sum+=indf[n3e(j,kk,i)];
          if ((sum==0)||(sum%2)) {
              indf[n3e(j,kk,0)]=0;
              flagn1=0;
            }
          else {
              l=2;
              k=kk-1;
              break;
            }
          if (j>0)    {
              sum=0; // Левый верхний квадрат
              for (int i=0; i<4; i++) sum+=indf[n3e(j-1,kk,i)];
              if ((sum==0)||(sum%2)) {
                  indf[n3e(j-1,kk,1)]=0;
                  flagn2=0;
                }
              else {
                  k=kk;
                  l=3;
                  break;
                }
              sum=0; // Правый квадрат
              for (int i=0; i<4; i++) sum+=indf[n3e(j-1,k,i)];
              if ((sum==0)||(sum%2)) {
                  indf[n3e(j-1,k,2)]=0;
                  flagn3=0;
                }
              else {
                  k++;
                  j--;
                  l=0;
                  break;
                }
            }
          else {
              flagn2=0;
              flagn3=0;
            }
        }
      else if (j==0) flag1=0;
      if (!(flagn1||flagn2||flagn3)) flag1=0;
      break;
    default :
      break;
    }
  if (flag1==0) return 0; // Некуда идти из начальной точки
   switch (l) {
    case 0:
      k--;
      l=2;
      iso.append(meteo::GeoPoint(argf[n3e(j,k,l)],field_->getLa(k+1)));
      break;
    case 1:
      j++;
      l=3;
      iso.append(meteo::GeoPoint(field_->getFi(j), argf[n3e(j,k,l)]));
      break;
    case 2:
      k++;
      l=0;
      iso.append(meteo::GeoPoint(argf[n3e(j,k,l)], field_->getLa(k)));
      break;
    case 3:
      j--;
      l=1;
      iso.append(meteo::GeoPoint( field_->getFi(j+1),argf[n3e(j,k,l)]));
      break;
    default :
      break;
    }
  indf[n3e(j,k,l)]=0;
  // Ищем следующую точку в этом квадрате (справа, слева, напротив, если смотреть
  //         внутрь квадрата)
  for (;;) {
      int i;
      for ( i=0; i<3; i++) {
          l1=l+i_obhod[i];
          if (l1>3) l1-=4;
          else if (l1==-1) l1=3;
          if (indf[n3e(j,k,l1)]==1) break;
        }
      if ( i==3) {                //Если точка не найдена
          if (flag==0) break;//Если изолиния построена до конца
          else { // Меняем начало и конец изолинии
              int num1 = iso.size()/2;
              for (i=0; i<num1; i++) {
                  iso.swap(i,iso.size() - i-1);
                }
              flag=0;
              j=j0;
              k=k0;
              l=l0;
              indf[n3e(j,k,l)]=0;
              continue;
            }
        }
      // Если точка найдена
      l=l1;
      switch (l) {
        case 0:
          iso.append(meteo::GeoPoint(argf[n3e(j,k,l)],field_->getLa(k)));
          break;
        case 1:
          iso.append(meteo::GeoPoint(field_->getFi(j+1),argf[n3e(j,k,l)]));
          break;
        case 2:
          iso.append(meteo::GeoPoint(argf[n3e(j,k,l)],field_->getLa(k+1)));
          break;
        case 3:
          iso.append(meteo::GeoPoint(field_->getFi(j),argf[n3e(j,k,l)]));
          break;
        default :
          break;
        }
      indf[n3e(j,k,l)]=0;
      if (iso.size() == max_point_in_iso){


          ret_vector = ret_vector.fromList(iso);
          //          qDebug()<<"1  ret_vector" <<ret_vector->size();

          return iso.size(); // Получено предельное количество точек
        }
      if ((j==j0)&&(k==k0)&&(l==l0)) break; // Изолиния замкнулась
      // Проверяем, есть ли куда идти
      flagn1=1;
      flagn2=1;
      flagn3=1;
      switch (l) {
        case 0:
          kk=k-1;
          if (kk<0) kk=koly2;
          if (isEqualHARD(field_->getFi(j),argf[n3e(j,k,l)])) {
              sum=0; // Нижний квадрат
              for (i=0; i<4; i++) sum+=indf[n3e(j,kk,i)];
              if ((sum==0)||(sum%2)) {
                  indf[n3e(j,kk,3)]=0;
                  flagn1=0;
                }
              else {
                  j--;
                  k=kk;
                  l=1;
                  break;
                }
              if (j>0) {
                  sum=0; // Левый нижний квадрат
                  for (i=0; i<4; i++) sum+=indf[n3e(j-1,kk,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j-1,kk,2)]=0;
                      flagn2=0;
                    }
                  else {
                      j--;
                      k=kk+1;
                      break;
                    }

                  sum=0; // Левый квадрат
                  for (i=0; i<4; i++) sum+=indf[n3e(j-1,k,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j-1,k,1)]=0;
                      flagn3=0;
                    }
                  else {
                      l=3;
                      break;
                    }
                }
              else {
                  flagn2=0;
                  flagn3=0;
                }
            }
          else k=kk+1;
          if (!(flagn1||flagn2||flagn3)) flag1=0;
          break;
        case 1:
          if (isEqualHARD(field_->getLa(k),argf[n3e(j,k,l)])) {
              kk=k-1;
              if (kk<0) kk=koly2;
              sum=0; // Нижний квадрат
              for (i=0; i<4; i++) sum+=indf[n3e(j,kk,i)];
              if ((sum==0)||(sum%2)) {
                  indf[n3e(j,kk,2)]=0;
                  flagn1=0;
                }
              else {
                  l=0;
                  k=kk+1;
                  break;
                }
              if (j<kolx2) {
                  sum=0; // Правый нижний квадрат
                  for (i=0; i<4; i++) sum+=indf[n3e(j+1,kk,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j+1,kk,3)]=0;
                      flagn2=0;
                    }
                  else {
                      k=kk;
                      break;
                    }

                  sum=0; // Правый квадрат
                  for (i=0; i<4; i++) sum+=indf[n3e(j+1,k,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j+1,k,0)]=0;
                      flagn3=0;
                    }
                  else {
                      k--;
                      j++;
                      l=2;
                      break;
                    }
                }
              else {
                  flagn2=0;
                  flagn3=0;
                }
            }
          else if (j==kolx2) flag1=0;
          if (!(flagn1||flagn2||flagn3)) flag1=0;
          break;
        case 2:
          kk=k+1;
          if (kk>koly2) kk=0;
          if (isEqualHARD(field_->getFi(j+1),argf[n3e(j,k,l)])) {
              sum=0; // Верхний квадрат
              for (i=0; i<4; i++) sum+=indf[n3e(j,kk,i)];
              if ((sum==0)||(sum%2)) {
                  indf[n3e(j,kk,1)]=0;
                  flagn1=0;
                }
              else {
                  j++;
                  k=kk;
                  l=3;
                  break;
                }
              if (j<kolx2) {
                  sum=0; // Правый верхний квадрат
                  for (i=0; i<4; i++) sum+=indf[n3e(j+1,kk,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j+1,kk,0)]=0;
                      flagn2=0;
                    }
                  else {
                      j++;
                      k=kk-1;
                      break;
                    }

                  sum=0; // Правый квадрат
                  for (i=0; i<4; i++) sum+=indf[n3e(j+1,k,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j+1,k,3)]=0;
                      flagn3=0;
                    }
                  else {
                      l=1;
                      break;
                    }
                }
              else {
                  flagn2=0;
                  flagn3=0;
                }
            }
          else k=kk-1;
          if (!(flagn1||flagn2||flagn3)) flag1=0;
          break;
        case 3:
          if (isEqualHARD(field_->getLa(k+1), argf[n3e(j,k,l)])) {
              kk=k+1;
              if (kk>koly2) kk=0;
              sum=0; // Верхний квадрат
              for (i=0; i<4; i++) sum+=indf[n3e(j,kk,i)];
              if ((sum==0)||(sum%2)) {
                  indf[n3e(j,kk,0)]=0;
                  flagn1=0;
                }
              else {
                  l=2;
                  k=kk-1;
                  break;
                }
              if (j>0)    {
                  sum=0; // Левый верхний квадрат
                  for (i=0; i<4; i++) sum+=indf[n3e(j-1,kk,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j-1,kk,1)]=0;
                      flagn2=0;
                    }
                  else {
                      k=kk;
                      l=3;
                      break;
                    }
                  sum=0; // Правый квадрат
                  for (i=0; i<4; i++) sum+=indf[n3e(j-1,k,i)];
                  if ((sum==0)||(sum%2)) {
                      indf[n3e(j-1,k,2)]=0;
                      flagn3=0;
                    }
                  else {
                      k++;
                      j--;
                      l=0;
                      break;
                    }
                }
              else {
                  flagn2=0;
                  flagn3=0;
                }
            }
          else if (j==0) flag1=0;
          if (!(flagn1||flagn2||flagn3)) flag1=0;
          break;
        default :
          break;
        }
      // Проверяем, не на границе ли области мы находимся
      if (flag1==0)
        // Находимся на границе области
        if (flag==0) break;//Если изолиния построена до конца
        else { // Меняем начало и конец изолинии
            int num1 = iso.size()/2;
            for (i=0; i<num1; i++) {
                iso.swap(i,iso.size() - 1 - i);
              }
            flag=0;
            j=j0;
            k=k0;
            l=l0;
            indf[n3e(j,k,l)]=0;
            flag1=1;
            continue;
          }
      // Находимся не на границе области
      else {
          // Переходим в соседний квадрат
          switch (l) {
            case 0:
              k--;
              l=2;
              break;
            case 1:
              j++;
              l=3;
              break;
            case 2:
              k++;
              l=0;
              break;
            case 3:
              j--;
              l=1;
              break;
            default :
              break;
            }
          indf[n3e(j,k,l)]=0;
        }
    }
  ret_vector = ret_vector.fromList(iso);
  return iso.size();
}

inline int Num3_Elem(int i, int j, int k, int m, int l)
{
  return (i*m+j)*l+k;
}

inline int Num2_Elem(int i, int j, int m)
{
  return i*m+j;
}


void TIsoLineData::Iso_Vspom(float iso, const  float *netx, const float *nety,const float *fun,const bool *obres,
                             char *indf, float *argf, int kolx,  int koly)
{
  int i, j, i1, j1, nvs, kolx1=kolx-1, koly1=koly-1, kolx2=kolx-2, koly2=koly-2;
  float dy, dx;
  int a, b = 0;
  if(!netx||!nety||!fun) return;
  dx=(netx[1]-netx[0]);
  dy=(nety[1]-nety[0]);
  nvs=(kolx-1)*(koly-1)*4;

  bzero(indf,nvs);
  bzero(argf,nvs);
  //for(i=0; i<nvs; i++) {indf[i]=0; argf[i]=0.;}
  //**************************************************************
  for(j=0; j<koly1; j++) {// Слева  и справа
      j1=j+1;
      if(!obres[Num2_Elem(0,j,koly)]&&!obres[Num2_Elem(0,j1,koly)] // Слева
         &&!obres[Num2_Elem(kolx1,j,koly)]&&!obres[Num2_Elem(kolx1,j1,koly)] // Справа
         &&!obres[Num2_Elem(kolx2,j1,koly)]) continue;

      if(isEqualHARD(iso,fun[Num2_Elem(0,j,koly)])) a=1; else a=0;
      if(isEqualHARD(iso,fun[Num2_Elem(0,j1,koly)])) b=1; else b=0;


      if(obres[Num2_Elem(0,j,koly)]&&obres[Num2_Elem(0,j1,koly)]) // Слева
        if(((a||fun[Num2_Elem(0,j,koly)]<iso)&&(b||iso<fun[Num2_Elem(0,j1,koly)]))||
           ((a||iso<fun[Num2_Elem(0,j,koly)])&&(b||iso>fun[Num2_Elem(0,j1,koly)])))
          {
            if(b) {
                indf[Num3_Elem(0,j,3,koly1,4)]=1;
                argf[Num3_Elem(0,j,3,koly1,4)]=nety[j1];
              }
            else if(!a) {
                indf[Num3_Elem(0,j,3,koly1,4)]=1;
                argf[Num3_Elem(0,j,3,koly1,4)]=nety[j]+
                    dy*(iso-fun[Num2_Elem(0,j,koly)])/
                    (fun[Num2_Elem(0,j1,koly)]-fun[Num2_Elem(0,j,koly)]);
              }

          }

      if(isEqualHARD(iso,fun[Num2_Elem(kolx1,j,koly)])) a=1; else a=0;
      if(isEqualHARD(iso,fun[Num2_Elem(kolx1,j1,koly)])) b=1; else b=0;
      if(obres[Num2_Elem(kolx1,j,koly)]&&obres[Num2_Elem(kolx1,j1,koly)]) // Справа

        if(((a||fun[Num2_Elem(kolx1,j,koly)]<iso)&&(b||iso<fun[Num2_Elem(kolx1,j1,koly)]))||
           ((a||iso<fun[Num2_Elem(kolx1,j,koly)])&&(b||iso>fun[Num2_Elem(kolx1,j1,koly)])))
          {
            if(a) {
                indf[Num3_Elem(kolx2,j,1,koly1,4)]=1;
                argf[Num3_Elem(kolx2,j,1,koly1,4)]=nety[j];
              }
            else {
                if(!b) {
                    indf[Num3_Elem(kolx2,j,1,koly1,4)]=1;
                    argf[Num3_Elem(kolx2,j,1,koly1,4)]=nety[j]+
                        dy*(iso-fun[Num2_Elem(kolx1,j,koly)])/
                        (fun[Num2_Elem(kolx1,j1,koly)]-fun[Num2_Elem(kolx1,j,koly)]);
                  }
              }
          }

      if(j<koly2) {// Справа по горизонтали
          if(isEqualHARD(iso,fun[Num2_Elem(kolx2,j,koly)])) a=1; else a=0;
          if(obres[Num2_Elem(kolx2,j1,koly)]&&obres[Num2_Elem(kolx1,j1,koly)])
            if(((a||fun[Num2_Elem(kolx2,j1,koly)]<iso)&&(b||iso<fun[Num2_Elem(kolx1,j1,koly)]))||
               ((a||iso<fun[Num2_Elem(kolx2,j1,koly)])&&(b||iso>fun[Num2_Elem(kolx1,j1,koly)])))
              { if(b) {
                    indf[Num3_Elem(kolx2,j,2,koly1,4)]=1;
                    argf[Num3_Elem(kolx2,j,2,koly1,4)]=netx[kolx1];
                  }
                else
                  if(!a)
                    {indf[Num3_Elem(kolx2,j,2,koly1,4)]=1;
                      indf[Num3_Elem(kolx2,j1,0,koly1,4)]=1;
                      argf[Num3_Elem(kolx2,j,2,koly1,4)]=netx[kolx2]+
                          dx*(iso-fun[Num2_Elem(kolx2,j1,koly)])/
                          (fun[Num2_Elem(kolx1,j1,koly)]-fun[Num2_Elem(kolx2,j1,koly)]);
                      argf[Num3_Elem(kolx2,j1,0,koly1,4)]=argf[Num3_Elem(kolx2,j,2,koly1,4)];
                    }
                if(a) {
                    indf[Num3_Elem(kolx2,j+1,0,koly1,4)]=1;
                    argf[Num3_Elem(kolx2,j+1,0,koly1,4)]=netx[kolx2];
                  }
              }
        }
    }

  //***************************************************************
  for(i=0; i<kolx1; i++) { // Снизу и сверху
      i1=i+1;
      if(!obres[Num2_Elem(i,0,koly)]&&!obres[Num2_Elem(i1,0,koly)] // Снизу
         &&!obres[Num2_Elem(i,koly1,koly)]&&!obres[Num2_Elem(i1,koly1,koly)] // Сверху
         &&!obres[Num2_Elem(i1,koly2,koly)]&&!obres[Num2_Elem(i1,koly1,koly)]){// Сверху по вертикали
          continue;
        }

      if(isEqualHARD(iso,fun[Num2_Elem(i,0,koly)])) a=1; else a=0;
      if(isEqualHARD(iso,fun[Num2_Elem(i1,0,koly)])) b=1; else b=0;
      if(obres[Num2_Elem(i,0,koly)]&&obres[Num2_Elem(i1,0,koly)]) // Снизу
        if(((a||fun[Num2_Elem(i,0,koly)]<iso)&&(b||iso<fun[Num2_Elem(i1,0,koly)]))||
           ((a||iso<fun[Num2_Elem(i,0,koly)])&&(b||iso>fun[Num2_Elem(i1,0,koly)])))
          {
            if(a) {
                indf[Num3_Elem(i,0,0,koly1,4)]=1;
                argf[Num3_Elem(i,0,0,koly1,4)]=netx[i];
              } else {
                if(!b) {
                    indf[Num3_Elem(i,0,0,koly1,4)]=1;
                    argf[Num3_Elem(i,0,0,koly1,4)]=netx[i]+
                        dx*(iso-fun[Num2_Elem(i,0,koly)])/
                        (fun[Num2_Elem(i1,0,koly)]-fun[Num2_Elem(i,0,koly)]);
                  }
              }
          }
      if(isEqualHARD(iso,fun[Num2_Elem(i,koly1,koly)])) a=1; else a=0;
      if(isEqualHARD(iso,fun[Num2_Elem(i1,koly1,koly)])) b=1; else b=0;
      if(obres[Num2_Elem(i,koly1,koly)]&&obres[Num2_Elem(i1,koly1,koly)]) // Сверху
        if(((a||fun[Num2_Elem(i,koly1,koly)]<iso)&&(b||iso<fun[Num2_Elem(i1,koly1,koly)]))||
           ((a||iso<fun[Num2_Elem(i,koly1,koly)])&&(b||iso>fun[Num2_Elem(i1,koly1,koly)])))
          {
            if(b) {
                indf[Num3_Elem(i,koly2,2,koly1,4)]=1;
                argf[Num3_Elem(i,koly2,2,koly1,4)]=netx[i1];
              }
            else {
                if(!a) {
                    indf[Num3_Elem(i,koly2,2,koly1,4)]=1;
                    argf[Num3_Elem(i,koly2,2,koly1,4)]=netx[i]+
                        dx*(iso-fun[Num2_Elem(i,koly1,koly)])/
                        (fun[Num2_Elem(i1,koly1,koly)]-fun[Num2_Elem(i,koly1,koly)]);
                  }
              }
          }


      if(i<kolx2) {// Сверху по вертикали
          if(isEqualHARD(iso,fun[Num2_Elem(i1,koly2,koly)])) a=1; else a=0;
          if(obres[Num2_Elem(i1,koly2,koly)]&&obres[Num2_Elem(i1,koly1,koly)])
            if(((a||fun[Num2_Elem(i1,koly2,koly)]<iso)&&(b||iso<fun[Num2_Elem(i1,koly1,koly)]))||
               ((a||iso<fun[Num2_Elem(i1,koly2,koly)])&&(b||iso>fun[Num2_Elem(i1,koly1,koly)])))
              { if(a) {
                    indf[Num3_Elem(i,koly2,1,koly1,4)]=1;
                    argf[Num3_Elem(i,koly2,1,koly1,4)]=nety[koly2];
                  }
                else if(!b)
                  {indf[Num3_Elem(i,koly2,1,koly1,4)]=1;
                    indf[Num3_Elem(i1,koly2,3,koly1,4)]=1;
                    argf[Num3_Elem(i,koly2,1,koly1,4)]=nety[koly2]+
                        dy*(iso-fun[Num2_Elem(i1,koly2,koly)])/
                        (fun[Num2_Elem(i1,koly1,koly)]-fun[Num2_Elem(i1,koly2,koly)]);
                    argf[Num3_Elem(i1,koly2,3,koly1,4)]=argf[Num3_Elem(i,koly2,1,koly1,4)];
                  }
                if(b) {
                    indf[Num3_Elem(i1,koly2,3,koly1,4)]=1;
                    argf[Num3_Elem(i1,koly2,3,koly1,4)]=nety[koly1];
                  }
              }
        }
    }
  //***************************************************************
  for(i=0; i<kolx2; i++) {// Внутренность
      i1=i+1;
      for(j=0; j<koly2; j++) {
          // По вертикали
          j1=j+1;
          if(obres[Num2_Elem(i1,j,koly)]&&obres[Num2_Elem(i1,j1,koly)]){
            if(isEqualHARD(iso,fun[Num2_Elem(i1,j,koly)])) a=1; else a=0;
            if(isEqualHARD(iso,fun[Num2_Elem(i1,j1,koly)])) b=1; else b=0;
            if(((a||fun[Num2_Elem(i1,j,koly)]<iso)&&(b||iso<fun[Num2_Elem(i1,j1,koly)]))||
               ((a||iso<fun[Num2_Elem(i1,j,koly)])&&(b||iso>fun[Num2_Elem(i1,j1,koly)])))
              {
                if(a) {
                    indf[Num3_Elem(i,j,1,koly1,4)]=1;
                    argf[Num3_Elem(i,j,1,koly1,4)]=nety[j];
                  }
                else if(!b)
                  {indf[Num3_Elem(i,j,1,koly1,4)]=1;
                    indf[Num3_Elem(i1,j,3,koly1,4)]=1;
                    argf[Num3_Elem(i,j,1,koly1,4)]=nety[j]+
                        dy*(iso-fun[Num2_Elem(i1,j,koly)])/
                        (fun[Num2_Elem(i1,j1,koly)]-fun[Num2_Elem(i1,j,koly)]);
                    argf[Num3_Elem(i1,j,3,koly1,4)]=argf[Num3_Elem(i,j,1,koly1,4)];
                  }
                if(b) {
                    indf[Num3_Elem(i1,j,3,koly1,4)]=1;
                    argf[Num3_Elem(i1,j,3,koly1,4)]=nety[j1];
                  }
              }
            }
          // По горизонтали
          if(obres[Num2_Elem(i,j1,koly)]&&obres[Num2_Elem(i1,j1,koly)]){
            if(isEqualHARD(iso,fun[Num2_Elem(i,j1,koly)])) a=1; else a=0;
            if(((a||fun[Num2_Elem(i,j1,koly)]<iso)&&(b||iso<fun[Num2_Elem(i1,j1,koly)]))||
               ((a||iso<fun[Num2_Elem(i,j1,koly)])&&(b||iso>fun[Num2_Elem(i1,j1,koly)])))
              {
                if(b) {
                    indf[Num3_Elem(i,j,2,koly1,4)]=1;
                    argf[Num3_Elem(i,j,2,koly1,4)]=netx[i1];
                  }
                else if(!a)
                  {indf[Num3_Elem(i,j,2,koly1,4)]=1;
                    indf[Num3_Elem(i,j1,0,koly1,4)]=1;
                    argf[Num3_Elem(i,j,2,koly1,4)]=netx[i]+
                        dx*(iso-fun[Num2_Elem(i,j1,koly)])/
                        (fun[Num2_Elem(i1,j1,koly)]-fun[Num2_Elem(i,j1,koly)]);
                    argf[Num3_Elem(i,j1,0,koly1,4)]=argf[Num3_Elem(i,j,2,koly1,4)];
                  }
                if(a) {
                    indf[Num3_Elem(i,j1,0,koly1,4)]=1;
                    argf[Num3_Elem(i,j1,0,koly1,4)]=netx[i];
                  }
              }
            }
        }
    }

}

int TIsoLineData::One_Iso(int j, int k, int l, const float *netx, const float *nety, char *indf,
                          float *argf, float *iso,  int kolx,  int koly,
                          int Kol_Num)
{
  int flag=1, flag1=1, flagn1=1, flagn2=1, flagn3=1, num=0, num1=0,
      kolx2=kolx-2, koly2=koly-2, koly1=koly-1, j0=j, k0=k, l0=l, l1 = 0, sum;
  int kk;
  float vs, vs1;
  int l_Obhod[3]={-1,1,2}, i, i1, i2;
  // Проверяем, есть ли куда идти
  switch(l) {
    case 0:
      kk=k-1;
      if(isEqualHARD(netx[j],argf[Num3_Elem(j,k,l,koly1,4)])) {
          if(k>0)
            {
              sum=0; // Нижний квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j,kk,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j,kk,3,koly1,4)]=0; flagn1=0;}
              else {j--; k=kk; l=1; break;}
            }
          else flagn1=0;
          if(j>0)
            {
              if(k>0)
                {
                  sum=0; // Левый нижний квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j-1,kk,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j-1,kk,2,koly1,4)]=0; flagn2=0;}
                  else {j--; k=kk+1; break;}
                }
              else flagn2=0;
              sum=0; // Левый квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j-1,k,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j-1,k,1,koly1,4)]=0;  flagn3=0;}
              else {l=3; break;}
            }
          else {flagn2=0; flagn3=0;}
        }
      else if(k==0) {flag1=0; break;}
      if(!(flagn1||flagn2||flagn3)) flag1=0;
      break;
    case 1:
      if(isEqualHARD(nety[k],argf[Num3_Elem(j,k,l,koly1,4)]))
        {
          kk=k-1;
          if(k>0)
            {
              sum=0; // Нижний квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j,kk,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j,kk,2,koly1,4)]=0; flagn1=0;}
              else {l=0; k=kk+1; break;}
            }
          else flagn1=0;
          if(j<kolx2)
            {
              if(k>0)
                {
                  sum=0; // Правый нижний квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j+1,kk,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j+1,kk,3,koly1,4)]=0; flagn2=0;}
                  else {k=kk; break;}
                }
              else flagn2=0;
              sum=0; // Правый квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j+1,k,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j+1,k,0,koly1,4)]=0; flagn3=0;}
              else {k--; j++; l=2; break;}
            }
          else {flagn2=0; flagn3=0;}
        }
      else if(j==kolx2) {flag1=0; break;}
      if(!(flagn1||flagn2||flagn3)) flag1=0;
      break;
    case 2:
      kk=k+1;
      if(isEqualHARD(netx[j+1],argf[Num3_Elem(j,k,l,koly1,4)]))
        {
          if(k<koly2)
            {
              sum=0; // Верхний квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j,kk,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j,kk,1,koly1,4)]=0; flagn1=0;}
              else {j++; k=kk; l=3; break;}
            }
          else flagn1=0;
          if(j<kolx2)
            {
              if(k<koly2)
                {
                  sum=0; // Правый верхний квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j+1,kk,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j+1,kk,0,koly1,4)]=0; flagn2=0;}
                  else {j++;k=kk-1; break;}
                }
              else flagn2=0;
              sum=0; // Правый квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j+1,k,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j+1,k,3,koly1,4)]=0; flagn3=0;}
              else {l=1; break;}
            }
          else {flagn2=0;flagn3=0;}
        }
      else if(k==koly2) {flag1=0; break;}
      if(!(flagn1||flagn2||flagn3)) flag1=0;
      break;
    case 3:
      if(isEqualHARD(nety[k+1],argf[Num3_Elem(j,k,l,koly1,4)]))
        {
          kk=k+1;
          if(k<koly2)
            {
              sum=0; // Верхний квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j,kk,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j,kk,0,koly1,4)]=0; flagn1=0;}
              else {l=2; k=kk-1; break;}
            }
          else flagn1=0;
          if(j>0)
            {
              if(k<koly2)
                {
                  sum=0; // Левый верхний квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j-1,kk,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j-1,kk,1,koly1,4)]=0; flagn2=0;}
                  else {k=kk; l=3; break;}
                }
              else flagn2=0;
              sum=0; // Правый квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j-1,k,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j-1,k,2,koly1,4)]=0; flagn3=0;}
              else {k++; j--; l=0; break;}
            }
          else {flagn2=0;flagn3=0;}
        }
      else if(j==0) flag1=0;
      if(!(flagn1||flagn2||flagn3)) flag1=0;
      break;
    default : break;
    }
  if(flag1==0) return 0; // Некуда идти из начальной точки
  switch(l) {
    case 0: k--;
      l=2;
      iso[num*2]=argf[Num3_Elem(j,k,l,koly1,4)];
      iso[num*2+1]=nety[k+1];
      break;
    case 1: j++;
      l=3;
      iso[num*2]=netx[j];
      iso[num*2+1]=argf[Num3_Elem(j,k,l,koly1,4)];
      break;
    case 2: k++;
      l=0;
      iso[num*2]=argf[Num3_Elem(j,k,l,koly1,4)];
      iso[num*2+1]=nety[k];
      break;
    case 3: j--;
      l=1;
      iso[num*2]=netx[j+1];
      iso[num*2+1]=argf[Num3_Elem(j,k,l,koly1,4)];
      break;
    default : break;
    }
  indf[Num3_Elem(j,k,l,koly1,4)]=0;
  num++;
  // Ищем следующую точку в этом квадрате (справа, слева, напротив, если смотреть
  //         внутрь квадрата)
  for(;;) {
      for(i=0; i<3; i++) {
          l1=l+l_Obhod[i];
          if(l1>3) l1-=4; else if(l1==-1) l1=3;
          if(indf[Num3_Elem(j,k,l1,koly1,4)]==1) break;
        }
      if(i==3) {               //Если точка не найдена
          if(flag==0){
              break;//Если изолиния построена до конца
            }
          else { // Меняем начало и конец изолинии
              num1=num/2;
              for(i=0; i<num1; i++) {
                  i1=i*2;
                  i2=(num-1-i)*2;
                  vs=iso[i1]; vs1=iso[i1+1];
                  iso[i1]=iso[i2]; iso[i1+1]=iso[i2+1];
                  iso[i2]=vs; iso[i2+1]=vs1;
                }
              flag=0;
              j=j0; k=k0; l=l0;
              indf[Num3_Elem(j,k,l,koly1,4)]=0;
              continue;
            }
        }
      // Если точка найдена
      l=l1;
      switch(l) {
        case 0: iso[num*2]=argf[Num3_Elem(j,k,l,koly1,4)];
          iso[num*2+1]=nety[k];
          break;
        case 1: iso[num*2]=netx[j+1];
          iso[num*2+1]=argf[Num3_Elem(j,k,l,koly1,4)];
          break;
        case 2: iso[num*2]=argf[Num3_Elem(j,k,l,koly1,4)];
          iso[num*2+1]=nety[k+1];
          break;
        case 3: iso[num*2]=netx[j];
          iso[num*2+1]=argf[Num3_Elem(j,k,l,koly1,4)];
          break;
        default : break;
        }
      indf[Num3_Elem(j,k,l,koly1,4)]=0;
      num++; if(num==Kol_Num) return num; // Получено предельное количество точек
      if((j==j0)&&(k==k0)&&(l==l0)) break; // Изолиния замкнулась
      // Проверяем, есть ли куда идти
      flagn1=1;
      flagn2=1;
      flagn3=1;
      switch(l) {
        case 0:
          kk=k-1;
          if(isEqualHARD(netx[j],argf[Num3_Elem(j,k,l,koly1,4)])) {
              if(k>0)
                {
                  sum=0; // Нижний квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j,kk,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j,kk,3,koly1,4)]=0; flagn1=0;}
                  else {j--; k=kk; l=1; break;}
                }
              else flagn1=0;
              if(j>0)
                {
                  if(k>0)
                    {
                      sum=0; // Левый нижний квадрат
                      for(i=0; i<4; i++) sum+=indf[Num3_Elem(j-1,kk,i,koly1,4)];
                      if((sum==0)||(sum%2)) {indf[Num3_Elem(j-1,kk,2,koly1,4)]=0; flagn2=0;}
                      else {j--; k=kk+1; break;}
                    }
                  else flagn2=0;

                  sum=0; // Левый квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j-1,k,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j-1,k,1,koly1,4)]=0;  flagn3=0;}
                  else {l=3; break;}
                }
              else {flagn2=0; flagn3=0;}
            }
          else if(k==0) {flag1=0; break;}
          if(!(flagn1||flagn2||flagn3)) flag1=0;
          break;
        case 1:
          if(isEqualHARD(nety[k],argf[Num3_Elem(j,k,l,koly1,4)]))
            {
              kk=k-1;
              if(k>0)
                {
                  sum=0; // Нижний квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j,kk,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j,kk,2,koly1,4)]=0; flagn1=0;}
                  else {l=0; k=kk+1; break;}
                }
              else flagn1=0;
              if(j<kolx2)
                {
                  if(k>0)
                    {
                      sum=0; // Правый нижний квадрат
                      for(i=0; i<4; i++) sum+=indf[Num3_Elem(j+1,kk,i,koly1,4)];
                      if((sum==0)||(sum%2)) {indf[Num3_Elem(j+1,kk,3,koly1,4)]=0; flagn2=0;}
                      else {k=kk; break;}
                    }
                  else flagn2=0;

                  sum=0; // Правый квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j+1,k,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j+1,k,0,koly1,4)]=0; flagn3=0;}
                  else {k--; j++; l=2; break;}
                }
              else {flagn2=0; flagn3=0;}
            }
          else if(j==kolx2) {flag1=0; break;}
          if(!(flagn1||flagn2||flagn3)) flag1=0;
          break;
        case 2:
          kk=k+1;
          if(isEqualHARD(netx[j+1],argf[Num3_Elem(j,k,l,koly1,4)]))
            {
              if(k<koly2)
                {
                  sum=0; // Верхний квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j,kk,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j,kk,1,koly1,4)]=0; flagn1=0;}
                  else {j++; k=kk; l=3; break;}
                }
              else flagn1=0;
              if(j<kolx2)
                {
                  if(k<koly2)
                    {
                      sum=0; // Правый верхний квадрат
                      for(i=0; i<4; i++) sum+=indf[Num3_Elem(j+1,kk,i,koly1,4)];
                      if((sum==0)||(sum%2)) {indf[Num3_Elem(j+1,kk,0,koly1,4)]=0; flagn2=0;}
                      else {j++;k=kk-1; break;}
                    }
                  else flagn2=0;

                  sum=0; // Правый квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j+1,k,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j+1,k,3,koly1,4)]=0; flagn3=0;}
                  else {l=1; break;}
                }
              else {flagn2=0;flagn3=0;}
            }
          else if(k==koly2) {flag1=0; break;}
          if(!(flagn1||flagn2||flagn3)) flag1=0;
          break;
        case 3:
          if(isEqualHARD(nety[k+1],argf[Num3_Elem(j,k,l,koly1,4)]))
            {
              kk=k+1;
              if(k<koly2)
                {
                  sum=0; // Верхний квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j,kk,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j,kk,0,koly1,4)]=0; flagn1=0;}
                  else {l=2; k=kk-1; break;}
                }
              else flagn1=0;
              if(j>0)
                {
                  if(k<koly2)
                    {
                      sum=0; // Левый верхний квадрат
                      for(i=0; i<4; i++) sum+=indf[Num3_Elem(j-1,kk,i,koly1,4)];
                      if((sum==0)||(sum%2)) {indf[Num3_Elem(j-1,kk,1,koly1,4)]=0; flagn2=0;}
                      else {k=kk; l=3; break;}
                    }
                  else flagn2=0;
                  sum=0; // Правый квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j-1,k,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j-1,k,2,koly1,4)]=0; flagn3=0;}
                  else {k++; j--; l=0; break;}
                }
              else {flagn2=0;flagn3=0;}
            }
          else if(j==0) flag1=0;
          if(!(flagn1||flagn2||flagn3)) flag1=0;
          break;
        default : break;
        }
      // Проверяем, не на границе ли области мы находимся
      if(flag1==0)
        // Находимся на границе области
        if(flag==0) break;//Если изолиния построена до конца
        else { // Меняем начало и конец изолинии
            num1=num/2;
            for(i=0; i<num1; i++) {
                i1=i*2;
                i2=(num-1-i)*2;
                vs=iso[i1]; vs1=iso[i1+1];
                iso[i1]=iso[i2]; iso[i1+1]=iso[i2+1];
                iso[i2]=vs; iso[i2+1]=vs1;
              }
            flag=0;
            j=j0; k=k0; l=l0;
            indf[Num3_Elem(j,k,l,koly1,4)]=0;
            flag1=1;
            continue;
          }
      // Находимся не на границе области
      else {
          // Переходим в соседний квадрат
          switch(l) {
            case 0: k--;
              l=2;
              break;
            case 1: j++;
              l=3;
              break;
            case 2: k++;
              l=0;
              break;
            case 3: j--;
              l=1;
              break;
            default : break;
            }
          indf[Num3_Elem(j,k,l,koly1,4)]=0;
        }
    }
  return num;
}

int TIsoLineData::One_Iso_Sphere(int j, int k, int l, const float *netx, const float *nety, char *indf,
                                 float *argf, float *iso,  int kolx,  int koly,
                                 int Kol_Num)
{
  int flag=1, flag1=1, flagn1=1, flagn2=1, flagn3=1, num=0, num1=0,
      kolx2=kolx-2, koly2=koly-2, koly1=koly-1, j0=j, k0=k, l0=l, l1 = 0, sum;
  int kk;
  float vs, vs1;
  int l_Obhod[3]={-1,1,2}, i, i1, i2;
  // Проверяем, есть ли куда идти
  switch(l) {
    case 0:
      kk=k-1;
      if(kk<0) kk=koly2;
      if(isEqualHARD(netx[j],argf[Num3_Elem(j,k,l,koly1,4)])) {
          sum=0; // Нижний квадрат
          for(i=0; i<4; i++) sum+=indf[Num3_Elem(j,kk,i,koly1,4)];
          if((sum==0)||(sum%2)) {indf[Num3_Elem(j,kk,3,koly1,4)]=0; flagn1=0;}
          else {j--; k=kk; l=1; break;}
          if(j>0) {   sum=0; // Левый нижний квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j-1,kk,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j-1,kk,2,koly1,4)]=0; flagn2=0;}
              else {j--; k=kk+1; break;}

              sum=0; // Левый квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j-1,k,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j-1,k,1,koly1,4)]=0;  flagn3=0;}
              else {l=3; break;}
            }
          else {flagn2=0; flagn3=0;}
        }
      else k=kk+1;
      if(!(flagn1||flagn2||flagn3)) flag1=0;
      break;
    case 1:
      if(isEqualHARD(nety[k],argf[Num3_Elem(j,k,l,koly1,4)])) {
          kk=k-1;
          if(kk<0) kk=koly2;
          sum=0; // Нижний квадрат
          for(i=0; i<4; i++) sum+=indf[Num3_Elem(j,kk,i,koly1,4)];
          if((sum==0)||(sum%2)) {indf[Num3_Elem(j,kk,2,koly1,4)]=0; flagn1=0;}
          else {l=0; k=kk+1; break;}
          if(j<kolx2) {sum=0; // Правый нижний квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j+1,kk,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j+1,kk,3,koly1,4)]=0; flagn2=0;}
              else {k=kk; break;}

              sum=0; // Правый квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j+1,k,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j+1,k,0,koly1,4)]=0; flagn3=0;}
              else {k--; j++; l=2; break;}
            }
          else {flagn2=0; flagn3=0;}
        }
      else if(j==kolx2) flag1=0;
      if(!(flagn1||flagn2||flagn3)) flag1=0;
      break;
    case 2:
      kk=k+1;
      if(kk>koly2) kk=0;
      if(isEqualHARD(netx[j+1],argf[Num3_Elem(j,k,l,koly1,4)])) {
          sum=0; // Верхний квадрат
          for(i=0; i<4; i++) sum+=indf[Num3_Elem(j,kk,i,koly1,4)];
          if((sum==0)||(sum%2)) {indf[Num3_Elem(j,kk,1,koly1,4)]=0; flagn1=0;}
          else {j++; k=kk; l=3; break;}
          if(j<kolx2) {
              sum=0; // Правый верхний квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j+1,kk,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j+1,kk,0,koly1,4)]=0; flagn2=0;}
              else {j++;k=kk-1; break;}

              sum=0; // Правый квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j+1,k,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j+1,k,3,koly1,4)]=0; flagn3=0;}
              else {l=1; break;}
            }
          else {flagn2=0;flagn3=0;}
        }
      else k=kk-1;
      if(!(flagn1||flagn2||flagn3)) flag1=0;
      break;
    case 3:
      if(isEqualHARD(nety[k+1],argf[Num3_Elem(j,k,l,koly1,4)])) {
          kk=k+1;
          if(kk>koly2) kk=0;
          sum=0; // Верхний квадрат
          for(i=0; i<4; i++) sum+=indf[Num3_Elem(j,kk,i,koly1,4)];
          if((sum==0)||(sum%2)) {indf[Num3_Elem(j,kk,0,koly1,4)]=0; flagn1=0;}
          else {l=2; k=kk-1; break;}
          if(j>0)    {sum=0; // Левый верхний квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j-1,kk,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j-1,kk,1,koly1,4)]=0; flagn2=0;}
              else {k=kk; l=3; break;}
              sum=0; // Правый квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j-1,k,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j-1,k,2,koly1,4)]=0; flagn3=0;}
              else {k++; j--; l=0; break;}
            }
          else {flagn2=0;flagn3=0;}
        }
      else if(j==0) flag1=0;
      if(!(flagn1||flagn2||flagn3)) flag1=0;
      break;
    default : break;
    }
  if(flag1==0) return 0; // Некуда идти из начальной точки
  switch(l) {
    case 0: k--;
      l=2;
      iso[num*2]=argf[Num3_Elem(j,k,l,koly1,4)];
      iso[num*2+1]=nety[k+1];
      break;
    case 1: j++;
      l=3;
      iso[num*2]=netx[j];
      iso[num*2+1]=argf[Num3_Elem(j,k,l,koly1,4)];
      break;
    case 2: k++;
      l=0;
      iso[num*2]=argf[Num3_Elem(j,k,l,koly1,4)];
      iso[num*2+1]=nety[k];
      break;
    case 3: j--;
      l=1;
      iso[num*2]=netx[j+1];
      iso[num*2+1]=argf[Num3_Elem(j,k,l,koly1,4)];
      break;
    default : break;
    }
  indf[Num3_Elem(j,k,l,koly1,4)]=0;
  num++;
  // Ищем следующую точку в этом квадрате (справа, слева, напротив, если смотреть
  //         внутрь квадрата)
  for(;;) {
      for(i=0; i<3; i++) {
          l1=l+l_Obhod[i];
          if(l1>3) l1-=4; else if(l1==-1) l1=3;
          if(indf[Num3_Elem(j,k,l1,koly1,4)]==1) break;
        }
      if(i==3)    {            //Если точка не найдена
          if(flag==0) {
              break;//Если изолиния построена до конца
            }
          else { // Меняем начало и конец изолинии
              num1=num/2;
              for(i=0; i<num1; i++) {
                  i1=i*2;
                  i2=(num-1-i)*2;
                  vs=iso[i1]; vs1=iso[i1+1];
                  iso[i1]=iso[i2]; iso[i1+1]=iso[i2+1];
                  iso[i2]=vs; iso[i2+1]=vs1;
                }
              flag=0;
              j=j0; k=k0; l=l0;
              indf[Num3_Elem(j,k,l,koly1,4)]=0;
              continue;
            }
        }
      // Если точка найдена
      l=l1;
      switch(l) {
        case 0: iso[num*2]=argf[Num3_Elem(j,k,l,koly1,4)];
          iso[num*2+1]=nety[k];
          break;
        case 1: iso[num*2]=netx[j+1];
          iso[num*2+1]=argf[Num3_Elem(j,k,l,koly1,4)];
          break;
        case 2: iso[num*2]=argf[Num3_Elem(j,k,l,koly1,4)];
          iso[num*2+1]=nety[k+1];
          break;
        case 3: iso[num*2]=netx[j];
          iso[num*2+1]=argf[Num3_Elem(j,k,l,koly1,4)];
          break;
        default : break;
        }
      indf[Num3_Elem(j,k,l,koly1,4)]=0;
      num++; if(num==Kol_Num) return num; // Получено предельное количество точек
      if((j==j0)&&(k==k0)&&(l==l0)) break; // Изолиния замкнулась
      // Проверяем, есть ли куда идти
      flagn1=1;
      flagn2=1;
      flagn3=1;
      switch(l) {
        case 0:
          kk=k-1;
          if(kk<0) kk=koly2;
          if(isEqualHARD(netx[j],argf[Num3_Elem(j,k,l,koly1,4)])) {
              sum=0; // Нижний квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j,kk,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j,kk,3,koly1,4)]=0; flagn1=0;}
              else {j--; k=kk; l=1; break;}
              if(j>0) {   sum=0; // Левый нижний квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j-1,kk,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j-1,kk,2,koly1,4)]=0; flagn2=0;}
                  else {j--; k=kk+1; break;}

                  sum=0; // Левый квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j-1,k,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j-1,k,1,koly1,4)]=0;  flagn3=0;}
                  else {l=3; break;}
                }
              else {flagn2=0; flagn3=0;}
            }
          else k=kk+1;
          if(!(flagn1||flagn2||flagn3)) flag1=0;
          break;
        case 1:
          if(isEqualHARD(nety[k],argf[Num3_Elem(j,k,l,koly1,4)])) {
              kk=k-1;
              if(kk<0) kk=koly2;
              sum=0; // Нижний квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j,kk,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j,kk,2,koly1,4)]=0; flagn1=0;}
              else {l=0; k=kk+1; break;}
              if(j<kolx2) {sum=0; // Правый нижний квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j+1,kk,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j+1,kk,3,koly1,4)]=0; flagn2=0;}
                  else {k=kk; break;}

                  sum=0; // Правый квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j+1,k,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j+1,k,0,koly1,4)]=0; flagn3=0;}
                  else {k--; j++; l=2; break;}
                }
              else {flagn2=0; flagn3=0;}
            }
          else if(j==kolx2) flag1=0;
          if(!(flagn1||flagn2||flagn3)) flag1=0;
          break;
        case 2:
          kk=k+1;
          if(kk>koly2) kk=0;
          if(isEqualHARD(netx[j+1],argf[Num3_Elem(j,k,l,koly1,4)])) {
              sum=0; // Верхний квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j,kk,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j,kk,1,koly1,4)]=0; flagn1=0;}
              else {j++; k=kk; l=3; break;}
              if(j<kolx2) {
                  sum=0; // Правый верхний квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j+1,kk,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j+1,kk,0,koly1,4)]=0; flagn2=0;}
                  else {j++;k=kk-1; break;}

                  sum=0; // Правый квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j+1,k,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j+1,k,3,koly1,4)]=0; flagn3=0;}
                  else {l=1; break;}
                }
              else {flagn2=0;flagn3=0;}
            }
          else k=kk-1;
          if(!(flagn1||flagn2||flagn3)) flag1=0;
          break;
        case 3:
          if(isEqualHARD(nety[k+1],argf[Num3_Elem(j,k,l,koly1,4)])) {
              kk=k+1;
              if(kk>koly2) kk=0;
              sum=0; // Верхний квадрат
              for(i=0; i<4; i++) sum+=indf[Num3_Elem(j,kk,i,koly1,4)];
              if((sum==0)||(sum%2)) {indf[Num3_Elem(j,kk,0,koly1,4)]=0; flagn1=0;}
              else {l=2; k=kk-1; break;}
              if(j>0)    {sum=0; // Левый верхний квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j-1,kk,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j-1,kk,1,koly1,4)]=0; flagn2=0;}
                  else {k=kk; l=3; break;}
                  sum=0; // Правый квадрат
                  for(i=0; i<4; i++) sum+=indf[Num3_Elem(j-1,k,i,koly1,4)];
                  if((sum==0)||(sum%2)) {indf[Num3_Elem(j-1,k,2,koly1,4)]=0; flagn3=0;}
                  else {k++; j--; l=0; break;}
                }
              else {flagn2=0;flagn3=0;}
            }
          else if(j==0) flag1=0;
          if(!(flagn1||flagn2||flagn3)) flag1=0;
          break;
        default : break;
        }
      // Проверяем, не на границе ли области мы находимся
      if(flag1==0)
        // Находимся на границе области
        if(flag==0) break;//Если изолиния построена до конца
        else { // Меняем начало и конец изолинии
            num1=num/2;
            for(i=0; i<num1; i++) {
                i1=i*2;
                i2=(num-1-i)*2;
                vs=iso[i1]; vs1=iso[i1+1];
                iso[i1]=iso[i2]; iso[i1+1]=iso[i2+1];
                iso[i2]=vs; iso[i2+1]=vs1;
              }
            flag=0;
            j=j0; k=k0; l=l0;
            indf[Num3_Elem(j,k,l,koly1,4)]=0;
            flag1=1;
            continue;
          }
      // Находимся не на границе области
      else {
          // Переходим в соседний квадрат
          switch(l) {
            case 0: k--;
              l=2;
              break;
            case 1: j++;
              l=3;
              break;
            case 2: k++;
              l=0;
              break;
            case 3: j--;
              l=1;
              break;
            default : break;
            }
          indf[Num3_Elem(j,k,l,koly1,4)]=0;
        }
    }
  return num;
}


//#pragma GCC diagnostic pop











