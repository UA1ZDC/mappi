#include "colors.h"

#include <commons/mathtools/mnmath.h>
#include <cross-commons/debug/tlog.h>

#include <math.h>


/*! 
 * \brief Автовыравнивание гистограммы изображения
 * \param src  Исходные данные изображения, grayscale
 * \param dst  Результат
 * \param size Размер изображения
 * \param w    Ширина изображения
 * \param h    Высота изображения
 */
void colors::equalization(const uchar* src, uchar* dst, uint size, uint w, uint h)
{
  uint64_t cdf[256] = {0};
  
  //histogram 
  for (uint idx = 0; idx < size; idx++) {
    cdf[src[idx]] += 1;
  }
  
  uint64_t min = cdf[0];
  for (uint idx = 1; idx < 256; idx++ ) {
    if (min == 0) min = cdf[idx];
    cdf[idx] += cdf[idx-1];
  }

  for (uint64_t i=0; i<size; i++) {
    dst[i] = (uchar)rint((cdf[src[i]]-min)*255./(w*h-min));
    //dst[i] = (uchar)rint(256-1)*cdf[src[i]];
  }
}


/*! 
 * \brief Автовыравнивание гистограммы изображения
 * \param data Данные изображения, grayscale
 * \param size Размер изображения
 * \param w    Ширина изображения
 * \param h    Высота изображения
 */
void colors::equalization(uchar* data, uint size, uint w, uint h)
{
  equalization(data, data, size, w, h);
}



/*! 
 * \brief Растягивание гистограммы
 * \param src  Исходные данные изображения, grayscale
 * \param dst  Результат
 * \param size Размер изображения
 */
void colors::stretchHist(const uchar* src, uchar* dst, uint size)
{
  unsigned count[256] = {0};
  
  //histogram 
  for (uint i=0; i<size; i++) {
    count[src[i]] += 1;
  }
  
  float maxArr[5] = {0};
  for (int j = 0;j < 5; j++) {
    for (int i=0; i<256; i++ ) {
      if (count[i] > maxArr[j]) {
	bool fl = true;
	for (int k=0; k<j-1; k++) {
	  if (count[i] == maxArr[k]) {
	    fl = false;
	    break;
	  }
	}
	if (fl) maxArr[j] = count[i];
      }
      //printf("i=%3d j=%d max=%f count=%d\n", i, j, maxArr[j], count[i]);
    }
  }

  float s1=0;float s2=0, sum=0, min=0, max=255;
  for (int i=0; i<256; i++ ) {
    sum += count[i];
  }
  s2 = sum - count[0];
  for (int i=0; i<256; i++ ) {
    if (s2 !=0 && s1/s2 >= 0.01) {
      min = i-1;
      break;
    }
    s1 += count[i];
    s2 -= count[i];
  }
  s1 = sum - count[255];
  s2 = 0;
  for (int i=255; i>=0; i-- ) {
    if (s1 !=0 && s2/s1 >= 0.01) {
      max = i+1;
      break;
    }
    s1 -= count[i];
    s2 += count[i]; 
  }
  //printf("ss min=%f max=%f\n", min, max);





  // unsigned min = 255;
  // unsigned max = 0;
  // float maxCount = MnMath::meanValue(maxArr, 5);
  // maxCount *= 0.01; // отбросим все значения, количество которых меньше 1% от максимального

  // for (uint i=0; i<256-2; i++ ) {
  //   //для защиты от помех проверяем, если два значения из трёх не попадают в пределы
  //   if ((count[i]   > maxCount && count[i+1] > maxCount) || 
  // (count[i]   > maxCount && count[i+2] > maxCount) ||
  // (count[i+2]   > maxCount && count[i+1] > maxCount)) {
  //     if (min > i || min > (i+1) || min > (i+2)) min = i+1;
  //     if (max < i || max < (i+1) || max < (i+2)) max = i+1;
  //   }
  //   printf("i=%3d count=%6u maxCount=%f min=%u max=%u\n", i, count[i], maxCount, min, max);
  // }

  if (max<min) { //не возможно изменить гистограмму
    debug_log << "max < min" << "max" << max << "min=" << min;
    memcpy(dst, src, size);
    return;
  }

  float k = 255.0 / (max - min);
  for (uint i=0; i<size; i++) {
    if (src[i] > max) dst[i] = 255;
    else if (src[i] < min) dst[i] = 0; 
    else dst[i] = (uchar)(k*( src[i]-min ));
  }
}

/*! 
 * \brief Растягивание гистограммы
 * \param data Данные изображения, grayscale
 * \param size Размер изображения
 */
void colors::stretchHist(uchar* data, uint size)
{
  stretchHist(data, data, size);
}

/*! 
 * \brief  Баланс белого
 * \param data Данные изображения, rgb
 * \param size Размер изображения
 */
void colors::whiteBalance(QRgb* data, uint size)
{
  whiteBalance(data, data, size);
}


/*! 
 * \brief  Баланс белого
 * \param src  Исходные данные изображения, QRgb
 * \param dst  Результат
 * \param size Размер изображения
 */
void colors::whiteBalance(const QRgb* src, QRgb* dst, /*u*/int size)
{
  // for (/*u*/int i=0; i<size; i++) {
  //   dst[i] = src[i];
  // }
  // return;
  // whiteBalance(src, dst, size);
  uchar* r = new uchar[size];
  uchar* g = new uchar[size];
  uchar* b = new uchar[size];

  for (int i=0; i < size; i++) {
    r[i] = qRed (src[i]);
    g[i] = qGreen(src[i]);
    b[i] = qBlue(src[i]);
  }

  stretchHist(r, size);
  stretchHist(g, size);
  stretchHist(b, size);

  for (int i=0; i < size; i++) {
    dst[i]=qRgba(r[i], g[i], b[i], qAlpha(src[i])); 
  }

  if (r) delete[] r;
  if (g) delete[] g;
  if (b) delete[] b;
}

