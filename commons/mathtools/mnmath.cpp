#define _USE_MATH_DEFINES 
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "mnmath.h"
#include "mathematical_constants.h"

#include <cross-commons/debug/tlog.h>

namespace MnMath {
  const double AE           = 1.0;
  const double TWO_THRD     = 2.0/3.0;
  const double M_2PI        = 2 * M_PI;
  const float M_2PIf        = 2 * M_PIf;
  //  const double DEG_PER_RAD  = 57.29577951308232286;
  const double DEG_PER_RAD = 180.0 / M_PI;
  const double RAD_PER_DEG = M_PI / 180.0;
  const float RAD_PER_DEGf = M_PIf / 180.0f;
  const float DEG_PER_RADf = 180.0f / M_PIf;

  // const double FLATTENING   = (1.0/298.3);       //Сжатие Земли(отношение разницы полуосей к большой полуоси)
  // const double EQUATOR_RAD  = 6378.245;       //Большая полуось (экваториальный радиус), км
  //const double FLATTENING  = (1.0 / 298.26); // Earth flattening (WGS '72)
  //const double EQUATOR_RAD = 6378.135;     // Earth equatorial radius - km (WGS '72)
  const double EQUATOR_RAD  = 6378.137;//   // Earth equatorial radius - km (WGS '84)
  const double MEAN_RAD = 6371; //geometric mean radius - km (WGS '84)
  const double FLATTENING   = 1.0 / 298.26; // Earth flattening (WGS '84)
  //398600.8 - Earth gravitational parameter (WGS '72) in km^3/s^2
  //398600.5 (WGS '84)
  const double XKE = 60.0 * sqrt( 398600.5 /
				  ( EQUATOR_RAD*EQUATOR_RAD*EQUATOR_RAD ));

//   const double Pi = 3.141592653590;
  const double EARTH_RAD = 6370000.; // радиус Земли
  const double EARTH_SPEED  = 1.00273790934; //оборот/сидерический день //7.29211510e-5;  // скорость вращения Земли, рад/с
  const double SEC_PER_DAY  = 86400.0;       // Seconds per day (solar)
  const double MIN_PER_DAY  = 1440.0;        // Minutes per day (solar)

  const double J2           = 1.0826158E-3; // J2 harmonic (WGS '72)
  const double J3           = -2.53881E-6;  // J3 harmonic (WGS '72)
  const double J4           = -1.65597E-6;  // J4 harmonic (WGS '72)
  const double CK2          = J2 / 2.0;
  const double CK4          = -3.0 * J4 / 8.0;
  const double XJ3          = J3;

// какая-то высота в км вроде... забыла...
//   enum H
//   {
//     LEN_FT,
//     LEN_SH,
//     LEN_LN,
//     LEN_CT,
//     DIV_SH,
//     DIV_LN,
//     H_COUNT
//   };

  const double DIST[ H_COUNT ] = { 32.0,6.5,12.0,11.0,3.0,6.0 };
  const double WSCALE = 33.0/11.0;  // масштаб ?? (коэффициент сжатия)

  const double rsat = 42110000.159;  // радиус спутника
  const double beta = 8.5119305717719e-005;
  const double alfa = 8.3850271342027e-005; // углы спутника

};

/*! 
 * \brief Найти ближайшее к заданному значение в массиве
 * \param val заданное значение
 * \param arr массив
 * \param num размер массива
 * \return индекс массива с ближайшим значением
 */
int MnMath::closest(float val, float* arr, int num)
{
  if (!arr || num == 0) return 0;
  int idx = 0;
  float cl = arr[0];
  
  for (int i=1; i<num; i++) {
    if (fabs(val-arr[i]) < fabs(val-cl)) {
      cl = arr[i];
      idx = i;
    }
  }
  return idx;
}

double  MnMath::max(double v1, double v2)
{
  return (v1>=v2 ? v1:v2);
}

double  MnMath::min(double v1, double v2)
{
  return (v1<=v2 ? v1:v2);
}

//! возведение во вторую степень
double MnMath::sqr( double x )
{
/*
  QVector<double> vec;
  vec << 180  << 200;
  double g=Max(vec);
  printf("%f",g);
*/  
  return x*x;
}

//! приведение значения к интервалу от 0 до M_2PI
double MnMath::fmod2p( double x )
{
  double modu = fmod(x, M_2PI);

  if  (modu < 0.0)
      modu += M_2PI;

   return modu;
}

/*!
 * \brief приведение значения к отрезку [-M_PI, +M_PI]
 */
double MnMath::PiToPi(double d)
{
  if ( d >= -M_PI && d <= M_PI) return d;

  double modu = fmod(d, M_2PI);

  if (modu < -M_PI) {
    modu += M_2PI;
  }

  if (modu > M_PI) {
    modu -= M_2PI;
  }

  return modu;
}

/*!
 * \brief приведение значения к отрезку [-M_PI, +M_PI]
 */
float MnMath::PiToPi(float d)
{
  if (MnMath::isEqual( d, -M_PIf) || MnMath::isEqual(d, M_PIf)) return d;
  if ( d > -M_PIf && d < M_PIf) return d;

  float modu = fmodf(d, M_2PIf);

  if (modu < -M_PIf) {
    modu += M_2PIf;
  }

  if (modu > M_PIf) {
    modu -= M_2PIf;
  }

  return modu;
}

//! перевод градусов в радианы
double MnMath::deg2rad( double deg )
{
  return deg * RAD_PER_DEG;
}

//! перевод  радиан в градусы
double MnMath::rad2deg( double rad )
{
  return rad * DEG_PER_RAD;
}
//! перевод градусов в радианы
float MnMath::deg2rad( float deg )
{
  return deg * RAD_PER_DEGf;
}

//! перевод  радиан в градусы
float MnMath::rad2deg( float rad )
{
  return rad * DEG_PER_RADf;
}

//! перевод градусов в градусы, минуты, секунды
void MnMath::deg2deg60( const float deg, Angle& angle)
{
  double degree = 0.0;
  double minute = 0.0;
  double second = 0.0;

  minute = modf(deg, &degree)*60;
  second = modf(minute, &minute)*60;
  modf(second, &second);
  angle.degree = int(degree);
  angle.minute = int(minute);
  angle.second = int(second);
}

//! перевод радиан в градусы, минуты, секунды
void MnMath::rad2deg60( const float rad, Angle& angle)
{
  deg2deg60( rad2deg(rad), angle);
}

//! создание маски для заданного количества бит
unsigned short MnMath::getMask( unsigned bits )
{
  unsigned short masks[17] = { 0x0,  0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f,
			      0xff, 0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3ff, 0x7ff, 0xffff };
  if (bits < 17) {
    return masks[bits];
  }
  return (unsigned short)(-1);
}

//! Вычисление расстояния Хэмминга
int MnMath::hemDistance(unsigned char a, unsigned char b)
{
  int dist=0;
  for (unsigned i=0, mask =1; i<8; i++, mask <<= 1 ) {
    if ((a&mask) != (b&mask)) dist++;
  }
  return dist;
}

