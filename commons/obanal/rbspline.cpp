
#include "rbspline.h"
#include <cmath>
#include <float.h>
#include <stdio.h>
#include <QDebug>

#if defined WIN32

void bzero (void *to, size_t count) { memset (to, 0, count); }

#endif 


void rbasis(int c,float t,int npts,int *x,float *h,float *r)

{
  int nplusc;
  int i,k;
  float d,e;
  float sum;
  
  nplusc = npts + c;
  
  
  float *temp= new float [nplusc];
  
  for (i = 1; i<= nplusc-1; i++){
    if (( t >= x[i]) && (t < x[i+1]))
      temp[i] = 1;
    else
      temp[i] = 0;
  }
  
  
  for (k = 2; k <= c; k++){
    for (i = 1; i <= nplusc-k; i++){
      if (temp[i] != 0)    /* if the lower order basis function is zero skip the calculation */
        d = ((t-x[i])*temp[i])/(x[i+k-1]-x[i]);
      else
        d = 0;
      if (temp[i+1] != 0)     /* if the lower order basis function is zero skip the calculation */
        e = ((x[i+k]-t)*temp[i+1])/(x[i+k]-x[i+1]);
      else
        e = 0;
      
      temp[i] = d + e;
    }
  }
  
  if (t == (float)x[nplusc]){     /*    pick up last point    */
    temp[npts] = 1;
  }
  sum = 0.;
  for (i = 1; i <= npts; i++){
    sum = sum + temp[i]*h[i];
  }
  
  for (i = 1; i <= npts; i++){
    if (sum != 0){
      r[i] = (temp[i]*h[i])/(sum);}
      else
        r[i] = 0;
  }
  delete []temp;
  
}

void rbspline(int npts,int k, int p1, float *b,float *h,float *p, int dim)
{
	int i,j,icount,jcount;
	int i1;
	int nplusc;

	float step;
	float t;
	float temp;
    
    nplusc = npts + k;
    int *x = new int [nplusc+1];     
    float *nbasis = new float [npts+1];
    bzero(x,sizeof(int)*nplusc+1);
    bzero(nbasis,sizeof(float)*npts+1);
    
	knot(npts,k,x);
	icount = 0;
	t = 0;
	step = ((float)x[nplusc])/((float)(p1-1));

	for (i1 = 1; i1<= p1; i1++){

		if ((float)x[nplusc] - t < 5e-6){
			t = (float)x[nplusc];
		}

	    rbasis(k,t,npts,x,h,nbasis);    

        for (j = 1; j <= dim; j++){      
			jcount = j;
			p[icount+j] = 0.;

			for (i = 1; i <= npts; i++){ 
				temp = nbasis[i]*b[jcount];
			    p[icount + j] = p[icount + j] + temp;
                jcount = jcount + dim;
			}
		}

		icount = icount + dim;
		t = t + step;
	}
	
	delete []x;
    delete []nbasis;
}

void knot(int n, int c,int *x)

{
  int nplusc,nplus2,i;
  
  nplusc = n + c;
  nplus2 = n + 2;
  x[1] = 0;
  for (i = 2; i <= nplusc; i++){
    if ( (i > c) && (i < nplus2) )
      x[i] = x[i-1] + 1;
    else
      x[i] = x[i-1];
  }
}


//сплайн Акимы
void calcspline( const QPolygonF &vxy, QPolygonF *res, double d)
{
if (vxy.size() < 4)  return;

QPolygonF vs=vxy;

int n = vxy.size();

double dl = 0.;


vs.prepend(vxy.at(1));
vs.prepend(vxy.at(0));
vs.append(vxy.at(1));
vs.append(vxy.at(0));

n = vs.size();

QVector<double> dx(n), dy(n);         
QVector<double> mx(n), my(n);             
for (int i=2; i < n-3; i++) {
  dx[i] = vs.at(i+1).x() -vs.at(i).x(); 
  dy[i] = vs.at(i+1).y() -vs.at(i).y();
  dl += sqrt(dx[i]*dx[i]+dy[i]*dy[i]);
  my[i] = dy[i];
  mx[i] = dx[i];
 // qDebug() << dx[i];
}

//qDebug() << "***************";

if(vxy.first() != vxy.last()){
  mx[0]   = 3.*mx[2]-2.*mx[3];
  mx[1]   = 2.*mx[2]-2.*mx[3];
  mx[n-2] = 2.*mx[n-3]-2.*mx[n-4];
  mx[n-1]   = 3.*mx[n-3]-2.*mx[n-4];

  my[0]   = 3.*my[2]-2.*my[3];
  my[1]   = 2.*my[2]-2.*my[3];
  my[n-2] = 2.*my[n-3]-2.*my[n-4];
  my[n-1]   = 3.*my[n-3]-2.*my[n-4];
} else {
  mx[0]   = mx[n-4];
  mx[1]   = 2.*mx[n-3];
  mx[n-2] = 2.*mx[2];
  mx[n-1] = 3.*mx[3];

  my[0]   = my[n-4];
  my[1]   = 2.*my[n-3];
  my[n-2] = 2.*my[2];
  my[n-1] = 3.*my[3];
}

QVector<double> xtl(n), xtr(n);  
QVector<double> ytl(n), ytr(n);  
ytl[0]=0.0; ytl[1]=0.0;
ytr[0]=0.0; ytr[1]=0.0;
xtl[0]=0.0; xtl[1]=0.0;
xtr[0]=0.0; xtr[1]=0.0;
for (int i = 2; i < n-1; i++) {
  double mm1 = fabs( mx[i+1] - mx[i] );
  double mm2 = fabs( mx[i-1] - mx[i-2] );
  
  double ne = mm1 + mm2;
  if(std::fpclassify(ne) == FP_ZERO ){
    xtl[i] = mx[i-1];
    xtr[i] = mx[i];
  } else {
    xtl[i] = mx[i-1] + (mx[i]-mx[i-1])*mm2/ne;
    xtr[i] = xtl[i];
  }
  
  mm1 = fabs( my[i+1] - my[i] );
  mm2 = fabs( my[i-1] - my[i-2] );
  ne = mm1 + mm2;
  if(std::fpclassify(ne) == FP_ZERO ){
    ytl[i] = my[i-1];
    ytr[i] = my[i];
  } else {
    ytl[i] = my[i-1] + (my[i]-my[i-1])*mm2/ne;
    ytr[i] = ytl[i];
  }
}

QVector<double> Cy(n), Dy(n);             /* the coefficients */
QVector<double> Cx(n), Dx(n);             /* the coefficients */
double ddd = ((n-4)*d)/dl;

for (int i = 2; i < n-3; i+=1) {
  Cx[i] = (3.*mx[i] - 2.*xtr[i] - xtl[i+1]);
  Dx[i] = (xtr[i] + xtl[i+1] - 2.*mx[i]);
  Cy[i] = (3.*my[i] - 2.*ytr[i] - ytl[i+1]);
  Dy[i] = (ytr[i] + ytl[i+1] - 2.*my[i]);
  double r = ddd * sqrt(dx[i]*dx[i]+dy[i]*dy[i]);
  d = r;
  double xstep = 1.;
  if(d != 0) xstep = 1. /d;

  for(double xd = 0.; xd < 1. ; xd += xstep){
    double xv = vs.at(i).x() + (xtr[i] + (Cx[i] + Dx[i]*xd)*xd)*xd;
    double yv = vs.at(i).y() + (ytr[i] + (Cy[i] + Dy[i]*xd)*xd)*xd;
    res->append(QPointF(xv,yv));
  }
}
res->append(vs.at(n-3));

}

