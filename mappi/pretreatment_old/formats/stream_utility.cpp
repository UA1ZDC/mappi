#include "stream_utility.h"

#include <commons/mathtools/mnmath.h>


const unsigned short masks[17] = { 0x0,   0x1,   0x3,   0x7,   0xf,    0x1f,  0x3f,  0x7f, 0xff, 
				 0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3ff, 0x7ff, 0xffff };

bool meteo::cif2raw(const QByteArray& cifBuf, uint offset,
			QByteArray* rawBuf, int* remain, mappi::conf::FrameType type, bool invert)
{
  int wordSize = 10;
  if (type == mappi::conf::kMhrpt) {
    wordSize = 8;
  } 

  return cif2raw(cifBuf, offset, rawBuf, remain,  wordSize, invert);
}

bool meteo::cif2raw(const QByteArray& cifBuf, uint offset,
		    QVector<ushort>* rawBuf, int* remain, bool invert)
{
  int wordSize = 10;

  uint shift_byte= offset / 8;
  uint shift_bit= offset - shift_byte*8;

  signed char rem10 = wordSize;//количество бит, которое ещё осталось записать в raw слово
  signed char rem8 = 8 - shift_bit;//количество бит, которое осталось не прочитанным в cif

  uint8_t cifWord = 0;
  ushort rawWord = 0;
  bool firstRead = true;

  int idx = 0;
  for (; (int)(idx + shift_byte) < cifBuf.size();) {
    if (0 == rem8 || firstRead) {
      cifWord = (uint8_t)cifBuf[idx+shift_byte];
      idx++;
      if (! firstRead) {
	rem8 = 8;
      }
      cifWord &= masks[rem8];
      firstRead = false;
    }
    signed char cnt = rem10 < rem8 ? rem10:rem8; //количство бит, которое будет записано в raw из текущего cif
    rawWord <<= cnt;
    rawWord += (cifWord >> (rem8-cnt)) & masks[cnt];
    rem10 -= cnt;
    rem8 -= cnt;

    if (0 == rem10) {
      if (invert) {
	rawWord ^= 0xFF;
      }
      rawWord &= masks[wordSize];
      rawBuf->append(rawWord);

      rem10 = wordSize;
      rawWord = 0;
    }

  }

  *remain = (cifBuf.size() - shift_byte - idx)*8 + rem8;

  return true;
}


/*! 
 * \brief Преобразование cif в raw16
 * \param cifBuf буфер с данными cif
 * \param offset смещение до данных, которые надо преобразовать, в битах
 * \param rawBuf  буфер с данными raw
 * \param remain количество непреобразованных бит
 * \param wordSize размер слова (8 или 10 бит)
 * \param invert  необходимость инвертировать биты
 * \return  ERR_NOERR в случае успеха, иначе код ошибки
 */
bool meteo::cif2raw(const QByteArray& cifBuf, uint offset,
			QByteArray* rawBuf, int* remain, int wordSize, bool invert)
{
  if (wordSize >= 17) return false;

  uint shift_byte= offset / 8;
  uint shift_bit= offset - shift_byte*8;

  signed char rem10 = wordSize;//количество бит, которое ещё осталось записать в raw слово
  signed char rem8 = 8 - shift_bit;//количество бит, которое осталось не прочитанным в cif

  uint8_t cifWord = 0;
  ushort rawWord = 0;
  bool firstRead = true;

  int idx = 0;
  for (; (int)(idx + shift_byte) < cifBuf.size();) {
    if (0 == rem8 || firstRead) {
      cifWord = (uint8_t)cifBuf[idx+shift_byte];
      idx++;
      if (! firstRead) {
	rem8 = 8;
      }
      cifWord &= masks[rem8];
      firstRead = false;
    }
    signed char cnt = rem10 < rem8 ? rem10:rem8; //количство бит, которое будет записано в raw из текущего cif
    rawWord <<= cnt;
    rawWord += (cifWord >> (rem8-cnt)) & masks[cnt];
    rem10 -= cnt;
    rem8 -= cnt;

    if (0 == rem10) {
      if (invert) {
	rawWord ^= 0xFF;
      }
      rawWord &= masks[wordSize];
      if (8 == wordSize) {
	uint8_t val = rawWord;
        rawBuf->append(val);
      } else {
	uint8_t val = (rawWord >> 8) & masks[8];
        rawBuf->append(val);
	val = rawWord & masks[8];
        rawBuf->append(val);
      }
      rem10 = wordSize;
      rawWord = 0;
    }

  }

  *remain = (cifBuf.size() - shift_byte - idx)*8 + rem8;

  return true;
}


