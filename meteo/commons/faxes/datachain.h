#include <qlist.h>
#include <qobject.h>
#include <qbytearray.h>
#include <qhash.h>

#ifndef METEO_COMMONS_FAXES_DATACHAIN_H
#define METEO_COMMONS_FAXES_DATACHAIN_H

namespace meteo {

namespace internal {

/**
  ChainHeaderStruct - структура заголовка телеграммы в соответствии с руководством
  */
//Выравнивание структуры - отключаем
#pragma pack(1)
struct ChainSegmentHeaderStruct {
  char	signature[8];
  short	type;
  short	types;
  int	frameN;
  int	packedSize;
  int	packedOffset;
  int	unpackedSize;
  int	unpackedOffset;
};
//Возвращаем выравнивание структуры к значению по умолчанию
#pragma pack()

}
/**
 * @brief The ChainSegmentHeader class
 * Класс для доступа к полям заголовка сегмента(бинарный заголовок talarina)
 */
class ChainSegmentHeader {
public:
  ChainSegmentHeader();
  ChainSegmentHeader(const internal::ChainSegmentHeaderStruct& data);

  bool isValid() const;

  QString signature() const;
  short	type() const;
  short	types() const;
  long	frameN() const;
  long	packedSize() const;
  long	packedOffset() const;
  long	unpackedSize() const;
  long	unpackedOffset() const;
private:
  bool isEmpty = true;
  QString signature_;
  short	type_;
  short	types_;
  long	frameN_;
  long	packedSize_;
  long	packedOffset_;
  long	unpackedSize_;
  long	unpackedOffset_;
};

/**
 * @brief The FormatString class
 * Класс для доступа к полям форматной строки сегмента(FAX MAGIC LPM IOC NNNN COD TYP)
 */
class FormatString {
public:
  FormatString();
  FormatString(const QString& str);

  enum ChainDocType {
    KFax,
    kDoc,
    kUncDocType
  };

  enum LPM {
    kLPM_060,
    kLPM_090,
    kLPM_120,
    kLPM_UNK
  };

  enum IOC {
    kIOC_288,
    kIOC_567,
    kIOC_UNK
  };

  enum Coding {
    kCompress,
    kRTX,
    kUncknowCodding
  };
  enum SegmentType {
    kBegin,
    kBody,
    kEnd,
    kUncSegmentType
  };

  ChainDocType chainDocType() const;
  int magic() const;
  LPM lpm() const;
  IOC ioc() const;
  int nnnn() const;
  Coding code() const;
  SegmentType segType() const;

private:
  bool isEmpty_ = true;
  bool valid_ = false;

  ChainDocType docType_ = ChainDocType::kUncDocType;
  int magic_ = 0;
  LPM lpm_ = kLPM_UNK;
  IOC ioc_ = kIOC_UNK;
  int nnnn_ = 0;
  Coding code_ = kUncknowCodding;
  SegmentType segType_ = kBegin;
};

//Размер форматной строки факсов
static const QString& formatString = QObject::tr("FAX MAGIC LPM IOC NNNN COD TYP\r\r\n");
static const int formatStringSize = 33;

/**
 * @brief The ChainSegment class
 * Класс для доступа к данным сегмента: заголовок, форматная строка
 */
class ChainSegment {

public:
  ChainSegment();
  /**
   * @brief DataChainMessage - раскодировать сообщение в формате DataChain
   * @param data
   */
  ChainSegment(const QByteArray& data);

  bool isValid() const;

  ChainSegmentHeader header() const;
  FormatString formatString() const;
  QByteArray data() const;

private:
  bool isEmpty_ = true;
  FormatString formatString_;
  ChainSegmentHeader header_;
  QByteArray data_;
};

/**
 * @brief The Chain class
 * Класс для сбора набора сегментов в общий единый буфер
 */
class Chain
{
public:
  /**
   * @brief DataChain - раскодировать цепочку сообщений в формате DataChain
   * @param messages
   */
  Chain(QList<ChainSegment> segments);

  /**
   * @brief tryExtract - попытка извлечь данные из цепочки сегментов
   * @return : собранный массив данных или пустой массив в случае ошибки
   */
  QByteArray tryExtract();

private:
  QList<ChainSegment> segments_;
  QList<ChainSegment> invalidSegments_;
};

/**
 * @brief The FileBuffer class
 * Класс для разбора файлового буфера(DOCCHAIN)
 */
class FileBuffer
{
public:
  FileBuffer(const QByteArray& input);
  FileBuffer(const QByteArray& input, const QString &fileName);

  bool isValid();
  bool isEmpty();

  QString fileName() const;
  const QByteArray &data() const;

private:
  bool isValid_ = false;
  QString fileName_;
  QByteArray data_;

};

/**
 * @brief The FaxBuffer class
 * Класс для разбора буфера факса(FAXCHAIN)
 */

class FaxBuffer
{
public:
  FaxBuffer(const QByteArray& input, FormatString::LPM lpm);
  QByteArray tiffBuffer() const;
  bool isValid() const;

private:
  static const QByteArray littleEndian2bigEndian(const QByteArray& le);
  static const QByteArray normalizeData(const QByteArray& src, int lpm, int offset);

private:
  static const int offsetForNormalized =14;
  QByteArray faxAsTiff_;
};

}

#endif