/*!
 * \brief Вычисление арксинуса. При значении аргумента, выходящем за пределы +/-1, он приравнивается граничному значению.
 * \param x аргумент
 * \return арксинус аргумента
 */
double MnMath::spAsin(double x)
{
  if (x > 1.0) x = 1.0;
  if (x < -1.0) x = -1.0;

  return asin(x);
}
float MnMath::spAsin(float x)
{
  if (x > 1.0) x = 1.0f;
  if (x < -1.0) x = -1.0f;

  return asinf(x);
}

/*!
 * \brief Вычисление арккосинуса. При значении аргумента, выходящем за пределы +/-1, он приравнивается граничному значению.
 * \param x аргумент
 * \return арккосинус аргумента
 */
double MnMath::spAcos(double x)
{
  if (x > 1.0) x = 1.0;
  if (x < -1.0) x = -1.0;

  return acos(x);
}
float MnMath::spAcos(float x)
{
  if (x > 1.0) x = 1.0f;
  if (x < -1.0) x = -1.0f;

  return acosf(x);
}
int MnMath::distancePow2( const QPoint& pnt1, const QPoint& pnt2 )
{
  QPoint res = pnt2 - pnt1;
  return res.x()*res.x() + res.y()*res.y();
}

double MnMath::distance( const QPoint& pnt1, const QPoint& pnt2 )
{
  QPointF res = pnt2 - pnt1;
  return ::hypot( res.x(), res.y() );
}

/**
 * Теорема косинусов. Рассчитывает длину стороны треугольника по известным двум сторонам и углу между ними
 * @param b длина первой стороны
 * @param c длина второй стороны
 * @param alpha угол между ними
 * @return длина третьей стороны
 */
double MnMath::teorCosA(double b,double c,double alpha ){
  double a;
  a = b*b + c*c - 2.*b*c*cos(alpha);
  if(a<0.) return -999999.;
  return sqrt(a);
}



/**
 * Теорема косинусов. Рассчитывает угол между  двумя сторонами (b и c ) треугольника
 * @param a длина первой стороны
 * @param b длина второй стороны
 * @param c длина третьей стороны
 * @return alpha угол между сторонами b и c
 */
double MnMath::teorCosAlpha(double a,double b,double c){
  return  MnMath::spAcos((b*b + c*c - a*a)*0.5/(b*c));
}

/**
 * Теорема косинусов. Рассчитывает длину стороны c треугольника  по известным двум сторонам b и a углу между сторонами b и c
 * @param b сторона a
 * @param c сторона b
 * @param alpha углу между сторонами b и c
 * @param a1 возвращаемое значение короткой стороны c
 * @param a2 возвращаемое значение длинной стороны c
 * @return возвращает 0 если таких сторон может быть две, -1 если треугольник не существует, 1 если сторона одна
 */
int MnMath::teorCosC(double a,double b,double alpha, double *c1,double *c2){
  double b1,c,d,x1,x2;
  b1 = -2.*b*cos(alpha);
  c = b*b - a*a;
  d = b*b - 4.*c;
  if(d>0.){
    x1 = (-b1 + sqrt(d))*0.5;
    x2 = (-b1 - sqrt(d))*0.5;
  } else{
    if(d<0.) {
      return -1;
    } else {
      x1 = (-b1 + sqrt(d))*0.5;
      *c1 = x1;
      *c2 = x1;
      return 1;
    }
  }
  *c1 = x1 < x2 ? x1:x2;
  *c2 = x1 < x2 ? x2:x1;
  return 0;
}

/*!
*\brief найти экстремум
*\param unsigned char *&c1 - массив
*\param int width - ширина изображения
*\param int height - высота изображения
*\param int x,y - координаты верхнего левого угла области
*\param int length - длина области
*\param int &x_extr, &y_extr - координаты экстремума
*/
int MnMath::extremum (unsigned short/*char*/ *&c1,int width, int height, int x,int y, int length, int &x_extr, int &y_extr)
{
  int lastSign = 1, i1=0, j1=0;
  int maxBright = 1;

  lastSign = getSign( c1[ y + 1 ],c1[ x ]);

  for ( int i = x; i < x + length; i++)
    for ( int j = y + 1; j < y + length; j++)
    {
      if (pointInCircle( width/2, height/2 ,(int)EARTH_RAD,x,y)) continue; // если точки не принадлежат окружности - отбрасываем

      if ( getSign( c1[ i*width + j ],c1[ i*width + j - 1 ]) != lastSign )
        if ( lastSign >= 0 )
        {
          if ( c1[ i*width + j ] > maxBright )
          {
            maxBright = c1[ i*width + j ];
            j1 = j;
            i1 = i;
          }
          lastSign = 1;
        }
    }
  x_extr = i1;
  y_extr = j1;

  return 0;
}

/*!
*\brief определить знак
*\param unsigned char c1 - значение из первого массива
*\param unsigned char c2 - значение из второго массива
*/
int MnMath::getSign (/*unsigned char*/unsigned short c1,/*unsigned char*/unsigned short c2)
{
  return ( c1 >= c2 ) ? 1 : -1 ;
//  if ( c1 >= c2 )
//    return 1;
//  return -1;
}

/*!
*\brief приведение угла от 0 до 360
*\param double d - угол
*\return 0 <= угол <= 360
*/
double MnMath::normAngle(double d)
{
  while ( d < -0.) d+=360.;
  while ( d >= 360.) d-=360.;
  return d;
}

/*!
*\brief дисперсия
*\param unsigned char * nIz - массив яркостей?? (x,y - )
*\param int width - ширина
*\param int xSeg - координата
*\param int ySeg - координата
*\param int segSide - сторона сегменат
*\param double sZ - мат ожидание
*/
double MnMath::dsp(unsigned short* &nIz,int width, int xSeg, int ySeg, int segSide, double sZ)
{
  double Dsp = 0;

  int i0 = ( abs( xSeg - segSide ))*width + ySeg - segSide;

  for( int k = 0; k < segSide*2 + 1 ;k++ )
    for( int j = 0; j < segSide*2 + 1; j++ )
    {
      Dsp += pow( nIz[ i0 + j + k*width ] - sZ ,2);
    }

  Dsp /= pow(float( segSide*2 + 1 ),2 );

  return Dsp;
}

/*!
*\brief определить направление
*\param int px - начальная координата
*\param int py - начальная координата
*\param double angle - угол
*\param double len - длина
*\param int &epx - смещение по X  ( возвращаемое значение)
*\param int &epy - смещение по Y ( возвращаемое значение)
*/
void MnMath::getDirection(int px, int py, double angle, double len, int &epx, int &epy)
{
  double angle1 = angle*DEG_PER_RAD;

  if ( 0. <= angle1 && angle1 < 90.)
  {
    epx = ftoi_norm( px - double( len ) * sin( angle ));
    epy = ftoi_norm( py + double( len ) * cos( angle ));
  }

  if ( 90. <= angle1 && angle1 < 180. )
  {
    epx = ftoi_norm(px - double(len) * cos( angle - M_PI/2.0 ));
    epy = ftoi_norm(py - double(len) * sin( angle - M_PI/2.0 ));
  }

  if ( 180. <= angle1 && angle1 < 270. )
  {
    epx = ftoi_norm( px + double( len ) * sin( angle - M_PI ));
    epy = ftoi_norm( py - double( len ) * cos( angle - M_PI ));
  }

  if ( 270. <= angle1 && angle1 < 360. )
  {
    epx = ftoi_norm( px + double( len ) * sin( 2.*M_PI - angle ));
    epy = ftoi_norm( py + double( len ) * cos( 2.*M_PI - angle ));
  }
}

