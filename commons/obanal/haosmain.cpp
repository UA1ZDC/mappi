
#include "haosmain.h"
#include <QDebug>

#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmisleading-indentation"


namespace obanal{


  int Geogr_Plos(double *a, int n, int Napr)
  {
   int i, i1;
   double r, f, x, y;
   double GrToRad=3.1415926/180, RadToGr=180./3.1415926;
   switch(Napr) {
         case 0: // Географические в декартовы
                 for(i=0; i<n; i++)
                             {
                     i1=i*2;
                     r=tan(0.5*(90.-a[i1])*GrToRad);
                     f=a[i1+1]*GrToRad;
                     a[i1]=r*cos(f);
                     a[i1+1]=r*sin(f);
                             }
                 break;
         case 1: // Декартовы в географические
                 for(i=0; i<n; i++)
                             {
                     i1=i*2;
                                     x=a[i1];
                                     y=a[i1+1];
                     r=sqrt(x*x+y*y);
                     a[i1]=90.-2.*atan(r)*RadToGr;
                     a[i1+1]=atan2(y,x)*RadToGr;
                     if(a[i1+1]>180.) a[i1+1]-=360.;
                             }
                 break;
         default : return 1;// Не понятно направление преобразования;
                }
   return 0;
  }

  int Preobr_Koord(double *a, parCub *p,  int n, double eps, int Napr)
  {
  int i,i1, n1;
  double x_max, y_max, x_min, y_min, xvs, yvs;

  switch(Napr)
  {
  case 0:
           n1=n-1;
           for(i=0; i<n1; i++)
           {
                   i1=i*2;
                   xvs=a[i1];
                   yvs=a[i1+1];
             for(int j=i+1; j<n; j++) {
                  int j1=j*2;
                  double r=(a[j1]-xvs)*(a[j1]-xvs)+(a[j1+1]-yvs)*(a[j1+1]-yvs);
                  if(sqrt(r)<eps) return 3; // Есть совпадающие точки
                                   }
           }
           x_max=a[0];
           y_max=a[1];
           x_min=a[0];
           y_min=a[1];
           for(i=1; i<n; i++)
           {
                   i1=i*2;
                   if(x_max<a[i1]) x_max=a[i1];
                   if(y_max<a[i1+1]) y_max=a[i1+1];
                   if(x_min>a[i1]) x_min=a[i1];
                   if(y_min>a[i1+1]) y_min=a[i1+1];
           }
           p->x0=x_min;
           p->y0=y_min;
           p->pX=x_max-x_min;
           p->pY=y_max-y_min;
           if((fabs(p->pX)<eps)||(fabs(p->pY)<eps)) return 2;// Область вырождена
           else
           {
                   p->Ed_pX=1./p->pX;
                   p->Ed_pY=1./p->pY;
                   for(i=0; i<n; i++)
                   {
                           i1=i*2;
               a[i1]=(a[i1]-p->x0)*p->Ed_pX;
               a[i1+1]=(a[i1+1]-p->y0)*p->Ed_pY;
                   }
           }
           break;
   case 1:
           for(i=0; i<n; i++)
           {
                   i1=i*2;
                   a[i1]=a[i1]*p->pX+p->x0;
                   a[i1+1]=a[i1+1]*p->pY+p->y0;
           }
           break;
   default: return 1;// Не понятно направление преобразования;
   }

   return 0;
  }
  int interpolHaosRazbien(const meteo::GeoData &src_data, TField *rezult){

    double eps = 1e-6;
    int ret_val=0;
    int kol_la = rezult->kolLa();
    int kol_fi = rezult->kolFi();

    int n = 0;
    for(const meteo::MData &md : src_data){
        if(1 != md.mask) continue;
        ++n;
      }
    double *fi = new double[n];
    double *la = new double[n];
    double *z = new double[n];

    int i = 0;
    for(const meteo::MData &md : src_data){
        if(1 != md.mask) continue;

        z[i] = md.data;
        //type_point.append(md.type_data);
        fi[i] = md.point.fiDeg();
        la[i] = md.point.laDeg();
        ++i;
      }

   if(5 > n )   return ERR_NODATA_OBANAL;


    ////////////////////IF<<<<<<PREDEL_RZB/////////////////////////////////

  int n1=n*2;



  int i1, i2, ii, j = 0, jj, k;

  int kol_rzb;
  double *net;
  int **kol=nullptr;
  int **kol_vs=nullptr;
  double ***xy_vs=nullptr;
  double ***coef_vs=nullptr;
  double *z_vs=nullptr;
  int max_kol=0;;
  ////////////////////IF<<<<<<PREDEL_RZB/////////////////////////////////
  int kol_rzb3000=4;
  double net3000[5]={-100.,0.4,0.6,0.8,100.};
  int kol3000[4][4];
  int kol_vs3000[3][3];
  double *xy_vs3000[3][3];
  double *coef_vs3000[3][3];
  ////////////////////IF>>>>>>PREDEL_RZB/////////////////////////////////
  int kol_rzb5000=7;
  double net5000[8]={-100.,0.25,0.4,0.5,0.6,0.7,0.8,100.};
  int kol5000[7][7];
  int kol_vs5000[6][6];
  double *xy_vs5000[6][6];
  double *coef_vs5000[6][6];
  /////////////////////////////////////////////////////
  double *xy = nullptr;
  double *w  = nullptr;

  double x, y, alfa;
  double dx, dy, ddx, ddy;
  double vs1 = 0.0, vs2 = 0.0, vs3 = 0.0, vs4 = 0.0, vs5 = 0.0, vs6 = 0.0;
  bool   flag1, flag2, flag3, flag4, flag5, flag6;
  double flxy[2];
  parCub p;

  if(n<5) return 125;

  if( n < 3015 )
    {
     kol_rzb=kol_rzb3000;
     net=net3000;
     kol=new int* [kol_rzb];
     for(i=0;i<kol_rzb;i++) kol[i]=kol3000[i];
     kol_vs=new int* [kol_rzb-1];
     xy_vs=new double** [kol_rzb-1];
     coef_vs=new double** [kol_rzb-1];
     for(i=0;i<kol_rzb-1;i++)
      {
       kol_vs[i]=kol_vs3000[i];
       xy_vs[i]=xy_vs3000[i];
       coef_vs[i]=coef_vs3000[i];
      }
    }
     else
      {
       kol_rzb=kol_rzb5000;
       net=net5000;
       kol=new int* [kol_rzb];
       for(i=0;i<kol_rzb;i++) kol[i]=kol5000[i];
       kol_vs=new int* [kol_rzb-1];
       xy_vs=new double** [kol_rzb-1];
       coef_vs=new double** [kol_rzb-1];
       for(i=0;i<kol_rzb-1;i++)
       {
        kol_vs[i]=kol_vs5000[i];
        xy_vs[i]=xy_vs5000[i];
        coef_vs[i]=coef_vs5000[i];
       }
      }

   xy= new double[n1];
   w= new double[n];
   z_vs= new double[n];

   alfa=0.;
   for(i=0; i<n; i++)
    {
     w[i]=1.;
     xy[i*2]=fi[i];
     xy[i*2+1]=la[i];
    }
   Geogr_Plos(xy,n,0);

   ret_val=Preobr_Koord(xy,&p,n,eps,0);
   if(ret_val) goto m1;

  for(i1=0; i1<kol_rzb; i1++)
   for(j=0; j<kol_rzb; j++)
    {
     kol[i1][j]=0;
     for(i=0; i<n; i++)
      if(net[i1]<=xy[i*2] && xy[i*2]<net[i1+1] && net[j]<=xy[i*2+1] && xy[i*2+1]<net[j+1]) kol[i1][j]++;
    }
  for(i1=0; i1<kol_rzb-1; i1++)
   for(j=0; j<kol_rzb-1; j++)
    {
     kol_vs[i1][j]=kol[i1][j]+kol[i1+1][j]+kol[i1][j+1]+kol[i1+1][j+1];
     if(max_kol<kol_vs[i1][j]) max_kol=kol_vs[i1][j];
    }

  for(i1=0; i1<kol_rzb-1; i1++)
   for(j=0; j<kol_rzb-1; j++)
    {
     xy_vs[i1][j]=nullptr;
     coef_vs[i1][j]=nullptr;
    }
  for(i1=0; i1<kol_rzb-1; i1++)
   for(j=0; j<kol_rzb-1; j++)
    {
     xy_vs[i1][j]= new double[kol_vs[i1][j]*2];
     coef_vs[i1][j]= new double[kol_vs[i1][j]+3];
    }

  for(i1=0; i1<kol_rzb-1; i1++)
   for(j=0; j<kol_rzb-1; j++)
    {
     k=0;
     for(i=0; i<n; i++)
      if(net[i1]<=xy[i*2] && xy[i*2]<net[i1+2] && net[j]<=xy[i*2+1] && xy[i*2+1]<net[j+2])
       {
        xy_vs[i1][j][k*2]=xy[i*2];
        xy_vs[i1][j][k*2+1]=xy[i*2+1];
        z_vs[k]=z[i];
        k++;
       }
     if(kol_vs[i1][j]>=4) ret_val=Coeff_NoIterat(xy_vs[i1][j],z_vs,w,coef_vs[i1][j],kol_vs[i1][j],alfa,eps);
     if(ret_val) goto m1;
    }

   for(i=0;i<kol_fi;i++)
    {
     i2=i*kol_la;
     for(k=0;k<kol_la;k++)
      {

       //flxy[0]= rezult->netFi(i);
       //flxy[1]= rezult->netLa(k);
       geogrToPlos(rezult->netFi(i), rezult->netLa(k), flxy);

       //Geogr_Plos(flxy,1,0);
       x=(flxy[0]-p.x0)*p.Ed_pX;
       y=(flxy[1]-p.y0)*p.Ed_pY;
       for(i1=0; i1<kol_rzb; i1++)
        for(j=0; j<kol_rzb; j++)
         if(net[i1]<=x && x<net[i1+1] && net[j]<=y && y<net[j+1]) goto m2;;
  m2:  if(i1==kol_rzb || j==kol_rzb)
        {
         fprintf(stderr,"пЫЙВЛБ!!!\n");
         continue;
        }
       ii=(i1==0)<<3 | (i1==(kol_rzb-1))<<2 | (j==0)<<1 | (j==(kol_rzb-1));
       if(ii)
        {
          switch(ii)
           {
            case 10:
                    if(kol_vs[i1][j]>=4)
                     {
                      double arez =ValPoint_Int_No_Preobr(xy_vs[i1][j],x,y,coef_vs[i1][j],kol_vs[i1][j]);
                      rezult->setData(i2+k,arez,true );
                      //mask[i2+k]=true;
                     }
                   break;
            case 9:
                    if(kol_vs[i1][j-1]>=4)
                     {
                      double arez = ValPoint_Int_No_Preobr(xy_vs[i1][j-1],x,y,coef_vs[i1][j-1],kol_vs[i1][j-1]);
                      rezult->setData(i2+k,arez,true );
                     }
                   break;
            case 6:
                    if(kol_vs[i1-1][j]>=4)
                     {
                      double arez = ValPoint_Int_No_Preobr(xy_vs[i1-1][j],x,y,coef_vs[i1-1][j],kol_vs[i1-1][j]);
                      rezult->setData(i2+k,arez,true );
                     }
                   break;
            case 5:
                    if(kol_vs[i1-1][j-1]>=4)
                     {
                      double arez = ValPoint_Int_No_Preobr(xy_vs[i1-1][j-1],x,y,coef_vs[i1-1][j-1],kol_vs[i1-1][j-1]);
                      rezult->setData(i2+k,arez,true );
                     }
                   break;
            case 4:
            case 8:
                   if(i1==0) ii=i1;
                      else ii=i1-1;
                   flag1=true; flag2=true;
                   dy=net[j+1]-net[j];
                   ddy=y-net[j];
                   if(kol_vs[ii][j-1]<4) flag1=false;
                     else vs1=ValPoint_Int_No_Preobr(xy_vs[ii][j-1],x,y,coef_vs[ii][j-1],kol_vs[ii][j-1]);
                   if(kol_vs[ii][j]<4) flag2=false;
                    else vs2=ValPoint_Int_No_Preobr(xy_vs[ii][j],x,y,coef_vs[ii][j],kol_vs[ii][j]);
                   if(flag1&&flag2) {
                       rezult->setData(i2+k,fun_skl_xy(ddy,dy,vs1,vs2),true );

                     }
                    else if(flag1) {
                       rezult->setData(i2+k,vs1,true );
                     }
                      else if(flag2) {
                       rezult->setData(i2+k,vs2,true );
                     }
                   break;
            case 2:
            case 1:
                   if(j==0) jj=j;
                      else jj=j-1;
                   flag1=true; flag2=true;
                   dx=net[i1+1]-net[i1];
                   ddx=x-net[i1];
                   if(kol_vs[i1-1][jj]<4) flag1=false;
                     else vs1=ValPoint_Int_No_Preobr(xy_vs[i1-1][jj],x,y,coef_vs[i1-1][jj],kol_vs[i1-1][jj]);
                   if(kol_vs[i1][jj]<4) flag2=false;
                    else vs2=ValPoint_Int_No_Preobr(xy_vs[i1][jj],x,y,coef_vs[i1][jj],kol_vs[i1][jj]);
                   if(flag1&&flag2) {
                       rezult->setData(i2+k,fun_skl_xy(ddx,dx,vs1,vs2),true );
                     }
                    else if(flag1) {
                       rezult->setData(i2+k,vs1,true );
                     }
                      else if(flag2) {
                       rezult->setData(i2+k,vs2,true );
                     }
                   break;
           }
        }
        else
         {
          flag1=true; flag2=true; flag3=true; flag4=true; flag5=true; flag6=true;
          dx=net[i1+1]-net[i1];
          dy=net[j+1]-net[j];
          ddx=x-net[i1];
          ddy=y-net[j];
          if(kol_vs[i1-1][j-1]<4) flag1=false;
           else vs1=ValPoint_Int_No_Preobr(xy_vs[i1-1][j-1],x,y,coef_vs[i1-1][j-1],kol_vs[i1-1][j-1]);
          if(kol_vs[i1][j-1]<4) flag2=false;
           else vs2=ValPoint_Int_No_Preobr(xy_vs[i1][j-1],x,y,coef_vs[i1][j-1],kol_vs[i1][j-1]);
          if(kol_vs[i1][j]<4) flag3=false;
           else vs3=ValPoint_Int_No_Preobr(xy_vs[i1][j],x,y,coef_vs[i1][j],kol_vs[i1][j]);
          if(kol_vs[i1-1][j]<4) flag4=false;
           else vs4=ValPoint_Int_No_Preobr(xy_vs[i1-1][j],x,y,coef_vs[i1-1][j],kol_vs[i1-1][j]);

          if(flag1&&flag2) vs5=fun_skl_xy(ddx,dx,vs1,vs2);
           else if(flag1) vs5=vs1;
                 else if(flag2) vs5=vs2;
                       else flag5=false;
          if(flag4&&flag3) vs6=fun_skl_xy(ddx,dx,vs4,vs3);
           else if(flag4) vs6=vs4;
                 else if(flag3) vs6=vs3;
                       else flag6=false;

          if(!(flag5||flag6)) continue;

          if(flag5&&flag6) {
              rezult->setData(i2+k,fun_skl_xy(ddy,dy,vs5,vs6),true );
            }
           else if(flag5){
              rezult->setData(i2+k,vs5,true );
            }
                 else {
              if(flag6){
                  rezult->setData(i2+k,vs6,true );
                }
            }
         }
      }
          }

  m1:
  if(kol!=nullptr) {delete []kol; kol=nullptr;}
  if(kol_vs!=nullptr) {delete []kol_vs; kol_vs=nullptr;}
  if(xy!=nullptr) {delete []xy; xy=nullptr;}
  if(w!=nullptr) {delete []w; w=nullptr;}
  if(z_vs!=nullptr) {delete []z_vs; z_vs=nullptr;}
  for(i1=0; i1<kol_rzb-1; i1++)
   for(j=0; j<kol_rzb-1; j++)
    {
     if(xy_vs[i1][j]!=nullptr) {delete []xy_vs[i1][j]; xy_vs[i1][j]=nullptr;}
     if(coef_vs[i1][j]!=nullptr) {delete []coef_vs[i1][j]; coef_vs[i1][j]=nullptr;}
    }
  if(xy_vs!=nullptr) {delete []xy_vs; xy_vs=nullptr;}
  if(coef_vs!=nullptr) {delete []coef_vs; coef_vs=nullptr;}
  if(fi!=nullptr) {delete []fi; fi=nullptr;}
  if(la!=nullptr) {delete []la; la=nullptr;}
  if(z!=nullptr) {delete []z; z=nullptr;}

  return ret_val;
  }