/*! 
 * \brief Баланс белого
 * \param src  Исходные данные изображения, rgb
 * \param dst  Результат
 * \param size Размер массива src и dst
 */
// void colors::whiteBalance(const unsigned* src, unsigned* dst, uint size)
// {
//   uchar* r = new uchar[size];
//   uchar* g = new uchar[size];
//   uchar* b = new uchar[size];

//   for (uint i=0; i < size; i++) {
//     r[i] = (src[i] >> 16) & 0xff;
//     g[i] = (src[i] >>  8) & 0xff;
//     b[i] = src[i] & 0xff;
//   }

//   stretchHist(r, size);
//   stretchHist(g, size);
//   stretchHist(b, size);

//   for (uint i=0; i < size; i++) {
//     dst[i] = (r[i] << 16) + (g[i] << 8) + b[i];
//   }

//   if (r) delete[] r;
//   if (g) delete[] g;
//   if (b) delete[] b;
// }
/*! 
 * \brief Баланс белого
 * \param src  Данные изображения, rgb
 * \param size Размер массива src и dst
 */
// void colors::whiteBalance(unsigned* data, uint size)
// {
//   whiteBalance(data, data, size);
// }
/*! 
 * \brief Баланс белого
 * \param src  Исходные данные изображения, rgb
 * \param dst  Результат
 * \param size Размер массива src и dst
 */
void colors::whiteBalance(const uchar* src, uchar* dst, uint size)
{
//  printf("size=%u\n", size);
  uchar* r = new uchar[size/4];
  uchar* g = new uchar[size/4];
  uchar* b = new uchar[size/4];

  for (uint i=0; i < size/4; i++) {
    r[i] = src[i*4 + 2];
    g[i] = src[i*4 + 1];
    b[i] = src[i*4];
  }

  stretchHist(r, size/4);
  stretchHist(g, size/4);
  stretchHist(b, size/4);

  for (uint i=0; i < size/4; i++) {
    dst[i*4 + 3] = 0;
    dst[i*4 + 2] = r[i];
    dst[i*4 + 1] = g[i];
    dst[i*4] = b[i];
  }

  if (r) delete[] r;
  if (g) delete[] g;
  if (b) delete[] b;
}
/*! 
 * \brief Баланс белого
 * \param src  Данные изображения, rgb
 * \param size Размер массива src и dst
 */
void colors::whiteBalance(uchar* data, uint size)
{
  whiteBalance(data, data, size);
}

/*! 
 * \brief Gamma correction для всех цветов rgb (т.е. яркости)
 * \param src  Исходные данные изображения, rgb
 * \param dst  Результат
 * \param size Размер массива src и dst
 * \param koef Коэффициент усиления/ослабления
 */
void colors::gammaCorrection(const uchar* src, uchar* dst, uint size, float koef)
{
  for (uint i=0; i < size; i++) {
    dst[i] = uchar(pow((float(src[i])/255.0), koef) * 255.0);
  }
}
/*! 
 * \brief Gamma correction для всех цветов rgb (т.е. яркости)
 * \param src  Данные изображения, rgb
 * \param size Размер массива src и dst
 * \param koef Коэффициент усиления/ослабления 
 */
void colors::gammaCorrection(uchar* data, uint size, float koef)
{
  gammaCorrection(data, data, size, koef);
}

/*! 
 * \brief Наложение одного изображения на другое
 * \param data Исходные данные
 * \param add Накладываемые данные
 * \param transp Цвет, который необходимо исключить при накладывании
 */
void colors::rgbRgbTransp(uchar* data, const uchar* add, uint size, QColor transp)
{
  for (uint i=0; i<size; i+=4) {
    if (! (add[i] == transp.blue() && 
	   add[i+1] == transp.green() &&
	   add[i+2] == transp.red()) ) {
      data[i] = add[i];
      data[i+1] = add[i+1];
      data[i+2] = add[i+2];
      data[i+3] = add[i+3];
    }
  }
}


void colors::grayRgbTransp(uchar* gray, const uchar* add, uint size, QColor transp, uchar* res)
{
  for (uint i=0; i<size; i+=4) {
    if (! (add[i] == transp.blue() && 
	   add[i+1] == transp.green() &&
	   add[i+2] == transp.red()) ) {
      res[i] = add[i];
      res[i+1] = add[i+1];
      res[i+2] = add[i+2];
      res[i+3] = add[i+3];
    } else {
      res[i] = gray[i/4];
      res[i+1] = res[i];
      res[i+2] = res[i];
      res[i+3] = res[i];
    }
  }
}

void colors::indexRgbTransp(uchar* ind, const uchar* add, uchar* pal, uint size, QColor transp, uchar* res)
{
  for (uint i=0; i<size; i+=4) {
    if (! (add[i] == transp.blue() && 
	   add[i+1] == transp.green() &&
	   add[i+2] == transp.red()) ) {
      res[i] = add[i];
      res[i+1] = add[i+1];
      res[i+2] = add[i+2];
      res[i+3] = add[i+3];
    } else {
      res[i] = pal[4*ind[i/4]];
      res[i+1] = pal[4*ind[i/4]+1];
      res[i+2] = pal[4*ind[i/4]+2];
      res[i+3] = pal[4*ind[i/4]+3];
    }
  }
}

void colors::invert(uchar* data, uint size)
{
  if (nullptr == data) {
    return;
  }
  
  for (uint idx = 0; idx < size; idx++) {
    data[idx] = 255 - data[idx];
  }
}
