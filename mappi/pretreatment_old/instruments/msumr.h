#ifndef MAPPI_PRETREATMENT_INSTRUMENTS_MSUMR_H
#define MAPPI_PRETREATMENT_INSTRUMENTS_MSUMR_H

#include <mappi/pretreatment/formats/instrument.h>
#include <mappi/proto/satellite.pb.h>
#include <mappi/pretreatment/formats/hrpt_frame.h>


#include <qvector.h>

namespace mappi {
  namespace po {

    class Frame;
    class MHrpt;

   //! Чтение данных МСУ
    class MsuMr : public InstrumentTempl<short> {
    public:
      MsuMr(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent);
      ~MsuMr();
    
      // enum {
      // 	Type = conf::kMsuMr
      // };

      virtual InstrFrameType type() const { return mappi::InstrFrameType(conf::kMsuMr,  mappi::conf::kMhrpt); }
      
      //      void setPretrSettings(const mappi::conf::PretrInstr& ) {};      

    protected:
      ChannelVector<short>& channels() { return _ch; };
      const ChannelVector<short>& channels() const { return _ch; };
      const Channel<short>& channel(int number) const { return _ch.at(number); };

      
    private:
      bool parse(const QByteArray& data, QDateTime* dtStart, QDateTime* dtEnd);
      //      bool parseRT(const QByteArray& adata, const QDateTime& dt, int* remainByte);
      bool correction(const SatViewPoint* ) { return true; }
      bool calibrate() { return false; }

      int64_t findStartFrame(const QByteArray& data, int goodCnt);
      int findStartTime(const QByteArray& data, int lineSize,  int linemsecs,
			const QDate& date, QDateTime& dtBeg);
      int  findEndTime(const QByteArray& buf, int lineSize, int linemsecs,
		       const QDate& date, QDateTime& dtEnd);
      bool findNextTrustTime(const QByteArray& data, long* aoffset, int lineSize, const QDate& date, 
			     QDateTime* nextDt, int* numDt);
      bool checkNearDt(const QByteArray& data, long offset, int lineSize, 
		       const QDateTime& curDt, QDateTime* nextDt, int* numDt);
      
      void fillMissingLines(QByteArray* data, int lineSize, const QDateTime& dtBeg, int startNum, int endNum);

      int getNextTrustDt(const QByteArray& data, long offset, int lineSize, 
			 const QDateTime& curDt, QDateTime* nextDt, int* numDt);
      bool checkDt(const QByteArray& buf, ulong offset, int lineSize, const QDateTime& verDt);
      int fillMissingParts(QByteArray* data);

      void readDt(const QByteArray& buf, const QDate& d, QDateTime& dtStream );
      void readAndCheckDt(const QByteArray& buf, int* numDt, int* check, const QDate& date, QDateTime& dtBeg );
    private:
      ChannelVector<short> _ch; //каналы

      mappi::conf::Instrument _conf; //!< описание формата прибора
      Frame* _frame;
    };
  
  }
}


#endif
