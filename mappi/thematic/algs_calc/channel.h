#ifndef MAPPI_THEMATIC_ALGS_CHANNEL_H
#define MAPPI_THEMATIC_ALGS_CHANNEL_H

#include <qvector.h>
#include <mappi/global/streamheader.h>

#include <meteo/commons/global/log.h>
#include <cross-commons/debug/tlog.h>

#include <QFile>


namespace mappi {
  namespace to {
  const float INVALID_VALUE = -9999.;

    //NOTE: не уверена всегда ли нужен float. Пока все при считывании в него преобразуется
    class Channel: public QVector<float> {
    public:
      Channel() = default;
      virtual ~Channel() = default;

      template<typename T>
      bool putValues(QDataStream &ds);

      void calculateMaxMin();

      static bool readHeader(const QString& fileName, meteo::global::PoHeader* header);
      bool readData(const QString& fileName);

      float min() const { return _min; }
      float max() const { return _max; }
      bool hasData() const {
        return !isEmpty() && _min != std::numeric_limits<float>::min() && _max != std::numeric_limits<float>::max();
      }

      int number() const  { return _header.channel; }
      int rows() const    { return _header.lines > 0 ? _header.lines : 1; }
      int columns() const { return _header.samples > 0 ? _header.samples : size(); }
      int cols() const { return columns(); }
      void scale(int new_size);
      void scale(int new_rows, int new_columns);
      const std::vector<int> noData() const {return no_data_idx_;}

      const meteo::global::PoHeader& header() { return  _header; }

      virtual float getValue(int idx) { return at(idx); }
      void setInstrument(mappi::conf::InstrumentType instrument){ instrument_ = instrument; }
      mappi::conf::InstrumentType instrument() { return instrument_;}
      void setName(const std::string &name){ name_ = name; }
      std::string name() const { return name_;}

    private:
      float _min = std::numeric_limits<float>::min(); //!< Минимально возможное значение в канале (напр., калиброванный 1 канал AVHRR/3 0-100%)
      float _max = std::numeric_limits<float>::max(); //!< Максимально возможное значение в канале
      mappi::conf::InstrumentType instrument_;
      std::string name_;
      std::vector<int> no_data_idx_;
      meteo::global::PoHeader _header = {};
    };

    class DummyChannel : public Channel {
    public:
      DummyChannel() = default;
      ~DummyChannel() = default;

      float getValue(int) { return 0; }
    };
  }
}

#endif

