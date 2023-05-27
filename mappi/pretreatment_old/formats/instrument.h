#ifndef MAPPI_PRETREATMENT_INSTRUMENTS_SATINSTRUMENT_H
#define MAPPI_PRETREATMENT_INSTRUMENTS_SATINSTRUMENT_H

#include <mappi/pretreatment/formats/satformat.h>
#include <mappi/pretreatment/formats/channel.h>

#include <mappi/proto/satellite.pb.h>
#include <cross-commons/debug/tlog.h>
#include <mappi/global/streamheader.h>
#include <mappi/pretreatment/images/geomcorr.h>
#include <mappi/pretreatment/images/image.h>


#include <qbytearray.h>
#include <qdatetime.h>
#include <qmap.h>

class SatViewPoint;

namespace mappi {
namespace po {
struct ChannelProperty;
}
}

//возвращает двоичное представление числа (для отладки)
extern QString db(unsigned value);

namespace mappi {
class SaveNotify;

namespace po {


//! Интерфейс для парсинга кадра
class Instrument {
public:
  Instrument(const mappi::conf::Instrument& conf, const conf::InstrFormat& format);
  virtual ~Instrument();

  virtual InstrFrameType type() const { return  InstrFrameType(conf::kInstrUnk, conf::kFrameUnk); }

  void setSaveNotify(SaveNotify* notify) { _notify = notify; }

  virtual void clear(); //!< Очистка перед началом обработки
  //      virtual void resetRT() { error_log << "Not realised"; } //!< Очистка данных каналов

  //! Добавление фрейма прибора из потока данных
  void addFrame(const QByteArray& frame) {
    if (_data.isEmpty()) { _data.append(frame); }
    else               { _data.last().append(frame); }
  }

  //! Добавление фрейма прибора из потока данных
  void addFrame(int counter, const QByteArray& frame) {
    if (_data.isEmpty()) { addNewFrame(counter, frame); }
    else                 { _data.last().append(frame);  }
  }

  void addNewFrame(int counter, const QByteArray& frame) {
    //if (!_data.isEmpty()) {
    //debug_log << "pkt_length" << _data.last().size();
    //}
    _data.append(frame);
    _counter.append(counter);
  }

  //! Обработка данных после получения всех данных
  bool process(SatViewPoint* sat,  const mappi::conf::PretrInstr& settings, QDateTime* dtStart, QDateTime* dtEnd);
  //      bool processRT(const mappi::conf::PretrInstr& settings, const QDateTime& dt);

  virtual bool save(const meteo::global::StreamHeader& header, const QString& path, const QString& baseName,
                    const QDateTime& dtStart, const QDateTime& dtEnd, SatViewPoint* sat); //!< сохранение данных
  //      virtual void serializedImgDataRT(QByteArray* ba); //!< сериализация исходных (некалиброванных) данных для передачи другим модулям в виде изображения
  void createCoords(uint rows, SatViewPoint* sat, float gridStep, float fovStep, Coords::GeoCoord* leftUp,
                    QVector<QMap<float, QPair<float, int64_t>>>* sortedCoords, int* resCols, int* resRows) const;
  void createCoords(uint rows, SatViewPoint* sat, float fovStep, QVector<Coords::GeoCoord>* points) const;

  //virtual void setPretrSettings(const mappi::conf::PretrInstr& settings) = 0;
  const conf::InstrFormat& format()const { return _format; }
  const mappi::conf::PretrInstr& pretrConfig() const { return _pretr; }

protected:

  virtual bool unionData();
  virtual bool needUnion() { return false; }
  int missedFrames(int counter, int idx);

  virtual bool parse(const QByteArray&, QDateTime*, QDateTime*) { error_log << "Not realised"; return false;} //!< парсинг
  virtual bool parse(const QList<QByteArray>&, QDateTime*, QDateTime*) { error_log << "Not realised"; return false; } //!< парсинг
  // virtual bool parseRT(const QByteArray&, const QDateTime&, int* ) { error_log << "Not realised"; return false;} //!< парсинг, realtime
  // virtual bool parseRT(const QList<QByteArray>&, const QDateTime&, int*) { error_log << "Not realised"; return false; } //!< парсинг, realtime
  virtual bool calibrate() = 0; //!< Калибровка данных
  virtual bool correction(const SatViewPoint* sat) = 0;  //!< коррекция данных специфическая для прибора (напр., фильтрация, геом. искажения)

  //virtual QVector<Channel*>& channels() = 0;
  virtual int channelsCount() const = 0;
  virtual const ChannelProperty& channelProperty(int number) const = 0;
  virtual void saveImage(int number, const meteo::global::PoHeader& pohead, const QString& path, SatViewPoint* sat) = 0;
  virtual QString saveData(int number, const QString& path, meteo::global::PoHeader& pohead) const = 0;
  virtual QByteArray imgRawData(int number, int thinning, int* resRows, int* resCols) const = 0;
  virtual void clearChannels() = 0;

