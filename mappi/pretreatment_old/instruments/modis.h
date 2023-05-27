#ifndef MAPPI_PRETREATMENT_INSTRUMENTS_MODIS_H
#define MAPPI_PRETREATMENT_INSTRUMENTS_MODIS_H

#include <mappi/pretreatment/formats/instrument.h>
#include <mappi/proto/satellite.pb.h>
#include <mappi/pretreatment/frames/m_ccsds.h>

namespace mappi {
  namespace po {

    struct ModisHeader {
      uint16_t day_count;
      uint32_t coarse_time;
      uint16_t fine_time;
      
      bool quick_look;
      uint8_t packet_type;
      uint8_t scan_count;
      uint8_t mirror_side;
      
      uint8_t type_flag;
      uint16_t earth_frame_data_count;

      void parse(const QByteArray& pkt) {
        day_count = uint8_t(pkt[0]) << 8 | uint8_t(pkt[1]);
        coarse_time = uint8_t(pkt[2]) << 24 | uint8_t(pkt[3]) << 16 | uint8_t(pkt[4]) << 8 | uint8_t(pkt[5]);
        fine_time = uint8_t(pkt[6]) << 8 | uint8_t(pkt[7]);
	
        quick_look  =  uint8_t(pkt[8]) >> 7;
        packet_type = (uint8_t(pkt[8]) >> 4) % (int)pow(2, 3);
        scan_count  = (uint8_t(pkt[8]) >> 1) % (int)pow(2, 3);
        mirror_side =  uint8_t(pkt[8]) % 2;
	
        type_flag = uint8_t(pkt[9]) >> 7;
        earth_frame_data_count = (uint8_t(pkt[9]) % (int)pow(2, 7)) << 4 | uint8_t(pkt[10]) >> 4;
      }

      QDateTime dt() {
	QDateTime dt;
	dt.setDate(QDate(1958, 1, 1));
	dt = dt.addDays(day_count);
	dt.setTime(QTime(0,0));
	dt = dt.addMSecs(coarse_time);
	return dt;
      }
    };

    //TODO
    //TLog& operator<<(TLog& out, const CCSDSHeader& header);
        
    class Modis: public InstrumentTempl<ushort> {
    public:
      Modis(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent);
      ~Modis();
          
      virtual InstrFrameType type() const { return mappi::InstrFrameType(_conf.type(), mappi::conf::kCadu); }

      virtual float fovStep(int channel) const;
      virtual float velocity(int channel) const;


    protected:
      ChannelVector<ushort>& channels() { return _ch; };
      const ChannelVector<ushort>& channels() const { return _ch; };
      const Channel<ushort>& channel(int number) const { return _ch.at(number); };

      QVector<ushort> bytesTo12bits(const QByteArray &bytes, unsigned int offset, unsigned int words_length);
        uint16_t compute_crc(const QVector<ushort> &data);
      void processNightPacket(const QByteArray &data, const CCSDSHeader& ccheader,  const ModisHeader &header);
      void processDayPacket(const QByteArray& data, const CCSDSHeader& ccheader, const ModisHeader &header);

    private:	
      bool parse(const QList<QByteArray>& dataAll, QDateTime* dtStart, QDateTime* dtEnd);
      //bool parse(const QByteArray& dataAll, const QDateTime& dt);

      bool correction(const SatViewPoint* ) { return true; }
      bool calibrate() { return false; }
      
    private:
      
      ChannelVector<ushort> _ch; //каналы
      mappi::conf::Instrument _conf; //!< описание формата прибора
      Frame* _frame = nullptr;

      int _lines = 0;
      int lastScanCount;
    };
    
    
  }
}


#endif