/**
 * проверка буфера на совпадение с синхрой synchro[]
 * совпадение может быть неполным, с допущением некоторого количества "битых бит"
 * @param buf буфер для проверки
 * @param n_error максимально допустимое количество "битых бит"
 * @return true, если синхра совпала
 */
bool meteo::checkSynchro(uint8_t *buf, const uint8_t *synchro, uint length, int n_error)
{
  for(uint i = 0; i < length; ++i) {
    if (MnMath::hemDistance(synchro[i], buf[i]) > n_error) {
      return false;  
    }
  }

  return true;
}

/**
 * проверка буфера на совпадение с синхрой synchro[]
 * совпадение может быть неполным, с допущением некоторого количества "битых бит"
 * @param buf буфер для проверки
 * @param n_error максимально допустимое количество "битых бит"
 * @return true, если синхра совпала
 */
bool meteo::checkSynchro(const QByteArray& buf, const uint8_t *synchro, uint length, int n_error)
{
  for(uint i = 0; i < length; ++i) {
    if (MnMath::hemDistance(synchro[i], (uint8_t)buf[i]) > n_error) {
      return false;  
    }
  }

  return true;
}

/**
 * сдвигает буфер на один бит влево
 * @param buf буфер для сдвига
 */
void meteo::shiftBit(uint8_t *buf, int size)
{
  if (0 == buf || size <= 0) return;

  for (int i = 0; i < size - 1; i++) {
    buf[i] <<= 1;
    buf[i] |= (buf[i+1] & 0x80) >> 7;
  }
  
  buf[size - 1] <<= 1;
}


/*! 
 * \brief Поиск синхропоследовательносити в буффере
 * \param abuf     буфер
 * \param abuf_len размер буфера
 * \param asynchro искомая синхропосл-ть
 * \param asynchro_len размер синхропослед-ти
 * \param asdvig_bit  количество бит до синхропослед-ти от начала буфера (возвращаемое)
 * \param asdvig_byte количество полных байт до синхропослед-ти от начала буфера (возвращаемое)
 * \param mask для NOAA и китайца - маска для сбрасывания последних 4 бит
 * \return ERR_NOERR в случае успеха, иначе код ошибки
 */
bool meteo::findSynchro(const QByteArray& abuf, const uint8_t *asynchro, int asynchro_len, int *asdvig_bit, 
			    int *asdvig_byte, int max_err_bits, int mask, bool invert)
{
  if (abuf.size() <= asynchro_len || asynchro_len < 1) {
    return false;
  }

  uint8_t old = 0;
  int  len = asynchro_len - 1;
  uint8_t buf[asynchro_len + 1];
  for (int idx = 0; idx <= len; idx++) {
    if (invert) {
      buf[idx] =  abuf[idx] ^ 0xFF;;
    } else {
      buf[idx] =  abuf[idx];
    }    
  }

  for(int idx = len; idx < abuf.size() - 1; idx++) {
    if (invert) {
      buf[len] = abuf[idx] ^ 0xFF;
      buf[asynchro_len] = abuf[idx+1] ^ 0xFF;
    } else {
      buf[len] = abuf[idx];
      buf[asynchro_len] = abuf[idx+1];
    }

    for(int bit = 0; bit < 8; bit++) {
      old = buf[len];
      buf[len] &= mask;

      if(checkSynchro(buf, asynchro, asynchro_len, max_err_bits)) {
        *asdvig_bit = ((idx - len) << 3) + bit;
        *asdvig_byte = (*asdvig_bit) >> 3;
        *asdvig_bit -= (*asdvig_byte)*8;
        return true;
      }

      buf[len] = old;
      shiftBit(buf, asynchro_len + 1);
    }
  }

  return false;
}


namespace meteo {
  template<typename T> void ManchesterState<T>::setWord(QByteArray* ba)
  {
    ba->append((uint8_t)(word >> 8));
    ba->append((uint8_t)(word & 0xff));
  } 
 
  template<> void ManchesterState<uint16_t>::setWord(QByteArray* ba)
  {
    ba->append((uint8_t)(word >> 8));
    ba->append((uint8_t)(word & 0xff));
  }

  template<> void ManchesterState<uint8_t>::setWord(QByteArray* ba)
  {
    ba->append(word);
  }
}
