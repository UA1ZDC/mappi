
#include "interpolorder.h"
#include "func_obanal.h"
#include <stdio.h>

//#include <boost/math/interpolators/catmull_rom.hpp>
//#include <boost/math/interpolators/barycentric_rational.hpp>
//#include <boost/range/adaptors.hpp>

int BandSpecialAny_Prod(float *x1, float *x2, float *xprod, int n, int n2, int m, int k)
{
  int i, j, i1, i2, l;
  double sum;

  if(n2!=(n+k-1)) return 2;
  for(i=0; i<n; i++) {
    i1=i*k;
    i2=i*m;
    for(j=0; j<m; j++) {
      sum=0.;
      for(l=0; l<k; l++) sum+=x1[i1+l]*x2[(i+l)*m+j];
      xprod[i2+j]=sum;
    }
  }
  return 0;
}

int BandSpecialAny_Prod(double *x1, double *x2, double *xprod, int n, int n2, int m, int k)
{
  int i, j, i1, i2, l;
  double sum;

  if(n2!=(n+k-1)) return 2;
  for(i=0; i<n; i++) {
    i1=i*k;
    i2=i*m;
    for(j=0; j<m; j++) {
      sum=0.;
      for(l=0; l<k; l++) sum+=x1[i1+l]*x2[(i+l)*m+j];
      xprod[i2+j]=sum;
    }
  }
  return 0;
}

int BandSpecialDiag_Prod(double *x1, double *x2, double *xprod, int n, int n2, int k)
{
  int i, j, i1;

  if(n2!=(n+k-1)) return 2;
  for(i=0; i<n; i++)
  {
    i1=i*k;
    for(j=0; j<k; j++) xprod[i1+j]=x1[i1+j]*x2[i+j];
  }
  return 0;
}


int BandStrongAny_Prod(double *x1, int n, int m1, int m2, int m,
                       double *x2, int nn, int mm, double *x3)
{
  int i, j, i1, i3, ii, jj, k;
  int nach, kon;
  double sum;

  if((m1+m2+1)!=m) return 2;
  if(n!=nn) return 2;
  for(i=0;i<nn*mm;i++) x3[i]=0.;

  for(i=0; i<n; i++)
  {
    i1=i*m;
    i3=i*mm;
    if(i-m1<0) nach=0;
    else nach=i-m1;
    if((i+m2)>(n-1)) kon=n;
    else kon=i+m2+1;
    for(j=0;j<mm;j++)
    {
      sum=0;
      for(k=nach;k<kon;k++)
      {
        jj=i1+(k-i)+m1;
        ii=k*mm+j;
        sum+=x1[jj]*x2[ii];
      }
      x3[i3+j]=sum;
    }
  }
  return 0;
}

int BandAny_Prod(float *x1, int n, int nm, int m1, int m2, int m,
                 float *x2, int nn, int mm, float *x3, int nnn, int mmm)
{
  int i, j, i1, i3, ii, jj, k;
  int nach, kon;
  double sum;

  if((m1+m2+1)!=m) return 2;
  if(nm<m) return 2;
  if(nm!=nn) return 2;
  if(n!=nnn) return 2;
  if(mm!=mmm) return 2;
  for(i=0;i<nnn*mmm;i++) x3[i]=0.;

  for(i=0; i<n; i++)
  {
    i1=i*m;
    i3=i*mm;
    if(i-m1<0) nach=0;
    else nach=i-m1;
    if((i+m2)>(nm-1)) kon=nm;
    else kon=i+m2+1;
    for(j=0;j<mm;j++)
    {
      sum=0;
      for(k=nach;k<kon;k++)
      {
        jj=i1+(k-i)+m1;
        ii=k*mm+j;
        sum+=x1[jj]*x2[ii];
      }
      x3[i3+j]=sum;
    }
  }
  return 0;
}

int BandStrongBandStrong_Prod(double *x1, int n, int m11, int m12, int m1,
                              double *x2, int m21, int m22, int m2,
                              double *x3, int m31, int m32, int m3)
{
  int i, j, i1, i3, j2, ii, jj, k;
  int i_nach, i_kon, j_nach, j_kon, nach, kon;
  double sum;

  if((m11+m12+1)!=m1) return 2;
  if((m21+m22+1)!=m2) return 2;
  if((m31+m32+1)!=m3) return 2;
  if((m11+m21)!=m31) return 2;
  if((m12+m22)!=m32) return 2;
  for(i=0;i<n*m3;i++) x3[i]=0.;

  for(i=0; i<n; i++)
  {
    i1=i*m1;
    i3=i*m3;
    if(i-m11<0) j_nach=0;
    else j_nach=i-m11;
    if((i+m12)>(n-1)) j_kon=n-1;
    else j_kon=i+m12;
    for(j=0;j<m3;j++)
    {
      if(m31-j>i) continue;
      j2=j-m31+i;
      if(j2>(n-1)) continue;
      if(j2-m22<0) i_nach=0;
      else i_nach=j2-m22;
      if((j2+m21)>(n-1)) i_kon=n-1;
      else i_kon=j2+m21;
      if(j_nach<i_nach) nach=i_nach;
      else nach=j_nach;
      if(j_kon<i_kon) kon=j_kon+1;
      else kon=i_kon+1;
      sum=0;
      for(k=nach;k<kon;k++)
      {
        jj=i1+(k-i)+m11;
        ii=k*m2+(j2-k)+m21;
        sum+=x1[jj]*x2[ii];
      }
      x3[i3+j]=sum;
    }
  }
  return 0;
}

int BandBand_Prod(float *x1, int n, int m, int m11, int m12, int m1,
                  float *x2, int nn, int mm, int m21, int m22, int m2,
                  float *x3, int nnn, int m31, int m32, int m3)
{
  int i, j, i1, i3, j2, ii, jj, k;
  int i_nach, i_kon, j_nach, j_kon, nach, kon;
  double sum;

  if((m11+m12+1)!=m1) return 2;
  if((m21+m22+1)!=m2) return 2;
  if((m31+m32+1)!=m3) return 2;
  if((m11+m21)!=m31) return 2;
  if((m12+m22)!=m32) return 2;
  if(m<m1) return 2;
  if(mm<m2) return 2;
  if(n!=nnn) return 2;


  for(i=0;i<n*m3;i++) x3[i]=0.;

  for(i=0; i<n; i++)
  {
    i1=i*m1;
    i3=i*m3;
    if(i-m11<0) j_nach=0;
    else j_nach=i-m11;
    if((i+m12)>(m-1)) j_kon=m-1;
    else j_kon=i+m12;
    for(j=0;j<m3;j++)
    {
      if(m31-j>i) continue;
      j2=j-m31+i;
      if(j2>(mm-1)) continue;
      if(j2-m22<0) i_nach=0;
      else i_nach=j2-m22;
      if((j2+m21)>(nn-1)) i_kon=nn-1;
      else i_kon=j2+m21;
      if(j_nach<i_nach) nach=i_nach;
      else nach=j_nach;
      if(j_kon<i_kon) kon=j_kon+1;
      else kon=i_kon+1;
      sum=0;
      for(k=nach;k<kon;k++)
      {
        jj=i1+(k-i)+m11;
        ii=k*m2+(j2-k)+m21;
        sum+=x1[jj]*x2[ii];
      }
      x3[i3+j]=sum;
    }
  }
  return 0;
}

int BandBand_Sum(float *x1, int n, int m11, int m12, int m1,
                 float *x2, int m21, int m22, int m2,
                 float *x3, int m31, int m32, int m3)
{
  int i, i1, i2, i3, j, j1, j2;
  int max;
  double sum;

  if((m11+m12+1)!=m1) return 2;
  if((m21+m22+1)!=m2) return 2;
  if((m31+m32+1)!=m3) return 2;
  if(m11<m21) max=m21;
  else max=m11;
  if(max!=m31) return 2;
  if(m12<m22) max=m22;
  else max=m12;
  if(max!=m32) return 2;
  for(i=0;i<n*m3;i++) x3[i]=0.;

  for(i=0;i<n;i++)
  {
    i1=i*m1;
    i2=i*m2;
    i3=i*m3;
    for(j=0;j<m3;j++)
    {
      sum=0.;
      j1=j-m31+m11;
      j2=j-m31+m21;
      if(j1>-1 && j1<m1) sum+=x1[i1+j1];
      if(j2>-1 && j2<m2) sum+=x2[i2+j2];
      x3[i3+j]=sum;
    }
  }
  return 0;
}

int BandStrong_Trans(double *x, int n, int m1, int m2, int m)
{
  int i, j, i1, i2, kol;
  double *matr_vs=nullptr;

  if((m1+m2+1)!=m) return 2;
  kol=n*m;
  matr_vs= new double[kol];
  if(matr_vs==nullptr) return 1;
  for(i=0;i<kol;i++) matr_vs[i]=x[i];
  for(i=0;i<kol;i++) x[i]=0.;

  for(i=0; i<n; i++)
  {
    i2=i*m;
    for(j=0;j<m;j++)
    {
      if(m2-j>i) continue;
      if(j-m2>(n-1)-i) continue;

      i1=(i-(m2-j))*m;
      x[i2+j]=matr_vs[i1+(m2-j+m1)];
    }
  }
  if(matr_vs!=nullptr) {delete []matr_vs; matr_vs=nullptr;}
  return 0;
}

