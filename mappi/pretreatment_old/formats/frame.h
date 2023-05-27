#ifndef MAPPI_PRETREATMENT_FRAMES_SATFORMAT_H
#define MAPPI_PRETREATMENT_FRAMES_SATFORMAT_H

#include <sat-commons/satellite/satviewpoint.h>
#include <mappi/proto/satellite.pb.h>
#include <mappi/global/streamheader.h>

#include <qmap.h>
#include <qstring.h>
#include <qdatetime.h>

class QByteArray;
class QDateTime;

namespace meteo {
class Decoder;
}

namespace mappi {
namespace po {

class Instrument;

float rotateAngle(Coords::GeoCoord geoB, Coords::GeoCoord geoE);

//! Интерфейс для форматов
class Frame {
public:
  Frame(const conf::Frame& conf, bool setSat, Frame* parent);
  virtual ~Frame();

  enum {
    Type = conf::kFrameUnk
  };
  virtual conf::FrameType type() const { return conf::kFrameUnk; }

  QString path() {
    if (nullptr != _parent) {
      return _parent->path() + _path;
    }
    return _path;
  }

  void setPath(const QString& path) { _path = path; }
  void setSuffix(const QString& suffix) { _suffix = suffix; }

  bool setData(const QByteArray& data);
  bool parse(const QDateTime& dt);
  bool parseL(const QDateTime& dt);
  bool processFramesL(const QDateTime& dt, const meteo::global::StreamHeader& header, SatViewPoint* sat);


  virtual bool save(const meteo::global::StreamHeader& header);
  virtual bool save(const meteo::global::StreamHeader& header, const QString& baseName, const QDateTime&, const QDateTime&,
                    SatViewPoint* sat);
  virtual void clear();
  virtual void addFrame(const QByteArray& frame) { _data.append(frame); }
  // virtual bool parse(const QDateTime&) { return false;}

  void setSatName(const QString& satName) { _satName = satName; }
  void setDtValid(bool valid)           { _dtValid = valid; }
  void setRecvMode(conf::RateMode mode) { _mode = mode; }
  void setDataLevel(conf::DataLevel level);

  void setTLE(const QString& satName, const QString& tleFile);
  void setTLE(const MnSat::TLEParams& tle);
  void setDateTime(const QDateTime& dtBeg, const QDateTime& dtEnd);
  // virtual void setInstruments(const google::protobuf::RepeatedPtrField<mappi::conf::PretrInstr>& instrs);
  //virtual void setInstrument(conf::InstrumentType type, const conf::PretrInstr& conf) { _instrPretr.insert(type, conf); }

  QString satName();
  conf::RateMode mode();
  bool isDtValid()      { return _dtValid; }
  QDateTime dtStart() {
    if (nullptr != _sat) return _sat->dtStart();
    if (nullptr != _parent) return _parent->dtStart();
    return QDateTime();
  }
  QDateTime dtEnd()   {
    if (nullptr != _sat) return _sat->dtEnd();
    if (nullptr != _parent) return _parent->dtEnd();
    return QDateTime();
  }
  SatViewPoint* satellite() {
    if (nullptr != _sat) return _sat;
    if (nullptr != _parent) return _parent->satellite();
    return nullptr;
  }
  QList<conf::InstrumentType> framesList() { return _instrPretr.keys(); }
  const conf::PretrInstr& instrPretr(conf::InstrumentType type) { return _instrPretr[type]; }

  QList<conf::FrameType> frames() { return _frames.keys(); }

  bool addFrame(conf::FrameType type, Frame* frame);
  bool contains(conf::FrameType type) { return _frames.contains(type); }
  Frame* frame(conf::FrameType type) { if (_frames.contains(type)) return _frames[type]; return nullptr; }
  void addInstrument(Instrument* instr, conf::InstrumentType type, const conf::PretrInstr& conf)
  {
    _instr.append(instr);
    _instrPretr.insert(type, conf);
  }
  void setProcId(const QList<int>& iid) { _procid = iid; }

  // bool parseRT(const QDateTime& dt, const QByteArray& data);
  // virtual bool getDataRT(QByteArray* data);

  const mappi::conf::Frame& format() const { return _format; }

protected:
  virtual bool parseFormatCustom(conf::DataLevel , const QDateTime& , QByteArray* ) { return true; }
  //      virtual bool parseFormatCustomRT(conf::DataLevel , const QDateTime& , QByteArray* ) { return true; }
  virtual bool readDt(const QDateTime& /*dt*/, const QByteArray& /*data*/) { return false; }

  virtual bool processFrames(const QDateTime& dt);
  //      bool processFramesRT(const QDateTime& dt);

  virtual int parseFrame(Instrument* , const conf::InstrFormat& , const QByteArray&) { return 0; }
  virtual int parseRaw(Instrument* , const conf::InstrFormat& , const QByteArray&, bool /*invert*/) { return 0; }

  virtual bool parseFramesL(const QDateTime& dt);


  virtual int processLPart(const QDateTime&) {return true;}
  virtual int parseFrameL(Instrument* , const conf::InstrFormat& , const QByteArray&) { return 0; }
  virtual bool setFrameDataL(const conf::Frame& , QByteArray*) { return true; }
  virtual bool setFrameDataL(Instrument* , const conf::InstrFormat&) { return true; }

  Frame* parent() { return _parent; }

  const QByteArray& data() { return _data; }
  void setFrameData(const QByteArray& data) { _data = data; }

  const QList<int> procid() { return _procid; }

private:
  bool createFrames(const QByteArray& data);

private:
  Frame* _parent = nullptr;
  QString _satName;
  conf::RateMode _mode = conf::kUnkRate;
  conf::DataLevel _level = conf::kUnkLevel;
  bool _dtValid = false; //!< true - указана полная дата, считывать из кадра не надо
  meteo::Decoder* _decoder = nullptr;

  float _rAngle = 0; //!< угол отклонения от севера траектории спутника
  //GeoCoord site;

  QMap<conf::InstrumentType, conf::PretrInstr> _instrPretr;
  SatViewPoint* _sat = nullptr; //!< класс для получения координат данных потока

  mappi::conf::Frame _format;
  QMap<conf::FrameType, Frame*> _frames;
  QList<Instrument*> _instr;
  QList<int> _procid; //!< id обрабатываемых кадров; пусто, если обработка всех или в этом типе кадре нет id
  QByteArray _data;

  QString _path;   //!< Путь для сохранения данных
  QString _suffix; //!< Добавление к имени файла (для уникальности имени при сохранении одних данных из разных источников)
};

template <class T> inline T frame_cast(Frame* f)
{
  return int(static_cast<T>(0)->Type) == int(Frame::Type)
      || ( 0 != f && int(static_cast<T>(0)->Type) == int(f->type()) ) ? static_cast<T>(f) : 0;
}

}
}

#endif 
