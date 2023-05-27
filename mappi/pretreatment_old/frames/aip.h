#ifndef MAPPI_PRETREATMENT_FRAMES_AIP_H
#define MAPPI_PRETREATMENT_FRAMES_AIP_H


#include <mappi/pretreatment/formats/frame.h>
#include <mappi/pretreatment/frames/frametypes.h>

namespace mappi {
  namespace po {

        //! обработка HRPT формата
    class Aip : public Frame {
    public:
      Aip(const conf::Frame& conf, Frame* parent);
      virtual ~Aip();

      enum {
	Type = conf::kAip
      };

      conf::FrameType type() const { return conf::kAip; }

      void clear();
      void addFrame(const QByteArray& frame);
      
    protected:
      bool processFrames(const QDateTime& dt);
      int parseFrame(Instrument* instr, const conf::InstrFormat& instrFormat, const QByteArray& data);

    private:
      void determDataWordMinor(const QByteArray& data, int max, int* minor);
      bool checkDataWordByte(uchar lo, uchar val);
      
    private:
      int _hrptMinor = -1;
      int _aipMinor = -1;
    };

  }
}

#endif