int Band_Trans(float *x, float *x_res, int n, int mm, int m1, int m2, int m)
{
  int i, j, i1, i2, kol;

  if((m1+m2+1)!=m) return 2;
  if(mm<m) return 2;
  kol=mm*m;
  for(i=0;i<kol;i++) x_res[i]=0.;

  for(i=0; i<mm; i++)
  {
    i2=i*m;
    for(j=0;j<m;j++)
    {
      if(m2-j>i) continue;
      if(j-m2>(n-1)-i) continue;

      i1=(i-(m2-j))*m;
      x_res[i2+j]=x[i1+(m2-j+m1)];
    }
  }
  return 0;
}

int DecompBand(float *a,int n,int m1,int m2,int m,
               float *a1,int *indx,int *znak)
{
  double vs;
  int i, j, k, l, i1, i2;
  if((m1+m2+1)!=m) return 2;


  for(i=0; i<m1; i++) { i1=i*m;
    i2=m1-i;
    for(j=i2; j<m; j++) a[i1+j-i2]=a[i1+j];
    for(j=m-i2; j<m; j++) a[i1+j]=0.;
  }

  *znak=1;
  l=m1;
  for(k=0; k<n; k++)
  {
    i1=k*m;
    vs=fabs(a[i1]);
    if(l<n) l++;
    i=k;
    for(j=k+1; j<l; j++) if(fabs(a[j*m])>vs) {vs=fabs(a[j*m]); i=j;}
    //if(vs<eps) return 2;
    if(MnMath::isZero(vs)) return 2;
    indx[k]=i;
    if(i!=k) { i2=i*m;
      *znak=-(*znak);
      for(j=0; j<m; j++) { vs=a[i1+j];
        a[i1+j]=a[i2+j];
        a[i2+j]=vs;
      }
    }
    for(i=k+1; i<l; i++) { i2=i*m;
      vs=a[i2]/a[i1];
      a1[i1+i-k-1]=vs;
      for(j=1; j<m; j++) a[i2+j-1]=a[i2+j]-vs*a[i1+j];
      a[i2+m-1]=0.;
    }
  }
  return 0;
}
int DecompBand(double *a,int n,int m1,int m2,int m,
               double *a1,int *indx,int *znak)
{
  double vs;
  int i, j, k, l, i1, i2;
  if((m1+m2+1)!=m) return 2;


  for(i=0; i<m1; i++) { i1=i*m;
    i2=m1-i;
    for(j=i2; j<m; j++) a[i1+j-i2]=a[i1+j];
    for(j=m-i2; j<m; j++) a[i1+j]=0.;
  }

  *znak=1;
  l=m1;
  for(k=0; k<n; k++)
  {
    i1=k*m;
    vs=fabs(a[i1]);
    if(l<n) l++;
    i=k;
    for(j=k+1; j<l; j++) if(fabs(a[j*m])>vs) {vs=fabs(a[j*m]); i=j;}
    //if(vs<eps) return 2;
    if(MnMath::isZero(vs)) return 2;
    indx[k]=i;
    if(i!=k) { i2=i*m;
      *znak=-(*znak);
      for(j=0; j<m; j++) { vs=a[i1+j];
        a[i1+j]=a[i2+j];
        a[i2+j]=vs;
      }
    }
    for(i=k+1; i<l; i++) { i2=i*m;
      vs=a[i2]/a[i1];
      a1[i1+i-k-1]=vs;
      for(j=1; j<m; j++) a[i2+j-1]=a[i2+j]-vs*a[i1+j];
      a[i2+m-1]=0.;
    }
  }
  return 0;
}

int ReshBand(float *a,int n,int m1,int m2,int m,
             float *a1,int *indx,float *b)
{
  double vs;
  int i, k, l, i1;
  if((m1+m2+1)!=m) return 2;
  l=m1;
  for(k=0; k<n; k++) {
    i=indx[k];
    if(i!=k) { vs=b[k];
      b[k]=b[i];
      b[i]=vs;
    }
    if(l<n) l++;
    i1=k*m-k;
    for(i=k+1; i<l; i++) {
      b[i]-=a1[i1+i-1]*b[k];
    }
  }
  l=1;
  for(i=n-1; i>-1; i--) {
    vs=b[i];
    i1=i*m;
    for(k=1; k<l; k++) vs-=a[i1+k]*b[k+i];
    b[i]=vs/a[i1];
    if(l<m) l++;
  }
  return 0;
}
int ReshBand(double *a,int n,int m1,int m2,int m,
             double *a1,int *indx,double *b)
{
  double vs;
  int i, k, l, i1;
  if((m1+m2+1)!=m) return 2;
  l=m1;
  for(k=0; k<n; k++) {
    i=indx[k];
    if(i!=k) { vs=b[k];
      b[k]=b[i];
      b[i]=vs;
    }
    if(l<n) l++;
    i1=k*m-k;
    for(i=k+1; i<l; i++) {
      b[i]-=a1[i1+i-1]*b[k];
    }
  }
  l=1;
  for(i=n-1; i>-1; i--) {
    vs=b[i];
    i1=i*m;
    for(k=1; k<l; k++) vs-=a[i1+k]*b[k+i];
    b[i]=vs/a[i1];
    if(l<m) l++;
  }
  return 0;
}
int initMatr_ParamSpline(float **a, float **h, int n)
{
  double p3=2./3., p6=1./6.;
  int m, i, n1;
  int ret_val=0;

  m=3*n;
  n1=n-1;
  (*a)=nullptr;
  (*h)=nullptr;

  (*a)= new float[m];
  (*h)= new float[m];
  for(i=0; i<m; i++) {(*a)[i]=0.; (*h)[i]=0.;}
  (*a)[0]=0.;
  (*a)[1]=p3;
  (*a)[2]=p6;
  (*h)[0]=1.;
  (*h)[1]=-2.;
  (*h)[2]=1.;
  (*a)[n*3-3]=p6;
  (*a)[n*3-2]=p3;
  (*a)[n*3-1]=0;
  (*h)[n*3-3]=1.;
  (*h)[n*3-2]=-2.;
  (*h)[n*3-1]=1.;
  for(i=1; i<n1; i++)
  {
    (*a)[i*3]=p6;
    (*a)[i*3+1]=p3;
    (*a)[i*3+2]=p6;
    (*h)[i*3]=1.;
    (*h)[i*3+1]=-2.;
    (*h)[i*3+2]=1.;
  }
  return 0;

  if((*a)!=nullptr) {delete [](*a); (*a)=nullptr;}
  if((*h)!=nullptr) {delete [](*h); (*h)=nullptr;}
  return ret_val;
}

int initMatr_ParamSpline(double **a, double **h, int n)
{
  double p3=2./3., p6=1./6.;
  int m, i, n1;
  int ret_val=0;

  m=3*n;
  n1=n-1;
  (*a)=nullptr;
  (*h)=nullptr;

  (*a)= new double[m];
  (*h)= new double[m];
  for(i=0; i<m; i++) {(*a)[i]=0.; (*h)[i]=0.;}
  (*a)[0]=0.;
  (*a)[1]=p3;
  (*a)[2]=p6;
  (*h)[0]=1.;
  (*h)[1]=-2.;
  (*h)[2]=1.;
  (*a)[n*3-3]=p6;
  (*a)[n*3-2]=p3;
  (*a)[n*3-1]=0;
  (*h)[n*3-3]=1.;
  (*h)[n*3-2]=-2.;
  (*h)[n*3-1]=1.;
  for(i=1; i<n1; i++)
  {
    (*a)[i*3]=p6;
    (*a)[i*3+1]=p3;
    (*a)[i*3+2]=p6;
    (*h)[i*3]=1.;
    (*h)[i*3+1]=-2.;
    (*h)[i*3+2]=1.;
  }
  return 0;

  if((*a)!=nullptr) {delete [](*a); (*a)=nullptr;}
  if((*h)!=nullptr) {delete [](*h); (*h)=nullptr;}
  return ret_val;
}

int factoriz(float **a, float **a1, float **h, int **indx, int *znak, int n)
{
  int ret_val=0;

  (*a1)=nullptr;
  (*indx)=nullptr;

  (*a1)= new float[n*3];
  (*indx)= new int[n];

  ret_val=initMatr_ParamSpline(a,h,n);
  if(ret_val!=0) goto m1;
  ret_val=DecompBand(*a,n,1,1,3,*a1,*indx,znak);
  if(ret_val!=0) goto m1;

  return 0;

m1:
  if((*a)!=nullptr) {delete [](*a); (*a)=nullptr;}
  if((*a1)!=nullptr) {delete [](*a1); (*a1)=nullptr;}
  if((*h)!=nullptr) {delete [](*h); (*h)=nullptr;}
  if((*indx)!=nullptr) {delete [](*indx); (*indx)=nullptr;}
  return ret_val;
}

int factoriz(double **a, double **a1, double **h, int **indx, int *znak, int n)
{
  int ret_val=0;

  (*a1)=nullptr;
  (*indx)=nullptr;

  (*a1)= new double[n*3];
  (*indx)= new int[n];

  ret_val=initMatr_ParamSpline(a,h,n);
  if(ret_val!=0) goto m1;
  ret_val=DecompBand(*a,n,1,1,3,*a1,*indx,znak);
  if(ret_val!=0) goto m1;

  return 0;

m1:
  if((*a)!=nullptr) {delete [](*a); (*a)=nullptr;}
  if((*a1)!=nullptr) {delete [](*a1); (*a1)=nullptr;}
  if((*h)!=nullptr) {delete [](*h); (*h)=nullptr;}
  if((*indx)!=nullptr) {delete [](*indx); (*indx)=nullptr;}
  return ret_val;
}

