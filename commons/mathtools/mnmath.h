#ifndef GLOBALCONSTS_H
#define GLOBALCONSTS_H

#include <stdint.h>
//#include <math.h>
#include <cmath>

#include <limits>

#include <qstring.h>
#include <qpoint.h>
#include <qglobal.h>
#include <qvector.h>

//#include <commons/defines/mn_defines.h>

#include "tgeometrymath.h"
#include "mathematical_constants.h"

#define PiI180 0.017453292520
#define PiI360 0.008726646260

# define M_PIf      float (3.141592653589793238462643383279502884) /* pi */
# define M_PI_2f    float (1.570796326794896619231321691639751442) /* pi/2 */
# define M_PI_4f    float (0.785398163397448309615660845819875721) /* pi/4 */

namespace MnMath {


  extern const double AE;
  extern const double TWO_THRD;
  extern const double M_2PI;
  extern const float M_2PIf;
  extern const double DEG_PER_RAD;
  extern const double RAD_PER_DEG;
  extern const float DEG_PER_RADf;
  extern const float RAD_PER_DEGf;

  extern const double EQUATOR_RAD;  //Большая полуось (экваториальный радиус), км
  extern const double EARTH_RAD;    // Радиус Земли в метрах
  extern const double MEAN_RAD;     //Средний геометрический радиус земли, км
  extern const double FLATTENING;   //Сжатие Земли(отношение разницы полуосей к большой полуоси) 
  extern const double XKE;


  extern const double EARTH_SPEED; //скорость вращения Земли
  extern const double SEC_PER_DAY; // Seconds per day (solar)
  extern const double MIN_PER_DAY; // Minutes per day (solar)
  extern const double CK2;
  extern const double CK4;
  extern const double XJ3;

   enum H
   {
     LEN_FT,
     LEN_SH,
     LEN_LN,
     LEN_CT,
     DIV_SH,
     DIV_LN,
     H_COUNT
   };

  // TODO понять откуда это берётся
  extern const double DIST[ H_COUNT ];
  extern const double WSCALE;  // масштаб ?? (коэффициент сжатия)

  extern const double rsat;  // радиус спутника 
  extern const double beta;
  extern const double alfa; // углы спутника

  //!параметры угла (градусы, минуты, секунды)
  struct Angle {
    int degree;  //градусы
    int minute;  //минуты
    int second; //секунды
  };

  int closest(float val, float* arr, int num);
  double max(double v1, double v2);
  double  min(double v1, double v2);
  double sqr( double );
  double fmod2p( double x);
  double PiToPi(double d);
  float PiToPi(float d);
  float deg2rad( float deg );
  float rad2deg( float rad );
  double deg2rad( double deg );
  double rad2deg( double rad );
  double spAsin(double);
  float spAsin(float);
  double spAcos(double);
  float spAcos(float);

  int distancePow2( const QPoint& pnt1, const QPoint& pnt2 );
  double distance( const QPoint& pnt1, const QPoint& pnt2 );

  void  rad2deg60( const float rad, Angle& angle);
  void  deg2deg60( const float rad, Angle& angle);
  unsigned short getMask( unsigned bits);
  int hemDistance(unsigned char a, unsigned char b);
  double teorCosA(double b,double c,double alpha );
  double teorCosAlpha(double a,double b,double c );
  int teorCosC(double a,double b,double alpha, double *c1,double *c2);




  // из trasser.h
  bool quadric_equation(double a, double b, double c, double &x);
  double normAngle(double d);   //! уже есть
  double midVal(unsigned short* &nIz,int width, int xSeg, int ySeg, int segSide);
  double dsp(unsigned short* &nIz,int width, int xSeg, int ySeg, int segSide, double sZ);
  double korrelation(unsigned short* &nIz1,unsigned short* &nIz2,int width,int XSeg1, int YSeg1,int XSeg2, int YSeg2, int segSide, double sZ1,double sZ2);
  double calculateKorrelation( double X[], double Y[], int n );

