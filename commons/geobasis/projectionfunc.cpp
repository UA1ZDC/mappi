#include "projectionfunc.h"
#include "stereoproj.h"
#include "merkatproj.h"
#include "geovector.h"

using namespace MnMath;

namespace meteo {

  //!преобразование геодезических координат в плоские прямоугольные (проекция Гаусса-Крюгера, СК-95)
  void latlon2xy_gkgost( float lat, float lon, int n, float* x, float* y )
  {
    float l = (RAD2DEG*lon - (3.0+6.0*(n - 1) ) ) / 57.29577951;
    *x = 6367558.4968*lat - ::sinf(2.0*lat)*(16002.8900 + 66.9607*::powf(::sinf(lat), 2.0) + 0.3515*::powf( ::sinf(lat), 4.0 )
      - ::powf( l, 2.0 )*(1594561.25 + 5336.535*::powf( ::sinf(lat), 2.0) + 26.790*::powf( ::sinf(lat), 4.0 ) + 0.149*::powf( ::sinf(lat), 6.0 )
        + ::powf( l, 2.0 )*(672483.4 - 811219.9*::powf( ::sinf(lat), 2.0 ) + 5420.0*::powf( ::sinf(lat), 4.0 ) - 10.6*::powf( ::sinf(lat), 6.0 )
          + ::powf( l, 2.0 )*(278194.0 - 830174.0*::powf( ::sinf(lat), 2.0 ) + 572434.0*::powf( ::sinf(lat), 4.0 )
            - 16010.0*::powf( ::sinf(lat), 6.0 ) + ::powf( l, 2.0 )*(109500.0 - 574700.0*::powf( sinf(lat), 2.0 ) + 863700.0*::powf( ::sinf(lat), 4.0 )
              - 398600*::powf( ::sinf(lat), 6.0 ) ) ) ) ) );
    *y = ( 5.0 + 10.0*n ) * 100000.0 + l*::cosf(lat)*(6378245.0 + 21346.1415*::powf( ::sinf(lat), 2.0) + 107.15900*::powf( ::sinf(lat), 4.0 )
      + 0.5977*::powf( ::sinf(lat), 6.0 ) + ::powf( l, 2.0 )*(1070204.16 - 2136826.66*::powf( ::sinf(lat), 2.0 ) + 17.98*::powf( ::sinf(lat), 4.0 )
        - 11.99*::powf( ::sinf(lat), 6.0 ) + ::powf( l, 2.0 )*(270806.0 - 1523417.0*::powf( ::sinf(lat), 2.0 ) + 1327645.0*::powf( ::sinf(lat), 4.0 )
          - 21701.0*::powf( ::sinf(lat), 6.0 ) + ::powf( l, 2.0)*(79690.0 - 866190.0*::powf( ::sinf(lat), 2.0 ) + 1730360*::powf( ::sinf(lat), 4.0 )
            -945460.0*::powf( ::sinf(lat), 6.0 )))));
  }

  //!преобразование плоских прямоугольных координат (проекция Гаусса-Крюгера, СК-95) в геодезические
  void xy2latlon_gkgost( float x, float y, float* lat, float* lon )
  {
    int n = int(y/1000000.0);
    float betta = x/6367558.4968;
    float b0 = betta + ::sinf(2.0*betta)*( 0.00252588685 - 0.00001491860*::powf( ::sinf(betta), 2.0 ) + 0.00000011904*::powf( ::sinf(betta), 4.0 ) );
    float z0 = ( y - (10*n + 5)*100000.0 )/( 6378245.0*::cosf(b0) );
    float deltab = -( ::powf( z0, 2.0 ) )*::sinf( 2.0*b0 )*( 0.251684631 - 0.003369263*::powf( ::sinf(b0), 2.0 ) + 0.000011276*::powf( ::sinf(b0), 4.0 )
      - ::powf( z0, 2.0 )*( 0.10500614 - 0.04559916*::powf( sinf(b0), 2.0 ) + 0.00228901*::powf( ::sinf(b0), 4.0 ) - 0.00002987*::powf( ::sinf(b0), 6.0 )
        - ::powf( z0, 2.0 )*( 0.042858 - 0.025318*::powf( ::sinf(b0), 2.0 ) + 0.014346*::powf( ::sinf(b0), 4.0 ) - 0.001264*::powf( ::sinf(b0), 6.0  )
          - ::powf( z0, 2.0 )*( 0.01672 - 0.00630*::powf( ::sinf(b0), 2.0 ) +  0.01188*::powf( ::sinf(b0), 4.0 ) - 0.00328*::powf( ::sinf(b0), 6.0  )))));
    float l = z0*( 1 - 0.0033467108*::powf( sinf(b0), 2.0 ) - 0.0000056002*::powf( ::sinf(b0), 4.0 ) - 0.0000000187*::powf( ::sinf(b0), 6.0 )
      -::powf( z0, 2.0 )*( 0.16778975 + 0.16273586*::powf( ::sinf(b0), 2.0 ) - 0.00052490*::powf( ::sinf(b0), 4.0 ) - 0.00000846*::powf( ::sinf(b0), 6.0 )
        -::powf( z0, 2.0 )*( 0.0420025 + 0.1487407*::powf( ::sinf(b0), 2.0 ) + 0.0059420*::powf( ::sinf(b0), 4.0 ) - 0.0000150*::powf( ::sinf(b0), 6.0 )
          -::powf( z0, 2.0 )*( 0.01225 + 0.09477*::powf( ::sinf(b0), 2.0 ) + 0.03282 *::powf( ::sinf(b0), 4.0 ) - 0.00034*::powf( ::sinf(b0), 6.0 )
            -::powf( z0, 2.0 )*( 0.0038 + 0.0524*::powf( ::sinf(b0), 2.0 ) + 0.0482*::powf( ::sinf(b0), 4.0 ) + 0.0032*::powf( ::sinf(b0), 6.0 ) )))));
    *lat = b0+deltab;
    *lon = 6.0*(7.0 - 0.5)/57.29577951 + l;
  }