int factoriz_smooth(float **a, float **a1, float **h, float **diag_h_tr, float *diag,
                    int **indx, int *znak, int n, int m)
{
  int ret_val=0;
  float *a_vs=nullptr;
  float *h_tr=nullptr;
  float *h_vs=nullptr;

  (*a)=nullptr;
  (*a1)=nullptr;
  (*h)=nullptr;
  (*diag_h_tr)=nullptr;
  (*indx)=nullptr;

  (*a)= new float[n*5];
  (*a1)= new float[n*5];
  (*indx)= new int[n];
  (*diag_h_tr)= new float[m*3];
  h_tr= new float[m*3];
  h_vs= new float[n*5];

  ret_val=initMatr_ParamSpline(&a_vs,h,n);
  if(ret_val!=0) goto m1;

  ret_val=Band_Trans(*h,h_tr,n,m,0,2,3);
  if(ret_val!=0) goto m1;

  ret_val=BandBand_Prod(diag,m,m,0,0,1,
                        h_tr,m,n,2,0,3,
                        *diag_h_tr,m,2,0,3);
  if(ret_val!=0) goto m1;

  ret_val=BandBand_Prod(*h,n,m,0,2,3,
                        *diag_h_tr,m,n,2,0,3,
                        h_vs,n,2,2,5);
  if(ret_val!=0) goto m1;

  ret_val=BandBand_Sum(a_vs,n,1,1,3,
                       h_vs,2,2,5,
                       *a,2,2,5);
  if(ret_val!=0) goto m1;

  ret_val=DecompBand(*a,n,2,2,5,*a1,*indx,znak);
  if(ret_val!=0) goto m1;

  if(a_vs!=nullptr) {delete []a_vs; a_vs=nullptr;}
  if(h_tr!=nullptr) {delete []h_tr; h_tr=nullptr;}
  if(h_vs!=nullptr) {delete []h_vs; h_vs=nullptr;}
  return 0;

m1:
  if((*a)!=nullptr) {delete [](*a); (*a)=nullptr;}
  if((*a1)!=nullptr) {delete [](*a1); (*a1)=nullptr;}
  if(a_vs!=nullptr) {delete []a_vs; a_vs=nullptr;}
  if(h_tr!=nullptr) {delete []h_tr; h_tr=nullptr;}
  if(h_vs!=nullptr) {delete []h_vs; h_vs=nullptr;}
  if((*h)!=nullptr) {delete [](*h); (*h)=nullptr;}
  if((*diag_h_tr)!=nullptr) {delete [](*diag_h_tr); (*diag_h_tr)=nullptr;}
  if((*indx)!=nullptr) {delete [](*indx); (*indx)=nullptr;}
  return ret_val;
}

int oprFunSpline(float *coef, float *fl, float *fun,
                 float *f, const float *net, int n, int kol)
{
  double par=1./6., x, vs, x3, vs3, a0, a1;
  int i, niz;
  for(i=0;i<kol;i++)
  {
    niz = MnObanal::searh_inf(net[i],fl,n);
    if(niz<0) return 3;
    x=(net[i]-fl[niz])/(fl[niz+1]-fl[niz]);
    vs=1.-x;
    x3=x*x*x;
    vs3=vs*vs*vs;
    a0=coef[niz]*par;
    a1=coef[niz+1]*par;
    f[i]=a0*vs3+a1*x3+(fun[niz]-a0)*vs+(fun[niz+1]-a1)*x;
  }
  return 0;
}

double oprFunSplineEasy(float *coef, float *fun, int niz, double x)
{
  double par=1./6., vs, x3, vs3, a0, a1;

  vs=1.-x;
  x3=x*x*x;
  vs3=vs*vs*vs;
  a0=coef[niz]*par;
  a1=coef[niz+1]*par;

  return a0*vs3+a1*x3+(fun[niz]-a0)*vs+(fun[niz+1]-a1)*x;
}
double oprFunSplineEasy(double *coef, double *fun, int niz, double x)
{
  double par=1./6., vs, x3, vs3, a0, a1;

  vs=1.-x;
  x3=x*x*x;
  vs3=vs*vs*vs;
  a0=coef[niz]*par;
  a1=coef[niz+1]*par;

  return a0*vs3+a1*x3+(fun[niz]-a0)*vs+(fun[niz+1]-a1)*x;
}
int InterpolLong(const float *netla, float *rez, float *la, float *fun, int *kol_point,
                 int kol_fi, int kol_la)
{
  int ret_val=0;
  float *a=nullptr;
  float *a1=nullptr;
  float *h=nullptr;
  float *fvs=nullptr;
  float *coef=nullptr;
  float *la_vs=nullptr;
  float *fun_vs=nullptr;
  int *indx=nullptr;
  int *num=nullptr;
  float *p_la, *p_fun, *pvs_la, *pvs_fun;
  int znak, start, kol, kol_pred;
  int i,j, n, n_start, i_start, n2;

  start=0;
  n_start=0;
  kol=kol_point[n_start];
  while(kol==1)
  {
    i_start=n_start*kol_la;
    for(i=0;i<kol_la;i++) rez[i_start+i]=fun[start];
    start+=kol;
    n_start++;
    if(n_start==kol_fi) return 0;
    kol=kol_point[n_start];
  }

  n=kol+5;
  n2=n-2;
  ret_val=factoriz(&a,&a1,&h,&indx,&znak,n2);
  if(ret_val!=0) goto m1;
  fvs= new float[n2];
  coef= new float[n];
  la_vs= new float[n];
  fun_vs= new float[n];
  num= new int[kol];
  coef[0]=0.;
  coef[n-1]=0.;

  kol_pred=kol;

  for(i=n_start;i<kol_fi;i++)
  {
    i_start=i*kol_la;
    kol=kol_point[i];
    if(kol==1) for(j=0;j<kol_la;j++) rez[i_start+j]=fun[start];
    else {
      if(kol!=kol_pred)
      {
        if(a!=nullptr) {delete []a; a=nullptr;}
        if(a1!=nullptr) {delete []a1; a1=nullptr;}
        if(h!=nullptr) {delete []h; h=nullptr;}
        if(indx!=nullptr) {delete []indx; indx=nullptr;}
        if(fvs!=nullptr) {delete []fvs; fvs=nullptr;}
        if(coef!=nullptr) {delete []coef; coef=nullptr;}
        if(la_vs!=nullptr) {delete []la_vs; la_vs=nullptr;}
        if(fun_vs!=nullptr) {delete []fun_vs; fun_vs=nullptr;}
        if(num!=nullptr) {delete []num; num=nullptr;}
        n=kol+5;
        n2=n-2;
        ret_val=factoriz(&a,&a1,&h,&indx,&znak,n2);
        if(ret_val!=0) goto m1;
        fvs= new float[n2];
        coef= new float[n];
        la_vs= new float[n];
        fun_vs= new float[n];
        num= new int[kol];
        coef[0]=0.;
        coef[n-1]=0.;
      }

      p_la=&la[start];
      p_fun=&fun[start];
      for(j=0;j<kol;j++) num[j]=j;
      MnObanal::sort2(p_la,num,kol);
      pvs_la=&la_vs[2];
      pvs_fun=&fun_vs[2];
      for(j=0;j<kol;j++) {pvs_la[j]=p_la[j]; pvs_fun[j]=p_fun[num[j]];}
      la_vs[kol+2]= MnMath::M_2PIf+pvs_la[0];
      fun_vs[kol+2]=pvs_fun[0];
      la_vs[kol+3]=MnMath::M_2PIf+pvs_la[1];
      fun_vs[kol+3]=pvs_fun[1];
      la_vs[kol+4]=MnMath::M_2PIf+pvs_la[2];
      fun_vs[kol+4]=pvs_fun[2];
      la_vs[1]=pvs_la[kol-1]-MnMath::M_2PIf;
      fun_vs[1]=pvs_fun[kol-1];
      la_vs[0]=pvs_la[kol-2]-MnMath::M_2PIf;
      fun_vs[0]=pvs_fun[kol-2];

      BandSpecialAny_Prod(h,fun_vs,fvs,n2,n,1,3);
      ret_val=ReshBand(a,n2,1,1,3,a1,indx,fvs);
      if(ret_val!=0) goto m1;
      pvs_fun=&coef[1];
      for(j=0;j<n2;j++) pvs_fun[j]=fvs[j];
      pvs_fun=&rez[i_start];
      ret_val=oprFunSpline(coef,la_vs,fun_vs,pvs_fun,netla,n,kol_la);
      if(ret_val!=0) goto m1;
    }

    start+=kol;
    kol_pred=kol;
  }

m1:
  if(a!=nullptr) {delete []a; a=nullptr;}
  if(a1!=nullptr) {delete []a1; a1=nullptr;}
  if(h!=nullptr) {delete []h; h=nullptr;}
  if(indx!=nullptr) {delete []indx; indx=nullptr;}
  if(fvs!=nullptr) {delete []fvs; fvs=nullptr;}
  if(coef!=nullptr) {delete []coef; coef=nullptr;}
  if(la_vs!=nullptr) {delete []la_vs; la_vs=nullptr;}
  if(fun_vs!=nullptr) {delete []fun_vs; fun_vs=nullptr;}
  if(num!=nullptr) {delete []num; num=nullptr;}
  return ret_val;
}


