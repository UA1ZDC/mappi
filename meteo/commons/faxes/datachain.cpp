#include "datachain.h"
#include "tiffconvert.h"

#include <commons/compresslib/mcompress.h>
#include <qregexp.h>
#include <cross-commons/debug/tlog.h>
#include <sstream>

#include <tiffio.h>
#include <tiffio.hxx>


namespace meteo {

ChainSegmentHeader::ChainSegmentHeader()
{

}

ChainSegmentHeader::ChainSegmentHeader(const internal::ChainSegmentHeaderStruct& data)
{
  isEmpty = false;
  const QByteArray& signatureData = QByteArray::fromRawData(data.signature, 8);
  signature_ = QString::fromUtf8(signatureData);
  type_ = data.type;
  types_ = data.types;
  frameN_ = data.frameN;
  packedSize_ = data.packedSize;
  packedOffset_ = data.packedOffset;
  unpackedSize_ = data.unpackedSize;
  unpackedOffset_ = data.unpackedOffset;
}

bool ChainSegmentHeader::isValid() const
{
  if ( true == isEmpty ){
    return false;
  }
  return true;
}

QString ChainSegmentHeader::signature() const
{
  return signature_;
}

short	ChainSegmentHeader::type() const
{
  return type_;
}

short	ChainSegmentHeader::types() const
{
  return types_;
}

long	ChainSegmentHeader::frameN() const
{
  return frameN_;
}

long	ChainSegmentHeader::packedSize() const
{
  return packedSize_;
}

long	ChainSegmentHeader::packedOffset() const
{
  return packedOffset_;
}

long	ChainSegmentHeader::unpackedSize() const
{
  return unpackedSize_;
}

long	ChainSegmentHeader::unpackedOffset() const
{
  return unpackedOffset_;
}

FormatString ChainSegment::formatString() const
{
  return formatString_;
}

ChainSegment::ChainSegment()
{
}

ChainSegment::ChainSegment(const QByteArray& data){
  if ( false == data.isEmpty() ){
    isEmpty_ = false;
  }
  QString formatString = data.mid(0, formatStringSize);
  formatString_ = FormatString(formatString);

  QByteArray headerData_ = data.mid(formatStringSize, sizeof(internal::ChainSegmentHeaderStruct) );

  if ( headerData_.size() == sizeof(internal::ChainSegmentHeaderStruct) ) {
    const internal::ChainSegmentHeaderStruct *headerStruct = reinterpret_cast<const internal::ChainSegmentHeaderStruct*>(headerData_.data());
    header_ = ChainSegmentHeader(*headerStruct);
  }

  data_ = data.mid(formatStringSize + sizeof (internal::ChainSegmentHeaderStruct), header().packedSize() );
}

bool ChainSegment::isValid() const
{
  if ( true == isEmpty_ ){
    return false;
  }
  if ( false == header_.isValid() ){
    return false;
  }
  return true;
}

ChainSegmentHeader ChainSegment::header() const
{
  return header_;
}

QByteArray ChainSegment::data() const
{
  return data_;
}


Chain::Chain(QList<ChainSegment> segments)
{
  for ( auto segment: segments ){
    if ( true == segment.isValid() ){
      segments_ << segment;
    }
    else {
      segments_ << segment;
    }
  }

  if ( 0 != invalidSegments_.size() ) {
    warning_log << QObject::tr("Предупреждение: получены невалидные сегменты. Количество сегментов: %1")
                   .arg(invalidSegments_.size());
  }

}

FormatString::FormatString()
{

}

FormatString::FormatString(const QString& str)
{
  static const QString& regexp = QObject::tr("^(\\w{3}) (\\d{5}) (\\d{3}) (\\d{3}) (\\d{4}) (\\w{3}) (\\w{3})\r\r\n$");
  QRegExp rx(regexp);
  if ( 0 != rx.indexIn(str) ){
    error_log << QObject::tr("Ошибка: полученная строка не соответствует требуемому формату: '%1'")
                 .arg(str);
    return;
  }

  int i = 1;
  const QString& type = rx.cap(i++);
  const QString& magic = rx.cap(i++);
  const QString& lpm = rx.cap(i++);
  const QString& ioc = rx.cap(i++);
  const QString& nnnn = rx.cap(i++);
  const QString& cod = rx.cap(i++);
  const QString& segmentType = rx.cap(i++);

  isEmpty_ = false;
  valid_ = true;

  if ( 0 == type.compare(QObject::tr("FAX"), Qt::CaseInsensitive)) {
    docType_ = FormatString::ChainDocType::KFax;
  }
  else if ( 0 == type.compare(QObject::tr("DOC"), Qt::CaseInsensitive) ){
    docType_ = FormatString::ChainDocType::kDoc;
  }
  else {
    docType_ = FormatString::ChainDocType::kUncDocType;
    //valid_ = false;
  }

  bool ok = false;
  magic_ = magic.toInt(&ok);
  if ( false == ok ){
    valid_ = false;
  }

  int lpmInt = lpm.toInt();
  switch (lpmInt) {
    case 60:{
      lpm_ = FormatString::LPM::kLPM_060;
      break;
    }
    case 90:{
      lpm_ = FormatString::LPM::kLPM_090;
      break;
    }
    case 120:{
      lpm_ = FormatString::LPM::kLPM_120;
      break;
    }
    default:{
      lpm_ = FormatString::LPM::kLPM_UNK;
      //     valid_ = false;
      break;
    }
  }

  int intIoc = ioc.toInt();
  switch (intIoc) {
    case 288:{
      ioc_ = FormatString::IOC::kIOC_288;
      break;
    }
    case 576:{
      ioc_ = FormatString::IOC::kIOC_567;
      break;
    }
    default:{
      ioc_ = FormatString::IOC::kIOC_UNK;
      //valid_ = false;
    }
  }

  ok = false;
  nnnn_ = nnnn.toInt(&ok);
  if ( false == ok ){
    debug_log << "FAIL" << nnnn_;
    valid_ = false;
  }

  if ( 0 == cod.compare(QObject::tr("COM"), Qt::CaseInsensitive) ){
    code_ = FormatString::Coding::kCompress;
  }
  else {
    if ( 0 == cod.compare(QObject::tr("RTX"), Qt::CaseInsensitive) ){
      code_ = FormatString::Coding::kRTX;
    } else {
      code_ = FormatString::Coding::kUncknowCodding;
      //valid_ = false;
    }
  }

  if ( 0 == segmentType.compare(QObject::tr("BEG"), Qt::CaseInsensitive ) ){
    segType_ = FormatString::SegmentType::kBegin;
  }
  else if ( 0 == segmentType.compare(QObject::tr("BOD"), Qt::CaseInsensitive ) ){
    segType_ = FormatString::SegmentType::kBody;
  }
  else if ( 0 == segmentType.compare(QObject::tr("END"), Qt::CaseInsensitive ) ){
    segType_ = FormatString::SegmentType::kEnd;
  }
  else {
    segType_ = FormatString::SegmentType::kUncSegmentType;
    //    valid_ = false;
  }

  if ( false == valid_ ){
    error_log << QObject::tr("Ошибка при раскодировании строки формата");
    debug_log << docType_
              << magic_
              << lpm_
              << ioc_
              << nnnn_
              << code_
              << segType_;
  }
}

FormatString::ChainDocType FormatString::chainDocType() const
{
  return docType_;
}

int FormatString::magic() const
{
  return magic_;
}

FormatString::LPM FormatString::lpm() const
{
  return lpm_;
}

FormatString::IOC FormatString::ioc() const
{
  return ioc_;
}

int FormatString::nnnn() const
{
  return nnnn_;
}

FormatString::Coding FormatString::code() const
{
  return code_;
}

FormatString::SegmentType FormatString::segType() const
{
  return segType_;
}

FileBuffer::FileBuffer(const QByteArray& input, const QString &fileName)
{
  QByteArray fileNameBuf = fileName.toUtf8();
  int32_t fns = fileNameBuf.size();
  data_.resize(input.size()+fns+sizeof (int32_t));
  memcpy(data_.data(),&fns,sizeof (int32_t));
  data_ = data_.replace(sizeof (int32_t), fns, fileNameBuf);
  data_ = data_.replace(sizeof (int32_t)+fns, input.size(), input);
  isValid_ = true;
}

FileBuffer::FileBuffer(const QByteArray& input)
{
  QByteArray sizeBuf = input.left(sizeof (int32_t));
  if ( true == sizeBuf.isEmpty() ){
    isValid_ = false;
  }
  uint32_t size = * reinterpret_cast<uint32_t*>(sizeBuf.data());
  QByteArray fileNameBuf = input.mid(sizeof (int32_t), size);

  if ( true == fileNameBuf.isEmpty() ){
    isValid_ = false;
  }
  else {
    fileName_ = QString::fromUtf8(fileNameBuf);
  }
  data_ = input.mid(sizeof(int32_t) + size);
  isValid_ = (false == data_.isEmpty());
}

QString FileBuffer::fileName() const
{
  return fileName_;
}

const QByteArray &FileBuffer::data() const
{
  return data_;
}

QByteArray Chain::tryExtract()
{
  auto getSegmentByIndex = [this](int n){
    for ( auto segment: segments_ ){
      if ( segment.formatString().nnnn() == n ){
        return segment;
      }
    }
    return ChainSegment();
  };

  int firstSegment = -1;
  int lastSegment = -1;
  for ( auto segment: segments_ ){
    if ( false == segment.isValid() ){
      continue;
    }
    auto formatString = segment.formatString();
    if ( FormatString::SegmentType::kBegin == formatString.segType() ){
      firstSegment = formatString.nnnn();
    }
    else if ( FormatString::SegmentType::kEnd == formatString.segType() ){
      lastSegment = formatString.nnnn();
    }
  }

  if ( -1 == firstSegment ) {
    error_log << QObject::tr("Не удается собрать объект из списка сегментов. Не найден начальный сегмент");
  }
  else if ( 1 != firstSegment ){
    error_log << QObject::tr("Не удается собрать объект из списка сегментов. Начальный сегмент имеет неверный индекс %1")
                 .arg(firstSegment);
    return QByteArray();
  }
  else if ( -1 == lastSegment ) {
    error_log << QObject::tr("Не удается собрать объект из списка сегментов. Не найден закрывающий сегмент");
  }
  else if ( lastSegment < firstSegment ){
    error_log << QObject::tr("Не удается собрать объект из списка сегментов. Индекс начального сегмента меньше, чем индекс закрываюшего сегмента");
  }

  compress::Compress compressor;
  QByteArray data;
  for ( int i = 1; i <= lastSegment; ++i ){
    const ChainSegment& segment = getSegmentByIndex(i);
    if ( false == segment.isValid() ){
      error_log << QObject::tr("Ошибка: не удается найти сегмент %1");
      return QByteArray();
    }
    auto header = segment.header();
    QByteArray packed = segment.data();
    if ( header.packedSize() != packed.size() ){
      warning_log << QObject::tr("Предупреждение: размер нераспакованного сегмента данных не совпадает с указанным в сообщении размером.");
    }

    QByteArray unpacked;
    switch ( segment.formatString().code() ) {
      case FormatString::Coding::kCompress: {
        unpacked = compressor.decompress(packed);
        break;
      }
      case FormatString::Coding::kRTX: {
        unpacked = packed;
        break;
      }
      case FormatString::Coding::kUncknowCodding: {
        unpacked = packed;
        break;
      }
    }

    if ( header.unpackedSize() != unpacked.size() ){
      error_log << QObject::tr("Предупреждение: размер распакованного сегмента данных не совпадает с указанным в сообщении размером");
    }
    data.append(unpacked);
    //data.replace(header.unpackedOffset(), header.unpackedSize(), unpacked);
  }
  return data;
}

QByteArray FaxBuffer::tiffBuffer() const
{
  return faxAsTiff_;
}

FaxBuffer::FaxBuffer(const QByteArray& inputData,  FormatString::LPM lpmEnum)
{
  int lpm = -1;
  switch (lpmEnum) {
    case FormatString::LPM::kLPM_060:{
      lpm = 60;
      break;
    }
    case FormatString::LPM::kLPM_090:{
      lpm = 90;
      break;
    }
    case FormatString::LPM::kLPM_120:{
      lpm = 120;
      break;
    }
    case FormatString::LPM::kLPM_UNK:{
      lpm = 120;
      break;
    }
  }
  if ( -1 == lpm ){
    error_log << QObject::tr("Ошибка - почему-то значение lpmEnum оказалось не обработано case. Значение %1")
                 .arg(lpmEnum);
  }


  QByteArray normalizedData = normalizeData(inputData, lpm, offsetForNormalized);

  QByteArray tiff;
  if ( false == internal::saveAsTiff(normalizedData, lpm, &tiff) ){
    error_log << QObject::tr("Произошла ошибка преобразования в TIFF");
    return;
  }
  faxAsTiff_ = tiff;
}

const QByteArray FaxBuffer::littleEndian2bigEndian(const QByteArray& le)
{
  QByteArray be;
  for (int i = le.size(); i > 0; --i) {
    be.append(le[i-1]);
  }
  return be;
}

bool FaxBuffer::isValid() const
{
  return false == faxAsTiff_.isEmpty();
}

const QByteArray FaxBuffer::normalizeData(const QByteArray& src, int lpm, int offset)
{
  QByteArray result(littleEndian2bigEndian(src));

  if (offset != 0) {
    int width = internal::imageWidth(lpm);
    int length = internal::imageLength(result.size(), lpm);
    for (int i = 0; i < length; ++i) {
      QByteArray strip = result.mid(i*width, width);
      if (offset > 0) {
        QByteArray stub = strip.left(offset);
        strip.remove(0, offset).append(stub);
      }
      else {
        QByteArray stub = strip.right(offset);
        strip.remove(width-offset, offset).prepend(stub);
      }
      result.replace(i*width, width, strip);
    }
  }
  return result;
}

bool FileBuffer::isValid()
{
  return isValid_;
}

}
