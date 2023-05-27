#ifndef METEO_COMMONS_FAXES_EXTRACT_H
#define METEO_COMMONS_FAXES_EXTRACT_H

#include <QByteArray>
#include <QString>

template<typename T> class QList;

namespace meteo {
namespace faxes {

struct FaxSegment
{
  enum DataType {
    FAX = 0,
    DOC = 1
  };

  enum Type {
    BEG = 0,
    BOD = 1,
    END = 2,
    UNKNOWN
  };

  enum LPM {
    LPM_060 = 60,
    LPM_090 = 90,
    LPM_120 = 120,
    LPM_UNKNOWN = 0
  };

  FaxSegment(const QByteArray& raw = QByteArray());
  bool isValid() const;

  const QString segtypeToString() const;
  void setSegtype(const QString& str) ;
  QString getSegtype()const;
  void setLpm(const QString& str);
  QString getLpm()const;
  QByteArray getFormatString() const;

  QString header() const ;
  QByteArray getHeader()const;
  DataType data_type;
  int faxid;
  LPM lpm;
  int32_t segnum;
  Type segtype;

  int32_t packsize;
  int32_t packoffset;
  int32_t unpacksize;
  int32_t unpackoffset;

  QByteArray content;

};

struct Fax
{
  QString t1;
  QString t2;
  QString a1;
  QString a2;
  QString ii;
  QString cccc;
  QString yygggg;
  QString dt;
  QString magic;
  QString filename;
  QByteArray data;
};

namespace internal {

const QByteArray littleEndian2bigEndian(const QByteArray& le);
const QString decToString(int dec, int length);

}
}
}

#endif // METEO_MASLO_FAXES_EXTRACT_H