/*!
*\brief коррелляционный момент
*\param unsigned char * nIz1 - массив яркостей первого снимка
*\param unsigned char * nIz2 - массивы с яркостями второго снимка
*\param int width - ширина
*\param int XSeg1,YSeg1 - координаты перв точки
*\param int XSeg2, int YSeg2, - координаты второй точки
*\param int segSide - сторона сегмента
*\param double sZ1,double sZ2 - мат ожидание
*/
double MnMath::korrelation(unsigned short* &nIz1,unsigned short* &nIz2,int width, int xSeg1, int ySeg1,int xSeg2, int ySeg2, int segSide, double sZ1,double sZ2)
{
  double kor = 0;

  int k0 = abs( xSeg1 - segSide )*width + ySeg1 - segSide;
  int k1 = abs( xSeg2 - segSide )*width + ySeg2 - segSide;

  for( int k = 0; k < segSide*2 + 1; k++ )
    for( int j = 0; j < segSide*2 + 1; j++ )
    {
      int i1 = k0 + j + k*width;
      int i2 = k1 + j + k*width;

      kor += ( nIz1[i1] - sZ1 )*( nIz2[i2] - sZ2 ) / pow(float( segSide*2 + 1 ),2);
    }

return kor;
}


/**
 * Общая функция для расчета коэффициента корреляции
 *
 * 
 * @param  X [массив со значениями]
 * @param  Y [массив со значениями]
 * @param  n [размер массива]
 * @return   [коэффициент корреляции]
 */
double MnMath::calculateKorrelation( double X[], double Y[], int n ){

  double sum_X    = 0, 
         sum_Y    = 0, 
         sum_XY   = 0,
         sqSum_X  = 0,
         sqSum_Y  = 0;

  for ( int i = 0; i < n; i++ ){
    // 
    sum_X = sum_X + X[i];
    // 
    sum_Y = sum_Y + Y[i];
    // 
    sum_XY = sum_XY + X[i] + Y[i];
    // 
    sqSum_X = sqSum_X + X[i] * X[i];
    // 
    sqSum_Y = sqSum_Y + Y[i] * Y[i];
  }

  double corr;
  corr = ( n * sum_XY - sum_X * sum_Y ) / sqrt( ( n * sqSum_X - sum_X * sum_X ) * ( n * sqSum_Y - sum_Y * sum_Y ) );

  return corr;
}


/**
 * считаем кумулятивное среднее
 * 
 * @return [description]
 */
double MnMath::calculateCumulateAverage( double &avg, int &n, double val  ){
  avg = ( val + n*avg )/(n+1);
  n  += 1;
  return avg;
}

/**
 * считаем кумулятивное среднеквадратичное отклонение
 * @param  cma  [получившееся СКО]
 * @param  n    [количество объектов]
 * @param  avg  [среднее значение]
 * @param  avg2 [среднее квадратов]
 * @param  val  [новое значение]
 * @return      [description]
 */
double MnMath::calculateCumulateDeviation( double &cma, int &n, double& avg, double& avg2, double val  ){
  int avgn, avg2n;
  // double avg, avg2;
  // сохраняем количество средних
  avgn  = n;
  avg2n = n;

  // рассчитываем среднее
  calculateCumulateAverage( avg, avgn, val );
  calculateCumulateAverage( avg2, avg2n, val*val );

  if ( n>=2 ){
    cma = sqrt( avg2 - avg*avg ) * sqrt( n/(n-1) );
  }

  n   += 1;
  return cma;
}

/*!
*\brief математическое ожидание (среднее значение)
*\param dunsigned char * nIz
*\param int width - ширина
*\param int xSeg - координата ??
*\param int ySeg - координата ??
*\param int segSide - сторона сегмента
*\return математическое ожидание
*/
double MnMath::midVal(unsigned short* &nIz,int width, int xSeg, int ySeg, int segSide)
{
  double szn = 0;

  int i0 = ( abs( xSeg - segSide ) )*width + ySeg - segSide;

  for ( int k = 0; k < segSide*2 + 1; k++)
  {
     for ( int j = 0; j < segSide*2 + 1; j++ )
      szn += nIz[ i0 + j + k*width ];
  }
  szn /= pow( float( segSide*2 + 1 ), 2 );
  return szn;
}

/*!
*\brief квадратное уравнение (меньший корень)
*\param double a - коэффициент
*\param double b - коэффициент
*\param double c - коэффициент
*\param double &x - переменная
*\return меньший корень
*/
bool MnMath::quadric_equation(double a, double b, double c, double &x)
{
  double x1,x2,D;

  D = pow( b,2 ) - 4.*a*c;

  if ( D < 0 )
  {
    x = ftoi_norm( -b / 2.*a );
  }
  else
  {
    x1 = ftoi_norm(( -b + sqrt( D )) / 2.*a );
    x2 = ftoi_norm(( -b - sqrt( D )) / 2.*a );

    // вернуть меньший корень
    if ( x1 <= x2 )
         x = x1;
    else x = x2;

  }

return true;
}

/*!
*\brief косинус
*\param int count - кол-во ..
*\param double sat
*\param double angular - угол
*\param double &z - косинус (возвращаемое значение)
*/
bool MnMath::TeoremaCos (int count, double sat, double angular, double &z)
{
  // teorCosA()
  double b,c;

  b = -2.*sat*cos( angular*count );
  c = pow( sat,2 ) - pow( EARTH_RAD,2 );

  quadric_equation(1.0,b,c,z);

  return true;
}

/*!
*\brief расстояние от точки A до точки B (географическое ???)
*\param int x1, int y1 - координаты точки A
*\param int x2, int y2 - координаты точки B
*\param int width - ширина
*\param int height - высота
*/
double MnMath::AB_length (int x1, int y1, int x2, int y2, int width, int height)
{
  double kmx=0., kmy=0., km=0.,dx=0.,dy=0.,length=0.;

  if ( x1 <= x2 )
  {
    for ( int i = x1; i < x2; i++)
      if ( kmPerPixel ( i,y2,width, height, dx,dy,length ) ) kmx += dx;
  }
  else
  {
    for ( int i = x2; i < x1; i++)
      if ( kmPerPixel( i,y2,width, height,dx,dy,length )) kmx += dx;
  }

  if ( y1 <= y2 )
  {
    for ( int j = y1; j < y2; j++ )
      if ( kmPerPixel ( x1,j,width, height,dx,dy,length )) kmy += dy;
  }
  else
  {
    for ( int j = y2; j < y1; j++)
      if ( kmPerPixel (x1,j,width, height,dx,dy,length) ) kmy += dy;
  }

  km = ftoi_norm( dist( kmx,kmy ) );

  return km;
}