  int interpolHaosOnePoint(int n, double *x, double *y, double *z, double xx,double yy, float *rezult)
  {
    int ret_val=0;
    double alfa=0.;
    parCub p;
    //    system("date");
    if(4 > n )   return ERR_NODATA_OBANAL;

    alfa=0.;
    QVector<double> fun;
    //QVector<QPointF> xy;

    int n1=n*2;
    double *xy= new double[n1];
    //    meteo::GeoData::iterator it;
    for (int i=0; i < n; ++i){
        fun.append(z[i]);
        xy[i*2]= x[i];
        xy[i*2+1] =y[i];
      }
    QVector<double> coef(n+3);
    QVector<double> w(n);
    w.fill(1.);

    ret_val = preobrKoordToPlos(xy,n,&p);

    //ret_val = preobrKoordToPlos(&xy,&p);
    if(ret_val != ERR_NOERR) {
        delete []xy;
        return ret_val;
      }

    alfa=0.;


    ret_val=Coeff_NoIterat(xy,fun.data(),w.data(),coef.data(),n,alfa,0);

    if(ret_val != ERR_NOERR) return ret_val;
    double net_xy[2];
    net_xy[0] = xx;
    net_xy[1] = yy;
    *rezult = valPointInt(xy,n, net_xy, coef, p) ;
    delete []xy;
    return 0;
  }

