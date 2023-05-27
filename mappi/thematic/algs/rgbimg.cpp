#include "rgbimg.h"

#include "channel.h"
#include "themformat.h"
#include "datastore.h"

#include <mappi/settings/mappisettings.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/textproto/tprototext.h>

#include <qimage.h>
#include <qfile.h>
#include <qcolor.h>

#define CONF_FILE  MnCommon::etcPath("mappi") + "/thematics/rgb.conf"


namespace {

mappi::to::ThemAlg* createRgbImg(mappi::conf::ThemType type, QSharedPointer<mappi::to::DataStore>& ds)
{
  return new mappi::to::RgbImg(ds, type);
}
static const bool res1 = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kFalseColor, createRgbImg);
static const bool res2 = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kRgbDayMicrophysics, createRgbImg);
static const bool res3 = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kRgbNightMicrophysics, createRgbImg);
static const bool res4 = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kRgbNatural, createRgbImg);
static const bool res5 = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kRgbClouds, createRgbImg);
static const bool res6 = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kRgbTrueColor, createRgbImg);

}

using namespace mappi;
using namespace to;

RgbImg::RgbImg(QSharedPointer<to::DataStore>& ds, conf::ThemType type):
  ThemAlg(ds),
  _type(type)
{
}

RgbImg::~RgbImg()
{
}


bool RgbImg::readConfig(QList<conf::InstrumentType>* valid)
{
  conf::RgbConf rgbconf;
  bool ok = ThemAlg::readConfig(CONF_FILE, &rgbconf, valid);
  if (!ok) {
    return false;
  }

  ok = false;
  for (const auto &them : rgbconf.them()) {
    if (them.type() == type()) {
      conf::RgbThem* rt = _conf.add_them();
      rt->CopyFrom(them);
      ok = true;
    }
  }
  
  return ok;
}


//! список псевдонимов каналов необходимых для обработки
bool RgbImg::channelsList(conf::InstrumentType instr, QList<uint8_t>* chl)
{
  if (!_conf.IsInitialized() || nullptr == chl) {
    return false;
  }

  bool calibrate = false; //TODO from header PO
  
  for (const auto &them : _conf.them()) {
    if (them.instr() == instr &&
        them.calibrate() == calibrate) {
      if (!chl->contains(them.red().ch1())) {
        *chl << them.red().ch1();
      }
      if (them.red().has_ch2() && !chl->contains(them.red().ch2())) {
        *chl << them.red().ch2();
      }
      if (!chl->contains(them.green().ch1())) {
        *chl << them.green().ch1();
      }
      if (them.green().has_ch2() && !chl->contains(them.green().ch2())) {
        *chl << them.green().ch2();
      }
      if (!chl->contains(them.blue().ch1())) {
        *chl << them.blue().ch1();
      }
      if (them.blue().has_ch2() && !chl->contains(them.blue().ch1())) {
        *chl << them.blue().ch2();
      }
      
      return true;
    }
  }
  
  return false;
}

bool RgbImg::getChannels(const conf::RgbRange& range, QSharedPointer<Channel>* ch1, QSharedPointer<Channel>* ch2,
                         float* gamma, float* coef)
{
  if (!range.has_ch1()) {
    return false;
  }
  *ch1 = channels().value(range.ch1());
  
  if (range.has_ch2()) {
    *ch2 = channels().value(range.ch2());
  } else {
    *ch2 = QSharedPointer<DummyChannel>(new DummyChannel());
  }

  *gamma = 1/range.gamma();
  *coef = 255. / pow(range.max() - range.min(), *gamma);

  return true;
}

bool RgbImg::process(conf::InstrumentType instr)
{
  trc;
  _data.clear();
  
  if (!_conf.IsInitialized()) {
    return false;
  }

  bool calibrate = false; //TODO from header PO
  
  int tidx = -1;
  for (int idx = 0 ; idx < _conf.them_size(); idx++) {
    if (_conf.them(idx).instr() == instr &&
        _conf.them(idx).calibrate() == calibrate) {
      tidx = idx;
      break;
    }
  }
  if (tidx == -1) {
    return false;
  }
  const conf::RgbThem& conf = _conf.them(tidx);

  QSharedPointer<Channel> rch1, rch2;
  QSharedPointer<Channel> gch1, gch2;
  QSharedPointer<Channel> bch1, bch2;
  float redGamma, redCoef;
  float greenGamma, greenCoef;
  float blueGamma, blueCoef;
  getChannels(conf.red(),   &rch1, &rch2, &redGamma,   &redCoef);
  getChannels(conf.green(), &gch1, &gch2, &greenGamma, &greenCoef);
  getChannels(conf.blue(),  &bch1, &bch2, &blueGamma,  &blueCoef);

  
  // if (nullptr == nir || nullptr == red ||
  //     nir->size() != red->size()) {
  //   return false;
  // }
  
  _data.resize(rch1->size()*4);

  var(rch1->size());
  
  debug_log<<QObject::tr("Начало hardcode: %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
  
  for (int idx = 0; idx < rch1->size(); idx++) {
    _data[idx*4]     = blueCoef  * pow(bch1->getValue(idx) - bch2->getValue(idx) - conf.blue().min(),  blueGamma);
    _data[idx*4 + 1] = greenCoef * pow(gch1->getValue(idx) - gch2->getValue(idx) - conf.green().min(), greenGamma);
    _data[idx*4 + 2] = redCoef   * pow(rch1->getValue(idx) - rch2->getValue(idx) - conf.red().min(),   redGamma);
    _data[idx*4 + 3] = 255;
  }
  debug_log<<QObject::tr("Конец hardcode: %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss"));


  QString path, name;
  getSavePath(rch1->header().start, rch1->header().satellite, &path, &name);
  name = path + name + "_rgb." + QString::number(type());

  saveImage(name, rch1,QImage::Format_RGB32);
  saveData(name);
  
  return true;
}


