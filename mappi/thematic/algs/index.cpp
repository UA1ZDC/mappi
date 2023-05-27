#include "index.h"
#include "channel.h"
#include "themformat.h"

#include <qimage.h>
#include <qfile.h>

#include <mappi/thematic/thematiccalc/thematiccalc.h>

using namespace mappi;
using namespace to;

namespace {
  mappi::to::ThemAlg* createIndex8()
  {
    return new mappi::to::Index(proto::kIndex8);
  }
  mappi::to::ThemAlg* createIndex16()
  {
    return new mappi::to::Index(proto::kIndex16);
  }
  mappi::to::ThemAlg* createIndex24()
  {
    return new mappi::to::Index(proto::kIndex24);
  }
  mappi::to::ThemAlg* createIndex32()
  {
    return new mappi::to::Index(proto::kIndex32);
  }

  static const bool res8 = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::proto::kIndex8, createIndex8);
  static const bool res16 = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::proto::kIndex16, createIndex16);
  static const bool res24 = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::proto::kIndex24, createIndex24);
  static const bool res32 = mappi::to::singleton::ThemFormat::instance()->registerThemHandler(mappi::proto::kIndex32, createIndex32);
}

Index::Index(proto::ThemType type) : _themType(type) { fillPalette(); }

Index::~Index() { }

void Index::fillPalette()
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

bool Index::process()
{  
  _data.clear();
  _pixels.clear();

  auto ch = channels();
  if (2 != ch.size() ||
      0 == ch.first() || 0 == ch.last() ||
      ch["R0.6"]->size() != ch["R0.9"]->size()) {
    return false;
  }
  auto a1 = ch["R0.6"];
  auto a2 = ch["R0.9"];

  _data.resize(a1->size());
  _pixels.resize(a1->size());

  debug_log<<QObject::tr("Начало hardcode: %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
  for (int idx = 0; idx < a1->size(); idx++) {
    _data[idx] = 100 * ( (a2->at(idx) - a1->at(idx)) / (a2->at(idx) + a1->at(idx)) + 1);
    _pixels[idx] = _data[idx];
  }
  debug_log<<QObject::tr("Конец hardcode: %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
/*
  debug_log << QObject::tr("Начало thematiccalc: %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
    thematic::ThematicCalc calc;
    calc.parsingExpression("100 * ((R09 - R06) / (R09 + R06) + 1)");
    calc.bindArray("R09", *a2);
    calc.bindArray("R06", *a1);
    _pixels = calc.dataProcessing();
    _data = calc.getResult();
  debug_log<<QObject::tr("Конец thematiccalc: %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
*/
  return true;
}

bool Index::process(const proto::ThematicProc &thematic)
{
  _data.clear();
  _pixels.clear();
  const auto chs = channels();
  if(thematic.vars_size() < chs.size() || false == isValid(thematic))
    return false;

  thematic::ThematicCalc calc;
  debug_log << QObject::tr("Привязка данных");
  for(auto themVar : thematic.vars())
  {
    bool ok = false;
    QString varName = QString::fromStdString(themVar.name());
    for(auto themCh : themVar.channel())
    {
      auto ch = chs.find(QString::fromStdString(themCh.channel()));
      if(chs.end() != ch &&
         ch.value()->header().satellite == QString::fromStdString(themCh.satellite()) &&
         ch.value()->header().instr == themCh.instrument()
      ) {
        calc.bindArray(varName, *ch.value());
        ok = true;
        break;
      }
    }
    if(!ok) {
      warning_log << QObject::tr("Не удалось связать все данные");
      return false; // Если хоть одна переменная не связана выход
    }
  }

  debug_log << QObject::tr("Парсинг выражения");
  calc.parsingExpression(QString::fromStdString(thematic.red()));

  _pixels = calc.dataProcessing();
  _data = calc.getResult();
  return !_data.isEmpty();
}

bool Index::saveImage(const QString& baseName)
{
 if (_pixels.isEmpty())
    return false;

  const auto ch = channels();
  if (ch.isEmpty())
    return false;
  
  auto src = ch.first();

  imageFileName_ = baseName + ".ndvi.png";

  QImage im(_pixels.data(), src->columns(), src->rows(), src->columns(), QImage::Format_Indexed8);
  im.setColorCount(256);
  im.setColorTable(_palette);
  bool ok = im.save(imageFileName_, "PNG");
  if (!ok) {
    error_log << QObject::tr("Ошибка сохранения файла %1").arg(imageFileName_);
  }
  
  debug_log << "write" << imageFileName_;
  
  return ok;
}

bool Index::saveData(const QString& baseName)
{ 
  if(_data.isEmpty())
    return false;

  dataFileName_ = baseName + ".dat";
  QFile file(dataFileName_);
  if(file.open(QIODevice::WriteOnly)) {
    QDataStream stream(&file);
    stream << channels().first()->header();
    for(auto it  :_data) {
      switch (_themType) {
      case proto::kIndex8:
        stream << (unsigned char)it;
        break;
      case proto::kIndex16:
        stream << (unsigned short)it;
        break;
      case proto::kIndex24:
        stream << ((unsigned char)((int)it & 0x000000ff));
        stream << ((unsigned char)(((int)it & 0x0000ff00) >> 8));
        stream << ((unsigned char)(((int)it & 0x00ff0000) >> 16));
        break;
      case proto::kIndex32:
        stream << it;
        break;
      default:
        stream << (unsigned char)it;
        break;
      }
    }
  }
  file.close();
  return true;
}

bool Index::isValid(const proto::ThematicProc& thematic) {
  bool ok = thematic.has_name();
  ok &= thematic.has_red() && thematic.has_enabled();
  for(auto themVar : thematic.vars()) {
    if(!ok) break;
    ok &= themVar.has_name();
    for(auto ch : themVar.channel()) {
      if(!ok) break;
      ok &= ch.has_satellite() && ch.has_instrument() && ch.has_channel();
    }
  }
  return ok;
}