  int interpolHaos(const meteo::GeoData &src_data, TField *rezult)
  {
    int ret_val=0;
    float alfa=0.;
    parCub p;

    alfa=0.;
    QVector<double> fun;
    QVector<meteo::GeoPoint> fl;
    QVector<meteo::typeGeoData> type_point;
    QVector<double> w;

    // meteo::GeoData tmp_data = src_data;
    //   const meteo::GeoData::iterator &it = tmp_data.begin();
    for(const meteo::MData &md : src_data){
        if(1 != md.mask) continue;
        fun.append(md.data);
        type_point.append(md.type_data);
        fl.append(md.point);
        w.append(md.w);
      }
    int n = fun.size();
    if(5 > n ) return ERR_NODATA_OBANAL;

    QVector<double> coef(n+3);

    int n1=n*2;
    double *xy= new double[n1];
    geogrToPlos(fl,xy);

    ret_val = preobrKoordToPlos(xy,n,&p);

    if ( ret_val != ERR_NOERR ) {
      delete[] xy;
      return ret_val;
    }

    alfa=0.;

    ret_val=Coeff_NoIterat(xy,fun.data(),w.data(),coef.data(),n,alfa,0);
    if(ret_val != ERR_NOERR) {
      delete[] xy;
      return ret_val;
    }

    int kol_la = rezult->kolLa();
    int kol_fi = rezult->kolFi();
    for(int i = 0; i < kol_fi; ++i)
      {
        int i1=i*kol_la;
        for(int j = 0; j < kol_la; ++j)
          {
            if(true != rezult->getNetMask(i,j))
              continue;
            double xy1[2];
            geogrToPlos(rezult->netFi(i), rezult->netLa(j), xy1);
            float arez = valPointInt(xy,n, xy1, coef, p) ;
            rezult->setData(i1+j,arez,true );
          }
      }

    delete[] xy;
    return ret_val;
  }

