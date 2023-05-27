#ifndef MAPPI_PRETREATMENT_INSTRUMENTS_VIRR_H
#define MAPPI_PRETREATMENT_INSTRUMENTS_VIRR_H

#include <mappi/pretreatment/formats/instrument.h>
#include <mappi/proto/satellite.pb.h>
#include <mappi/pretreatment/formats/hrpt_frame.h>
#include <mappi/pretreatment/instruments/hrpt_const.h>


namespace mappi {
  namespace po {

    class Frame;
    class Lrpt;

   //! Чтение данных VIRR
    class Virr : public InstrumentTempl<short> {
    public:
      Virr(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent);
      ~Virr();
    
      // enum {
      // 	Type = conf::kVirr
      // };

      virtual InstrFrameType type() const { return mappi::InstrFrameType(conf::kVirr, mappi::conf::kCadu); }
      
      //      void setPretrSettings(const mappi::conf::PretrInstr& ) {};      

    protected:
      virtual bool needUnion() { return true; }
      ChannelVector<short>& channels() { return _ch; };
      const ChannelVector<short>& channels() const { return _ch; };
      const Channel<short>& channel(int number) const { return _ch.at(number); };


    private:

      bool parse(const QByteArray& dataAll, QDateTime* dtStart, QDateTime* dtEnd);
      bool readDt(const QDateTime& dt, const QByteArray& dataAll);
      void readDt(const QByteArray& buf, const QDate& d, QDateTime& dtStream);
      bool readStartDt(const QDate& date, QDateTime& dtStream, const QByteArray& data);
      void determDt(QDateTime& dtB, int numDt, float velocity);
      bool correction(const SatViewPoint* ) { return true; }
      bool calibrate() { return false; }

      //      bool parseRT(const QByteArray& dataAll, const QDateTime& dt, int* remainByte);

    private:
      ChannelVector<short> _ch; //каналы

      mappi::conf::Instrument _conf; //!< описание формата прибора
      Frame* _frame;

    };
  
  }
}



#endif
