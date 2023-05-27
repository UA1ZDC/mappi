#ifndef MAPPI_PRETREATMENT_FORMATS_STREAM_UTILITY_H
#define MAPPI_PRETREATMENT_FORMATS_STREAM_UTILITY_H

#include <mappi/proto/satellite.pb.h>
#include <cross-commons/debug/tlog.h>

#include <qbytearray.h>

#include <stdint.h>

namespace meteo {
bool cif2raw(const QByteArray& cifBuf, uint offset,
             QByteArray* rawBuf, int* remain, mappi::conf::FrameType type, bool invert);
bool cif2raw(const QByteArray& cifBuf, uint offset,
             QByteArray* rawBuf, int* remain, int wordSize, bool invert);

bool cif2raw(const QByteArray& cifBuf, uint offset,
             QVector<ushort>* rawBuf, int* remain, bool invert);
bool findSynchro(const QByteArray& abuf, const uint8_t *asynchro, int asynchro_len, int *asdvig_bit,
                 int *asdvig_byte, int max_err_bits, int mask, bool invert);

bool checkSynchro(uint8_t *buf, const uint8_t *synchro, uint length, int n_error);
bool checkSynchro(const QByteArray& buf, const uint8_t *synchro, uint length, int n_error);
void shiftBit(uint8_t *buf, int size);


void manchesterDecode(const QByteArray& data, QByteArray* decoded);

enum DecodeMode {
  kFindSync = 1,
  kResync = 2,
  kCheckSync = 3,
};

//! Состояние при раскодировании
template <typename T> struct ManchesterState {
  bool isMidBit = false;	//!< true - середина бита слова данных
  bool isCheckMidBit = false; //!< true - середина бита синхры
  DecodeMode mode = kFindSync; //!< Режим раскодирования
  int threshold = 1; //!< Порог для смены режима

  uint64_t shifter = 0;	//!< для проверки синхры
  T word = 0;	//!< слово данных
  uint8_t bitcnt = 0;	//!< номер считанного бита данных
  uint16_t wordcnt = 0;	//!< номер считанного слова данных

  void setWord(QByteArray* ba);
};

class Decoder {
public:
  Decoder() {}
  virtual ~Decoder() {}

  virtual bool decode(const QByteArray& data, QByteArray* decoded) { Q_UNUSED(data); Q_UNUSED(decoded); return false; }
  virtual bool decodeInvert(const QByteArray& data, QByteArray* decoded) { Q_UNUSED(data); Q_UNUSED(decoded); return false; }
  virtual void setThresholds(int syncThresh, int resyncThresh) { Q_UNUSED(syncThresh); Q_UNUSED(resyncThresh); }

private:
};

//! Снятие манчестерского кода, поиск синхры, восстановление размеров кадров
template <typename T> class ManchesterDecoder : public Decoder {
public:
  ManchesterDecoder(const QByteArray& synchroba, uint64_t synchro, uint64_t syncmask, int frameSize, int wordSize);
  ~ManchesterDecoder();

  void setThresholds(int syncThresh, int resyncThresh) {
    _syncThresh = syncThresh;
    _resyncThresh = resyncThresh;
    _state.threshold = _syncThresh;
  }

  bool decode(const QByteArray& data, QByteArray* decoded);
  bool decodeInvert(const QByteArray& data, QByteArray* decoded);

private:
  void setup(const QByteArray& synchroba, uint64_t synchro, uint64_t syncmask, int frameSize, int wordSize);

  bool isSynEndPosition();
  void restoreFrameSize(QByteArray* decoded);

  void decodeWord(uint8_t bit, QByteArray* decoded);

  void findSyncMode(uint8_t bit, uint8_t diff, QByteArray* decoded);
  void findResyncMode(uint8_t bit, uint8_t diff,  QByteArray* decoded);
  void checkSyncMode(uint8_t bit, uint8_t diff, QByteArray* decoded);

private:
  QByteArray _synchroba; //!< Синхрострока в виде массива
  uint64_t _synchro = 0; //!< Синхрострока числом
  uint64_t _syncmask = 0; //!< Маска для выделения синхростроки
  uint8_t _syncwords = 4; //!< Число слов, занимаемых синхрой
  int _frameSize = 0; //!< Размер кадра данных
  int _wordSize = 0;  //!< Размер слова данных

  QByteArray _frame;

  int _syncThresh = 1; //!< Порог для переключения в режим kCheckSync (число верно найденных синхропоследовательностей)
  int _resyncThresh = 5; //!< Порог для переключения в режим kResync (число ошибок при проверке синхропоследовательности)

  ManchesterState<T> _state;
};
}