int InterpolY(const float *netla, QVector<float>  *rez,
              QVector<float> *la, QVector<float> *fun, const QVector<int> &kol_point,
              int kol_fi,int kol_la)
{
  int ret_val=0;
  int  start, kol, kol_pred;
  int i, j, n, n_start, i_start, n2;

  start=0;
  n_start=0;
  kol=kol_point.at(n_start);

  while(kol == 1)
  {
    i_start = n_start*kol_la;
    for(i=0;i<kol_la;i++){
      (*rez)[i_start+i] = fun->at(start);
    }
    start+=kol;
    n_start++;
    if(n_start == kol_fi) return 0;
    kol=kol_point.at(n_start);
  }
  n=kol;
  //n2=n-2;
  n2=n-2;
  if(0 == n || n2 == 0) return -1;


  float *a=nullptr;
  float *a1=nullptr;
  float *h=nullptr;
  float *fvs=nullptr;
  float *coef=nullptr;
  float *la_vs=nullptr;
  float *fun_vs=nullptr;
  int *indx=nullptr;
  int *num=nullptr;

  int znak = 0;
  ret_val = factoriz(&a,&a1,&h,&indx,&znak,n2);
  if(ret_val!=0)  return ret_val;
  fvs= new float[n2];
  coef= new float[n];
  la_vs= new float[n];
  fun_vs= new float[n];
  num= new int[kol];
  coef[0]=0.;
  coef[n-1]=0.;

  kol_pred=kol;

  for(i=n_start;i<kol_fi;i++)
  {
    i_start=i*kol_la;
    kol=kol_point[i];
    if(kol==1) for(j=0;j<kol_la;j++) {
      (*rez)[i_start+j] = fun->at(start);
    }
    else {
      if(kol!=kol_pred)
      {
        if(a!=nullptr) {delete []a; a=nullptr;}
        if(a1!=nullptr) {delete []a1; a1=nullptr;}
        if(h!=nullptr) {delete []h; h=nullptr;}
        if(indx!=nullptr) {delete []indx; indx=nullptr;}
        if(fvs!=nullptr) {delete []fvs; fvs=nullptr;}
        if(coef!=nullptr) {delete []coef; coef=nullptr;}
        if(la_vs!=nullptr) {delete []la_vs; la_vs=nullptr;}
        if(fun_vs!=nullptr) {delete []fun_vs; fun_vs=nullptr;}
        if(num!=nullptr) {delete []num; num=nullptr;}
        n=kol+5;
        n2=n-2;
        ret_val=factoriz(&a,&a1,&h,&indx,&znak,n2);
        if(ret_val!=0) goto m1;
        fvs= new float[n2];
        coef= new float[n];
        la_vs= new float[n];
        fun_vs= new float[n];
        num= new int[kol];
        coef[0]=0.;
        coef[n-1]=0.;
      }
      float *p_la, *p_fun, *pvs_la, *pvs_fun;

      p_la=&(la->data()[start]);
      p_fun=&(fun->data()[start]);
      for(j=0;j<kol;j++) num[j]=j;
      MnObanal::sort2(p_la,num,kol);
      pvs_la=&la_vs[0];
      pvs_fun = &(fun_vs[0]);
      for(j=0;j<kol;j++) {pvs_la[j]=p_la[j]; pvs_fun[j]=p_fun[num[j]];}

      BandSpecialAny_Prod(h,fun_vs,fvs,n2,n,1,3);
      ret_val=ReshBand(a,n2,1,1,3,a1,indx,fvs);
      if(ret_val!=0) return ret_val;
      pvs_fun=&coef[1];
      for(j=0;j<n2;j++) pvs_fun[j]=fvs[j];
      pvs_fun = &(rez->data()[i_start]);

      ret_val=oprFunSpline(coef,la_vs,fun_vs,pvs_fun,netla,n,kol_la);
      if(ret_val!=0) return ret_val;
    }

    start+=kol;
    kol_pred=kol;
  }

m1:
  if(a!=nullptr) {delete []a; a=nullptr;}
  if(a1!=nullptr) {delete []a1; a1=nullptr;}
  if(h!=nullptr) {delete []h; h=nullptr;}
  if(indx!=nullptr) {delete []indx; indx=nullptr;}
  if(fvs!=nullptr) {delete []fvs; fvs=nullptr;}
  if(coef!=nullptr) {delete []coef; coef=nullptr;}
  if(la_vs!=nullptr) {delete []la_vs; la_vs=nullptr;}
  if(fun_vs!=nullptr) {delete []fun_vs; fun_vs=nullptr;}
  if(num!=nullptr) {delete []num; num=nullptr;}
  return ret_val;

}


int InterpolLat(const float *netfi, obanal::TField *rez, float *netfi_fact,
                float *rez_vs, int kol_fi, int kol_fi_fact, int kol_la)
{
  int ret_val=0;
  float *a=nullptr;
  float *a1=nullptr;
  float *h=nullptr;
  float *fvs=nullptr;
  float *coef=nullptr;
  float *fi_vs=nullptr;
  float *fun_vs=nullptr;
  float *fun=nullptr;
  int *indx=nullptr;
  float *pvs_fun;
  int znak, kol, sdvig, dop;
  int i,j, n,n2;
  bool flag1, flag2;

  if( MnMath::isEqual( netfi_fact[0], -M_PI_2f) ){
    flag1=true;
  }
  else {
    flag1=false;
  }
  if( MnMath::isEqual( netfi_fact[kol_fi_fact-1], M_PI_2f) ){
    flag2=true;
  }
  else {
    flag2=false;
  }

  sdvig=(kol_la-1)/2;
  if(flag1||flag2) {if(kol_fi_fact<4) return 125;}
  else if(kol_fi_fact<2) return 125;

  kol=kol_fi_fact;
  n=kol+4;
  n2=n-2;
  ret_val=factoriz(&a,&a1,&h,&indx,&znak,n2);
  if(ret_val!=0) goto m1;
  fvs= new float[n2];
  coef= new float[n];
  fi_vs= new float[n];
  fun_vs= new float[n];
  fun= new float[kol_fi];
  coef[0]=0.;
  coef[n-1]=0.;


  for(i=0;i<kol_fi_fact;i++) fi_vs[i+2]=netfi_fact[i];
  if(flag1) {fi_vs[1]=-M_PIf-netfi_fact[1]; fi_vs[0]=-M_PIf-netfi_fact[2];}
  else   {fi_vs[1]=-M_PIf-netfi_fact[0]; fi_vs[0]=-M_PIf-netfi_fact[1];}
  if(flag2) {fi_vs[n-2]=M_PIf-netfi_fact[kol_fi_fact-2]; fi_vs[n-1]=M_PIf-netfi_fact[kol_fi_fact-3];}
  else   {fi_vs[n-2]=M_PIf-netfi_fact[kol_fi_fact-1]; fi_vs[n-1]=M_PIf-netfi_fact[kol_fi_fact-2];}

  for(i=0;i<kol_la;i++)
  {
    dop=(i+sdvig)%(kol_la-1);
    pvs_fun=&fun_vs[2];
    for(j=0;j<kol_fi_fact;j++) pvs_fun[j]=rez_vs[j*kol_la+i];
    if(flag1) {fun_vs[1]=rez_vs[kol_la+dop]; fun_vs[0]=rez_vs[2*kol_la+dop];}
    else   {fun_vs[1]=rez_vs[dop]; fun_vs[0]=rez_vs[kol_la+dop];}
    if(flag2) {fun_vs[n-2]=rez_vs[kol_la*(kol_fi_fact-2)+dop]; fun_vs[n-1]=rez_vs[kol_la*(kol_fi_fact-3)+dop];}
    else   {fun_vs[n-2]=rez_vs[kol_la*(kol_fi_fact-1)+dop]; fun_vs[n-1]=rez_vs[kol_la*(kol_fi_fact-2)+dop];}

    BandSpecialAny_Prod(h,fun_vs,fvs,n2,n,1,3);
    ret_val=ReshBand(a,n2,1,1,3,a1,indx,fvs);
    if(ret_val!=0) goto m1;
    pvs_fun=&coef[1];
    for(j=0;j<n2;j++) pvs_fun[j]=fvs[j];
    ret_val=oprFunSpline(coef,fi_vs,fun_vs,fun,netfi,n,kol_fi);
    if(ret_val!=0) goto m1;
    //for(j=0;j<kol_fi;j++) rez[j*kol_la+i]=fun[j];
    for(j=0;j<kol_fi;j++) rez->setData(j*kol_la+i,fun[j],true);


  }
m1:
  if(a!=nullptr) {delete []a; a=nullptr;}
  if(a1!=nullptr) {delete []a1; a1=nullptr;}
  if(h!=nullptr) {delete []h; h=nullptr;}
  if(indx!=nullptr) {delete []indx; indx=nullptr;}
  if(fvs!=nullptr) {delete []fvs; fvs=nullptr;}
  if(coef!=nullptr) {delete []coef; coef=nullptr;}
  if(fi_vs!=nullptr) {delete []fi_vs; fi_vs=nullptr;}
  if(fun_vs!=nullptr) {delete []fun_vs; fun_vs=nullptr;}
  if(fun!=nullptr) {delete []fun; fun=nullptr;}
  return ret_val;
}


