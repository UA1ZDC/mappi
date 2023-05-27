#include "ndvi.h"

#include "channel.h"
#include "themformat.h"
#include "datastore.h"

#include <mappi/settings/mappisettings.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/textproto/tprototext.h>
#include <mappi/thematic/thematiccalc/thematiccalc.h>

#include <qimage.h>
#include <qfile.h>

#define CONF_FILE  MnCommon::etcPath("mappi") + "/thematics/ndvi.conf"


namespace {
mappi::to::ThemAlg* createNdvi(mappi::conf::ThemType, QSharedPointer<mappi::to::DataStore>& ds)
{
  return new mappi::to::Ndvi(ds);
}

static const bool res = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::conf::kNdvi, createNdvi);
}

using namespace mappi;
using namespace to;

Ndvi::Ndvi(QSharedPointer<to::DataStore>& ds):
  ThemAlg(ds)
{
  int alpha = 255;
  _palette.resize(201);

  for (unsigned char i=0; i<= 50; i++) {
    _palette[i] = QColor(0, 0, 0, alpha).rgba();
  }
  for (unsigned char i=51; i<= 60; i++) {
    _palette[i] = QColor(0, 0, (i-50)*5, alpha).rgba();
  }
  for (unsigned char i=61; i<= 75; i++) {
    _palette[i] = QColor(0, 0, 50+(i-60)*9, alpha).rgba();
  }
  for (unsigned char i=76; i<= 85; i++) {
    _palette[i] = QColor(0, (i-75)*10, 140+(i-75), alpha).rgba();
  }
  for (unsigned char i=86; i<= 95; i++) {
    _palette[i] = QColor((i-85)*4, 100+(i-85)*15, 150+(i-85)*10, alpha).rgba();
  }
  for (unsigned char i=96; i<= 98; i++) {
    _palette[i] = QColor(40+(i-95)*46, 250+(i-95), 250+(i-95), alpha).rgba();
  }
  for (unsigned char i=99; i<= 101; i++) {
    _palette[i] = QColor(178+(i-98)*25, 255, 255, alpha).rgba();
  }
  for (unsigned char i=102; i<= 106; i++) {
    _palette[i] = QColor(255-(i-101)*15, 255-(i-101)*15, 255-(i-101)*15, alpha).rgba();
  }
  for (unsigned char i=107; i<= 116; i++) {
    _palette[i] = QColor(195 - (i-107)*4, 150 - (i-107)*6, 95 - (i-107)*8, alpha).rgba();
  }
  for (unsigned char i=117; i<= 130; i++) {
    _palette[i] = QColor(155 - (i-117)*8, 190 - (i-117), 80 - (i-117)*4, alpha).rgba();
  }
  for (unsigned char i=131; i<= 159; i++) {
    _palette[i] = QColor(51 - (i-131), 177 - (i-131), 28 - (i-131), alpha).rgba();
  }

  for (unsigned char i=160; i<= 200; i++) {
    _palette[i] = QColor(0, 149 - (i-159)*3, 0, alpha).rgba();
  }
}

Ndvi::~Ndvi()
{
}


bool Ndvi::readConfig(QList<conf::InstrumentType>* valid)
{
  return ThemAlg::readConfig(CONF_FILE, &_conf, valid);
}

//! список псевдонимов каналов необходимых для обработки
bool Ndvi::channelsList(conf::InstrumentType instr, QList<uint8_t>* chl)
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


bool Ndvi::process(conf::InstrumentType instr)
{
  _data.clear();
  
  if (!_conf.IsInitialized()) {
    return false;
  }
  
  QSharedPointer<Channel> nir;
  QSharedPointer<Channel> red;
  
  for (const auto & them : _conf.them()) {
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

/*
  debug_log<<QObject::tr("Начало hardcode: %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
  for (int idx = 0; idx < nir->size(); idx++) {
    _data[idx] = 100 * ( (red->at(idx) - nir->at(idx)) / (red->at(idx) + nir->at(idx)) + 1);
  }
  debug_log<<QObject::tr("Конец hardcode: %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
*/

  debug_log << QObject::tr("Начало thematiccalc: %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
  thematic::ThematicCalc calc;
  calc.parsingExpression("100 * ((R09 - R06) / (R09 + R06) + 1)");
  calc.bindArray("R09", *nir);
  calc.bindArray("R06", *red);
  _data = calc.dataProcessing();
  QVector<float> data;
  data = calc.getResult();
  debug_log<<QObject::tr("Конец thematiccalc: %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss"));


  QString path, name;
  getSavePath(nir->header().start, nir->header().satellite, &path, &name);
  name = path + name + "_ndvi";
  
  saveImage(name, nir, QImage::Format_Indexed8);
  saveData(name);
  
  return true;
}




