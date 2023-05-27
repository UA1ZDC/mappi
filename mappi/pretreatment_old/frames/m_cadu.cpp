#include "m_cadu.h"


#include <mappi/pretreatment/formats/instrument.h>
#include <mappi/pretreatment/formats/satformat.h>

#include <cross-commons/debug/tlog.h>

#include <qbytearray.h>
#include <qdatetime.h>

using namespace mappi;
using namespace po;

namespace {
  mappi::po::Frame* createCadu(const mappi::conf::Frame& conf, Frame* parent)
  {
    return new mappi::po::Cadu(conf, parent);
  }

  static const bool res = mappi::po::singleton::SatFormat::instance()->registerFrameHandler(mappi::conf::kCadu, createCadu);
  static const bool res1 = mappi::po::singleton::SatFormat::instance()->registerFrameHandler(mappi::conf::kCaduElectro, createCadu);
  static const bool res2 = mappi::po::singleton::SatFormat::instance()->registerFrameHandler(mappi::conf::kCaduClipped, createCadu);
}

Cadu::Cadu(const conf::Frame& conf, Frame* parent): 
  Frame(conf, true, parent)
{
}

Cadu::~Cadu()
{
}

