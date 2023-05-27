#include "falsecolor.h"
#include "channel.h"
#include "themformat.h"

#include <qimage.h>
#include <qsharedpointer.h>

#include <mappi/thematic/images/colors.h>

#include <mappi/thematic/thematiccalc/thematiccalc.h>


using namespace mappi;
using namespace to;

namespace {
  mappi::to::ThemAlg* createFalseColor()
  {
    return new mappi::to::FalseColor();
  }

  mappi::to::ThemAlg* createDayMicrophysicsColor()
  {
    return new mappi::to::FalseColor();
  }

  mappi::to::ThemAlg* createCloudsColor()
  {
    return new mappi::to::FalseColor();
  }
  mappi::to::ThemAlg* createNaturalColor()
  {
    return new mappi::to::FalseColor();
  }
  mappi::to::ThemAlg* createNightMicrophysicsColor()
  {
    return new mappi::to::FalseColor();
  }
  
  static const bool res = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::proto::kFalseColor, createFalseColor);
  static const bool res1 = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::proto::kDayMicrophisicsColor, createDayMicrophysicsColor);
  static const bool res2 = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::proto::kCloudsColor, createCloudsColor);
  static const bool res3 = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::proto::kNaturalColor, createNaturalColor);
  static const bool res4 = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::proto::kNightMicrophysicsColor, createNightMicrophysicsColor);
}


FalseColor::FalseColor()
{
}

FalseColor::~FalseColor()
{
}

bool FalseColor::process()
{
  trc;
  _pixels.clear();
  _data.clear();
  _bitmap.clear();
  normalize(0, 255);

  const auto ch = channels();
  if (!ch.contains("CH1") || !ch.contains("CH2") || !ch.contains("CH3")) {
    return false;
  }
  auto r = ch["CH1"]
     , g = ch["CH2"]
     , b = ch["CH3"];
  uchar alpha = 255;

  int size = std::min(r->size(), std::min(g->size(), b->size()));

  _bitmap.resize(size * 4);
  for (int idx = 0; idx < size; ++idx) {
    _bitmap[idx * 4 + 3] = alpha;
    _bitmap[idx * 4 + 2] = r->at(idx);
    _bitmap[idx * 4 + 1] = g->at(idx);
    _bitmap[idx * 4]     = b->at(idx);
  }
 

  return true;
}

bool FalseColor::process(const proto::ThematicProc &thematic)
{
  trc;
  _data.clear();
  _pixels.clear();
  _bitmap.clear();
  const auto chs = channels();

  if(thematic.vars_size() < chs.size() || false == isValid(thematic)) {
    return false;
  }

 
  thematic::ThematicCalc calc;
  debug_log << QObject::tr("Привязка данных");
  for(auto themVar : thematic.vars())
  {
    bool ok = false;
    for(auto themCh : themVar.channel())
    {
      auto ch = chs.find(QString::fromStdString(themCh.channel()));
      if(chs.end() != ch &&
         ch.value()->header().satellite == QString::fromStdString(themCh.satellite()) &&
         ch.value()->header().instr == themCh.instrument()
      ) {
        calc.bindArray(QString::fromStdString(themVar.name()), *ch.value());
        ok = true;
        break;
      }
    }
    if(!ok) {
      warning_log << QObject::tr("Не удалось связать все данные");
      return false; // Если хоть одна переменная не связана выход
    }
  }

  calc.parsingExpression(QString::fromStdString(thematic.red()));
  _pixels.insert(Color::kRed, calc.dataProcessing());
  _data.insert(Color::kRed, calc.getResult());

  calc.parsingExpression(QString::fromStdString(thematic.green()));
  _pixels.insert(Color::kGreen, calc.dataProcessing());
  _data.insert(Color::kGreen, calc.getResult());

  calc.parsingExpression(QString::fromStdString(thematic.blue()));
  _pixels.insert(Color::kBlue, calc.dataProcessing());
  _data.insert(Color::kBlue, calc.getResult());

    
  for(auto it : _pixels) {
    colors::whiteBalance(it.data(), it.size());
  }

  return !_data.isEmpty();
}

bool FalseColor::saveImage(const QString& baseName)
{
  trc;

  const auto chs = channels();
  if (chs.isEmpty()) {
    debug_log << "empty";
    return false;
  }

  if(_bitmap.isEmpty()) {
      
    if (_pixels.size() < 3) {
      debug_log << "err size" << _pixels.size();
      return false;
    }
    
    var(_pixels.first().size());
    
    int size = _pixels.first().size();
    _bitmap.resize(size * 4);
    uchar alpha = 255;
    for(int i = 0; i < size; ++i)
    {
      _bitmap[i * 4]     = _pixels[Color::kBlue  ].at(i);
      _bitmap[i * 4 + 1] = _pixels[Color::kGreen].at(i);
      _bitmap[i * 4 + 2] = _pixels[Color::kRed ].at(i);
      _bitmap[i * 4 + 3] = alpha;
    }
  }

  imageFileName_ = baseName + ".falsecolor.png";
  var(baseName);
  var(imageFileName_);

  QImage im(_bitmap.data(), chs.first()->columns(), chs.first()->rows(), chs.first()->columns()*4, QImage::Format_ARGB32);
  bool ok = im.save(imageFileName_, "PNG");
  if (!ok) {
    error_log << QObject::tr("Ошибка сохранения файла %1").arg(imageFileName_);
  }

  debug_log << "write" << imageFileName_;

  return ok;
}

bool FalseColor::saveData(const QString& /*baseName*/)
{
  /* Сохранить QMap _data в каком-нибудь формате*/
  return true;
}

bool FalseColor::isValid(const proto::ThematicProc& thematic) {
  bool ok = thematic.has_name();
  ok &= thematic.has_enabled() && thematic.enabled();
  ok &= thematic.has_red();
  ok &= thematic.has_green();
  ok &= thematic.has_blue();
  for(auto themVar : thematic.vars())
  {
    if(!ok) break;
    ok &= themVar.has_name();
    for(auto ch : themVar.channel()) {
      if(!ok) break;
      ok &= ch.has_satellite() && ch.has_instrument() && ch.has_channel();
    }
  }
  return ok;
}