/*!
*\brief получить максимальную и минимальную яркость
*\param unsigned char *&c3 - массив
*\param int width - ширина
*\param int height - высота
*\param int &maxBright - максимальная яркость
*\param int &minBright - минимальная яркость
*/
int MnMath::maxminBright(/*unsigned char*/unsigned short *&c3, int width, int height, int &maxBright, int &minBright)
{
  int i, j, sign = 1, lastsign = 1;
  maxBright = 1;
  minBright = 100;

  lastsign = getSign( c3[1], c3[0] );

  for ( i = 1; i < width; i++ )
    for ( j = 1; j < height; j++ )
    {
      if ( getSign( c3[ i*width + j ], c3[ i*width + j - 1 ] ) != lastsign )
      {
        if ( lastsign >= 0 )
          if ( c3[ i*width + j ] > maxBright )
            maxBright = c3[ i*width + j ];
        lastsign = sign;
      }
    }
  lastsign = getSign( c3[0], c3[1] );

  for ( i = 1; i < width; i++)
    for ( j = 1; j < height; j++)
    {
      if ( getSign(c3[ i*width + j - 1 ],c3[ i*width + j ]) != lastsign )
      {
        if ( lastsign >= 0 )
          if ( c3[ i*width + j ] < minBright )
            minBright = c3[ i*width + j ];
        lastsign = sign;
      }
    }
  return 0;
}

/*!
*\brief длина вектора
*/
double MnMath::dist (double x, double y)
{
  return sqrt( pow( x, 2 ) + pow( y, 2 ) );
}

/*!
*\brief размер пикселя в километрах
*\param int x
*\param int y
*\param double &rx - проекция на ось х
*\param double &ry - проекция на ось у ??
*\param double &r
*/
bool MnMath::kmPerPixel (int x, int y, int width, int height, double &rx, double &ry, double &r)
{
  int nx1,nx2,ny1,ny2;
  double rx1 = 0.,ry1 = 0.,rx2 = 0.,ry2 = 0.;

  r = 0.;
  rx = 0.;
  ry = 0.;

  if ( x <= width/2 )
       nx1 = width/2-x;
  else
       nx1 = x - (width/2);

  nx2 = nx1 - 1;

  if (!TeoremaCos( nx1, rsat, alfa, rx1)) return false;
  if (!TeoremaCos( nx2, rsat, alfa, rx2)) return false;

  rx = teorCosA( rx1,rx2,alfa );

  if ( y <= height/2 )
       ny1 = height/2 - y;
  else
       ny1 = y - ( height/2 );

  ny2 = ny1 - 1;

  if (!TeoremaCos( ny1,rsat,beta,ry1 )) return false;
  if (!TeoremaCos( ny2,rsat,beta,ry2 )) return false;

  ry = teorCosA( ry1, ry2, beta );
  r = ftoi_norm( dist(rx,ry) );

  return true;
}


int MnMath::ftoi_norm(double d)
{ 
  if (d<0 ) {
    return int(d-0.5);
  }
  else {
    return int(d+0.5);
  }
}

int MnMath::ftoi_norm(float d)
{ 
  if ( d<0 ) {
    return int(d-0.5f);
  }
  else {
    return  int(d+0.5f);
  }
}

/*!
*\brief принадлежит ли точка окружности
*\param int x0,y0 - координаты центра окружности
*\param int r - радиус
*\param int x,y - координаты точки
*/
bool MnMath::pointInCircle(int x0, int y0, int r, int x, int y)
{
  // переход в новую систему координат Ox1y1
  int x1 = x0 - r;
  int y1 = y0 - r;

  return (pow( float(x - x1),2 ) + pow( float(y - y1),2 ) <= pow(float(r),2));
}

/*!
*\brief получить скорость и направление ветра по вектору
*\param const QPoint& p_beg - начало вектора
*\param const QPoint& p_end - конец вектора
*\param double &dd - угол (направление)
*\param double &ff - скорость ветра
*/
void MnMath::convertXYtoDF(int x_beg, int y_beg, int x_end, int y_end, int w, int h, double &dd,double &ff)
{
  int dx = x_end - x_beg;
  int dy = y_end - y_beg;

  double dc = dist( dx,dy );

  if ( dc == 0 )
  {
    dd = 0;
    ff = 0;
    return;
  }

  float time = 3600.;
  ff = AB_length( x_beg,y_beg,x_end,y_end,w,h) / time;

  if ( dx >= 0 && dy >= 0 )
  {
    dd = acos(dx/dc) * DEG_PER_RAD;
    dd = 90 + dd;
  }

  if ( dx >= 0 && dy < 0 )
  {
    dd = acos( dx/dc ) * DEG_PER_RAD;
    dd = 90 - dd;
  }

  if ( dx < 0 && dy < 0 )
  {
    dd = acos( -dx/dc ) * DEG_PER_RAD;
    dd = 270 + dd;
  }

  if ( dx < 0 && dy >= 0 )
  {
    dd = acos( -dx/dc ) * DEG_PER_RAD;
    dd = 270 - dd;
  }

  if ( dd < 0 || ff < 0 )
  {
    dd = 9999;
    ff = 9999;
  }
}

/*!
*\brief вычисляем координаты конечн точки, куда дует ветер ??
*\param unsigned char *&c1 - массив 1
*\param unsigned char *&c2 - массив 2
*\param  const QPoint& p_extr - точка экстремума
*\param  QPoint& p_end - возвращаемое значение. конечная точка
*/
void MnMath::getWind(unsigned short *&c1,unsigned short *&c2, int w, int h,int x_extr, int y_extr, int &x_end, int &y_end)
{
  int segSide = 30;
  int kvSide = 60;

    int k1 = 0;
    int m1 = 0;
    double maxK = -10.; // максимальный коэффициент корреляции

    for ( int k = 0; k < kvSide + 1; k++ )
      for ( int m = 0; m < kvSide + 1; m++ )
      {
        int p1_x = x_extr - kvSide/2 + k;
        int p1_y = y_extr - kvSide/2 + m;

        if ( p1_y < segSide/2 + 1 ) continue;
        if ( p1_x < segSide/2 + 1 ) continue;
        if ( ( w - p1_x ) < segSide/2 + 1 ) continue;
        if ( ( h - p1_y ) < segSide/2 + 1 ) continue;

        // вычисляем мат ожидания
        double sM1 = midVal( c1,w,x_extr,y_extr,kvSide/2 );
        double sM2 = midVal( c2,w,p1_x,p1_y,kvSide/2 );

        // вычисляем дисперсию
        double dM1 = dsp( c1,w,x_extr,y_extr,kvSide/2,sM1 );
        double dM2 = dsp( c2,w,p1_x,p1_y,kvSide/2,sM2 );

        // вычисляем коэффициент СКО ??
        double Sko1 = sqrt( dM1 );
        double Sko2 = sqrt( dM2 );

        // вычисляем коэффициент корреляции
        double kk = korrelation( c1,c2,w,x_extr,y_extr,p1_x,p1_y,segSide/2,sM1,sM2);
        kk /= Sko1*Sko2;

        if ( kk > maxK )
        {
          maxK = kk;
          k1 = k;
          m1 = m;
        }
      }

    x_end = x_extr - kvSide/2 + m1;
    y_end = y_extr - kvSide/2 + k1;

    //FIXME нужна ли проверка на выход за границы окружности и соотв приведение к точке на окружности ??
}

