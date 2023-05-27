#ifndef MAPPI_PRETREATMENT_INSTRUMENTS_MTVZA_H
#define MAPPI_PRETREATMENT_INSTRUMENTS_MTVZA_H

#include <mappi/pretreatment/formats/instrument.h>
#include <mappi/proto/satellite.pb.h>


#include <qvector.h>

namespace mappi {
  namespace po {

    class Frame;
    class MHrpt;

   //! Чтение данных МСУ
    class Mtvza : public InstrumentTempl<short> {
    public:
      Mtvza(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent);
      ~Mtvza();
    
      // enum {
      // 	Type = conf::kMtvza
      // };

      virtual InstrFrameType type() const { return mappi::InstrFrameType(conf::kMtvza,  mappi::conf::kMhrpt); }
      
      //      void setPretrSettings(const mappi::conf::PretrInstr& ) {};      

      void saveImage(const QByteArray& data, const QString& ch);
      void saveImage1(const QByteArray& data, const QString& ch);

    protected:
      ChannelVector<short>& channels() { return _ch; };
      const ChannelVector<short>& channels() const { return _ch; };
      const Channel<short>& channel(int number) const { return _ch.at(number); };


    private:
      bool parse(const QByteArray& data, QDateTime* dtStart, QDateTime* dtEnd);
      bool parsetest(const QByteArray& adata, const QDateTime& dt);

      bool correction(const SatViewPoint* ) { return true; }
      bool calibrate() { return false; }

      int64_t findStartFrame(const QByteArray& data, int goodCnt);
    private:
      ChannelVector<short> _ch; //каналы

      mappi::conf::Instrument _conf; //!< описание формата прибора
      Frame* _frame;
    };
  
  }
}



#endif
