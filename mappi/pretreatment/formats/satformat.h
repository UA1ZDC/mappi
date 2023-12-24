#ifndef MAPPI_PRETREATMENT_FORMATS_SATFORMAT_H
#define MAPPI_PRETREATMENT_FORMATS_SATFORMAT_H

#include <cross-commons/singleton/tsingleton.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/textproto/tprototext.h>
#include <commons/textproto/pbtools.h>
#include <mappi/proto/satellite.pb.h>
#include <mappi/proto/reception.pb.h>
#include <mappi/global/global.h>

#include <QFile>
#include <QMap>
#include <QMultiMap>
#include <QDateTime>
#include <QString>

#define RECEPTION_FILE MnCommon::etcPath("mappi") + "reception.conf"
#define INSTRCONF_FILE MnCommon::etcPath("mappi") + "satinstr.conf"
#define SATCONF_FILE   MnCommon::etcPath("mappi") + "satpretr.conf"
#define COMPCONF_FILE  MnCommon::etcPath("mappi") + "composites.conf"

class QDomNode;

namespace mappi {

  using InstrFrameType = QPair<::mappi::conf::InstrumentType, ::mappi::conf::FrameType> ;
  class SaveNotify;
  
  namespace po {

    //! создание приборов, в соответствии с настройками
    class SatFormat {
    public:
      SatFormat();
      ~SatFormat();

      bool isValid(){ return _isValid; };

      //! настройки по имени спутника
      QString getReceptionStoragePath() { return _receptionStoragePath; }
      QMap <QString, QString> getSatPipelines() { return _satPipelines; }
      conf::InstrCollect getInstruments() { return _instrConf; }

      const mappi::conf::PretrSat* getPretreatmentFor(const QString& name, conf::RateMode mode);
      QString getWeatherFilePath(const QDateTime dateTime = QDateTime::currentDateTimeUtc());
      QString getCompositesSettingsFilePath();
      bool getFlagDeleteInputFile() const { return _deleteInputFile; }
      bool getFlagCopyInputFile() const { return _copyInputFile; }
      bool getFlagCopyCaduFile() const { return _copyCaduFile; }

      QString getPipelineFor(const QString &satName);
      const mappi::conf::ReceptionParam getReceptionFor(const QString &satName);
      QVector<conf::InstrumentType> getInstrumentTypesBy(const QString &instrName);
      bool hasInstrument(const conf::InstrumentType &instr_type);
      conf::Instrument getInstrumentBy(const conf::InstrumentType &instr_type);

      QString getSiteName() const;
      meteo::GeoPoint getSiteCoord() const;

    private:
      bool readReceptionConf();
      bool readInstrsConf();
      bool readSatsConf();

      bool _isValid = true;

      bool _deleteInputFile = true;
      bool _copyInputFile = false;
      bool _copyCaduFile = false;
      QString _receptionStoragePath;
      QString _tleStoragePath;
      QMap <QString, QString> _satPipelines;

      conf::InstrCollect _instrConf;
      mappi::conf::Reception _receptionConf;
      conf::Pretreatment _pretrConf;

      QMap<conf::InstrumentType, mappi::conf::Instrument> _instrConfCol;
      QMultiMap<QString, mappi::conf::PretrSat> _satConf;
      QMap<QString, mappi::conf::ReceptionParam> _satReception;
    };

    namespace singleton {
      typedef TSingleton<mappi::po::SatFormat> SatFormat;
    }
  }
}

#endif 