//! Снятие манчестерского кода, поиск синхры, восстановление размеров кадров
template <typename T>meteo::ManchesterDecoder<T>::ManchesterDecoder(const QByteArray& synchroba, uint64_t synchro, uint64_t syncmask, 
                                                                    int frameSize, int wordSize)
{
  setup(synchroba, synchro, syncmask, frameSize, wordSize);
}

template <typename T> meteo::ManchesterDecoder<T>::~ManchesterDecoder()
{
}

/*! 
  \param synchroba  синхропоследовательность
  \param frameSize  размер кадра, байт
  \param wordSize   размер слова, бит
  \param syncThresh порог для установления нахождения синхропоследовательности
  \param resyncThresh порог для установления потери синхропоследовательности
*/
template <typename T> void meteo::ManchesterDecoder<T>::setup(const QByteArray& synchroba, uint64_t synchro, uint64_t syncmask, int frameSize, int wordSize)
{
  _synchroba = synchroba;
  // for (int idx = synchroba.size() - 1, sh = 0; idx >=0; idx--, sh += 8) {
  //   _synchro += ((uint64_t)((uchar)synchroba[idx]) << sh);
  // }
  _synchro = synchro;
  _syncmask = syncmask;
  if (wordSize == 8) {
    _syncwords = synchroba.size();
  } else {
    _syncwords = synchroba.size() / 2;
  }

  _frameSize = frameSize;
  _wordSize = wordSize;

  _state.threshold = _syncThresh;
  _state.bitcnt = wordSize;

  _state.wordcnt = frameSize - _syncwords;
}

template <typename T> bool meteo::ManchesterDecoder<T>::decode(const QByteArray& data, QByteArray* decoded)
{
  bool ok = false;
  uint8_t prev = 0;
  uint8_t bit = 0;

  for (int64_t byte = 0; byte < data.size(); byte++) {
     if (byte % 1000000 == 0) {
       debug_log << byte << data.size() << decoded->size();
     }
    for (int shift = 7; shift >= 0; shift--) {
      bit = (((uint8_t)data.at(byte)) >> shift) & 0x1;
      uint8_t diff = bit ^ prev;
      //debug_log << bit << prev << diff << _state.isMidBit << byte << shift << db1((uchar)data.at(byte)) << _state.word;
      prev = bit;
      
      switch (_state.mode) {
        case kFindSync:
          findSyncMode(bit, diff, decoded);
        break;
        case kResync:
          findResyncMode(bit, diff, decoded);
        break;
        case kCheckSync:
          ok = true;
          checkSyncMode(bit, diff, decoded);
        break;
      }

    }
  }

  return ok;
}

template <typename T> bool meteo::ManchesterDecoder<T>::decodeInvert(const QByteArray& data, QByteArray* decoded)
{
  bool ok = false;
  uint8_t prev = 0;
  uint8_t bit = 0;
  
  for (int64_t byte = 0; byte < data.size(); byte++) {
    // if (byte % 1000000 == 0) {
    //   debug_log << byte << data.size() << decoded->size();
    // }
    for (int shift = 7; shift >= 0; shift--) {
      bit = ((~(uint8_t)data.at(byte)) >> shift) & 0x1;
      uint8_t diff = bit ^ prev;
      //debug_log << bit << prev << diff << _state.isMidBit << byte << shift << db1((uchar)data.at(byte)) << _state.word;
      prev = bit;
      
      switch (_state.mode) {
        case kFindSync:
          findSyncMode(bit, diff, decoded);
        break;
        case kResync:
          findResyncMode(bit, diff, decoded);
        break;
        case kCheckSync:
          ok = true;
          checkSyncMode(bit, diff, decoded);
        break;
      }

    }
  }

  return ok;
}

//! Синхронизация без записи данных (начальное состояние)
template <typename T> void meteo::ManchesterDecoder<T>::findSyncMode(uint8_t bit, uint8_t diff, QByteArray* decoded)
{
  // debug_log << _state.isMidBit << diff;

  if (_state.isMidBit && 0 != diff) {
    _state.shifter = (_state.shifter << 1) | bit;

    //debug_log << _state.shifter << db1(_synchro) << bit << _synchroba.toHex();

    if ((_state.shifter & _syncmask) == _synchro) {
      _state.mode = kResync;
      _state.bitcnt = _wordSize;
      _state.wordcnt = _frameSize - _syncwords;
      _state.word = 0;
      _frame.append(_synchroba);
      _state.isCheckMidBit = false;
      debug_log << "sync found" << decoded->size() << _frame.size();
    }

    _state.isMidBit = false;
  } else {
    _state.isMidBit = true;
  }
}

