#ifndef MAPPI_PRETREATMENT_FRAMES_VCDU_H
#define MAPPI_PRETREATMENT_FRAMES_VCDU_H

#include <mappi/pretreatment/formats/frame.h>
#include <mappi/pretreatment/frames/frametypes.h>
#include <mappi/proto/satellite.pb.h>

namespace mappi {
namespace po {

struct VcduHeader {
  uint8_t version = 0;
  uint8_t scid = 0; //id спутника
  uint8_t vcid = 0; //id виртуального канала (прибор)
  uint32_t counter = 0; //номер пакета
  uint8_t replayFlag  = 0;
  //uint16_t insertZone = 0;
  bool hasInsertZone = false; //наличие insert zone (шифрование)
  uint16_t packOffset = 0xff;  //сдвиг до начала заголовка;
  bool hasMpdu = true; //наличие MPDU кадра
  VcduHeader(bool hasIZ):hasInsertZone(hasIZ) {}
  bool parse(const QByteArray& data, uint pos);
};

TLog& operator<<(TLog& out, const VcduHeader& header);


class Instrument;

//! обработка HRPT формата
class Vcdu : public Frame {
public:
  Vcdu(const conf::Frame& conf, Frame* parent);
  virtual ~Vcdu();

  enum {
    Type = conf::kVcdu
  };

  virtual conf::FrameType type() const { return conf::kVcdu; }

private:
  int parseFrame(Instrument* frame, const conf::InstrFormat& frameFormat, const QByteArray& data);
  int64_t findStartCCSDS(const QByteArray& data);
  bool packIsValid(QByteArray cur, QByteArray prev);

};


}
}


#endif
