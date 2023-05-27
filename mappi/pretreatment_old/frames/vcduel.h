#ifndef MAPPI_PRETREATMENT_FRAMES_VCDUEL_H
#define MAPPI_PRETREATMENT_FRAMES_VCDUEL_H

#include <mappi/pretreatment/formats/frame.h>
#include <mappi/pretreatment/frames/frametypes.h>
#include <mappi/proto/satellite.pb.h>

namespace mappi {
  namespace po {

    class Instrument;

    //! обработка HRPT формата
    class VcduElectro : public Frame {
    public:
      VcduElectro(const conf::Frame& conf, Frame* parent);
      virtual ~VcduElectro();

      enum {
	Type = conf::kVcduElectro
      };

      virtual conf::FrameType type() const { return conf::kVcduElectro; }

    private:
      int processLPart(const QDateTime& dt);
      bool setFrameDataL(Instrument* instr, const conf::InstrFormat& instrFormat);

      
      //int parseFrameL(Instrument* frame, const conf::InstrFormat& frameFormat, const QByteArray& data);
      int64_t findStartCCSDS(const QByteArray& data);
      
    
      // int parseFrameL(Instrument* instr, const conf::InstrFormat& instrFormat, const QByteArray& data);
    private:
      QMap<int, QString> _files;
    };


  }
}


#endif
