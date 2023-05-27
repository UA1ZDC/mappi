#include <stdlib.h>

#include <QVector>
#include <QtDebug>
#include <qglobal.h>


#include "linalgebra.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmisleading-indentation"

inline int n2(int i, int j, int n){
  return i*n+j;
}

int Num_Elem(int i, int j)
{
  int k;
  if (i > j) {
      k=i;
      i=j;
      j=k;
    }

  return j*(j+1)/2 + i;
}


void Basis(double *x, double *u, double *z, double *w, int n)
{
  int n1=n-3, m = 0, l, i, ii, ii1;
  double a, b, c, d, vs, vs1;
  i=0;
  l=0;
  do {
      ii=i*2;
      ii1=i*3;
      vs=x[ii+2]-x[ii];
      vs1=x[ii+3]-x[ii+1];
      if (fabs(vs)>fabs(vs1)) {
          vs=1.f/vs;
          a=(x[ii+4]-x[ii])*vs;
          b=(x[ii+6]-x[ii])*vs;
          d=x[ii+5]-x[ii+1]-vs1*a;
          c=x[ii+7]-x[ii+1]-vs1*b;
        }
      else {
          vs1=1.f/vs1;
          a=(x[ii+5]-x[ii+1])*vs1;
          b=(x[ii+7]-x[ii+1])*vs1;
          d=x[ii+4]-x[ii]-vs*a;
          c=x[ii+6]-x[ii]-vs*b;
        }
      //       if (fabs(d)>eps) {
      if(false == MnMath::isZero(d)){
          c/=d;
          u[ii1]=-1.f+c+b-a*c;
          u[ii1+1]=a*c-b;
          u[ii1+2]=-c;
          ++i;
        }
      //else if (fabs(c)<eps) {
      else if (MnMath::isZero(c)) {
          u[ii1]=-1.f+b;
          u[ii1+1]=-b;
          u[ii1+2]=0;
          m=i+3;
          if (l==0) l=i;
          ++i;
        }
      else {
          if (l==0) {
              l=i;
              m=i+2;
            }
          ii=l+2;
          ii1=i+3;
          a=z[ii];
          b=w[ii];
          z[ii]=z[ii1];
          w[ii]=w[ii1];
          z[ii1]=a;
          w[ii1]=b;
          ii*=2;
          ii1*=2;
          a=x[ii];
          b=x[ii+1];
          x[ii]=x[ii1];
          x[ii+1]=x[ii1+1];
          x[ii1]=a;
          x[ii1+1]=b;
          i=l-1;
          if (i<0) i=0;
          l+=3;
          ++m;
          if ((m-l)<2) l=0;
        }
    } while (i<n1);
}
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
int Fac_Chol(double *a, int n)
{
  double *vspom = nullptr;
  double sum=0;
  // Копируем матрицу
  int k=n*(n+1)/2;
  vspom = new double [k];

  for(int i=0; i<k; i++) {
      vspom[i]=a[i];
    }

  // Факторизуем матрицу
  for(int i=0; i<n; i++)
    {
      int p=Num_Elem(i,i);
      for (int j=i; j<n; j++)
        {
          int s=Num_Elem(i,j);
          sum = vspom[s];
          for (k=i-1; -1<k; k--){
              sum -= a[Num_Elem(k,i)]*a[Num_Elem(k,j)];
            }
          if(i==j) {
              //if( sum<=eps) {
              if(sum<0) {
                  delete []vspom;
                  qDebug(">>>>>>%1.15f",sum);
                  fprintf(stderr,"Sum=%g  i=%d\n",sum, i);
                  return 2;
                } // Матрица не положительно определенная
              a[p]=sqrtl(sum);
            }
          else {
                a[s]=sum/a[p];;
            }
        }
    }
  delete []vspom;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
void Resh_Chol(double *a, double *b, double *x, int n)
{
  int i, j;
  double sum;
  // Прямой ход
  for (i=0; i<n; ++i)
    {
      sum=b[i];
      for (j=i-1; -1<j; j--) sum-=a[Num_Elem(i,j)]*x[j];
      x[i]=sum/a[Num_Elem(i,i)];
   //   debug_log<<i<< x[i]<< sum<<a[Num_Elem(i,i)];
    }
  // Обратный ход
  for (i=n-1; -1<i; i--)
    {
      sum=x[i];
      for (j=i+1; j<n; ++j) sum-=a[Num_Elem(i,j)]*x[j];
      x[i]=sum/a[Num_Elem(i,i)];
    }
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void BasisDiff(double *x, double *u, double *z, double *w, int *type, int n)
{
  int n1=n-3, m = 0, l, i, ii, ii1, type_vs;
  double a, b, c, d, vs, vs1;
  i=0;
  l=0;
  do {
      ii=i*2;
      ii1=i*3;
      vs=x[ii+2]-x[ii];
      vs1=x[ii+3]-x[ii+1];
      if (fabs(vs)>fabs(vs1)) {
          vs=1.f/vs;
          a=(x[ii+4]-x[ii])*vs;
          b=(x[ii+6]-x[ii])*vs;
          d=x[ii+5]-x[ii+1]-vs1*a;
          c=x[ii+7]-x[ii+1]-vs1*b;
        }
      else {
          vs1=1.f/vs1;
          a=(x[ii+5]-x[ii+1])*vs1;
          b=(x[ii+7]-x[ii+1])*vs1;
          d=x[ii+4]-x[ii]-vs*a;
          c=x[ii+6]-x[ii]-vs*b;
        }
      //if (fabs(d)>eps) {
      if (false == MnMath::isZero(d)) {
          c/=d;
          u[ii1]=-1.f+c+b-a*c;
          u[ii1+1]=a*c-b;
          u[ii1+2]=-c;
          ++i;
        }
//      else if (fabs(c)<eps) {
      else if (MnMath::isZero(c)) {
          u[ii1]=-1.f+b;
          u[ii1+1]=-b;
          u[ii1+2]=0;
          m=i+3;
          if (l==0) l=i;
          ++i;
        }
      else {
          if (l==0) {
              l=i;
              m=i+2;
            }
          ii=l+2;
          ii1=i+3;
          a=z[ii];
          b=w[ii];
          type_vs=type[ii];
          z[ii]=z[ii1];
          w[ii]=w[ii1];
          type[ii]=type[ii1];
          z[ii1]=a;
          w[ii1]=b;
          type[ii1]=type_vs;
          ii*=2;
          ii1*=2;
          a=x[ii];
          b=x[ii+1];
          x[ii]=x[ii1];
          x[ii+1]=x[ii1+1];
          x[ii1]=a;
          x[ii1+1]=b;
          i=l-1;
          if (i<0) i=0;
          l+=3;
          ++m;
          if ((m-l)<2) l=0;
        }
    } while (i<n1);
}

/////////////////////////////////////////////////////////////////////////////
int Diag_Inv(double *x, double *xinv, int n)
{
  int i;
  for (i=0; i<n; ++i) {
      if(MnMath::isZero(x[i])) {
          return 1; // Матрица особенная
        }
      else xinv[i]=1.f/x[i];
    }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
void Diag_Prod(double *x1, double *x2, double *xprod, int n)
{
  int i;
  for (i=0; i<n; ++i) xprod[i]=x1[i]*x2[i];
}

/////////////////////////////////////////////////////////////////////////////
double Scalar_Prod(double *x1, double *x2, int n)
{
  int i;
  double sum=0.;
  for (i=0; i<n; ++i) sum+=x1[i]*x2[i];
  return sum;
}

/////////////////////////////////////////////////////////////////////////////
void DiagAny_Prod(double *x1, double *x2, double *xprod, int n, int m)
{
  int i, j, i1;
  for (i=0; i<n; ++i)  {
      i1=i*m;
      for (j=0; j<m; ++j)
        xprod[i1+j]=x1[i]*x2[i1+j];

    }
}

/////////////////////////////////////////////////////////////////////////////
void DiagAnyPol_Prod(double *x1, double *x2, double *xprod, int n, int m)
{
  int i, j, i1;
  for (i=0; i<n; i++)  {
      i1=i*m;
      xprod[i1]=x1[i];
      for (j=0; j<m; j++) xprod[i1+j+1]=x1[i]*x2[i1+j];
    }
}

/////////////////////////////////////////////////////////////////////////////
void AnyDiag_Prod(double *x1, double *x2, double *xprod, int n, int m)
{
  int i, j, i1;
  for (i=0; i<n; i++)  {
      i1=i*m;
      for (j=0; j<m; j++) xprod[i1+j]=x1[i1+j]*x2[j];
    }
}

/////////////////////////////////////////////////////////////////////////////
void AnyPolDiag_Prod(double *x1, double *x2, double *xprod, int n, int m)
{
  int i, j, i1;
  for (i=0; i<n; i++)  {
      i1=i*m;
      xprod[i1]=x1[0];
      for (j=1; j<m; j++) xprod[i1+j]=x1[i1+j-1]*x2[j];
    }
}

/////////////////////////////////////////////////////////////////////////////
void Any_Prod(double *x1, double *x2, double *xprod, int n, int m, int k)
{
  int i, j, i1, i2, l;
  double sum;
  for (i=0; i<n; i++) {
      i1=i*m;
      i2=i*k;
      for (j=0; j<k; j++) {
          sum=0.;
          for (l=0; l<m; l++) sum+=x1[i1+l]*x2[l*k+j];
          xprod[i2+j]=sum;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
void AnyAnyT_Prod(double *a, double *xprod, int n, int m)
{
  int i, j, i1, i2, l;
  double sum;
  for (i=0; i<n; i++) {
      i1=i*m;
      for (j=i; j<n; j++) {
          sum=0.;
          i2=j*m;
          for (l=0; l<m; l++) sum+=a[i1+l]*a[i2+l];
          xprod[Num_Elem(i,j)]=sum;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
void SymVec_Prod(double *a,double *x, double *xprod, int n)
{
  int i, j;
  double sum;
  for (i=0; i<n; i++) {
      sum=0.;
      for (j=0; j<n; j++) sum+=a[Num_Elem(i,j)]*x[j];
      xprod[i]=sum;
    }
}

/////////////////////////////////////////////////////////////////////////////
int SymLVec_Prod(double *a,double *x, double *xprod, int n, int k)
{
  int i, j, i_n, i_v, k1;
  double sum;
  if (k>=n) return 2; // Неправильное значение k или n
  k1=k+1;
  for (i=0; i<n; i++) {
      i_n=i-k;
      if (i_n<0) i_n=0;
      i_v=i+k1;
      if (i_v>n) i_v=n;
      sum=0.;
      for (j=i_n; j<i_v; j++) {
          if (i>j) sum+=a[j*k1+i-j]*x[j];
          else sum+=a[i*k1+j-i]*x[j];
        }
      xprod[i]=sum;
    }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
void BasAny_Prod(double *x1, double *x2, double *xprod, int n, int m, int k)
{
  int i, j, i1, i2, l;
  double sum;
  k--;
  for (i=0; i<n; i++) {
      i1=i*k;
      i2=i*m;
      for (j=0; j<m; j++) {
          sum=x2[(i+k)*m+j];
          for (l=0; l<k; l++) sum+=x1[i1+l]*x2[(i+l)*m+j];
          xprod[i2+j]=sum;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
void BasTAny_Prod(double *x1, double *x2, double *xprod, int n, int m, int k)
{
  int i, j, i1, i2, l, n1, i3;
  double sum=0;
  k--;
  n1=n+k;
  for (i=0; i<k; i++) {
      i2=i*m;
      for (j=0; j<m; j++) {
          sum=x1[i]*x2[j];
          for (l=0; l<i; l++) {
              sum+=x1[(i-l)*k+l]*x2[(i-l)*m+j];
            }
          xprod[i2+j]=sum;
        }
    }
  for (i=k; i<n; i++) {
      i2=i*m;
      for (j=0; j<m; j++) {
          sum=x2[(i-k)*m+j];
          for (l=0; l<k; l++) {
              sum+=x1[(i-l)*k+l]*x2[(i-l)*m+j];
            }
          xprod[i2+j]=sum;
        }
    }
  for (i=n; i<n1; i++) {
      i1=n1-i;
      i3=i-k;
      i2=i*m;
      for (j=0; j<m; j++) {
          sum=x2[i3*m+j];
          for (l=1; l<i1; l++) {
              sum+=x1[(i3+l)*k+k-l]*x2[(i3+l)*m+j];
            }
          xprod[i2+j]=sum;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
int Matr_H(double *x1, double *x2, double *xprod, int n, int k)
{
  int i, j, m, i1, l;
  double sum, *vspom;
  k--;
  m=n+k;
  // Выделяем память под матрицу U*G
  vspom=(double *)calloc(m, sizeof(double));
  if (vspom == nullptr) return 1; // Ошибка распределения памяти
  // Находим vspom=U*G
  for (i=0; i<n; i++) {
      i1=i*k;
      for (j=0; j<m; j++) {
          sum=x2[Num_Elem(i+k,j)];
          for (l=0; l<k; l++) sum+=x1[i1+l]*x2[Num_Elem(i+l,j)];
          vspom[j]=sum;
        }
      // Находим xprod=vspom*(транспонированная U)
      for (j=i; j<n; j++) {
          i1=j*k;
          sum=vspom[j+k];
          for (l=0; l<k; l++) {
              sum+=x1[i1+l]*vspom[j+l];
              //debug_log<<sum<<x1[i1+l]<<vspom[j+l];
            }


          xprod[Num_Elem(i,j)]=sum;
        //  debug_log<<i<<j<<Num_Elem(i,j)<<xprod[Num_Elem(i,j)];
        }
    }
  free(vspom);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
int Matr_B(double *x1, double *x2, double *xprod, int n, int k)
{
  int i, j, i1, i2, l, k1, n1;
  double sum, *vspom;
  k1=k-1;
  //Выделяем память под расширенную матрицу базиса и находим ее произведение на G
  vspom=(double *)calloc(n*k, sizeof(double));
  if (vspom == nullptr) return 1; // Ошибка распределения памяти
  for (i=0; i<n; i++) {
      i1=i*k;
      i2=i*k1;
      for (j=0; j<k1; j++) vspom[i1+j]=x1[i2+j]*x2[i+j];
      vspom[i1+k1]=x2[i+k1];
    }
  // Находим конечную матрицу
  for (j=0; j<k; j++) {
      n1=n-j;
      for (i=0; i<n1; i++) {
          i1=i*k;
          sum=0.;
          for (l=j; l<k; l++) sum+=vspom[i1+l]*vspom[(i+j)*k+l-j];
          xprod[i1+j]=sum;
        }
    }
  free(vspom);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
void SymSym_Sum(double *a, double *b, double *xsum, double c, int n)
{
  int i, j, k;
  for (i=0; i<n; i++)  for (j=i; j<n; j++) {
      k=Num_Elem(i,j);
      xsum[k]=a[k]+c*b[k];
    }
}

/////////////////////////////////////////////////////////////////////////////
void SymDiag_Sum(double *a, double *b, double *xsum, double c, int n)
{
  int i, k;
  for (i=0; i<n; i++) {
      k=Num_Elem(i,i);
      xsum[k]=a[k]+c*b[i];
    }
}

/////////////////////////////////////////////////////////////////////////////
void Vec_Dif(double *a, double *b,  int n)
{
  int i;
  for (i=0; i<n; ++i) a[i]-=b[i];
}





/////////////////////////////////////////////////////////////////////////////
double Fun_Grin(double x1, double y1, double x2, double y2)
{
  double r, x,y;
  x=x1-x2;
  y=y1-y2;
  r=x*x+y*y;
//  if(r<eps) return 0.;
  if(MnMath::isZero(r)) return  0;
  else return 0.5f*r*logf(r);
}

double Fun_Grin1x(double x1, double y1, double x2, double y2)
{double r, x,y;
  x=x1-x2;
  y=y1-y2;
  r=x*x+y*y;
//  if(r<eps) return 0.;
  if(MnMath::isZero(r)) return  0;
  else return x*(logf(r)+1);
}

double Fun_Grin1y(double x1, double y1, double x2, double y2)
{double r, x,y;
  x=x1-x2;
  y=y1-y2;
  r=x*x+y*y;
  //if(r<eps) return 0.;
  if(MnMath::isZero(r)) return  0;
  return y*(logf(r)+1);
}

double Fun_Grin2x(double x1, double y1, double x2, double y2)
{double r, x,y;
  x=x1-x2;
  y=y1-y2;
  r=x*x+y*y;
//  if(r<eps) return 0.;
  if(MnMath::isZero(r)) return  0;
  return logf(r)+1+2*x/r;
}

double Fun_Grin2y(double x1, double y1, double x2, double y2)
{double r, x,y;
  x=x1-x2;
  y=y1-y2;
  r=x*x+y*y;
  //if(r<eps) return 0.;
  if(MnMath::isZero(r)) return  0;
  return logf(r)+1+2*y/r;
}

double Fun_Grin2xy(double x1, double y1, double x2, double y2)
{double r, x,y;
  x=x1-x2;
  y=y1-y2;
  r=x*x+y*y;
  if(MnMath::isZero(r)) return  0;
  return 2*x*y/r;
}


void funMatrG(double *x, int *type, double *matr_g, int n)
{
  int i,j,i1,i2;

  for(i=0; i<n; i++)
    {
      i1=i*2;
      for(j=0; j<n+3; j++)
        {
          i2=j*2;
          switch(type[i])
            {
            case 0:
              switch(type[j])
                {
                case 0: matr_g[i*(n+3)+j]=Fun_Grin(x[i1],x[i1+1],x[i2],x[i2+1]); break;
                case 1: matr_g[i*(n+3)+j]=Fun_Grin1x(x[i1],x[i1+1],x[i2],x[i2+1]); break;
                case 2: matr_g[i*(n+3)+j]=Fun_Grin1y(x[i1],x[i1+1],x[i2],x[i2+1]); break;
                default: break;
                }
              break;
            case 1:
              switch(type[j])
                {
                case 0: matr_g[i*(n+3)+j]=Fun_Grin1x(x[i1],x[i1+1],x[i2],x[i2+1]); break;
                case 1: matr_g[i*(n+3)+j]=Fun_Grin2x(x[i1],x[i1+1],x[i2],x[i2+1]); break;
                case 2: matr_g[i*(n+3)+j]=Fun_Grin2xy(x[i1],x[i1+1],x[i2],x[i2+1]); break;
                default: break;
                }
              break;
            case 2:
              switch(type[j])
                {
                case 0: matr_g[i*(n+3)+j]=Fun_Grin1y(x[i1],x[i1+1],x[i2],x[i2+1]); break;
                case 1: matr_g[i*(n+3)+j]=Fun_Grin2xy(x[i1],x[i1+1],x[i2],x[i2+1]); break;
                case 2: matr_g[i*(n+3)+j]=Fun_Grin2y(x[i1],x[i1+1],x[i2],x[i2+1]); break;
                default: break;
                }
              break;
            }
        }
    }



  for(i=0; i<n; i++)
    {
      i1=i*2;
      for(j=n; j<n+3; j++)
        {
          i2=j*2;
          switch(type[i])
            {
            case 0:
              {
                if(j==n) matr_g[i*(n+3)+j]=1.;
                if(j==(n+1)) matr_g[i*(n+3)+j]=x[i1];
                if(j==(n+2)) matr_g[i*(n+3)+j]=x[i1+1];
                break;
              }
            case 1:
              {
                if(j==n) matr_g[i*(n+3)+j]=0.;
                if(j==(n+1)) matr_g[i*(n+3)+j]=1.;
                if(j==(n+2)) matr_g[i*(n+3)+j]=0.;
                break;
              }
            case 2:
              {
                if(j==n) matr_g[i*(n+3)+j]=0.;
                if(j==(n+1)) matr_g[i*(n+3)+j]=0.;
                if(j==(n+2)) matr_g[i*(n+3)+j]=1.;
                break;
              }
            default: break;
            }
        }
    }

  for(i=n; i<n+3; i++)
    {
      i1=i*2;
      for(j=0; j<n; j++)
        {
          i2=j*2;
          switch(type[j])
            {
            case 0:
              {
                if(i==n) matr_g[i*(n+3)+j]=1.;
                if(i==(n+1)) matr_g[i*(n+3)+j]=x[i2];
                if(i==(n+2)) matr_g[i*(n+3)+j]=x[i2+1];
                break;
              }
            case 1:
              {
                if(i==n) matr_g[i*(n+3)+j]=0.;
                if(i==(n+1)) matr_g[i*(n+3)+j]=-1.;
                if(i==(n+2)) matr_g[i*(n+3)+j]=0.;
                break;
              }
            case 2:
              {
                if(i==n) matr_g[i*(n+3)+j]=0.;
                if(i==(n+1)) matr_g[i*(n+3)+j]=0.;
                if(i==(n+2)) matr_g[i*(n+3)+j]=-1.;
                break;
              }
            default: break;
            }
        }
    }



  for(i=n; i<n+3; i++)
    {
      i1=i*2;
      for(j=n; j<n; j++)
        {
          i2=j*2;
          matr_g[Num_Elem(i,j)]=0.;
        }
    }
}


void funMatrG(double *x, double *matr_g, int n)
{
  int i,j,i1,i2;

  for(i=0; i<n; i++)
    {
      i1=i*2;
      for(j=0; j<n+3; j++)
        {
          i2=j*2;
          matr_g[i*(n+3)+j]=Fun_Grin(x[i1],x[i1+1],x[i2],x[i2+1]);
        }
    }

  for(i=0; i<n; i++)
    {
      i1=i*2;
      for(j=n; j<n+3; j++)
        {
          i2=j*2;
          if(j==n) matr_g[i*(n+3)+j]=1.;
          if(j==(n+1)) matr_g[i*(n+3)+j]=x[i1];
          if(j==(n+2)) matr_g[i*(n+3)+j]=x[i1+1];
        }
    }

  for(i=n; i<n+3; i++)
    {
      i1=i*2;
      for(j=0; j<n; j++)
        {
          i2=j*2;
          if(i==n) matr_g[i*(n+3)+j]=1.;
          if(i==(n+1)) matr_g[i*(n+3)+j]=x[i2];
          if(i==(n+2)) matr_g[i*(n+3)+j]=x[i2+1];
        }
    }

  for(i=n; i<n+3; i++)
    {
      i1=i*2;
      for(j=n; j<n; j++)
        {
          i2=j*2;
          matr_g[Num_Elem(i,j)]=0.;
        }
    }
}

#pragma GCC diagnostic pop