  int preobrKoordToPlos(double * xy,int n, parCub *p )
  {
    int  j=0, n1=0;
    double x_max=0., y_max=0., x_min=0., y_min=0.;
    double r = 0.;
    n1=n-1;

    int i =0;
    for(i = 0;  i < n1; ++i)
      {
        double xvs = xy[i*2]; //fi
        double yvs = xy[i*2+1]; //la
        for(j=i+1; j<n; ++j) {
            double x1vs = xy[j*2]; //fi
            double y1vs = xy[j*2+1]; //la

            r=(x1vs - xvs)*(x1vs - xvs)
                +(y1vs - yvs)*(y1vs - yvs);
            //if(r < eps2 ) {
            if(MnMath::isZero(r) ) {
                std::cout << " - "<<r<< std::endl;
                return 3;  //
              }
          }
      }
    x_max = xy[0];
    y_max = xy[1];
    x_min = xy[0];
    y_min = xy[1];
    for(i=1; i<n; ++i)
      {
        double xvs = xy[i*2]; //fi
        double yvs = xy[i*2+1]; //la
        if(x_max < xvs) x_max = xvs;
        if(y_max < yvs) y_max = yvs;
        if(x_min > xvs) x_min = xvs;
        if(y_min > yvs) y_min = yvs;
      }

    p->x0=x_min;
    p->y0=y_min;
    p->pX=x_max-x_min;
    p->pY=y_max-y_min;
    if(MnMath::isZero(p->pX)|| MnMath::isZero(p->pY)) {
        std::cout << "---"<< std::endl;
        return 2;//
      }
    p->Ed_pX=1./p->pX;
    p->Ed_pY=1./p->pY;
    // n*=2;
    for(i=0; i<n; ++i)
      {
        xy[i*2] =   (xy[i*2] - p->x0)*p->Ed_pX;
        xy[i*2+1] = (xy[i*2+1] - p->y0)*p->Ed_pY;
      }
    return ERR_NOERR;
  }


