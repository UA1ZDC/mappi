#ifndef MAPPI_PRETREATMENT_FRAMES_CADU_H
#define MAPPI_PRETREATMENT_FRAMES_CADU_H

#include <mappi/pretreatment/formats/frame.h>
#include <mappi/pretreatment/frames/frametypes.h>
#include <mappi/proto/satellite.pb.h>

namespace mappi {
  namespace po {

    //! обработка HRPT формата
    class Cadu : public Frame {
    public:
      Cadu(const conf::Frame& conf, Frame* parent);
      virtual ~Cadu();

      enum {
	Type = conf::kCadu
      };

      virtual conf::FrameType type() const { return conf::kCadu; }

    };


  }
}


#endif