/*!
*\brief преобразование координат в географические
*\param unsigned char *&c3 - массив данных
*\param int x - координата
*\param int y - координата
*\param double &fi - фи (возвращаемое значение)
*\param double &la - лямбда (возвращаемое значение)
*/
bool MnMath::convertXYtoFiLa (const int& x, const int& y, int w, int h, double &fi, double &la )
{
  double line,ob4,ab1,ab4,oc1,ob1,b1b2,ob2;
  double gamma; // углы спутника
  double longalfa, longbeta;

  if ( x <= w/2 )
  {
    longalfa = alfa*( w/2 - x );
  }
  else
    longalfa = alfa *( x - w/2 );

  if ( y <= h/2 )
  {
    longbeta = beta*( h/2 - y );
  }
  else
    longbeta = beta*( y - h/2 );

  oc1 = rsat*tan( longalfa );
  ob1 = rsat*tan( longbeta );

  ob4 = dist( oc1,ob1);
  gamma = atan( ob4 / rsat );

  quadric_equation( 1.,-2.*rsat*cos(gamma), pow( rsat,2 ) - pow( EARTH_RAD,2 ), *&line );

  ab1 = dist( rsat,ob1 );
  ab4 = dist( oc1, ab1 );

  b1b2 = ab1 - line*ab1/ab4;
  ob2 = sqrt( pow( ob1,2 ) + pow( b1b2,2 ) - 2.*b1b2*ob1*cos( M_PI/2. - longbeta ));

  fi = acos( ( pow( ob1,2 ) + pow( ob2,2 ) - pow( b1b2,2 ) )/( 2.*ob1*ob2 ) )*DEG_PER_RAD;
  la = acos( ob2/EARTH_RAD )*DEG_PER_RAD;

  if ( x <= w / 2 ) la *= -1;
  if ( y > h / 2)  fi *= -1;

  return true;
}

/*!
*\brief преобразование угла и скорости ветра к uv
*\param double dd - угол ветра
*\param double ff - скорость ветра
*\param double &u -
*\param double &v -
*/
void MnMath::convertDFtoUV(float dd, float ff, float *u, float *v)
{
  double angl;
  angl = dd * RAD_PER_DEG;

 
  *u = -ff*sin(angl);
  *v = -ff*cos(angl);

  // *u *=100; 
  //  *v *=100; 
  dd = M0To360(dd); 
return ;  //TODO пока убрал проверки ниже
  /*
  if ( 0. <= dd && dd < 90. )
    {
      *u = ff*sin(angl);
      *v = ff*cos(angl);
    }
  if ( 90. <= dd && dd < 180. )
    {
      *u = ff*sin( M_PI - angl );
      *v = -ff*cos( M_PI - angl );
    }
  if ( 180. <= dd && dd < 270. )
    {
      *u = -ff*sin( angl - M_PI );
      *v = -ff*cos( angl - M_PI );
    }
  if ( 270. <= dd && dd < 360. )
    {
      *u = -ff*sin( M_PI*2 - angl );
      *v = ff*cos( M_PI*2 - angl );
    }
*/
}

void MnMath::preobrUVtoDF(float u, float v, float* dd, float* ff)
{
  double rtog=180./3.1415926;

  *ff=ftoi_norm(sqrt(u*u+v*v));
  // dx=u/100.; //TODO  убрал деление на 100
  //  dy=v/100.; //TODO
  // *dd = (u < 0.)? ftoi_norm(360. + atan2(u,v)*rtog) : ftoi_norm(atan2(u,v)*rtog);
  if((*ff)==0) {(*dd)=0; return;}
  (*dd) = ftoi_norm(atan2(double(v),double(u))*rtog);
  *dd=ftoi_norm(M0To360(270.-(*dd)));

}

//! Расчет среднего ветра для случая равномерного шага по высоте между уровнями
/*! 
  \param ddff Массив параметров ветра: направление (градусы, 0-360) и скорость
  \param ddavg Среднее направление
  \param ffavg Средняя скорость
*/
void MnMath::averageWind(QList<QPair<float, float> > ddff, float* ddavg, float* ffavg)
{
  //может это не очень оптимально, но по-другому у меня не получилось
  *ffavg = 0;
  *ddavg = 0;
  
  if (ddff.count() == 0) return;

  float uusum = 0;
  float vvsum = 0;
  float u = 0, v = 0;

  for (int i = 0; i < ddff.count() ; i++) {
    MnMath::convertDFtoUV(ddff.at(i).first, ddff.at(i).second, &u, &v);
    uusum += u;
    vvsum += v;
  }
  
  MnMath::preobrUVtoDF(uusum, vvsum, ddavg, ffavg);
  *ffavg /=  ddff.count();
}

//! Преобразование температуры точки росы в дефицит точки росы
/*! 
  \param T  температура
  \param Td температура точки росы
  \param D  дефицит точки росы
*/
void MnMath::Td2D(float T, float Td, float* D)
{
  *D = T - Td;
}

//! Преобразование влажности в дефицит точки росы
/*! 
  \param T температура, в K
  \param U влажность, в %
  \param D дефицит
*/
void MnMath::U2D_kelvin(float T, float U, float* D)
{
  U2D(T - 273.15, U, D);
}

//! Преобразование влажности в дефицит точки росы
/*! 
  \param T температура, в С
  \param U влажность, в %
  \param D дефицит
*/
void MnMath::U2D(float T, float U, float* D)
{
  float vs = 7.63*T / (T + 241.9);
  float E = 6.1078 * pow(10., vs);
  float e = E * U * 0.01;
  vs = log10(e/6.1078);
  *D = 241.9*vs / (7.63 - vs);
  *D = T - *D;
}

//! Преобразование точки росы в влажность
/*! 
  \param T фактическая температура, С
  \param Td точка росы, С
  \return влажность, %
*/
float MnMath::Td2U(float T, float Td)
{
  
  Td += 273.15;
  T  += 273.15;
  return 100 * pow(Td, -5.3627) * pow(T, 5.3627) * exp(6888.2*(1/T - 1/Td));
}


/*!
 * \brief Нормирование значения переменной
 * \param val переменная
 * \param min минимальное значение переменной
 * \param max максимальное значение переменной
 * \param nMin новое минимальное значение
 * \param nMax новое максимальное значение
 * \return нормированное значение
 */
float MnMath::normalize(float val, float min, float max, float nMin, float nMax)
 {
   float k =  (nMax - nMin)/(max - min);
   if (val > max) val = nMax;
   else if (val < min) val = nMin;
   else val = k * (val - min) + nMin;
   return val;
 }

/*!
 * \brief Нормирование значений массива
 * \param val массив
 * \param cnt количество элементов массива
 * \param min минимальное значение переменной
 * \param max максимальное значение переменной
 * \param nMin новое минимальное значение
 * \param nMax новое максимальное значение
 * \return нормированное значение
 */