  bool isCalibrate() { return _isCalibrate; }

  virtual float fovStep(int /*channel*/)const { return _conf.fovstep(); }
  virtual float velocity(int /*channel*/)const { return _conf.velocity(); }
  virtual float scanAngle(int /*channel*/)const { return _conf.scan_angle(); }

  virtual SaveNotify* notify() { return _notify; }

private:
  mappi::conf::Instrument _conf; //!< описание формата прибора
  conf::InstrFormat _format;
  QList<QByteArray> _data;
  QList<int> _counter;
  bool _isCalibrate = false;
  mappi::conf::PretrInstr _pretr; //!< настройки предваритальной обработки

  SaveNotify* _notify = nullptr;
};


//---------

//для работы с набором каналов
template<class T> class InstrumentTempl : public Instrument {
public:
  InstrumentTempl(const mappi::conf::Instrument& conf, const conf::InstrFormat& format):
    Instrument(conf, format)
  {}

  ~InstrumentTempl() {}

  //      void resetRT() { channels().resetRT(); }

protected:
  virtual ChannelVector<T>& channels() = 0;
  virtual const ChannelVector<T>& channels() const = 0;
  virtual const Channel<T>& channel(int number) const = 0;


  int channelsCount() const {
    return channels().size();
  }

  const ChannelProperty& channelProperty(int number) const {
    return channel(number).property();
  }

  void saveImage(int number, const meteo::global::PoHeader& pohead, const QString& path, SatViewPoint* sat) ;



  QString saveData(int number, const QString& path, meteo::global::PoHeader& pohead) const {
    return channel(number).saveData(path, pohead);
  }
  QByteArray imgRawData(int number, int thinning, int* resRows, int* resCols) const {
    return channel(number).imgRawData(thinning, resRows, resCols);
  }

  void clearChannels() { channels().clear(); }

};


template<class T> void InstrumentTempl<T>::saveImage(int number, const meteo::global::PoHeader& pohead, const QString& path, SatViewPoint* sat)
{

  if (nullptr == sat) {
    return;
  }

  float gridStep = MnMath::deg2rad(pretrConfig().geostep());
  if (pretrConfig().geostep() < 0.005) {
    gridStep = MnMath::deg2rad(0.005);
    warning_log << QObject::tr("Маленький шаг сетки %1 заменен на %2 градусов").arg(pretrConfig().geostep()).arg(0.005);
  }

  po::GeomCorrection geom(*sat);

  for (auto imconf : pretrConfig().image()) {
    debug_log << "channel" << imconf.ch_rx() << number << channel(number).name();
    QRegExp rx(QString::fromStdString(imconf.ch_rx()));

    if (rx.exactMatch(channel(number).name())) {

      if (imconf.geocoding()) {
        geom.createCoords(channel(number).rows(), gridStep,
                          MnMath::deg2rad(scanAngle(number)), MnMath::deg2rad(fovStep(number)));
      }

      debug_log << QObject::tr("img size %1(%2x%3)")
                        .arg(channel(number).rows() * channel(number).columns())
                        .arg(channel(number).columns())
                        .arg(channel(number).rows());
      QString baseName = path + channel(number).name();
      Image im(channel(number).toImageData(), pohead, notify());
      im.save(baseName, imconf, geom);

      break;
    }
  }
}


} //namespace po
} //namespace mappi


//NOTE пока пусть будет
// void saveImage(int number, const google::protobuf::RepeatedPtrField<mappi::conf::ImageTransform>& conf,
// 		     const QString& path, SatViewPoint* sat) const {

// 	QVector<Coords::GeoCoord> points;

// 	float gridStep = MnMath::deg2rad(pretrConfig().geostep());
// 	if (pretrConfig().geostep() < 0.005) {
// 	  gridStep = MnMath::deg2rad(0.005);
// 	  warning_log << QObject::tr("Маленький шаг сетки %1 заменен на %2 градусов").arg(pretrConfig().geostep()).arg(0.005);
// 	}

// 	float fovstep = 0;

// 	Coords::GeoCoord leftUp;

// 	for (auto imconf : conf) {
// 	  debug_log << imconf.ch_rx() << number << channel(number).name();
// 	  QRegExp rx(QString::fromStdString(imconf.ch_rx()));
// 	  if (rx.exactMatch(channel(number).name())) {
// 	    if (imconf.geocoding() && (points.size() == 0 || fovstep != fovStep(number))) {
// 	      debug_log << "Calc geom";
// 	      fovstep = fovStep(number);
// 	      createCoords(channel(number).rows(), sat, fovstep, &points);
// 	      debug_log << "img size" << channel(number).rows() * channel(number).columns();
// 	    }
// 	    var(number);
// 	    channel(number).saveImage(imconf, path, points, gridStep);
// 	    break;
// 	  }
// 	}
// }


#endif //TSAT_INSTR_H
