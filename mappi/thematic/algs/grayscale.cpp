#include "grayscale.h"
#include "channel.h"
#include "themformat.h"
#include "datastore.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/textproto/tprototext.h>

#include <qimage.h>
#include <qfile.h>


#include <mappi/thematic/thematiccalc/thematiccalc.h>

#define CONF_FILE  MnCommon::etcPath("mappi") + "/thematics/grayscale.conf"


namespace {
mappi::to::ThemAlg* createGrayScale(mappi::conf::ThemType, QSharedPointer<mappi::to::DataStore>& ds)
{
  return new mappi::to::GrayScale(ds);
}

static const bool res = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kGrayScale, createGrayScale);
}

using namespace mappi;
using namespace to;


GrayScale::GrayScale(QSharedPointer<to::DataStore>& ds):
  ThemAlg(ds)
{
  int alpha = 255;
  _palette.resize(256);
  for (uint i=0; i< 256; i++) {
    _palette[i] = QColor(i, i, i, alpha).rgba();
  }
}

GrayScale::~GrayScale()
{
}

//! чтение конфиг файла
bool GrayScale::readConfig(QList<conf::InstrumentType>* valid)
{
  return ThemAlg::readConfig(CONF_FILE, &_conf, valid);
}

//! список каналов необходимых для обработки
bool GrayScale::channelsList(conf::InstrumentType instr, QList<uint8_t>* chl)
{
  if (!_conf.IsInitialized() || nullptr == chl) {
    return false;
  }

  for (const auto &them : _conf.them()) {
    if (them.instr() == instr) {
      return true; //все каналы, какие есть в БД, список не заполняем
    }
  }
  
  return false;
}

//! Обработка данных прибора спутника
bool GrayScale::process(const QDateTime& start, const QString& satname, conf::InstrumentType instrument)
{
  if (nullptr == store()) {
    return false;
  }
  
  bool ok = false;
  QList<uint8_t> chNum;

  QMap<uint8_t, QString> chfiles; //номер канала, название файла
  store()->getFiles(start, satname, instrument, chNum, &chfiles);

  for (const auto &number : chfiles.keys()) {
    ok = readChannelData(number, chfiles.value(number));
    if (!ok) {
      continue;
    }
    
    const QSharedPointer<Channel>& ch = channels().value(number);
    if (ch.isNull()) {
      continue;
    }
    ok = processChannel(ch);
    
    if (ok) {
      QString path, name;
      getSavePath(start, satname, &path, &name);
      name = path + name + "_" + QString::number(instrument) + "." + QString::number(ch->number());
      
      saveImage(name, ch, QImage::Format_Indexed8);
      saveData(name);
    }

    clearChannels();
  }
  
  return ok;
}

//отдельный канал
bool GrayScale::processChannel(const QSharedPointer<Channel>& src)
{
  _data.clear();
  
  //TODO пока так для макета.  Должно быть в настройках, зависит от калибровки
  bool invert = false;
  if (src->number() >= 4 && src->header().satellite == "METEOR-M 2")  {
    invert = true;
  }

  _data.resize(src->size());

  float min = src->min();
  float max = src->max();

  for(int idx = 0, sz = src->size(); idx < sz; ++idx)
  {
    if (src->at(idx) > max || src->at(idx) < min) { //-9999 попадает
      _data[idx] = 0;
    } else {
      _data[idx] = uchar((src->at(idx) - min) * 255 / (max - min));
      if (invert) {
        _data[idx] = 255 - _data[idx];
      }
    }
  }

  return true;
}


