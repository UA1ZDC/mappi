#ifndef MAPPI_THEMATIC_ALGS_CHANNEL_H
#define MAPPI_THEMATIC_ALGS_CHANNEL_H

#include <qvector.h>
#include <mappi/global/streamheader.h>


namespace mappi {
  namespace to {

    //NOTE: не уверена всегда ли нужен float. Пока все при считывании в него преобразуется
    class Channel: public QVector<float> {
    public:
      Channel() = default;
      virtual ~Channel() = default;

      static bool readHeader(const QString& fileName, meteo::global::PoHeader* header);
      
      bool readData(const QString& fileName);
      float min() { return _min; }
      float max() { return _max; }

      int number()  { return _header.channel; }
      int rows()    { return _header.lines;   }
      int columns() { return _header.samples; }

      const meteo::global::PoHeader& header() { return  _header; }

      virtual float getValue(int idx) { return at(idx); }
      
    private:
      meteo::global::PoHeader _header;

      // int _rows = -1; //!< количество строк
      // int _cols = -1; //!< количество колонок

      float _min = std::numeric_limits<float>::min(); //!< Минимально возможное значение в канале (напр., калиброванный 1 канал AVHRR/3 0-100%)
      float _max = std::numeric_limits<float>::max(); //!< Максимально возможное значение в канале

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

