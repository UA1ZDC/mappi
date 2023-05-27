#ifndef MAPPI_PRETREATMENT_INSTRUMENTS_MSUGS_H
#define MAPPI_PRETREATMENT_INSTRUMENTS_MSUGS_H

#include <mappi/pretreatment/formats/instrument.h>
#include <mappi/proto/satellite.pb.h>
#include <mappi/pretreatment/formats/hrpt_frame.h>
#include <mappi/pretreatment/instruments/hrpt_const.h>

#include <qvector.h>

namespace mappi {
  namespace po {

    class Frame;

   //! Чтение данных МСУ-ГС
    class MsuGs : public InstrumentTempl<short> {
    public:
      MsuGs(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent);
      ~MsuGs();
    
      // enum {
      // 	Type = conf::kMsuGs
      // };

      virtual InstrFrameType type() const { return mappi::InstrFrameType(conf::kMsuGs, mappi::conf::kCadu); }
      
      //      void setPretrSettings(const mappi::conf::PretrInstr& ) {};      

    protected:
      ChannelVector<short>& channels() { return _ch; };
      const ChannelVector<short>& channels() const { return _ch; };
      const Channel<short>& channel(int number) const { return _ch.at(number); };

      virtual bool needUnion() { return true; }
      
    private:
      bool parse(const QByteArray& dataAll, QDateTime* dtStart, QDateTime* dtEnd);
      bool parseIR(const QByteArray& data, const QDateTime& dt);
      bool parseIR(const QByteArray& data, int chidx, uint64_t start, uint64_t length, uint64_t offset);

      bool correction(const SatViewPoint* ) { return true; }
      bool calibrate() { return false; }


      
    private:
      ChannelVector<short> _ch; //каналы

      mappi::conf::Instrument _conf; //!< описание формата прибора
      Frame* _frame;

    };
  
  }
}



#endif