void MnMath::normalize(float* val, int cnt, float min, float max, float nMin, float nMax)
{
  if (!val) return;
  float k =  (nMax - nMin)/(max - min);
  for (int i=0; i< cnt; i++) {
    if (val[i] > max) val[i] = nMax;
    else if (val[i] < min) val[i] = nMin;
    else val[i] = k * (val[i] - min) + nMin;
  }
}

/*!
 * \brief Обрезание значения переменной по новым границам
 * \param val переменная
 * \param min новое минимальное значение
 * \param max новое максимальное значение
 * \return новое значение
 */
float MnMath::limit(float val, float min, float max)
{
  if (val > max) val = max;
  if (val < min) val = min;
  return val;
}


/*!
 * \brief Расчёт среднего квадратического отклонения
 * \param arr Набор данных, для которых необходимо рассчитать
 * \param num Количество данных
 * \param retMean Возвращаемое среднее значение
 */
template<class T> float MnMath::standartDeviation(T* arr, unsigned num, float* retMean/* = NULL*/);

/*!
 * \brief Расчёт среднего арифметического
 * \param arr Набор данных, для которых необходимо рассчитать
 * \param num Количество данных
 */
template<class T> float MnMath::meanValue(T* arr, unsigned num);

double MnMath::M180To180(double d)
{
  while(d<=-180.) d+=360.;
  while(d>180.) d-=360.;
  return d;
}

float MnMath::M180To180(float d)
{
  while(d<=-180.f) d+=360.f;
  while(d>180.f) d-=360.f;
  return d;
}

double MnMath::M0To360(double d)
{
  while(d<-0.) d+=360.;
  while(d>=360.) d-=360.;
  return d;
}
//линейная интерполяция - нахождение  x по y
bool MnMath::linInterpolY(const double& x1,const double& y1,const double& x2,const double& y2,const double& x,double *y){
  QPointF res_xy;
  res_xy.setX(x);
  bool res = linInterpolY( QPointF(x1,y1), QPointF(x2,y2), &res_xy);
  *y=res_xy.y();
  return res;
}

//линейная интерполяция - нахождение  y по x
bool MnMath::linInterpolX(const double& x1,const double& y1,const double& x2,const double& y2,const double& y,double *x){
  QPointF res_xy;
  res_xy.setY(y);
  bool res = linInterpolX( QPointF(x1,y1), QPointF(x2,y2), &res_xy);
  *x=res_xy.x();
  return res;
}
//линейная интерполяция - нахождение  x по y
bool MnMath::linInterpolY(float x1,float y1,float x2,float y2,float x,float *y){
  float dx = x2 - x1;
  float dy = y2 - y1;
  if(isZero(dx)) return false;
  if((x1<=x&&x<x2)||(x1<x&&x<=x2)||(x2<=x&&x<x1)||(x2<x&&x<=x1)) {
    *y = y1 + dy/dx*(x-x1);
    return true;
  }
  else return false;
}

//линейная интерполяция - нахождение  y по x
bool MnMath::linInterpolX(float x1,float y1,float x2,float y2,float y,float *x){
  float dx = x2 - x1;
  float dy = y2 - y1;
  if(isZero(dy)) return false;
  if((y1<=y&&y<y2)||(y1<y&&y<=y2)||(y2<=y&&y<y1)||(y2<y&&y<=y1)) {
    *x = x1+dx/dy*(y-y1);
    return true;
  }
  else return false;
}

//линейная интерполяция - нахождение  res_xy.y по res_xy.x
bool MnMath::linInterpolY(const QPointF& xy1, const QPointF& xy2, QPointF *res_xy){
  QPointF dxy = xy2 - xy1;
  double x = res_xy->x();
  if(isZero(dxy.x())) return false;
  if((xy1.x()<=x&&x<xy2.x())||(xy1.x()<x&&x<=xy2.x())||(xy2.x()<=x&&x<xy1.x())||(xy2.x()<x&&x<=xy1.x())) {
    res_xy->setY(xy1.y()+dxy.y()/dxy.x()*(x-xy1.x()));
    return true;
  }
  else return false;
}

//линейная интерполяция - нахождение  res_xy.x по res_xy.y
bool MnMath::linInterpolX(const QPointF& xy1, const QPointF& xy2, QPointF *res_xy){
  QPointF dxy = xy2 - xy1;
  double y = res_xy->y();
  if(isZero(dxy.y())) return false;
  if((xy1.y()<=y&&y<xy2.y())||(xy1.y()<y&&y<=xy2.y())||(xy2.y()<=y&&y<xy1.y())||(xy2.y()<y&&y<=xy1.y())) {
    res_xy->setX(xy1.x()+dxy.x()/dxy.y()*(y-xy1.y()));
    return true;
  }
  else return false;
}

bool MnMath::integrTrapec(double *par, double *net, int kol, double &rez){
  int i;
  if(kol==0) return false;
  rez=0.;
  for(i=0;i<kol-1;i++)
    rez+=((par[i]+par[i+1])*0.5*(net[i+1]-net[i]));
  return true;
}

bool MnMath::integrTrapec(const QList<double> &par,const QList<double> &net, double &rez){
  if(par.size() != net.size() || par.isEmpty() || net.isEmpty()) return false;
  int i;
  int kol = par.size();
  rez=0.;
  for(i=0;i<kol-1;i++)
    rez+=((par[i]+par[i+1])*0.5*(net[i+1]-net[i]));
  return true;
}

bool MnMath::integrTrapec(const QList<float> &par,const QList<float> &net, float *rez){
  if(par.size() != net.size() || par.isEmpty() || net.isEmpty()) return false;
  int i;
  int kol = par.size();
  *rez=0.;
  for(i=0;i<kol-1;i++)
    (*rez)+=((par[i]+par[i+1])*0.5f*(net[i+1]-net[i]));
  return true;
}

 
/*!
 * \brief Расчёт кубического корня числа
 * \param base заданное число
 */

double MnMath::cbrt(double base)
{
 return  (base>0)? pow(fabs(base), 1./3.):-pow(fabs(base), 1./3.);
}

/*!
 * \brief Приведение заданного угла к диапазону 0 до 2 Пи
 * \param angle заданный угол, радианы
 */
double MnMath::angle0to2PI(double angle)
{
 while(angle < 0.) angle += 2.*M_PI;
 while(angle >= 2.*M_PI) angle -= 2.*M_PI; 
 return angle;
}

/*!
 * \brief Приведение заданного угла к диапазону 0 до 360 град.
 * \param angle заданный угол, градусы
 */

double MnMath::angle0to360(double angle)
{
 while(angle < 0.) angle += 360.;
 while(angle >= 360.) angle -= 360.; 
 return angle;
}


/*!
*\brief Вычисление корней уравнения четвертой степени (метод Феррари)
*\param double a0 - коэффициент
*\param double b0 - коэффициент
*\param double c0 - коэффициент
*\param double d0 - коэффициент
*\param double e0 - коэффициент
*\param double &x1, &x2, &x3, &x4 - корни уравнения четвертой степени
*\param bool &ok1 - корни &x1, &x2 - действительные
*\param bool &ok2 - корни &x3, &x4 - действительные
*\return наличие корней резольвенты (кубического уравнения)
*/

