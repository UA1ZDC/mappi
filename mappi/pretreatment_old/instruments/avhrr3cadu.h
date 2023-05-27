#ifndef MAPPI_PRETREATMENT_INSTRUMENTS_AVHRR3CADU_H
#define MAPPI_PRETREATMENT_INSTRUMENTS_AVHRR3CADU_H

#include <mappi/pretreatment/instruments/avhrr3.h>

namespace mappi {
  namespace po {
    class Avhrr3Cadu : public Avhrr3 {
    public:
      Avhrr3Cadu(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent);
      ~Avhrr3Cadu();
    
      // enum {
      // 	Type = conf::kAvhrr3Cadu
      // };
      virtual InstrFrameType type() const { return mappi::InstrFrameType(conf::kAvhrr3, mappi::conf::kCadu); }

      bool parse(const QList<QByteArray>&, QDateTime* dtStart, QDateTime* dtEnd); //!< парсинг
      virtual bool parseRT(const QList<QByteArray>&, const QDateTime&, int*); //!< парсинг, realtime

    protected:
      ChannelVector<short>& channels() { return _ch; };
      const ChannelVector<short>& channels() const { return _ch; };
      const Channel<short>& channel(int number) const { return _ch.at(number); };

      void serializedImgDataRT(QByteArray* ba);

    private:
      bool findStartPackNum(int* cnt, int* idx, QDateTime* adt, const QList<QByteArray>& dataAll);
      //      bool readChannelData(const QVector<ushort>& data, int length);
      bool readChannelData(const QByteArray& data, int length);
	
      int missedFrames(int counter, int idx, const QList<QByteArray>& data);
    private:
      int _chan3x =0;           //!< 0=AVHRR Ch3B, 1=AVHRR Ch3A
      int _lineChan3x = 0;       //!< Номер линии сканирования, если изменился 3A/B канал,иначе -1

      ChannelVector<short> _ch; //каналы
      mappi::conf::Instrument _conf; //!< описание формата прибора
      Frame* _frame = nullptr;
    };
    
    
  }
}

#endif
