#ifndef MAPPI_PRETREATMENT_INSTRUMENTS_MSU_H
#define MAPPI_PRETREATMENT_INSTRUMENTS_MSU_H

#include <mappi/pretreatment/formats/instrument.h>
#include <mappi/proto/satellite.pb.h>
#include <mappi/pretreatment/formats/hrpt_frame.h>
#include <mappi/pretreatment/instruments/hrpt_const.h>

#include <qvector.h>

namespace mappi {
  namespace po {

    class Frame;
    class Lrpt;

   //! Чтение данных МСУ
    class Msu : public InstrumentTempl<short> {
    public:
      Msu(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent);
      ~Msu();
    
      // enum {
      // 	Type = conf::kMsu
      // };

      virtual InstrFrameType type() const { return mappi::InstrFrameType(mappi::conf::kMsuMrLo, mappi::conf::kCadu); }
      
      //      void setPretrSettings(const mappi::conf::PretrInstr& ) {};      

    protected:
      ChannelVector<short>& channels() { return _ch; };
      const ChannelVector<short>& channels() const { return _ch; };
      const Channel<short>& channel(int number) const { return _ch.at(number); };

    private:
      bool parse(const QList<QByteArray>& data, QDateTime* dtStart, QDateTime* dtEnd);
      //      bool parseRT(const QList<QByteArray>& dataAll, const QDateTime& adt, int* remainBytes);
      bool correction(const SatViewPoint* ) { return true; }
      bool calibrate() { return false; }
      int  getMissedCount(int* idx, const QList<QByteArray>& dataAll);

      QByteArray createMcu(const QByteArray& data);
      bool findStartPackNum(int* cnt, int* idx, QDateTime* dt, const QList<QByteArray>& src);
      bool isValidPacket(const QByteArray& current, const QByteArray& prev);

      void setStripe(const QMap<int, QMap<int, QByteArray> >& chan, int channel, QMap<int, QByteArray>* rdata);

    private:
       ChannelVector<short> _ch; //каналы

      mappi::conf::Instrument _conf; //!< описание формата прибора
      Frame* _frame;

      QVector<QRgb> _palette;
    };
  
  }
}



#endif