  //! Преобразование из проекции Меркатор (1SP) в геодезические координаты
  /*! 
    \param pr Параметры эллипсоида и проекции
    \param east Координата по горизонтали, м
    \param north Координата по вертикали, м
    \param lat Широта, рад
    \param lon Долгота, рад
  */
  void xy2f_merc1sp(const ProjectionParams& pr, float east, float north, float* lat, float* lon)
  {
    float t = expf((pr.fn - north) / (pr.a * pr.k0));
    float chi = M_PI_2f - 2 * atanf(t);
    float e2 = pr.e * pr.e;
    
    *lat = chi + (e2/2 + 5*powf(e2,2)/24 + powf(e2,3)/12 + 13*powf(e2,4)/360) *  sinf(2 * chi)
      + (7*powf(e2,2)/48 + 29*powf(e2,3)/240 + 811*powf(e2,4)/11520) * sinf(4 *chi)
      + (7*powf(e2,3)/120 +  81*powf(e2,4)/1120)* sinf(6*chi)  + (4279*powf(e2,4)/161280)* sinf(8*chi);
    
    *lon  =  ((east - pr.fe) / (pr.a * pr.k0)) + pr.la0;
  }


  //! Преобразование из геодезических координат в проекцию Меркатор (1SP)
  /*! 
    \param pr Параметры эллипсоида и проекции
    \param lat Широта, рад
    \param lon Долгота, рад
    \param east Координата по горизонтали, м
    \param north Координата по вертикали, м
  */
  void f2xy_merc1sp(const ProjectionParams& pr, float lat, float lon, float* east, float* north)
  {
    *east = pr.fe + pr.a*pr.k0*(lon - pr.la0);
    *north = pr.fn + pr.a*pr.k0*logf(tanf(M_PI_4f + lat/2) * powf((1-pr.e*sinf(lat))/(1+pr.e*sinf(lat)), pr.e/2));
  }

  //! Преобразование из геодезических координат в проекцию Полярную стереографическую
  /*! 
    \param pr Параметры эллипсоида и проекции
    \param lat Широта, рад
    \param lon Долгота, рад
    \param east Координата по горизонтали, м
    \param north Координата по вертикали, м
  */

