#include "tgribformat.h"

#include <math.h>
#include <stdio.h>
#include <commons/mathtools/mnmath.h>

// #define BYTE1 0xff
// #define BYTE2 0xffff
// #define BYTE4 0xffffffff


//! Восстановление величины представленное в стандарте IEEE
/*! 
  \param data указатель на исходный массив
  \param version версия GRIB
  \return восстновленное значение
*/
float grib::ieee2double(const void *data, uchar version)
{
  if (version == 1) {
    return MnMath::binary2double(data);
  }

  return MnMath::ieee2double(data);
}

int grib::forecastToHour(uchar type, uint32_t val)
{
  int hour = val;

  const int mul[] = {60, 1, 24, 24*30, 365*24, 10*365*24, 30*365*24, 100*365*24, 1, 1, 3, 6, 12};
  if (type == 0) {
    hour = val / mul[0];
  } else {
    hour = val * mul[type];
  }

  return hour;
}