int InterpolX(const float* netfi, obanal::TField *rez,
              const QVector<float> &netfi_fact, const QVector<float> &rez_vs, int kol_fi, int kol_fi_fact, int kol_la)
{

  int ret_val=0;

  int znak, kol;
  int i,j, n,n2;

  if(kol_fi_fact<2) return 125;
  if(2 == kol_fi_fact) return InterpolX2(netfi,rez,netfi_fact,rez_vs, kol_fi, kol_fi_fact,kol_la);
  kol = kol_fi_fact;
  n = kol;
  n2 = n-2;
  float *a=nullptr;
  float *a1=nullptr;
  float *h=nullptr;
  int *indx=nullptr;
  float *fvs=nullptr;
  float *coef=nullptr;
  float *fi_vs=nullptr;
  float *fun_vs=nullptr;
  float *fun=nullptr;
  float *pvs_fun;

  ret_val = factoriz(&a,&a1,&h,&indx,&znak,n2);
  if(ret_val != 0) { goto m1;}

  if(ret_val!=0) goto m1;
  fvs= new float[n2];
  coef= new float[n];
  fi_vs= new float[n];
  fun_vs= new float[n];
  fun= new float[kol_fi];
  coef[0]=0.;
  coef[n-1]=0.;

  for(i=0;i<kol;i++) fi_vs[i]=netfi_fact[i];

  for(i=0;i<kol_la;i++)
  {
    pvs_fun=&fun_vs[0];
    for(j=0;j<kol_fi_fact;j++) pvs_fun[j]=rez_vs[j*kol_la+i];
    BandSpecialAny_Prod(h,fun_vs,fvs,n2,n,1,3);
    ret_val=ReshBand(a,n2,1,1,3,a1,indx,fvs);
    if(ret_val != 0)   { goto m1;}
    pvs_fun=&coef[1];
    for(j=0;j<n2;j++) pvs_fun[j]=fvs[j];
    ret_val=oprFunSpline(coef,fi_vs,fun_vs,fun,netfi,n,kol_fi);
    if(ret_val != 0) {
      //debug_log<<"oprFunSpline";
      goto m1;
    }
    for(j=0;j<kol_fi;j++) {
      rez->setData(rez->num(j,i),fun[j],true);
    }

  }

m1:
  if(a!=nullptr) {delete []a; a=nullptr;}
  if(a1!=nullptr) {delete []a1; a1=nullptr;}
  if(h!=nullptr) {delete []h; h=nullptr;}
  if(indx!=nullptr) {delete []indx; indx=nullptr;}
  if(fvs!=nullptr) {delete []fvs; fvs=nullptr;}
  if(coef!=nullptr) {delete []coef; coef=nullptr;}
  if(fi_vs!=nullptr) {delete []fi_vs; fi_vs=nullptr;}
  if(fun_vs!=nullptr) {delete []fun_vs; fun_vs=nullptr;}
  if(fun!=nullptr) {delete []fun; fun=nullptr;}
  return ret_val;
}

int InterpolX2(const float* netfi, obanal::TField *rez,
               const QVector<float> &netfi_fact, const QVector<float> &rez_vs, int kol_fi, int kol_fi_fact, int kol_la)
{
  int ret_val=0;
  int kol;
  int i,j, n;
  if(kol_fi_fact!=2) return 125;

  kol = kol_fi_fact;
  n = kol;

  float *coef=nullptr;
  float *fi_vs=nullptr;
  float *fun_vs=nullptr;
  float *fun=nullptr;
  float *pvs_fun;

  coef= new float[n];
  fi_vs= new float[n];
  fun_vs= new float[n];
  fun= new float[kol_fi];
  coef[0]=0.;
  coef[n-1]=0.;

  for(i=0;i<kol;i++) fi_vs[i]=netfi_fact[i];

  for(i=0;i<kol_la;i++)
  {
    pvs_fun=&fun_vs[0];
    for(j=0;j<kol_fi_fact;j++) pvs_fun[j]=rez_vs[j*kol_la+i];
    ret_val=oprFunSpline(coef,fi_vs,fun_vs,fun,netfi,n,kol_fi);
    if(ret_val != 0) {
      goto m1;
    }
    for(j=0;j<kol_fi;j++) {
      rez->setData(rez->num(j,i),fun[j],true);
    }
  }
m1:
  if(coef!=nullptr) {delete []coef; coef=nullptr;}
  if(fi_vs!=nullptr) {delete []fi_vs; fi_vs=nullptr;}
  if(fun_vs!=nullptr) {delete []fun_vs; fun_vs=nullptr;}
  if(fun!=nullptr) {delete []fun; fun=nullptr;}
  return ret_val;
}


int smootchLong(QVector <float> *fun, const QVector <float> &net_fi,
                int kol_la, int nach, int kon, double ur_smooth)
{
  int ret_val=0;
  int n= kol_la+20;
  int n2 = n-2;

  float *a=nullptr;
  float *a1=nullptr;
  float *h=nullptr;
  float *diag_h_tr=nullptr;
  int *indx=nullptr;
  float *diag=nullptr;
  float *fvs=nullptr;
  float *fun_vs=nullptr;


  float *p_fun, *pvs_fun;
  int znak, start;
  int i, j;

  diag= new float[n];

  for(i=0;i<n;i++) diag[i]=ur_smooth;

  ret_val=factoriz_smooth(&a,&a1,&h,&diag_h_tr,diag,&indx,&znak,n2,n);
  if(ret_val!=0) { goto m1;}
  fvs= new float[n2];
  fun_vs= new float[n];

  for(i=nach;i<kon;i++)
  {
    if( MnMath::isEqual( net_fi[i], -M_PI_2f)
        || MnMath::isEqual( net_fi[i], M_PI_2f)) continue;

    start=i*kol_la;
    p_fun=& (fun->data()[start]);
    pvs_fun=&fun_vs[10];
    for(j=0;j<kol_la;j++){
      pvs_fun[j]=p_fun[j];
      //  debug_log << "11"<<j<<p_fun[j];
    }

    for(j=0;j<10;j++) {
      fun_vs[j]=p_fun[kol_la-1-10+j];
      //  debug_log << "22"<<j<<fun_vs[j];
    }
    for(j=0;j<10;j++){
      pvs_fun[kol_la+j]=p_fun[1+j];
      // debug_log << "33"<<kol_la+j<<fun_vs[kol_la+j];

    }

    BandAny_Prod(h,n2,n,0,2,3,
                 fun_vs,n,1,
                 fvs,n2,1);
    ret_val=ReshBand(a,n2,2,2,5,a1,indx,fvs);
    if(ret_val!=0) { goto m1;}

    BandAny_Prod(diag_h_tr,n,n2,2,0,3,
                 fvs,n2,1,
                 fun_vs,n,1);

    for(j=0;j<(kol_la-1);j++) p_fun[j]=p_fun[j]-pvs_fun[j];
    p_fun[kol_la-1]=p_fun[0];
  }


m1:
  if(a!=nullptr) {delete []a; a=nullptr;}
  if(a1!=nullptr) {delete []a1; a1=nullptr;}
  if(h!=nullptr) {delete []h; h=nullptr;}
  if(diag_h_tr!=nullptr) {delete []diag_h_tr; diag_h_tr=nullptr;}
  if(diag!=nullptr) {delete []diag; diag=nullptr;}
  if(indx!=nullptr) {delete []indx; indx=nullptr;}
  if(fvs!=nullptr) {delete []fvs; fvs=nullptr;}
  if(fun_vs!=nullptr) {delete []fun_vs; fun_vs=nullptr;}

  return ret_val;

}

int smootchY(QVector<float> *fun, const QVector<float> &,
             int kol_la, int nach, int kon, double ur_smooth)
{
  int ret_val=0;
  int n= kol_la+20;
  int n2 = n-2;

  float *p_fun, *pvs_fun;
  int znak, start;
  int i, j;

  float *a=nullptr;
  float *a1=nullptr;
  float *h=nullptr;
  float *diag_h_tr=nullptr;
  int *indx=nullptr;
  float *diag=nullptr;
  float *fvs=nullptr;
  float *fun_vs=nullptr;

  diag= new float[n];
  for(i=0;i<n;i++) diag[i]=ur_smooth;

  ret_val=factoriz_smooth(&a,&a1,&h,&diag_h_tr,diag,&indx,&znak,n2,n);
  if(ret_val!=0) return ret_val;

  for(i=nach;i<kon;i++)
  {
    start=i*kol_la;
    p_fun=& (fun->data()[start]);
    pvs_fun=&fun_vs[10];
    for(j=0;j<kol_la;j++) pvs_fun[j]=p_fun[j];

    for(j=0;j<10;j++) fun_vs[j]=p_fun[kol_la-1-10+j];
    for(j=0;j<10;j++) pvs_fun[kol_la+j]=p_fun[1+j];

    BandAny_Prod(h,n2,n,0,2,3,
                 fun_vs,n,1,
                 fvs,n2,1);
    ret_val=ReshBand(a,n2,2,2,5,a1,indx,fvs);
    if(ret_val!=0) return ret_val;

    BandAny_Prod(diag_h_tr,n,n2,2,0,3,
                 fvs,n2,1,
                 fun_vs,n,1);

    for(j=0;j<(kol_la-1);j++) p_fun[j]=p_fun[j]-pvs_fun[j];
    p_fun[kol_la-1]=p_fun[0];
  }

  if(a!=nullptr) {delete []a; a=nullptr;}
  if(a1!=nullptr) {delete []a1; a1=nullptr;}
  if(h!=nullptr) {delete []h; h=nullptr;}
  if(diag_h_tr!=nullptr) {delete []diag_h_tr; diag_h_tr=nullptr;}
  if(diag!=nullptr) {delete []diag; diag=nullptr;}
  if(indx!=nullptr) {delete []indx; indx=nullptr;}
  if(fvs!=nullptr) {delete []fvs; fvs=nullptr;}
  if(fun_vs!=nullptr) {delete []fun_vs; fun_vs=nullptr;}


  return ret_val;
}