  void geogrToPlos(const QVector<meteo::GeoPoint> &fl, double *xy){
    int n = fl.size();
    for (int i = 0; i< n ; ++i){
        geogrToPlos(fl.at(i), &xy[i*2]);
      }
  }

  inline void geogrToPlos(const meteo::GeoPoint &fl, double *xy)
  {
    double r= tanf( 0.5* (M_PI_2 -  fl.fi() ));
    double f =  fl.la();
    xy[0]= r * cosf( f );
    xy[1]= r * sinf( f );
  }

  inline void geogrToPlos(float fi, float la, double *xy)
  {
    double r = tanf( 0.5f* (M_PI_2f -  fi ));
    xy[0]= r * cosf( la );
    xy[1]= r * sinf( la );
  }

  float valPointInt(double *xy, int n, double *netxy, const QVector<double> &coef)
  {
    double vs = 0.;
    double r = 0., xx=0.,yy=0.;
    double x = netxy[0]  ;
    double y = netxy[1] ;

    vs=coef.at(n) + x*coef.at(n+1)+y*coef.at(n+2);
    for(int i=0; i<n; ++i){
      xx= x - xy[i*2];
      yy= y - xy[i*2+1];
      r=xx*xx+yy*yy;
      //if(r>eps) vs+=coef[i]*0.5*r*log(r);
      if(false == MnMath::isZero(r)) vs+=coef[i]*0.5*r*logf(r);
    }
    return vs;
  }