  double calculateCumulateAverage( double &avg, int &n, double val  );
  double calculateCumulateDeviation( double &cma, int &n, double &avg, double &avg2, double val  );

  int getSign (/*unsigned char */unsigned short c1,/*unsigned char*/unsigned short c2);//! уже есть
  int extremum (unsigned short/*char*/ *&c1, int width,int height, int x,int y, int length, int &x_extr, int &y_extr);
  double dist (double x, double y);
  bool TeoremaCos (int count, double sat, double angular, double &z);
  double AB_length (int x1, int y1, int x2, int y2, int width, int height);
  int maxminBright(/*unsigned char*/unsigned short *&c3, int width, int height, int &maxBright, int &minBright);
  void getDirection(int px, int py, double angle, double len, int &epx, int &epy);
  bool kmPerPixel (int x, int y, int width, int height, double &rx, double &ry, double &r);
  int ftoi_norm(double d);  //! уже есть
  int ftoi_norm(float d);

  bool pointInCircle(int x0, int y0, int r, int x, int y);
  void convertXYtoDF(int x_beg, int y_beg, int x_end, int y_end, int w, int h, double &dd,double &ff);
  bool convertXYtoFiLa(const int& x,const int& y,int w, int h,double &fi,double &la );
  void convertDFtoUV(float dd, float ff, float *u, float *v);
  void getWind(unsigned short *&c1,unsigned short *&c2, int w, int h,int x_extr, int y_extr, int &x_end, int &y_end);

  void preobrUVtoDF(float u, float v, float* dd, float* ff); //!< скорость и направление ветра из векторных составляющих
  void averageWind(QList<QPair<float, float> > ddff, float* ddavg, float* ffavg);
  void Td2D(float T, float Td, float* D); //!< температура точки росы в дефицит
  void U2D(float T, float U, float* D);   //!< влажность в дефицит
  void U2D_kelvin(float T, float U, float* D); //!< влажность в дефицит
  float Td2U(float T, float Td); //!< точка росы с влажность

  qint32 swabi4( const qint32 i4 );

  double M180To180(double d);
  double M0To360(double d);
  float M180To180(float d);

  float normalize(float val, float min, float max, float nMin, float nMax);
  void normalize(float* val, int cnt, float min, float max, float nMin, float nMax);
  float limit(float val, float min, float max);
  template<class T> float standartDeviation(T* arr, unsigned num, float* retMean = 0);
  template<class T> float meanValue(T* arr, unsigned num);
  template<class T> T maxValue( const QVector<T>& list );
  template<class T> int sign( const T& x );

  bool isZero( double val );
  bool isEqual( double val1, double val2 );
  bool isZero( float val );
  bool isEqual( float val1, float val2 );

  bool isEqual( double val1, double val2, double delta );
  bool isEqual( float val1, float val2, float delta );


//  bool twoLineIntersection( const QPointF& p11, const QPointF& p12, const QPointF& p21, const QPointF& p22, QPointF* resPoint );
//  bool normalToLine( const QPoint& p, const QPoint& l11, const QPoint& l12, QPoint* res );
 
  //численные методы
  bool intervalContainsPoint( double start, double end, double point );
  bool lineContainsPoint( const QPointF& start, const QPointF& end, const QPointF& point );
   bool linInterpolY(const QPointF&,const QPointF&, QPointF *);
   bool linInterpolX(const QPointF&,const QPointF&, QPointF *);
   bool linInterpolY(const double&,const double&,const double&,const double&,const double&,double *);
   bool linInterpolX(const double&,const double&,const double&,const double&,const double&,double *);

   bool linInterpolY( float, float, float, float,float,float *);
   bool linInterpolX( float, float, float, float,float,float *);
  //  double integrTrapec(const QVector<double>& par, const QVector<double>& net);
  template<class T> double integrTrapec(const T& par, const T& net);
   bool integrTrapec(double *par, double *net, int kol, double &rez);
   bool integrTrapec(const QList<double> &par, const QList<double> &net, double &rez);
   bool integrTrapec(const QList<float> &par, const QList<float> &net, float *rez);