int smootchX( QVector <float> *fun, const QVector <float> &,
              int kol_la, int nach, int kon, double ur_smooth)
{
  int ret_val=0;
  bool flag1=false, flag2=false;
  int i, j;

  if(flag1||flag2) {if((kon-nach)<4) return 125;}
  else if((kon-nach)<2) return 125;

  int n = (kon-nach);
  int n2 = n-2;

  float *a=nullptr;
  float *a1=nullptr;
  float *h=nullptr;
  float *diag_h_tr=nullptr;
  int *indx=nullptr;
  float *diag=nullptr;
  float *fvs=nullptr;
  float *fun_vs=nullptr;
  float *pvs_fun;
  int znak;

  diag= new float[n];
  for(i=0;i<n;i++) diag[i]=ur_smooth;

  ret_val=factoriz_smooth(&a,&a1,&h,&diag_h_tr,diag,&indx,&znak,n2,n);
  if(ret_val!=0) {goto m1;}

  for(i=0;i<kol_la-1;i++)
  {
    //   i=0;
    // int dop=(i+sdvig)%(kol_la-1);
    pvs_fun=&fun_vs[0];
    /*   for(j=nach;j<kon;j++) pvs_fun[j-nach]=(*fun)[j*kol_la+i];
     *   if(flag1) {fun_vs[1]=(*fun)[kol_la+dop]; fun_vs[0]=(*fun)[2*kol_la+dop];}
     *   else   {fun_vs[1]=(*fun)[kol_la*nach+dop]; fun_vs[0]=(*fun)[kol_la*(nach+1)+dop];}
     *   if(flag2) {fun_vs[n-2]=(*fun)[kol_la*(kon-2)+dop]; fun_vs[n-1]=(*fun)[kol_la*(kon-3)+dop];}
     *   else   {fun_vs[n-2]=(*fun)[kol_la*(kon-1)+dop]; fun_vs[n-1]=(*fun)[kol_la*(kon-2)+dop];}
     */
    BandAny_Prod(h,n2,n,0,2,3,
                 fun_vs,n,1,
                 fvs,n2,1);
    ret_val=ReshBand(a,n2,2,2,5,a1,indx,fvs);
    if(ret_val!=0) {goto m1;}

    BandAny_Prod(diag_h_tr,n,n2,2,0,3,
                 fvs,n2,1,
                 fun_vs,n,1);

    for(j=nach;j<kon;j++) (*fun)[j*kol_la+i] = (*fun)[j*kol_la+i]-pvs_fun[j-nach];
  }

  for(j=nach;j<kon;j++) (*fun)[j*kol_la+kol_la-1]=(*fun)[j*kol_la];

m1:
  if(a!=nullptr) {delete []a; a=nullptr;}
  if(a1!=nullptr) {delete []a1; a1=nullptr;}
  if(h!=nullptr) {delete []h; h=nullptr;}
  if(diag_h_tr!=nullptr) {delete []diag_h_tr; diag_h_tr=nullptr;}
  if(diag!=nullptr) {delete []diag; diag=nullptr;}
  if(indx!=nullptr) {delete []indx; indx=nullptr;}
  if(fvs!=nullptr) {delete []fvs; fvs=nullptr;}
  if(fun_vs!=nullptr) {delete []fun_vs; fun_vs=nullptr;}

  return ret_val;
}


int smootchLat( QVector <float> *fun, const QVector <float> &net_fi,
                int kol_la, int nach, int kon, double ur_smooth)
{

  int ret_val=0;
  float *a=nullptr;
  float *a1=nullptr;
  float *h=nullptr;
  float *diag_h_tr=nullptr;
  int *indx=nullptr;
  float *diag=nullptr;
  float *fvs=nullptr;
  float *fun_vs=nullptr;
  float *pvs_fun;
  int znak;
  int i, j, n, n2, sdvig;
  bool flag1, flag2;

  if( MnMath::isEqual( net_fi[nach], float(-M_PI_2)) ) flag1=true; else flag1=false;
  if( MnMath::isEqual( net_fi[kon-1], float(M_PI_2)) ) flag2=true; else flag2=false;
  sdvig=(kol_la-1)/2;
  if(flag1||flag2) {if((kon-nach)<4) return 125;}
  else if((kon-nach)<2) return 125;

  n=(kon-nach)+4;
  n2=n-2;


  diag= new float[n];
  for(i=0;i<n;i++) diag[i]=ur_smooth;


  ret_val=factoriz_smooth(&a,&a1,&h,&diag_h_tr,diag,&indx,&znak,n2,n);
  if(ret_val!=0) {goto m1;}
  fvs= new float[n2];
  fun_vs= new float[n];

  for(i=0;i<kol_la-1;i++)
  {
    //   i=0;
    int dop=(i+sdvig)%(kol_la-1);
    pvs_fun=&fun_vs[2];
    for(j=nach;j<kon;j++) pvs_fun[j-nach]=(*fun)[j*kol_la+i];
    if(flag1) {fun_vs[1]=(*fun)[kol_la+dop]; fun_vs[0]=(*fun)[2*kol_la+dop];}
    else   {fun_vs[1]=(*fun)[kol_la*nach+dop]; fun_vs[0]=(*fun)[kol_la*(nach+1)+dop];}
    if(flag2) {fun_vs[n-2]=(*fun)[kol_la*(kon-2)+dop]; fun_vs[n-1]=(*fun)[kol_la*(kon-3)+dop];}
    else   {fun_vs[n-2]=(*fun)[kol_la*(kon-1)+dop]; fun_vs[n-1]=(*fun)[kol_la*(kon-2)+dop];}

    BandAny_Prod(h,n2,n,0,2,3,
                 fun_vs,n,1,
                 fvs,n2,1);
    ret_val=ReshBand(a,n2,2,2,5,a1,indx,fvs);
    if(ret_val!=0) {goto m1;}

    BandAny_Prod(diag_h_tr,n,n2,2,0,3,
                 fvs,n2,1,
                 fun_vs,n,1);

    for(j=nach;j<kon;j++) (*fun)[j*kol_la+i] = (*fun)[j*kol_la+i]-pvs_fun[j-nach];
  }

  for(j=nach;j<kon;j++) (*fun)[j*kol_la+kol_la-1]=(*fun)[j*kol_la];



m1:
  if(a!=nullptr) {delete []a; a=nullptr;}
  if(a1!=nullptr) {delete []a1; a1=nullptr;}
  if(h!=nullptr) {delete []h; h=nullptr;}
  if(diag_h_tr!=nullptr) {delete []diag_h_tr; diag_h_tr=nullptr;}
  if(diag!=nullptr) {delete []diag; diag=nullptr;}
  if(indx!=nullptr) {delete []indx; indx=nullptr;}
  if(fvs!=nullptr) {delete []fvs; fvs=nullptr;}
  if(fun_vs!=nullptr) {delete []fun_vs; fun_vs=nullptr;}

  return ret_val;
}


int oprCoefparamSpline(double *x, double *y, double *pr2x, double *pr2y, int n)
{
  int ret_val=0;
  double *a=nullptr;
  double *a1=nullptr;
  double *h=nullptr;
  double *fvs=nullptr;
  int *indx=nullptr;
  int znak;
  int i, n2;

  pr2x[0]=0.;
  pr2x[n-1]=0.;
  pr2y[0]=0.;
  pr2y[n-1]=0.;

  n2=n-2;
  fvs= new double[n2];

  ret_val=factoriz(&a,&a1,&h,&indx,&znak,n2);
  if(ret_val!=0) goto m1;

  BandSpecialAny_Prod(h,x,fvs,n2,n,1,3);
  ret_val=ReshBand(a,n2,1,1,3,a1,indx,fvs);
  if(ret_val!=0) goto m1;
  for(i=0;i<n2;i++) pr2x[i+1]=fvs[i];

  BandSpecialAny_Prod(h,y,fvs,n2,n,1,3);
  ret_val=ReshBand(a,n2,1,1,3,a1,indx,fvs);
  if(ret_val!=0) goto m1;
  for(i=0;i<n2;i++) pr2y[i+1]=fvs[i];

m1:
  if(a!=nullptr) {delete []a; a=nullptr;}
  if(a1!=nullptr) {delete []a1; a1=nullptr;}
  if(h!=nullptr) {delete []h; h=nullptr;}
  if(indx!=nullptr) {delete []indx; indx=nullptr;}
  if(fvs!=nullptr) {delete []fvs; fvs=nullptr;}

  return ret_val;
}

int oprCoefparamSpline(float *x, float *y, float *pr2x, float *pr2y, int n)
{
  int ret_val=0;
  float *a=nullptr;
  float *a1=nullptr;
  float *h=nullptr;
  float *fvs=nullptr;
  int *indx=nullptr;
  int znak;
  int i, n2;

  pr2x[0]=0.;
  pr2x[n-1]=0.;
  pr2y[0]=0.;
  pr2y[n-1]=0.;

  n2=n-2;
  fvs= new float[n2];

  ret_val=factoriz(&a,&a1,&h,&indx,&znak,n2);
  if(ret_val!=0) goto m1;

  BandSpecialAny_Prod(h,x,fvs,n2,n,1,3);
  ret_val=ReshBand(a,n2,1,1,3,a1,indx,fvs);
  if(ret_val!=0) goto m1;
  for(i=0;i<n2;i++) pr2x[i+1]=fvs[i];

  BandSpecialAny_Prod(h,y,fvs,n2,n,1,3);
  ret_val=ReshBand(a,n2,1,1,3,a1,indx,fvs);
  if(ret_val!=0) goto m1;
  for(i=0;i<n2;i++) pr2y[i+1]=fvs[i];

m1:
  if(a!=nullptr) {delete []a; a=nullptr;}
  if(a1!=nullptr) {delete []a1; a1=nullptr;}
  if(h!=nullptr) {delete []h; h=nullptr;}
  if(indx!=nullptr) {delete []indx; indx=nullptr;}
  if(fvs!=nullptr) {delete []fvs; fvs=nullptr;}

  return ret_val;
}