  float valPointInt(double *xy, int n, double *netxy, const QVector<double> &coef,   const parCub &p)
  {
    double vs = 0.;
    double r = 0., xx=0.,yy=0.;
    double x = (netxy[0] - p.x0) * p.Ed_pX;
    double y = (netxy[1] - p.y0) * p.Ed_pY;

    vs=coef.at(n) + x*coef.at(n+1)+y*coef.at(n+2);
    for(int i=0; i<n; ++i){
      xx= x - xy[i*2];
      yy= y - xy[i*2+1];
      r=xx*xx+yy*yy;
      //if(r>eps) vs+=coef[i]*0.5*r*log(r);
      if(false == MnMath::isZero(r)) vs+=coef[i]*0.5*r*logf(r);
    }
    return vs;
  }

  int Coeff_NoIterat(double *x, double *z, double *w, double *d, int n,
                     double alfa, int kolw)
  {
    double c[3], left[3],  dvs[6], *matr_d = nullptr, *matr_u = nullptr,
        *matr_h = nullptr, *matr_b = nullptr,
        *matr_g = nullptr, *matr_l = nullptr,
        *h = nullptr, *zvs = nullptr, *winv = nullptr,
        *leftvs = nullptr;

    int i, i1, i2, j, nvs, n1, k, k1,ttt;
    int retval = 0;

    if(n<4) return 2;
    k=4;
    k1=k-1;
    n1=n-k1;
    nvs=n1*(n1+1)/2;

    //***************************************************************************
    matr_u=new double [k1*n1];
    Basis(x,matr_u,z,w,n);

    matr_g=new double [n*(n+1)/2];
    for(i=0; i<n; i++)
      {
        i1=i*2;
        for(j=i; j<n; j++)
          {
            i2=j*2;
            if(i<n-kolw-kolw && j<n-kolw-kolw){
                matr_g[Num_Elem(i,j)]=Fun_Grin(x[i1],x[i1+1],x[i2],x[i2+1]);

                //                   qDebug("%d\t%9.9f",Num_Elem(i,j),matr_g[Num_Elem(i,j)] );
              }
            if(i>=n-kolw-kolw && j>=n-kolw-kolw && i<n-kolw && j<n-kolw)
              matr_g[Num_Elem(i,j)]=Fun_Grin1x(x[i1],x[i1+1],x[i2],x[i2+1]);
            if(i>=n-kolw && j>=n-kolw && i<n && j<n)
              matr_g[Num_Elem(i,j)]=Fun_Grin1y(x[i1],x[i1+1],x[i2],x[i2+1]);
            //     qDebug()<<i<<j<< x[i1]<<x[i1+1]<<x[i2]<<x[i2+1];
          }

      }
    winv=new double [n];
    if(Diag_Inv(w,winv,n)) {retval =3; goto m1;};

    matr_d=new double [k1*n];

    for(j=0; j<n; j++) matr_d[j]=winv[j];
    for(i=0; i<(k1-1); i++)
      {
        i1=(i+1)*n;
        for(j=0; j<n; j++) matr_d[i1+j]=x[i+j*2]*winv[j];
      }
    AnyAnyT_Prod(matr_d,dvs,k1,n);
    if(Fac_Chol(dvs,k1)) {retval =4; goto m1;};
    zvs=new double [n1];
    BasAny_Prod(matr_u,z,zvs,n1,1,k);
    matr_h=new double [nvs];
    if(Matr_H(matr_u,matr_g,matr_h,n1,k)) {retval =1; goto m1; };
    //****************************************************************************
    //     qDebug()<<"***************************************************"<<n1<<k;
    h=new double[n1];
    leftvs=new double [n];
    if(MnMath::isZero(alfa)==0.)
      {
        if((ttt=Fac_Chol(matr_h,n1))) {retval =4;  fprintf(stderr,"Error fac_chol11=%d\n",ttt);  goto m1;};
        Resh_Chol(matr_h,zvs,h,n1);
        delete[] zvs;zvs=nullptr;
        delete[] matr_h;matr_h=nullptr;

      }
    else
      {
        matr_l=new double [nvs];
        matr_b=new double [k*n1];

        if(Matr_B(matr_u,w,matr_b,n1,k))    {retval =1; goto m1;};
        for(i=0; i<nvs; i++)
          matr_l[i]=matr_h[i];
        delete[] matr_h;matr_h=nullptr;
        for(i=0; i<n1; i++)
          for(j=0;j<k; j++)
            {
              i1=i+j;
              if(i1<n1)
                {
                  i1=Num_Elem(i,i1);
                  matr_l[i1]+=(alfa*matr_b[i*k+j]);
                }
            }

        if(Fac_Chol(matr_l,n1)) {retval =4; goto m1;};
        Resh_Chol(matr_l,zvs,h,n1);
        delete[] zvs;zvs=nullptr;
        SymLVec_Prod(matr_b,h,leftvs,n1,k1);
      }
    BasTAny_Prod(matr_u,h,d,n1,1,k);
    delete[] h; h = nullptr;
    delete[] matr_u;matr_u=nullptr;

    Diag_Prod(w,w,leftvs,n);
    SymDiag_Sum(matr_g,leftvs,matr_g,alfa,n);
    SymVec_Prod(matr_g,d,leftvs,n);
    delete[] matr_g;matr_g=nullptr;
    Vec_Dif(leftvs,z,n);
    for(i=0; i<n; i++) leftvs[i]=-leftvs[i]*winv[i];
    delete[] winv;winv=nullptr;
    Any_Prod(matr_d,leftvs,left,k1,n,1);
    delete[] leftvs;leftvs=nullptr;
    delete[] matr_d;matr_d=nullptr;
    Resh_Chol(dvs,left,c,k1);
    for(i=0; i<k1; i++) d[n+i]=c[i];

m1:
    delete[] matr_d;matr_d=nullptr;
    delete[] matr_u;matr_u=nullptr;
    delete[] matr_h;matr_h=nullptr;
    delete[] matr_l;matr_l=nullptr;
    delete[] matr_b;matr_b=nullptr;
    delete[] matr_g;matr_g=nullptr;
    delete[] h; h = nullptr;
    delete[] zvs;zvs=nullptr;
    delete[] winv;winv=nullptr;
    delete[] leftvs;leftvs=nullptr;
    return retval;
  }


