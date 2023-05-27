#ifndef TGRIBFORMAT_H
#define TGRIBFORMAT_H

#include <stdint.h>
#include <qstring.h>
#include <qdatetime.h>

namespace google {
  namespace protobuf {
     class Message;
  }
}

namespace grib {
  float ieee2double(const void *data, uchar version);
  int forecastToHour(uchar type, uint32_t val);

  //! преобразование из массива байт в целочисленное
  /*! 
    \param data массив байт
    \param size количество байт значения
    \param res возвращаемый результат
    \param sign true - знаковое, false - нет
    \param allOnes возвращает true, если все биты установлены в 1
  */
  template<class T> void char2dec(const char *data, ushort size, T* res, bool sign = false, bool* allOnes = nullptr)
  {
    *res = 0;
    if (nullptr == data) {
      return;
    }

    bool ones = true;
    if ((uchar)(data[0]) != 0xff) {
      ones = false;
    }
    
    for (uint i=1; i< size; i++) {
      (*res) += (uchar)(data[i]) << ((size-1 - i)*8);
      if ((uchar)(data[i]) != 0xff) {
	ones = false;
      }
    }    
   
    if (sign) {
      (*res) +=  (uchar)(data[0] & 0x7F) << ((size-1)*8);
      if (data[0] & 0x80) {
	*res = -(*res);
      }
    } else {
      (*res) +=  (uchar)(data[0]) << ((size-1)*8);
    }

    if (nullptr != allOnes) {
      *allOnes = ones;
    }    
  }

  //! преобразование из массива байт в целочисленное
  /*! 
    \param data массив байт
    \param size количество байт значения
    \param res возвращаемый результат
    \param sign true - знаковое, false - нет
    \param allOnes возвращает true, если все биты установлены в 1
  */
  template<class T> void char2dec(const uchar *data, ushort size, T* res, bool sign = false, bool* allOnes = nullptr)
  {
    *res = 0;
    if (!data) {
      return;
    }

    bool ones = true;
    if (data[0] != 0xff) {
      ones = false;
    }
    
    for (uint i=1; i< size; i++) {
      (*res) += (data[i]) << ((size-1 - i)*8);
      if (data[i] != 0xff) {
	ones = false;
      }      
    }    

    if (sign) {
      (*res) +=  (data[0] & 0x7F) << ((size-1)*8);
      if (data[0] & 0x80) {
	*res = -(*res);
      }
    } else {
      (*res) +=  (data[0]) << ((size-1)*8);
    }
    
    if (nullptr != allOnes) {
      *allOnes = ones;
    }
  }

}

#endif