int oprCoefparamSplineZam(float *x, float *y, float *pr2x, float *pr2y, int n)
{
  int ret_val=0;
  float *x_vs=nullptr;
  float *y_vs=nullptr;
  float *pr2x_vs=nullptr;
  float *pr2y_vs=nullptr;
  int i, nn, n1;
  int kol_dop = 3;
  n1=n-1;
  nn=n+ kol_dop*2;
  x_vs= new float[nn];
  y_vs= new float[nn];
  pr2x_vs= new float[nn];
  pr2y_vs= new float[nn];

  for(i=0;i<n;i++)
  {
    x_vs[kol_dop+i]=x[i];
    y_vs[kol_dop+i]=y[i];
  }
  for(i=0;i<kol_dop;i++)
  {
    x_vs[nn-kol_dop+i]=x[i+1];
    y_vs[nn-kol_dop+i]=y[i+1];
    x_vs[i]=x[n1-kol_dop+i];
    y_vs[i]=y[n1-kol_dop+i];
  }
  ret_val=oprCoefparamSpline(x_vs,y_vs,pr2x_vs,pr2y_vs,nn);
  if(ret_val==0) {

    for(i=0;i<n;i++)
    {
      pr2x[i]=pr2x_vs[kol_dop+i];
      pr2y[i]=pr2y_vs[kol_dop+i];
    }
  }

  if(x_vs!=nullptr) {delete []x_vs; x_vs=nullptr;}
  if(y_vs!=nullptr) {delete []y_vs; y_vs=nullptr;}
  if(pr2x_vs!=nullptr) {delete []pr2x_vs; pr2x_vs=nullptr;}
  if(pr2y_vs!=nullptr) {delete []pr2y_vs; pr2y_vs=nullptr;}

  return ret_val;
}

int raspredPoint(float *old_koord, int *kol_point, int old_kol, int new_kol)
{
  int ret_val=0;
  float *r=nullptr;
  int *num=nullptr;
  float r_all=0.;
  int i, n, kol_vs;

  n = old_kol-1;
  r = new float[n];

  for(i=0;i<n;i++) kol_point[i]=1;
  int kolkol=new_kol-old_kol-1;

  for(i=0;i<n;i++)
  {
    float x1 = old_koord[i*2];
    float x2 = old_koord[(i+1)*2];
    float y1 = old_koord[i*2+1];
    float y2 = old_koord[(i+1)*2+1];
    r[i]=(x2-x1)*(x2-x1);
    r[i]+=(y2-y1)*(y2-y1);
    r[i]=sqrt(r[i]);
    r_all+=r[i];
  }

  float dr_all =0;
  if(MnMath::isZero( r_all )) {ret_val=-2; goto m1;}
  dr_all = kolkol/r_all;
  for(i=0;i<n;i++){
    int kol_dop = MnMath::ftoi_norm(r[i]*dr_all);
    kol_point[i]+=kol_dop;
  }

  kol_vs=0;
  for(i=0;i<n;i++) kol_vs+=kol_point[i];
  if(kol_vs==(new_kol-1)) goto m1;

  num= new int[n];
  for(i=0;i<n;i++) num[i]=i;
  MnObanal::sort2(r,num,n);

  i=-1;
  while(kol_vs>(new_kol-1))
  {
    if(i<0) i=n-1;
    if(kol_point[num[i]]>1) {kol_point[num[i]]--; kol_vs--;}
    i--;
  }

  i=-1;
  while(kol_vs<(new_kol-1))
  {
    if(i<0) i=n-1;
    kol_point[num[i]]++; kol_vs++;
    i--;
  }

m1:
  if(num!=nullptr) {delete []num; num=nullptr;}
  if(r!=nullptr) {delete []r; r=nullptr;}

  return ret_val;
}

void uprost(float *old_koord,int old_kol, float **new_koord,  int *new_kol)
{
  if( old_kol < 3 ) return;
  *new_kol = 0;
  bool *mask = new bool [old_kol];
  bzero(mask,sizeof (bool)*old_kol);
  mask[0] = true;
  for ( int i= 0; i < old_kol - 3; i+=1) {
    float x1 = old_koord[i*2];
    float y1 = old_koord[i*2+1];
    float x2 = old_koord[(i+1)*2];
    float y2 = old_koord[(i+1)*2+1];
    float x3 = old_koord[(i+2)*2];
    float y3 = old_koord[(i+2)*2+1];
    float dx1 = x1 - x2;
    float dy1 = y1 - y2; //y1 - y2
    float dx2 = x3 - x2;//x3 - x2
    float dy2 = y3 - y2;//y3 - y2
    float a = dx1*dy2 - dy1*dx2;
    float b = dx1*dx2 + dy1*dy2;
    float q = atan2f(a,b)*(180.f/M_PIf);
    if(fabs(q) < 170. ) {
      mask[i+1] = true;
    } else {
      mask[i+2] = true;
      i++;
      //   debug_log <<"----"<<i<<  q;
    }
  }
  mask[old_kol-1] = true;
  mask[old_kol-2] = true;
  for( int i= 0; i < old_kol; i+=1){
    if(mask[i]) (*new_kol)++;
  }

  (*new_koord) = new float [*new_kol*2];
  int kk=0;
  for ( int i = 0; i < old_kol; i+=1) {
    if(mask[i]) {
      (*new_koord)[kk*2] = old_koord[i*2];
      (*new_koord)[kk*2+1] = old_koord[i*2+1];
      ++kk;
    }
  }
}



void uprost_po_ugol(float *old_koord,int old_kol, float **new_koord,  int *new_kol)
{
  if( old_kol < 3 ) return;
  *new_kol = 0;
  bool *mask = new bool [old_kol];
  bzero(mask,sizeof (bool)*old_kol);
  mask[0] = true;
  for ( int i= 0; i < old_kol - 3; i+=1) {
    float x1 = old_koord[i*2];
    float y1 = old_koord[i*2+1];
    float x2 = old_koord[(i+1)*2];
    float y2 = old_koord[(i+1)*2+1];
    float x3 = old_koord[(i+2)*2];
    float y3 = old_koord[(i+2)*2+1];
    float dx1 = x1 - x2;
    float dy1 = y1 - y2; //y1 - y2
    float dx2 = x3 - x2;//x3 - x2
    float dy2 = y3 - y2;//y3 - y2
    float a = dx1*dy2 - dy1*dx2;
    float b = dx1*dx2 + dy1*dy2;
    float q = atan2f(a,b)*(180.f/M_PIf);
    if(fabs(q) < 170. ) {
      mask[i+1] = true;
    } else {
      mask[i+2] = true;
      i++;
      //   debug_log <<"----"<<i<<  q;
    }
  }
  mask[old_kol-1] = true;
  mask[old_kol-2] = true;
  for( int i= 0; i < old_kol; i+=1){
    if(mask[i]) (*new_kol)++;
  }

  (*new_koord) = new float [*new_kol*2];
  int kk=0;
  for ( int i = 0; i < old_kol; i+=1) {
    if(mask[i]) {
      (*new_koord)[kk*2] = old_koord[i*2];
      (*new_koord)[kk*2+1] = old_koord[i*2+1];
      ++kk;
    }
  }
}

int  smootchPryam(float *old_koord, float *new_koord, int new_kol)
{
  float dt=1./float(new_kol-1), t;
  int i;

  for(i=0;i<new_kol;i++)
  {
    t=dt*i;
    new_koord[i*2]=(1.-t)*old_koord[0]+t*old_koord[2];
    new_koord[i*2+1]=(1.-t)*old_koord[1]+t*old_koord[3];
  }
  return 0;
}

int  smootchParab(float *old_koord, float *new_koord, int old_kol, int new_kol)
{
  int ret_val=0;
  int i, j, n=old_kol, n1;
  float dt, t, pr2x[3], pr2y[3], old_x[3], old_y[3];
  int kol[2];
  int count, kol_count;

  n1=n-1;
  ret_val=raspredPoint(old_koord,kol,old_kol,new_kol);
  if(ret_val!=0) goto m1;

  for(i=0;i<n;i++)
  {
    old_x[i]=old_koord[i*2];
    old_y[i]=old_koord[i*2+1];
    pr2x[i]=0.;
    pr2y[i]=0.;
  }
  pr2x[1]=old_x[0]+old_x[2]-2.*old_x[1];
  pr2y[1]=old_y[0]+old_y[2]-2.*old_y[1];

  count=0;
  for(i=0;i<n1;i++)
  {
    kol_count=kol[i];
    dt=1./double(kol_count);
    for(j=0;j<kol_count;j++)
    {
      t=dt*j;
      new_koord[count*2]=oprFunSplineEasy(pr2x,old_x,i,t);
      new_koord[count*2+1]=oprFunSplineEasy(pr2y,old_y,i,t);
      count++;
    }
  }
  new_koord[count*2]=old_x[n1];
  new_koord[count*2+1]=old_y[n1];

m1:
  return ret_val;
}