//Ferrary method
bool MnMath::fourPowerEquation(double a0,double b0, double c0, double d0, double e0, double &x1, double &x2, double &x3, double &x4, bool& ok1, bool& ok2)
{
 double a = b0/a0;
 double b = c0/a0;
 double c = d0/a0;
 double d = e0/a0;
 double root1, root2, root3;
 double p, q;
 bool ok  = cubeEquation(1., -b, a*c-4*d, -a*a*d+4*b*d-c*c, root1, root2, root3);
      p = 0.25*a*a-b+root1;
      q = 0.5*a*root1-c;
      ok1 = squareEquation(1., 0.5*a-sqrt(p), 0.5*root1-0.5*q/sqrt(p), x1, x2);
      ok2 = squareEquation(1., 0.5*a+sqrt(p), 0.5*root1+0.5*q/sqrt(p), x3, x4);

 return ok;
}


/*!
*\brief Вычисление корней кубического уравнения (метод Кардано)
*\param double a0 - коэффициент
*\param double b0 - коэффициент
*\param double c0 - коэффициент
*\param double d0 - коэффициент
*\param double &x1, &x2, &x3 - корни уравнения четвертой степени
*\return корни &x2, &x3 - действительные/мнимые
*/

//Kardano method
bool MnMath::cubeEquation(double a0,double b0, double c0, double d0, double &x1, double &x2, double &x3)
{
 double a = b0/a0;
 double b = c0/a0;
 double c = d0/a0;
 double p = -a*a/3+b;
 double q = 2.*a*a*a/27.-a*b/3.+c;
 double Q = p*p*p/27.+q*q/4.;
 double alpha;
// double rt1, rt2;
 bool ok;
 if(Q>0) {
   x1 = cbrt(-0.5*q+sqrt(Q))+cbrt(-0.5*q-sqrt(Q)) - a/3.;
   x2 = 0.;
   x3 = 0.;
   ok = false;  // 2 imaginary roots
 }
 if(Q == 0.) {
   x1 = 2.*cbrt(-0.5*q) - a/3.;
   x2 = -0.5*cbrt(-0.5*q+sqrt(Q))+cbrt(-0.5*q-sqrt(Q)) - a/3.;
   x3 = x2;
   ok = true; // all roots is real, 2 roots is equal
 }
 if(Q<0) {
   alpha = acos(-0.5*q/sqrt(-(p*p*p/27.)));
   x1 = 2*sqrt(-p/3.)*cos(alpha/3.) - a/3.;
   x2 = -2*sqrt(-p/3.)*cos(alpha/3.+M_PI/3.) - a/3.;
   x3 = -2*sqrt(-p/3.)*cos(alpha/3.-M_PI/3.) - a/3.;
   ok = true;
 }
 return ok; // all roots is real and differents
}

bool MnMath::squareEquation(double a,double b, double c, double &x1, double &x2)
{
 if(a == 0. && b == 0.) {x1 = 0.; x2 = 0.;  return false;}
 double D = b*b-4*a*c;
 bool ok = (D>=0)? true:false;
 if(a == 0.)
 {
    x1 = -c/b;
    x2 = x1;
    return true;
 }
 x1 = (ok)? (-b+sqrt(D))*0.5/a:0;
 x2 = (ok)? (-b-sqrt(D))*0.5/a:0;
 return ok;
}


//! Создание контрольной суммы массива байт
/*! 
  \param src массив байт
  \param size размер массива
  \return контрольная сумма
*/
uint16_t MnMath::makeCrc(const void* src, uint64_t size)
{
  const uchar* buf = reinterpret_cast<const uchar*>(src);
  uint16_t crc = 0xFFFF;
  uint64_t i;
  int bit;
  for( i = 0; i < size; ++i ) {
    crc ^= buf[ i ];
    for( bit = 0; bit < 8; ++bit ) {
      uint16_t carry = crc & 0x01;
      crc >>= 1;
      if ( carry )
	crc ^= 0xA001;
    }
  }
  return crc;
}

// CRC-32C (Castagnoli)
uint32_t MnMath::makeCrc32(const void* arr, uint64_t length)
{
  const uchar* buf = reinterpret_cast<const uchar*>(arr);

  uint32_t bitCounter = 0;
  int lowerBit;

  uint32_t crc32 = 0;
  for ( uint64_t dataCounter = 0; dataCounter < length; ++dataCounter ) {
    crc32 = crc32 ^ buf[dataCounter];
    for ( bitCounter = 0; bitCounter < 8; ++bitCounter ) {
      lowerBit = crc32 & 1;
      crc32 = crc32 >> 1;
      if ( lowerBit ) {
        crc32 = crc32 ^ 0x82F63B78;
      }
    }
  }
  return crc32;
}

//! Восстановление величины представленное в стандарте IEEE 32-бит
/*! 
  \param data указатель на исходный массив
  \return восстновленное значение
*/
double MnMath::ieee2double(const void *src)
{
  double fmant;
  int exp;
  const uchar* ieee = reinterpret_cast<const uchar*>(src);
  
  if ((ieee[0] & 127) == 0 && ieee[1] == 0 && ieee[2] == 0 && ieee[3] == 0)
    return (float) 0.0;
  
  exp = ((ieee[0] & 127) << 1) + (ieee[1] >> 7);
  fmant = (double) ((int) ieee[3] + (int) (ieee[2] << 8) + 
		    (int) ((ieee[1] | 128) << 16));
  if (ieee[0] & 128) fmant = -fmant;
  return ldexp(fmant, (int) (exp - 128 - 22));
}


//! Восстановление величины представленное в виде 'sAAAAAAA BBBBBBBB BBBBBBBB BBBBBBBB'
/*! 
  \param data указатель на исходный массив
  \return восстновленное значение
*/
double MnMath::binary2double(const void* src) 
{
  //R = (-1)^s * 2^(-24) * B * 16^(A-64)

  const uchar* ibm = reinterpret_cast<const uchar*>(src);
  int power;
  long int mant;

  mant = (ibm[1] << 16) + (ibm[2] << 8) + ibm[3];
  if (mant == 0) return 0.0;
  
  if (ibm[0] & 0x80) mant = -mant;
  power = (int) (ibm[0] & 0x7f) - 64;
  
  return ldexp(mant * pow(16, power), -24);
}

//! Восстановление исходного значения с плавающей запятой из масштабированной величины
/*! 
  \param val масштабированная величина
  \param r   величина начала отсчета
  \param e   двоичный масштабный коэффициент
  \param d   десятичный масштабный коэффициент
  \return    восстановленное значение
*/
double MnMath::recoverDouble(uint32_t val, uint32_t r, uint16_t e, uint16_t d)
{
  return (r + val*pow(2, e)) * pow(10, -d);
} 
//! Поиск точки нулевого значения или скачка возрастающей (убывающей) и функции func(x) на участке [a,b] методом двойного сечения 
/*! можно использовать в том числе для поиска экстремумов при соответствующем виде функции func(x)
  \param a   начало отрезка
  \param b   конец отрезка
  \param e   заданная точность
  \param func  заданная функция одного аргумента
  \param noNull возвращаемое значение ошибки true если нулей у функции нет или их более 1 на отрезке
  \return    точка с нулевым значением функции func(x)
*/

