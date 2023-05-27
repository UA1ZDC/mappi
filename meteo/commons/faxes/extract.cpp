#include "extract.h"
#include "tiffconvert.h"

#include <qtemporaryfile.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <meteo/commons/grib/parser/tgribformat.h>
#include <commons/compresslib/mcompress.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/msgstream/plugins/socketspecial/unimas.h>

#include <QByteArray>
#include <QCoreApplication>
#include <QList>
#include <QProcess>
#include <QStringList>

#include <qfile.h>

namespace {
const QString FaxformatStringName() { return QString("FAX"); }
const QString DocformatStringName() { return QString("DOC"); }
const QString segmentHeaderName() { return QString("Talarna"); }

}

namespace meteo {
namespace faxes {

FaxSegment::FaxSegment(const QByteArray& raw) :
  data_type(FAX),
  faxid(0),
  lpm(LPM_UNKNOWN),
  segnum(0),
  segtype(UNKNOWN),
  packsize(0),
  packoffset(0),
  unpacksize(0),
  unpackoffset(0),
  content(raw)
{
}

bool FaxSegment::isValid() const
{
  bool notValid = faxid <= 0 ||
      segnum <= 0 ||
      segtype == UNKNOWN ||
      lpm == LPM_UNKNOWN ||
      content.isEmpty();
  return !notValid;
}

const QString FaxSegment::segtypeToString() const
{
  switch (segtype) {
    case BEG:
      return "BEG";
    case BOD:
      return "BOD";
    case END:
      return "END";
    default:
      break;
    }
  return QString();
}

QString FaxSegment::getSegtype()const
{
  switch (this->segtype ) {
    case BEG:
      return "BEG";
    case BOD:
      return "BOD";
    case END:
      return "END";
    case UNKNOWN:
      break;
    }
  return "UNKNOWN";
}

void FaxSegment::setSegtype(const QString& str)
{
  if (str == "BEG") {
      this->segtype = BEG;
    }
  else if (str == "BOD") {
      this->segtype = BOD;
    }
  else if (str == "END") {
      this->segtype = END;
    }
  else {
      this->segtype = UNKNOWN;
    }
}

void FaxSegment::setLpm(const QString& str)
{
  switch (str.toInt()) {
    case LPM_060:
      this->lpm = LPM_060;
      break;
    case LPM_090:
      this->lpm = LPM_090;
      break;
    case LPM_120:
      this->lpm = LPM_120;
      break;
    default:
      this->lpm = LPM_UNKNOWN;
      break;
    }
}

QString FaxSegment::getLpm()const{
  return QString::number(this->lpm).rightJustified(3,'0');
}


static QByteArray fromDecValue(unsigned long src, int size, bool isLittleEndian = false)
{
  QByteArray ba;
  ba.resize(size);
  ::memcpy( ba.data(), &src, size );
  if (isLittleEndian == true) {
      ba = internal::littleEndian2bigEndian(ba);
    }
  return ba;
}

QByteArray FaxSegment::getHeader() const{
  QByteArray header;
  header = segmentHeaderName().toUtf8().leftJustified(8,'0');
  header += fromDecValue(segtype,sizeof (int16_t));
  header += fromDecValue(3,sizeof (int16_t));
  header += fromDecValue(segnum,sizeof (int32_t));
  header += fromDecValue(packsize,sizeof (int32_t));
  header += fromDecValue(packoffset,sizeof (int32_t));
  header += fromDecValue(unpacksize,sizeof (int32_t));
  header += fromDecValue(unpackoffset,sizeof (int32_t));
  return header;
}

QByteArray FaxSegment::getFormatString() const
{
  QString fsn = FaxformatStringName();
  if(FAX != data_type){
      fsn = DocformatStringName();
    }


  QByteArray ba = QString("%1 %2 %3 %4 %5 %6 %7")
      .arg(fsn)
      .arg( QString::number(faxid).rightJustified(5, '0'))
      .arg( getLpm() )
      .arg("576")
      .arg( QString::number(segnum).rightJustified(4, '0'))
      .arg("COM")
      .arg( getSegtype() ).toUtf8();
  return ba;
}


QString FaxSegment::header() const
{
  QString hdr = QString("[FAX SEGMENT HEAER]\n"
                        "\tFAX ID = %1\n"
                        "\tSEG NUM = %2\n"
                        "\tSEG TYPE = %3\n"
                        "\tLPM = %4\n"
                        "\tIS EMPTY = %5\n")
      .arg(faxid)
      .arg(segnum)
      .arg( segtypeToString() )
      .arg(lpm)
      .arg( content.isEmpty() )
      ;
  return hdr;
}

namespace internal {

  const QByteArray littleEndian2bigEndian(const QByteArray& le)
  {
    QByteArray be;
    for (int i = le.size(); i > 0; --i) {
        be.append(le[i-1]);
      }
    return be;
  }

  const QString decToString(int dec, int length)
  {
    QString result = QString::number(dec);
    while (result.length() < length) {
        result = QString("0%1").arg(result);
      }
    return result;
  }

} // internal
} // faxes
} // meteo