  double fun_skl_xy(double ddx, double dx, double a, double b)
  {
    double t=0., f=0.;

    t=ddx/dx;
    f=1.-(3.-2*t)*t*t;
    return a*f+(1.-f)*b;
  }


  double ValPoint_Int_No_Preobr(double *xy, double x, double y, double *d, int n)
  {
    double vs;
    double r, xx,yy;
    int i, i1;
    vs=d[n]+x*d[n+1]+y*d[n+2];
    for(i=0; i<n; i++) {
        i1=i<<1;
        xx=x-xy[i1];
        i1++;
        yy=y-xy[i1];
        r=xx*xx+yy*yy;

        //      if(r>eps) vs+=d[i]*0.5*r*log(r);
        if(false == MnMath::isZero(r)) vs+=d[i]*0.5*r*logf(r);
      }
    return vs;
  }

  /////////////////////////////////////////////////////////////////////////////

  int kolInKvadrat(const QMap<float, float> &xi,
                   float x0,float y0,float x1,float y1 ){
    QMap<float, float>::const_iterator itx = xi.begin();
    QMap<float, float>::const_iterator itxe = xi.end();
    int count = 0;
    while(itx != itxe ){
        if( itx.key() >= x0 && itx.key() < x1 && itx.value() >=y0 && itx.value() < y1){
            ++count;
          }
        ++itx;
      }

    return count;
  }