int  smootchSpline(float *old_koord, float *new_koord, int old_kol, int new_kol)
{
  int ret_val=0;
  int i, j, n=old_kol, n1;
  float dt, t;
  float *pr2x=nullptr;
  float *pr2y=nullptr;
  float *old_x=nullptr;
  float *old_y=nullptr;
  int *kol=nullptr;
  int count=0, kol_count=0;


  n1=n-1;
  kol= new int[n1];
  ret_val=raspredPoint(old_koord,kol,old_kol,new_kol);
  if(ret_val!=0) goto m1;
  old_x= new float[n];
  old_y= new float[n];
  pr2x= new float[n];
  pr2y= new float[n];

  for(i=0;i<n;i++)
  {
    //old_x[i]=new_koord_vs[i*2];
    // old_y[i]=new_koord_vs[i*2+1];
    old_x[i]=old_koord[i*2];
    old_y[i]=old_koord[i*2+1];
    pr2x[i]=0.;
    pr2y[i]=0.;
  }


  if(MnMath::isEqual(old_x[0],old_x[n1])&&MnMath::isEqual(old_y[0],old_y[n1])) {
    ret_val=oprCoefparamSplineZam(old_x,old_y,pr2x,pr2y,n);
  }
  else {
    ret_val=oprCoefparamSpline(old_x,old_y,pr2x,pr2y,n);
  }
  if(ret_val!=0) goto m1;

  count=0;
  for(i=0;i<n1;i++)
  {
    kol_count=kol[i];
    dt=1./float(kol_count);
    for(j=0;j<kol_count;j++)
    {
      t=dt*j;
      new_koord[count*2] = oprFunSplineEasy(pr2x,old_x,i,t);
      new_koord[count*2+1] = oprFunSplineEasy(pr2y,old_y,i,t);
      count++;
    }
  }
  new_koord[count*2]=old_x[n1];
  new_koord[count*2+1]=old_y[n1];

m1:
  if(kol!=nullptr) {delete []kol; kol=nullptr;}
  if(old_x!=nullptr) {delete []old_x; old_x=nullptr;}
  if(old_y!=nullptr) {delete []old_y; old_y=nullptr;}
  if(pr2x!=nullptr) {delete []pr2x; pr2x=nullptr;}
  if(pr2y!=nullptr) {delete []pr2y; pr2y=nullptr;}

  return ret_val;
}

int smootchCatmull(float *old_koord, float *new_koord, int old_kol, int new_kol)
{
  std::vector<std::array<float, 2>> points_intr;

  for(int i=0;i<old_kol;i++)
  {
    points_intr.push_back({old_koord[i*2],old_koord[i*2+1]});
  }
  bool isclosed = false;
  if(abs(points_intr.front()[0] - points_intr.back()[0]) <  std::numeric_limits<float>::epsilon()&&
     (abs(points_intr.front()[1] - points_intr.back()[1]) <  std::numeric_limits<float>::epsilon()))
  {
    isclosed = true;
    points_intr.pop_back();
  }
  interpol::catmull_rom<std::array<float, 2>> cr(std::move(points_intr),isclosed);
  cr.get_points();
  int n=old_kol;
  int *kol=nullptr;
  int n1 = n-1;
  kol= new int[n1];
  int ret_val=raspredPoint(old_koord,kol,old_kol,new_kol);
  if(ret_val!=0) {
    delete [] kol;
    return ret_val;
  }

  int count=0;
  std::array<float, 2> inter_points;

  for(int i=0;i<n1;i++)
  {
    int kol_count=kol[i];
    float dt=1./float(kol_count);
    for(int j=0;j<kol_count;j++)
    {
      float t=dt*j;
      float param = (1.-t)*cr.parameter_at_point(i)+t*cr.parameter_at_point(i+1);
      inter_points = cr(param);
      new_koord[count*2] = inter_points[0];
      new_koord[count*2+1] = inter_points[1];
      count++;
    }
  }
 /* new_koord[(new_kol-1)*2] = old_koord[(old_kol-1)*2];
  new_koord[(new_kol-1)*2+1] = old_koord[(old_kol-1)*2+1];
*/


  if( !MnMath::isEqual(new_koord[(new_kol-1)*2],new_koord[0]) ||
      !MnMath::isEqual(new_koord[(new_kol-1)*2+1],new_koord[1])){
    new_koord[(new_kol-1)*2] = old_koord[(old_kol-1)*2];
    new_koord[(new_kol-1)*2+1] = old_koord[(old_kol-1)*2+1];
  }

  delete [] kol;
  return 0;
}


bool prorejPoint(const QPolygon &ar_old, QPolygon *ar_new, int )
{
  double r_all=0.;
  int n = ar_old.size();
  QVector<double> r;
  r.resize(n);

  for(int i = 0; i < n-1; i++){
    r[i]=MnMath::distance(ar_old.at(i),ar_old.at(i+1));
    r_all += r[i];
  }
  if(ar_old.first() == ar_old.last()){
    r[n-1]=MnMath::distance(ar_old.first(),ar_old.last());
    r_all += r[n-1];
  }

  if( MnMath::isZero( r_all )) { return false; }
  if(ar_old.first() != ar_old.last()){
    ar_new->append(ar_old.first());
  }

  double dr_all = (n)/r_all;
  int koldist = 1;
  for(int i = 1; i < n ; i++){
    double dist = 0;
    for ( int j = 0; j < koldist; ++j ) {
      dist += r[i-1-j];
    }
    int kol_dop = MnMath::ftoi_norm(dist*dr_all);
    if(0 != kol_dop) {
      ar_new->append(ar_old.at(i));
      koldist = 1;
    }
    else {
      ++koldist;
    }
  }
  if(ar_old.first() != ar_old.last()){
    ar_new->append(ar_old.last());
  }

  if ( ar_old.first()==ar_old.last() && ar_new->first() != ar_new->last() ) {
    int kol_dop =  MnMath::ftoi_norm(dr_all*MnMath::distance(ar_new->last(), ar_new->first()));
    if (0 == kol_dop){
      ar_new->removeLast();
    }
    ar_new->append(ar_new->first());
  }

  return true;
}

int  smootchLine(const QPolygon &ar_old, QPolygon *ar_new, int sfactor)
{
  if(0 >= sfactor){
    *ar_new = ar_old;
    return 0;
  }
  float *old_koord=nullptr;
  float *new_koord=nullptr;
  int ret_val=0;
  int i;
  //QPolygon ar_new_vs=ar_old;

  int old_kol=ar_old.size();
  if(old_kol<2) return -1;

  int new_kol=old_kol*sfactor;

  old_koord= new float[old_kol*2];
  new_koord= new float[new_kol*2];

  for (i=0;i<old_kol;i++) {
    old_koord[i*2]=ar_old.at(i).x();
    old_koord[i*2+1]=ar_old.at(i).y();
  }

  ret_val=smootchLineDbl(old_koord,new_koord,old_kol,new_kol);
  if(ret_val!=0) goto m1;

  ar_new->resize(new_kol);
  for(i=0;i<new_kol;i++) ar_new->setPoint(i,MnMath::ftoi_norm(new_koord[i*2]),MnMath::ftoi_norm(new_koord[i*2+1]));

m1:
  if(old_koord!=nullptr) {delete []old_koord; old_koord=nullptr;}
  if(new_koord!=nullptr) {delete []new_koord; new_koord=nullptr;}

  return ret_val;
}

int  smootchLineDbl(float *old_koord, float *new_koord, int old_kol, int new_kol)
{
  int ret_val=0;

  if(new_kol<old_kol) return -1;

  switch(old_kol)
  {
    case 0:
    case 1:
      ret_val=-1;
    break;
    case 2:
      ret_val=smootchPryam(old_koord,new_koord,new_kol);
    break;
    case 3:
      ret_val=smootchParab(old_koord,new_koord,old_kol,new_kol);
    break;
    case 4:
      ret_val=smootchSpline(old_koord,new_koord,old_kol,new_kol);
    break;
    default :
      ret_val=smootchCatmull(old_koord,new_koord,old_kol,new_kol);
      //ret_val=smootchSpline(old_koord,new_koord,old_kol,new_kol);
    break;
  }
  return ret_val;
}

////////////////////////////////////////////////////////////////////////////


int smootchEasyFun(QVector<float> *data, const QVector<bool> &mask,
                   const QVector<float> &net_fi_vs,
                   int kol_fi, int kol_la, double ur_smootch)
{
  int ret_val=0;
  int i_nach, i_kon, j;

  for(i_nach=0;i_nach<kol_fi;i_nach++) {
    for(j=0;j<kol_la;j++) {
      if(mask[i_nach*kol_la+j]) {goto m1;}
    }
  }
m1:
  for(i_kon=kol_fi-1;-1<i_kon;i_kon--) {
    for(j=0;j<kol_la;j++) {
      if(mask[i_kon*kol_la+j]) {
        goto m2;
      }
    }
  }
m2:
  if((i_kon-i_nach)<4) {
    return ret_val;
  }

  ret_val=smootchLong(data,net_fi_vs,kol_la,i_nach,i_kon+1,ur_smootch);
  if(ret_val!=0) {
    return ret_val;
  }

  ret_val=smootchLat(data,net_fi_vs,kol_la,i_nach,i_kon+1,ur_smootch);

  return ret_val;
}


////////////////////////////////////////////////////////////////////////////