  void f2xy_stereo_polar(const ProjectionParams& pr, float lat, float lon, float* east, float* north)
  {
    if (pr.fi0 < 0) {
      float t = tanf(M_PI_4f + lat*.5f) * powf((1+pr.e*sinf(lat))/(1-pr.e*sinf(lat)), pr.e*.5f);
      float ro = 2 * pr.a * pr.k0 * t / powf(powf(1+pr.e, 1+pr.e) * powf(1-pr.e, 1-pr.e), 0.5);
      float dE = ro * sinf(lon - pr.la0);
      float dN = ro * cosf(lon - pr.la0);
      *east = dE + pr.fe;
      *north = dN + pr.fn;
    } else {
      float t = tanf(M_PI_4f - lat/2) * powf((1+pr.e*sinf(lat))/(1-pr.e*sinf(lat)), pr.e/2);
      float ro = 2 * pr.a * pr.k0 * t / powf(powf(1+pr.e, 1+pr.e) * powf(1-pr.e, 1-pr.e), 0.5);
      float dE = ro * sinf(lon - pr.la0);
      *east = dE + pr.fe;
      *north = pr.fn - ro * cosf(lon - pr.la0);
    }
  }

//int peresech(const QPolygon& scrPt)
//{
// int i,count, count_ar;
// float a11,a12,a21,a22,x11,y11,x21,y21,y1,y2;
// float t1,t2;
// float d,d1,d2;
//
// count_ar=scrPt.size();
// if(count_ar<3) return 0;
//
//for(count=2;count<count_ar;count++)
//{
// x11=scrPt.at (count-1).x();
// a11=scrPt.at(count).x()-x11;
// y11=scrPt.at(count-1).y();
// a21=scrPt.at(count).y()-y11;
// if((a11*a11+a21*a21)<1.) continue;
//
// for(i=1;i<count;i++)
//   {
//    x21=scrPt.at(i-1).x();
//    a12=scrPt.at(i).x()-x21;
//    y21=scrPt.at(i-1).y();
//    a22=scrPt.at(i).y()-y21;
//    if((a21*a21+a22*a22)<1.) continue;
//
//    d=a21*a12-a11*a22;
//    if(fabs(d)<LITTLE_VALUE) continue;
//
//    y1=x21-x11;
//    y2=y21-y11;
//    d1=y2*a12-y1*a22;
//    d2=a21*y1-a11*y2;
//    t1=d1/d;
//    t2=d2/d;
//    if(count==(count_ar-1) && i==1)
//    {
//     if((0.<t1 && t1<1.) && (0.<t2 && t2<1.))  return 2;
//       else continue;
//    }
//    if(i!=(count-1))
//    {
//     if((0.<t1 && t1<1.) && (0.<t2 && t2<1.))
//       return 3;
//    }
//     else if((0.<t1 && t1<1.) && (0.<t2 && t2<1.))
//         return 3;
//   }
//}
//return 0;
//}
//
//int nearPointPos(const Projection &aprj, const QPolygon &ar, const QPoint &pos, GeoPoint *fl, int *st)
//{
//  int dist=kMIN_DIST, rast;
//  int i,j;
//  int kol;
//  int x1,x2,y1,y2,x,y,vs;
//  float t,dx,dy;
//  float fx,fy;
//  QPoint vs_p;
//
//
//  if((kol=ar.size())<2) return 4;
//  fl->setFi(MnCommon::ABS_INVALID_VALUE);
//  fl->setLa(MnCommon::ABS_INVALID_VALUE);
//  *st=-1;
//
//  for(i=1;i<kol;i++)
//  {
//    x1=ar.at(i-1).x();
//    y1=ar.at(i-1).y();
//    x2=ar.at(i).x();
//    y2=ar.at(i).y();
//    dx=x2-x1;
//    dy=y2-y1;
//    fx=fabs(dx);
//    fy=fabs(dy);
//    if(fx<4. && fy<4.) continue;
//    if(fx>fy)
//    {
//      if(x2<x1) {vs=x1; x1=x2; x2=vs; vs=y1; y1=y2; y2=vs;}
//      for(j=x1+1;j<x2-1;j++)
//      {
//        t=float(j-x1)/fx;
//        x=j;
//        y=ftoi_norm((1.-t)*y1+t*y2);
//        rast=(pos.x()-x)*(pos.x()-x)+(pos.y()-y)*(pos.y()-y);
//        if(rast<dist) {dist=rast; vs_p.setX(x); vs_p.setY(y); *st=i-1;}
//      }
//    }
//    else
//    {
//      if(y2<y1) {vs=x1; x1=x2; x2=vs; vs=y1; y1=y2; y2=vs;}
//      for(j=y1+1;j<y2-1;j++)
//      {
//        t=float(j-y1)/fy;
//        y=j;
//        x=ftoi_norm((1.-t)*x1+t*x2);
//        rast=(pos.x()-x)*(pos.x()-x)+(pos.y()-y)*(pos.y()-y);
//        if(rast<dist) {dist=rast; vs_p.setX(x); vs_p.setY(y); *st=i-1;}
//      }
//    }
//  }
//
//  if(dist==kMIN_DIST) return 4;
//
//  aprj.X2F(vs_p,fl);
//  if( !fl->isValid() ) return 2;
//  else return 0;
//}
//
//
//int provGeo(const Projection& projection, const QPolygon &ar) 
//{
//  int i,count;
//  int ret_val=0;
//  GeoVector vs;
//
//  count=ar.size();
//  if(count<2) return 0;
//  vs.resize(count);
//  projection.X2F(ar,&vs);
//  count = vs.count();
//  for(i=0;i<count;i++)
//    if(MnCommon::ABS_INVALID_VALUE == vs[i].fi() ||
//       MnCommon::ABS_INVALID_VALUE == vs[i].la()){
//    return 2;
//  }
//  return ret_val;
//}




}