  double bisection( double a, double b, double e, double (*func)(double), bool& noNull );
  double bisection( double a, double b, double e, bool (*func)(double), bool& noNull );
  
  
//math from geosystem
 double cbrt(double base); //cube root
 double angle0to2PI(double angle);
 double angle0to360(double angle);

 bool fourPowerEquation(double a0,double b0, double c0, double d0, double e0, double &x1, double &x2, double &x3, double &x4, bool& ok1, bool& ok2);
 bool cubeEquation(double a0,double b0, double c0, double d0, double &x1, double &x2, double &x3);
 bool squareEquation(double a,double b, double c, double &x1, double &x2);

  uint16_t makeCrc(const void* buf, uint64_t size);
  uint32_t makeCrc32(const void* arr, uint64_t length);

  double ieee2double(const void *data);
  double recoverDouble(uint32_t val, uint32_t r, uint16_t e, uint16_t d);
  double binary2double(const void* src);

}

inline bool MnMath::isZero( double val )
{
  return qFuzzyIsNull(val);
}

inline bool MnMath::isEqual( double val1, double val2 )
{
  while ( 0.0 == val1 || 0.0 == val2 ) {
    val1 += 1.0;
    val2 += 1.0;
  }
  return qFuzzyCompare( val1, val2 );
}

inline bool MnMath::isZero( float val )
{
  return qFuzzyIsNull(val);
}

inline bool MnMath::isEqual( float val1, float val2 )
{
 /*
  while ( 0.f == val1 || 0.f == val2 ) {
    val1 += 1.0f;
    val2 += 1.0f;
  }
  return qFuzzyCompare( val1, val2 );*/
      return isZero(fabs(val1-val2));
}

inline bool MnMath::isEqual( float val1, float val2, float delta )
{
  if ( delta < fabsf( val1 - val2 ) ) {
    return false;
  }
  return true;
}

inline bool MnMath::isEqual( double val1, double val2, double delta )
{
  if ( delta < fabs( val1 - val2 ) ) {
    return false;
  }
  return true;
}

//inline bool MnMath::isZero( double val ) {
//  if ( LITTLE_VALUE > fabs(val) ) {
//    return true;
//  }
//  return false;
//}

inline qint32 MnMath::swabi4( const qint32 i4 ) {
  quint32 u4 = static_cast<quint32>(i4);
  return ( ( (u4) >> 24 ) + ( ( (u4) >> 8 ) & 65280 ) + ( ( (u4) & 65280 ) << 8 ) + ( ( (u4) & 255 ) << 24 ) );
}

template<class T> float MnMath::meanValue(T* arr, unsigned num)
{
  if (num==0) return 0;
  float sum = 0;
  for (unsigned i = 0; i < num; i++) {
    sum += arr[i];
  }
  return sum/num;
}

template<class T> float MnMath::standartDeviation(T* arr, unsigned num, float* retMean /*= 0*/)
{
  if (num==0) return 0;
  float sum = 0;
  float mean = meanValue(arr, num);
  if (retMean != 0) *retMean = mean;
  for (unsigned i = 0; i < num; i++) {
    sum += MnMath::sqr(arr[i]-mean);
  }
  return sqrt(sum/num);
}

template<class T> T MnMath::maxValue( const QVector<T>& list )
{
 T elem=list.first();
 for( const T& val : list ) {
   elem = qMax( val, elem );
 }
 return  elem;
}

template<class T> int MnMath::sign( const T& x )
{
  if ( false == std::numeric_limits<T>::is_signed ) {
    return 1;
  }
  return ( T(0) < x ) ? 1 : -1 ;
}

template<class T> double MnMath::integrTrapec(const T& par, const T& net)
{
  int i;
  double rez=0.;
  if(par.count() != net.count() || par.isEmpty || net.isEmpty()) return rez;
  for(i=0;i<par.count()-1; i++)
    rez+=(par.at(i) + par.at(i+1))*0.5*(net.at(i+1)-net.at(i));
  return rez;
}





#endif // GLOBALCONSTS_H

