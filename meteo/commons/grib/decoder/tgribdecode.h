#ifndef TGRIB_DECODE_H
#define TGRIB_DECODE_H

#include <meteo/commons/services/decoders/tservicestat.h>
#include <meteo/commons/grib/tgribdefinition.h>
#include <stdint.h>
#include <qfile.h>


class QString;
class QByteArray;
class TDecodeGribBase;

class TGridGrib2;
class TProductGrib;

namespace meteo {
  class Psql;
  namespace app {
    class AppState_OperationStatus;
  }
}

namespace grib {
  class TGribData;
  class TGridDefinition;
  class TProductDefinition;
}

//! read grib
class TDecodeGrib {
public:
  TDecodeGrib(meteo::Psql *db=0);
  virtual ~TDecodeGrib();

  static int edition(const QByteArray & data);
  
  virtual int decode(const QByteArray& ba);
  virtual int decode(const QString &tlgfile_name);//!< read from file and save in db
  //  virtual int decode(const QByteArray& ba, QList<QByteArray>* result, QString* error, const QDateTime& dt = QDateTime()); 
  virtual int decode(const QMap<QString, QString>& type, const QByteArray& ba);
  //  virtual bool readFromDb(uint64_t id); //!< read from db
  int edition(); //!< edition number
  QString file() {return _file.fileName();}

  const grib::TGribData* grib() const;
  const grib::TGridDefinition* gridDefinition() const;
  const grib::TProductDefinition* productDefinition() const; //!< only grib2
  const float* data(uint64_t* size) const;

protected:
  virtual QString sourceId() { return ""; }
  virtual void dataReady();
  meteo::Psql* db() { return _db;}

  const service::Status<grib::SourceType, grib::StatType>& status() const { return _stat; }

private:
  int decode();
  uint8_t detectGrib();
  uint8_t detectGrib(const QByteArray& arr, int64_t* idx);
  bool parseSection(uint16_t section);

private:
  meteo::Psql* _db;
  QFile _file;
  uint64_t _fileIdx; //!< текущее положение в файле
  uint64_t _size; //!< общая длина grib
  TDecodeGribBase* _grib;

  service::Status<grib::SourceType, grib::StatType> _stat; //!< Состояние раскодировщика
};

#endif
