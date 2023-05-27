#include "mhrpt.h"

#include <mappi/pretreatment/formats/instrument.h>
#include <mappi/pretreatment/formats/satformat.h>
#include <mappi/pretreatment/formats/stream_utility.h>

#include <cross-commons/debug/tlog.h>

using namespace mappi;
using namespace po;

namespace {
  mappi::po::Frame* createMHrpt(const mappi::conf::Frame& conf, Frame* parent)
  {
    return new mappi::po::MHrpt(conf, parent);
  }

  static const bool res = mappi::po::singleton::SatFormat::instance()->registerFrameHandler(mappi::conf::kMhrpt, createMHrpt);
}

// const uchar synchro_meteor_32[4] = { 0x1A, 0xCF, 0xFC, 0x1D };
// int synchro_len_meteor_32 = 4;


MHrpt::MHrpt(const conf::Frame& conf, Frame* parent):
  Frame(conf, true, parent), 
  _format(conf) 
{
}

MHrpt::~MHrpt()
{
  clear();
}


void MHrpt::clear()
{
  Frame::clear();
}


//! парсинг подкадров
/*! 
  \param frame прибор, отвечающий за обработку заданного типа подкадра
  \param frameFormat описание подкадра
  \return номер последнего считанного байта
*/
int MHrpt::parseFrame(Instrument* instr, const conf::InstrFormat& instrFormat, const QByteArray& data)
{
  int pos = 0;
  
  //debug_log << _format.size() << data.size();
  
  while ((pos + _format.size()) <=  data.size()) {
    for (int idx = 0; idx < instrFormat.sect_size(); idx++) {
      instr->addFrame(data.mid(pos + instrFormat.sect(idx).start(), instrFormat.sect(idx).size()));
    }
    pos +=  _format.size();
  }
  
  return pos;
}

