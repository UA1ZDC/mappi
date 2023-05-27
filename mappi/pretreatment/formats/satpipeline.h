#ifndef MAPPI_PRETREATMENT_SATPIPELINE_H
#define MAPPI_PRETREATMENT_SATPIPELINE_H

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <commons/mathtools/mnmath.h>
#include <sat-commons/satellite/satviewpoint.h>
#include <mappi/proto/satellite.pb.h>
#include <mappi/global/streamheader.h>
#include <mappi/pretreatment/formats/satformat.h>
#include <mappi/pretreatment/savenotify/savenotify.h>
#include <mappi/pretreatment/formats/satwrapper.h>
#include <mappi/pretreatment/images/geomcorr.h>
#include <mappi/pretreatment/images/image.h>

#include <qmap.h>
#include <qstring.h>
#include <qdatetime.h>

#include <qstring.h>

namespace mappi::po {
  float rotateAngle(Coords::GeoCoord geoB, Coords::GeoCoord geoE);

  class SatPipeline {
  public:
    SatPipeline() {
      _sat = new SatViewPoint;
      _satDumpWrapper = singleton::SatDumpWrapper::instance();
      _params.output_dir = MnCommon::randomPath(8, MnCommon::varPath() + "satdump_tmp/");
    };
    ~SatPipeline(){
      delete _sat;
      _sat = nullptr;
      _notify = nullptr;
      _satDumpWrapper = nullptr;
    };

    enum {
      Type = conf::kFrameUnk
    };

    void clear();

    QString invertBytesInFile(QString filePath);

    virtual conf::FrameType type() const { return conf::kFrameUnk; }

    void setName(const QString &name) { _params.pipeline_name = name; }

    void setFileName(const QString &file_name) { _params.input_file = file_name; }

    void setParams(const QString &params) { _params.additional_params = params; }

    void setSatName(const QString &satName) { _satName = satName; }

    void setRecvMode(conf::RateMode mode) { _mode = mode; }

    void setDataLevel(conf::DataLevel level);

    void setTLE(const QString &satName, const QString &tleFile);

    void setTLE(const MnSat::TLEParams &tle);

    void setDateTime(const QDateTime &dtBeg, const QDateTime &dtEnd);

    void setInstruments(const google::protobuf::RepeatedPtrField <mappi::conf::PretrInstr> &instrs);

    void setPath(const QString &path) { _path = path; }

    void setSaveNotify(SaveNotify* notify) { _notify = notify; }

    SatDump::Params params() {return _params; };

    QString satName() { return _satName; }

    conf::RateMode mode() { return _mode; }

    QString path() { return _path; }

    QDateTime dtStart() {
      return nullptr != _sat ? _sat->dtStart() : QDateTime();
    }

    QDateTime dtEnd() {
      return nullptr != _sat ? _sat->dtEnd() : QDateTime();
    }

    SaveNotify* notify() { return _notify; }

    SatViewPoint *satellite() { return _sat; }

    QString instruments(QString delimeter = ",", QString prefix = "");

    bool run();
    void filterOutInstrs();
    bool save(const meteo::global::StreamHeader &header);
    bool savePO(
        QString outputFilePath,
        meteo::global::PoHeader &pohead,
        meteo::global::PreHeader &pre,
        QVector<uint16_t> &data
    );

    meteo::global::PreHeader createPreHeader(
        meteo::global::PoHeader &pohead,
        QDate date = QDateTime::currentDateTimeUtc().date()
    );

    template<typename T>
    meteo::global::PoHeader createPoHeader(
        const meteo::global::StreamHeader &header,
        const SatDump::Image<uint16_t> &image,
        const conf::InstrumentType &instr_type,
        int32_t channel_num,
        QDateTime dtStart,
        QDateTime dtEnd
    );

    bool passImageTransform(
        int32_t channel_num,
        const mappi::conf::PretrInstr &pretrConfig,
        mappi::conf::ImageTransform &transform
    );

    po::GeomCorrection createGeomCorrection(
        const mappi::conf::Instrument &instr,
        const mappi::conf::PretrInstr &pretrConfig,
        meteo::global::PoHeader pohead,
        mappi::conf::ImageTransform imtransform
    );


  private:
    bool createFrames(const QByteArray &data);

  private:
    SatDump::Wrapper *_satDumpWrapper;
    SatDump::Params _params;
    QString _satName;
    conf::RateMode _mode = conf::kUnkRate;
    conf::DataLevel _level = conf::kUnkLevel;
    float _rAngle = 0; //!< угол отклонения от севера траектории спутника
    QVector<mappi::conf::PretrInstr> _instrPretr;
    SatViewPoint *_sat = nullptr; //!< класс для получения координат данных потока
    QString _path;   //!< Путь для сохранения данных
    QMap<QString, SatDump::Product> _instrProducts; //!< продукты, сохраненные SatDump
    QMap<QString, conf::InstrumentType> _instrCheckedTypes; //!< типы для продуктов SatDump;
    SaveNotify* _notify = nullptr;
  };
}
#endif //MAPPI_PRETREATMENT_SATPIPELINE_H