  int searchXY(const QMap<float, float> &xi,
               float x0,float y0,float *x1,float *y1, int numppr, float step)
  {
    int kol_rzb = static_cast<int>(1./step);
    int count =0;
    for(int j = 0; j < kol_rzb; ++j){
        *x1 = j*step;
        *y1 = j*step;
        count = kolInKvadrat(xi, x0 ,y0 , *x1 ,*y1 );
        //   debug_log <<  *x0 <<*y0 <<*x1 << *y1<<count;
        if(numppr < count ){
            break;
          }
      }
    return count;
  }

  int searchX(const QMap<float, float> &xi,
              float x0,float y0, float *x1, float y1, int numppr, float step)
  {
    int kol_rzb = static_cast<int>(1./step);
    int count =0;
    for(int j = 0; j < kol_rzb; ++j){
        *x1 = j*step;
        count = kolInKvadrat(xi, x0 ,y0 , *x1 ,y1 );
        //   debug_log <<  *x0 <<*y0 <<*x1 << *y1<<count;
        if(numppr < count ){
            break;
          }
      }
    return count;
  }

  int searchY(const QMap<float, float> &xi,
              float x0,float y0,float x1, float *y1, int numppr, float step)
  {
    int kol_rzb = static_cast<int>(1./step);
    int count =0;
    for(int j = 0; j < kol_rzb; ++j){
        *y1 = j*step;
        count = kolInKvadrat(xi, x0 ,y0 , x1 ,*y1 );
        if(numppr < count ){
            break;
          }
      }
    return count;
  }

  bool getPointXY(const QVector<meteo::GeoPoint>& points, int numppr,
                  QVector<float> * rx,QVector<float> * ry, int *min, int *max,QVector<int> *kol){

    int count_points = points.size();
    if(count_points <= numppr) return true;
    QMap<float, float> po_fi;
    QVector<meteo::GeoPoint>::const_iterator it = points.begin();
    while(it != points.end()){
        po_fi.insertMulti(it->fi(),it->la());
        ++it;
      }
    float step = 0.0001;
    QVector <float> x(1,-100);
    QVector <float> y(1,-100);
    float x_,y_;
    int count = searchXY(po_fi, x[0] ,y[0] , &x_ , &y_, numppr, step );
    x.append(x_);
    y.append(y_);
    bool shag_y=true, shag_x=true;

    do{
        count= kolInKvadrat(po_fi, x[x.size()-2], y[y.size()-1] , x[x.size()-1], 1 );
        //         debug_log << "left on  vertical " << count;
        if(count <2*numppr ) {
            shag_y = false;
          }
        count= kolInKvadrat(po_fi, x[x.size()-1], y[y.size()-2],  1, y[y.size()-1] );
        //         debug_log << "left on  horisontal" << count;
        if(count <2*numppr ) {
            shag_x = false;
          }
        count= kolInKvadrat(po_fi, x[x.size()-1], y[y.size()-1],  1, 1 );
        //         debug_log << "left in other " << count;
        if(count <2*numppr ) break;
        if(shag_y){
            count = searchY(po_fi, x[x.size()-2] ,y[y.size()-1] , x[x.size()-1] , &y_, numppr, step );
            y.append(y_);
            //           debug_log<<"y "<<y.size()<< y_ <<"count = "<<count;
          }
        if(shag_x){
            count = searchX(po_fi, x[ x.size() - 1 ] ,y[ y.size() - 2 ] , &x_ , y[ y.size() - 1 ], numppr, step );
            x.append(x_);
            //           debug_log<<"x "<<x.size()<< x_ <<"count = "<<count;
          }
      } while (shag_x && shag_y&& count >0);
    x.append(100.);
    y.append(100.);
    *min = po_fi.size();
    *max=0;
    (*kol).resize(x.size()*y.size());
    for(int i = 0;i < x.size()-1;++i){
        for(int j = 0;j < y.size()-1;++j){
            count= kolInKvadrat(po_fi, x[i], y[j],  x[i+1], y[j+1] );
            if(count < *min) *min = count;
            if(count > *max) *max = count;
            (*kol)[i*y.size()+j] =count;
            //   debug_log << " in kvadrat "<<x[i]<< y[j]<<  x[i+1] << y[j+1] << count;
          }
      }
    *rx = x;
    *ry = y;
    return true;
  }

}

#pragma GCC diagnostic pop