double MnMath::bisection( double a, double b, double e, double (*func)(double), bool& noNull )
{
  double res = 0.5*(a + b);
  noNull = false;
  
  if( func(a)*func(b) > 0.){
    noNull = true;
    return res;
  }

  if( func(res)*func(a) > 0.) {
    a = res;
  }
  else {
    b = res;
  }
  
  return (fabs(b-a) < e)?  0.5*(a + b) : MnMath::bisection(a, b, e, func, noNull );
}

//! Поиск точки изменения значения (скачка) логической функции func(x) на участке [a,b] методом двойного сечения 
/*! можно использовать в том числе для поиска экстремумов при соответствующем виде функции func(x)
  \param a   начало отрезка
  \param b   конец отрезка
  \param e   заданная точность
  \param func  заданная функция одного аргумента
  \param noNull возвращаемое значение ошибки true если нулей у функции нет или их более 1 на отрезке
  \return    точка изменения значения (скачка) функции func(x)
*/
double MnMath::bisection( double a, double b, double e, bool (*func)(double), bool& noNull )
{
  double res = 0.5*(a + b);
  noNull = false;
  
  if( func(a) == func(b)){
    noNull = true;
    return res;
  }

  if( func(res) == func(a)) {
    a = res;
  }
  else {
    b = res;
  }
  
  return (fabs(b-a) < e)?  0.5*(a + b) : MnMath::bisection(a, b, e, func, noNull );
}



////вспомогательная функция для twoLineIntersection. она ниже определена
//namespace MnMath {
//  bool calcNan( const QPointF& p11, const QPointF& p12, const QPointF& p21, const QPointF& p22, QPointF* resPoint );
//}



// * \brief Найти точку пересечения двух отрезков
// * \param p11 точка1 первого отрезка
// * \param p12 точка2 первого отрезка
// * \param p21 точка1 второго отрезка
// * \param p22 точка2 второго отрезка
// * \param resPoint точка пересечения
// * \return true - если линии пересекаются и не параллельны. иначе - false.
// *
// * если линии параллельны, то resPoint - не устанавливается.
// */
//bool MnMath::twoLineIntersection( const QPointF& p11, const QPointF& p12, const QPointF& p21, const QPointF& p22, QPointF* resPoint ) {
//  if ( 0 == p12.x() - p11.x() || 0 == p12.y() - p11.y() ) {
//    return calcNan( p11, p12, p21, p22, resPoint );
//  }
//  else if ( 0 == p22.x() - p21.x() || 0 == p22.y() - p21.y() ) {
//    return calcNan( p21, p22, p11, p12, resPoint );
//  }
//
//  if ( (p12.x() - p11.x())/(p12.y() - p11.y()) == //parallel
//       (p22.x() - p21.x())/(p22.y() - p21.y()) ) {
//    return false;
//  }
//
//  double x = 0.;
//  double y = 0.;
//
//  x = ( p11.x()*p12.y() - p12.x()*p11.y() )*( p22.x() - p21.x() ) - 
//    ( p21.x()*p22.y() - p22.x()*p21.y() ) * ( p12.x() - p11.x() );
//  x /= ( p11.y() - p12.y() ) * ( p22.x() - p21.x() ) - ( p21.y() - p22.y() ) * ( p12.x() - p11.x() );
//  x = -x;
//
//  y = ( ( p21.y() - p22.y() ) * (-x) - ( p21.x()*p22.y() - p22.x()*p21.y() ) ) / ( p22.x() - p21.x() );
//  resPoint->setX(x);
//  resPoint->setY(y);
//
//  if ( !lineContainsPoint( p11, p12, *resPoint )
//      || !lineContainsPoint( p21, p22, *resPoint ) ) {
//    return false;
//  }
//
//  return true;
//}
//
//bool MnMath::calcNan( const QPointF& p11, const QPointF& p12, const QPointF& p21, const QPointF& p22, QPointF* resPoint ) {
//  if ( p11.x() != p12.x() && p11.y() != p12.y() ) {
//    return false;
//  }
//  double x = 0.;
//  double y = 0.;
//  if ( p11.x() == p12.x() ) {
//    if ( 0 == p22.y() - p21.y() ) {
//      x = p12.x();
//      y = p22.y();
//      resPoint->setX(x);
//      resPoint->setY(y);
//      if ( !lineContainsPoint( p11, p12, *resPoint )
//          || !lineContainsPoint( p21, p22, *resPoint ) ) {
//        return false;
//      }
//      return true;
//    }
//    else if ( 0 == p22.x() - p21.x() ) {
//      //parallel
//      return false;
//    }
//    else {
//      x = p12.x();
//      y = ( p22.y() - p21.y() ) / ( p22.x() - p21.x() ) * ( x - p21.x() ) + p21.y();
//      resPoint->setX(x);
//      resPoint->setY(y);
//      if ( !lineContainsPoint( p11, p12, *resPoint )
//          || !lineContainsPoint( p21, p22, *resPoint ) ) {
//        return false;
//      }
//      return true;
//    }
//  }
//  else if ( p11.y() == p12.y() ) {
//    if ( 0 == p22.x() - p21.x() ) {
//      x = p22.x();
//      y = p12.y();
//      resPoint->setX(x);
//      resPoint->setY(y);
//      if ( !lineContainsPoint( p11, p12, *resPoint )
//          || !lineContainsPoint( p21, p22, *resPoint ) ) {
//        return false;
//      }
//      return true;
//    }
//    else if ( 0 == p22.y() - p21.y() ) {
//      //parallel
//      return false;
//    }
//    else {
//      y = p12.y();
//      x = ( p22.x() - p21.x() ) / ( p22.y() - p21.y() ) * ( y - p21.y() ) + p21.x();
//      resPoint->setX(x);
//      resPoint->setY(y);
//      if ( !lineContainsPoint( p11, p12, *resPoint )
//          || !lineContainsPoint( p21, p22, *resPoint ) ) {
//        return false;
//      }
//      return true;
//    }
//  }
//  return false;
//}
//
//bool MnMath::intervalContainsPoint( double start, double end, double point ) {
//  if ( isZero( point - start ) || isZero( point - end ) ) {
//    return true;
//  }
//  if ( point >= start && point <= end ) {
//    return true;
//  }
//  else if ( point >= end && point <= start ) {
//    return true;
//  }
//  return false;
//}
//
//bool MnMath::lineContainsPoint( const QPointF& start, const QPointF& end, const QPointF& point ) {
//  if ( !intervalContainsPoint( start.x(), end.x(), point.x() ) ) {
//    return false;
//  }
//  if ( !intervalContainsPoint( start.y(), end.y(), point.y() ) ) {
//    return false;
//  }
//  return true;
//}
