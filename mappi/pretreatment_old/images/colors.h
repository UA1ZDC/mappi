#ifndef SPCOLORS_H
#define SPCOLORS_H

#include <qstring.h>
#include <qcolor.h>

namespace colors {
  void equalization(const uchar* src, uchar* dst, uint size, uint w, uint h);
  void equalization(uchar* data, uint size, uint w, uint h);

  void stretchHist(const uchar* src, uchar* dst, uint size);
  void stretchHist(uchar* data, uint size);


  void whiteBalance(const QRgb* src, QRgb* dst, /*u*/int size);
  void whiteBalance(QRgb* data, uint size);
  /* void whiteBalance(const unsigned* src, unsigned* dst, uint size); */
  /* void whiteBalance(unsigned* data, uint size); */
  void whiteBalance(const uchar* src, uchar* dst, uint size);
  void whiteBalance(uchar* data, uint size);

  void gammaCorrection(const uchar* src, uchar* dst, uint size, float koef);
  void gammaCorrection(uchar* data, uint size, float koef);

  void rgbRgbTransp(uchar* data, const uchar* add, uint size, QColor transp);
  void grayRgbTransp(uchar* gray, const uchar* rgb, uint size, QColor transp, uchar* res);
  void indexRgbTransp(uchar* ind, const uchar* add, uchar* pal, uint size, QColor transp, uchar* res);

  void invert(uchar* data, uint size);
}

#endif //SPCOLORS_H