//TODO когда синхро не найдена, можно проверить расстояние Хемминга, и порог установить с учетом его

//! Сохранение данных с синхронизацией заново
template <typename T> void meteo::ManchesterDecoder<T>::findResyncMode(uint8_t bit, uint8_t diff, QByteArray* decoded)
{
  decodeWord(bit, decoded);
  
  //проверка синхры, синхронизация
  if (_state.isCheckMidBit && 0 != diff) {

    _state.shifter = (_state.shifter << 1) | bit;

    if ((_state.shifter & _syncmask) == _synchro) {
      //синхра найдена не там, где ожидалась
      if (!isSynEndPosition()) {

        debug_log << "resync" << _state.threshold << _state.wordcnt << _state.bitcnt;
        restoreFrameSize(&_frame);
        decoded->append(_frame.left(_frame.size() - _synchroba.size()));
        _frame = _synchroba;

        _state.threshold = _syncThresh;
        _state.bitcnt = _wordSize;
        _state.wordcnt = _frameSize - _syncwords;
        _state.word = 0;
        _state.isMidBit = false;
      } else {
        --_state.threshold;
        //debug_log << "resync ok" << _state.threshold;
      }
    } else if (isSynEndPosition()) {
      _state.threshold = _syncThresh; //синхры нет, где ожидалась
    }

    _state.isCheckMidBit = false;
  } else {
    _state.isCheckMidBit = true;
  }
  
  if (_state.threshold == 0) {
    _state.mode = kCheckSync;
    _state.threshold = _resyncThresh;
  }
}

//! Сохранение данных с проверкой синхры (без синхронизации заново)
template <typename T> void meteo::ManchesterDecoder<T>::checkSyncMode(uint8_t bit, uint8_t diff, QByteArray* decoded)
{ 
  decodeWord(bit, decoded);
  
  //проверка синхры
  if (_state.isCheckMidBit && 0 != diff) {

    _state.shifter = (_state.shifter << 1) | bit;

    if ((_state.shifter & _syncmask) == _synchro) {
      if (!isSynEndPosition()) {
        --_state.threshold;
        //debug_log << "chsync" << _state.threshold << _state.wordcnt << _state.bitcnt;
      } else {
        _state.threshold = _resyncThresh;
      }
    } else if (isSynEndPosition()) {
      --_state.threshold;
      //debug_log << "chsync" << _state.threshold;
    }

    _state.isCheckMidBit = false;
  } else {
    _state.isCheckMidBit = true;
  }
  
  if (_state.threshold == 0) {
    _state.mode = kResync;
    _state.threshold = _syncThresh;
    //debug_log << "syn lost " << decoded->size();
  }
}

//! Проверка положения синхры
template <typename T> bool meteo::ManchesterDecoder<T>::isSynEndPosition()
{
  return (_state.wordcnt == _frameSize - _syncwords) && _state.bitcnt == _wordSize;
}

//дополняем кадр до корректного размера, если новая синхра нашлась раньше, чем кадр кончился
template<typename T> void meteo::ManchesterDecoder<T>::restoreFrameSize(QByteArray* decoded)
{
  int f = sizeof(_state.word);

  if (_frameSize - _state.wordcnt < _syncwords) { //считано в новый кадр меньше, чем размер синхры
    decoded->append(QByteArray(f*(_syncwords - (_frameSize - _state.wordcnt)), char(0))); //дописываем до размера синхры
  } else if (_frameSize - _state.wordcnt > _syncwords) {
    decoded->append(QByteArray(f*(_state.wordcnt + _syncwords), char(0)));
  }

  decoded->replace(decoded->size() - _syncwords*f, _syncwords*f, _synchroba);
}

//сохранение раскодированного слова данных
template<typename T> void meteo::ManchesterDecoder<T>::decodeWord(uint8_t bit, QByteArray* decoded)
{
  if (_state.isMidBit) {
    _state.word = (_state.word << 1) | bit;
    if (--_state.bitcnt == 0) {
      _state.setWord(&_frame);
      _state.bitcnt = _wordSize;
      _state.word = 0;
      if (--_state.wordcnt == 0) {
        _state.wordcnt = _frameSize;
        decoded->append(_frame);
        _frame.clear();
      }
    }
    _state.isMidBit = false;
  } else {
    _state.isMidBit = true;
  }
}


#endif
