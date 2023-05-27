#include "surfacetempr.h"

#include "channel.h"
#include "themformat.h"
#include "datastore.h"

#include <mappi/settings/mappisettings.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/textproto/tprototext.h>

#include <qimage.h>
#include <qfile.h>

#define CONF_FILE  MnCommon::etcPath("mappi") + "/thematics/cloudtype.conf"


namespace {
  mappi::to::ThemAlg* createSurfTempr(mappi::conf::ThemType, QSharedPointer<mappi::to::DataStore>& ds)
  {
    return new mappi::to::kSurfTempr(ds);
  }

  static const bool res = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kSurfTempr, createSurfTempr);
}

using namespace mappi;
using namespace to;

kSurfTempr::kSurfTempr(QSharedPointer<to::DataStore>& ds):
  ThemAlg(ds)
{
  //TODO наверно не нужна здесь палитра...
  int alpha = 255;
  _palette.resize(256);
  for (uint i=0; i< 256; i++) {
    _palette[i] = QColor(i, i, i, alpha).rgba();
  }
}

kSurfTempr::~kSurfTempr()
{
}


bool kSurfTempr::readConfig(QList<conf::InstrumentType>* valid)
{
  return ThemAlg::readConfig(CONF_FILE, &_conf, valid);
}

//! список псевдонимов каналов необходимых для обработки
bool kSurfTempr::channelsList(conf::InstrumentType instr, QList<uint8_t>* chl)
{
  if (!_conf.IsInitialized() || nullptr == chl) {
    return false;
  }
  
  for (const auto &them : _conf.them()) {
    if (them.instr() == instr) {
      *chl << them.nir()
	   << them.red();
      return true;
    }
  }
  
  return false;
}


bool kSurfTempr::process(conf::InstrumentType instr)
{
  _data.clear();
  
  if (!_conf.IsInitialized()) {
    return false;
  }
  
  QSharedPointer<Channel> nir;
  QSharedPointer<Channel> red;
  
  for (const auto &them : _conf.them()) {
    if (them.instr() == instr) {
      nir = channels().value(them.nir());
      red = channels().value(them.red());
      break;
    }
  }

  if (nir.isNull() || red.isNull() ||
      nir->size() != red->size()) {
    return false;
  }
  
  _data.resize(nir->size());

    
  debug_log<<QObject::tr("Начало hardcode: %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
  for (int idx = 0; idx < nir->size(); idx++) {
    _data[idx] = 100 * ( (red->at(idx) - nir->at(idx)) / (red->at(idx) + nir->at(idx)) + 1);
  }
  debug_log<<QObject::tr("Конец hardcode: %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss"));

  QString path, name;
  getSavePath(nir->header().start, nir->header().satellite, &path, &name);
  name = path + name + "_surftemp";
  
  saveImage(name, nir, QImage::Format_Indexed8);
  saveData(name);
  
  return true;
}




