#ifndef MAPPI_PRETREATMENT_FRAMES_MHRPT_H
#define MAPPI_PRETREATMENT_FRAMES_MHRPT_H


#include <mappi/pretreatment/formats/frame.h>
#include <mappi/pretreatment/frames/frametypes.h>
#include <mappi/proto/satellite.pb.h>

class QByteArray;
class QDateTime;
class QDate;

namespace mappi {
  namespace po {

    class Instrument;

    //! обработка HRPT формата
    class MHrpt : public Frame {
    public:
      MHrpt(const conf::Frame& conf, Frame* parent);
      virtual ~MHrpt();

      enum {
	Type = conf::kMhrpt
      };

      virtual conf::FrameType type() const { return conf::kMhrpt; }

      void clear();

    private:
      int parseFrame(Instrument* frame, const conf::InstrFormat& instrFormat, const QByteArray& data);


    private:
      mappi::conf::Frame _format;
    };

  }
}

#endif
