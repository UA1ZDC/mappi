#include "satformat.h"
#include "frame.h"
#include "instrument.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/textproto/tprototext.h>

#include <qfile.h>

#define INSTRCONF_FILE MnCommon::etcPath("mappi") + "satinstr.conf"
#define FRAMECONF_FILE MnCommon::etcPath("mappi") + "satframe.conf"
#define SATCONF_FILE MnCommon::etcPath("mappi") + "satpretr.conf"

namespace mappi {
  namespace po {

    namespace singleton {
      template<> mappi::po::SatFormat* SatFormat::_instance = 0;
    }
  }
}

using namespace mappi;
using namespace po;

SatFormat::SatFormat()
{
  init();
}

SatFormat::~SatFormat()
{
}


bool SatFormat::init()
{
  bool ok = true;
  ok &= readInstrs();
  ok &= readFrames();
  ok &= readSats();

  return ok;
}

//! Чтение настроек приборов
bool SatFormat::readInstrs()
{
  conf::InstrCollect iconf;

  QFile file(INSTRCONF_FILE);
  if ( !file.open(QIODevice::ReadOnly) ) {
    error_log << QObject::tr("Ошибка при загрузке параметров приборов из файла '%1'").arg(INSTRCONF_FILE);
    return false;
  }

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  if ( !TProtoText::fillProto(text, &iconf) ) {
    error_log << QObject::tr("Ошибка в структуре файла параметров приборов '%1'").arg(INSTRCONF_FILE);
    return false;
  }

  for (int idx = 0; idx < iconf.instrs_size(); idx++) {
    _instrConf.insert(iconf.instrs(idx).type(), iconf.instrs(idx));
  }
  
  return true;
}

//! Чтение настроек форматов
bool SatFormat::readFrames()
{
  conf::FrameCollect iconf;

  QFile file(FRAMECONF_FILE);
  if ( !file.open(QIODevice::ReadOnly) ) {
    error_log << QObject::tr("Ошибка при загрузке параметров кадра из файла '%1'").arg(FRAMECONF_FILE);
    return false;
  }

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  if ( !TProtoText::fillProto(text, &iconf) ) {
    error_log << QObject::tr("Ошибка в структуре файла параметров кадра '%1'").arg(FRAMECONF_FILE);
    return false;
  }

  for (int idx = 0; idx < iconf.frame_size(); idx++) {
    _frameConf.insert(iconf.frame(idx).type(), iconf.frame(idx));
  }

  for (int idx = 0; idx < iconf.instr_size(); idx++) {
    _instrFrameConf.insert(InstrFrameType(iconf.instr(idx).type(), iconf.instr(idx).frame()), iconf.instr(idx));
  }
  
  return true;
}

//! Чтение настроек спутников
bool SatFormat::readSats()
{
  conf::Pretreatment conf;
  
  QFile file(SATCONF_FILE);
  if ( !file.open(QIODevice::ReadOnly) ) {
    error_log << QObject::tr("Ошибка при загрузке параметров спутника из файла '%1'").arg(SATCONF_FILE);
    return false;
  }

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  if ( !TProtoText::fillProto(text, &conf) ) {
    error_log << QObject::tr("Ошибка в структуре файла параметров спутника '%1'").arg(SATCONF_FILE);
    return false;
  }

  for (int idx = 0; idx < conf.satellite_size(); idx++) {
    _satConf.insertMulti(QString::fromStdString(conf.satellite(idx).name()), conf.satellite(idx));
  }
  
  return true;
}


bool SatFormat::registerInstrHandler(InstrFrameType type, CreateInstr func)
{
  //info_log << QObject::tr("Instr %1, %2 registered").arg(type.first).arg(type.second);
  if ( true == _instHandler.contains(type) ) {
    warning_log << QObject::tr("Обработчик для прибора с типом (%1, %2) уже установлен").arg(type.first).arg(type.second);
    return false;
  }
  _instHandler.insert(type, func);
  return true;
}


bool SatFormat::registerFrameHandler(conf::FrameType type, CreateFrame func)
{
  //info_log << QObject::tr("Frame %1 registered").arg(type);
  if ( true == _frameHandler.contains(type) ) {
    warning_log << QObject::tr("Обработчик для кадра с типом %1 уже установлен").arg(type);
    return false;
  }
  _frameHandler.insert(type, func);
  return true;
}


Instrument* SatFormat::createInstrument(const InstrFrameType& type, const conf::InstrFormat& format, Frame* parent) const
{
  // trc;
  // var(type.first);
  // var(type.second);
  if ( !_instHandler.contains(type) || !_instrConf.contains(type.first)) {
    error_log << QObject::tr("Тип %1,%2 не зарегистрирован или не имеет настроек").arg(type.first).arg(type.second);
    return nullptr;
  }
  return _instHandler[type](_instrConf.value(type.first), format, parent);
}


Frame* SatFormat::createFrame(conf::FrameType type, Frame* parent) const
{
  // trc;
  // var(type);
  if ( !_frameHandler.contains(type) || !_frameConf.contains(type)) {
    error_log << QObject::tr("Тип %1 не зарегистрирован или не имеет настроек").arg(type);
    return 0;
  }
  return _frameHandler[type](_frameConf.value(type), parent);
}


Frame* SatFormat::createFrame(const conf::Frame& conf, Frame* parent) const
{
  // trc;
  // var(conf.type());
  if (!_frameHandler.contains(conf.type())) {
    error_log << QObject::tr("Тип %1 не зарегистрирован").arg(conf.type());
    return 0;
  }
  return _frameHandler[conf.type()](conf, parent);
}


//! Создание иерархии кадров, подкадров для прибора. Если есть родитель, добавляет в существующую иерархию
Frame* SatFormat::createFrame(conf::FrameType frameType,
			      const conf::InstrumentType instrType,
			      const conf::PretrInstr& conf,
			      po::Frame* frame,
			      SaveNotify* notify)
{
  // trc;
  
  QList<conf::Frame> frameconf;
  
  bool ok = createInstrHierarchy(_frameConf.value(frameType), instrType, &frameconf);
  if (!ok) {
    error_log << QObject::tr("Иерархия не построена. Кадр = %1. Прибор = %2").arg(frameType).arg(instrType);
    return nullptr;
  }
  // var(frameconf.size());
  
  InstrFrameType itype = InstrFrameType(instrType, frameconf.first().type());

  if (!_instHandler.contains(itype) || 
      !_instrConf.contains(instrType)   || 
      !_instrFrameConf.contains(itype)) {
    error_log << QObject::tr("Нет полной информации о приборе %1, %2").arg(instrType).arg(frameconf.first().type());
    debug_log << _instHandler.contains(itype)
    	      << _instrConf.contains(instrType)
    	      << _instrFrameConf.contains(itype);
    return nullptr;
  }
  
  if (frame == nullptr) {
    frame = createFrame(frameType, 0);
  }

  po::Frame* parent = frame;
  po::Frame* sub = frame;
  const mappi::conf::Frame& frConf = _frameConf.value(frameType);
  
  for (int idx = 1; idx < frameconf.size(); idx++) {
    if (nullptr != parent) {
      sub = parent->frame(frameconf.at(idx).type()); //проверяем наличие подкадра
    }
    if (nullptr == sub) {
      sub = createFrame(frameconf.at(idx), parent);      
    }
    parent = sub;
  }
  // var(sub);

  if (nullptr == sub) {
    return nullptr;
  }
  
  QList<int> instrId;
  for (int idx = 0; idx < _instrFrameConf.value(itype).format_size(); idx++) {
    if (_instrFrameConf.value(itype).format(idx).has_id()) {
      instrId.append(_instrFrameConf.value(itype).format(idx).id());
    } 
    Instrument* instr = createInstrument(itype, _instrFrameConf.value(itype).format(idx), frame);
    if (nullptr != instr) {
      instr->setSaveNotify(notify);
      sub->addInstrument(instr, instrType, conf);
    }
  }
  
  return frame;
}

bool SatFormat::createInstrHierarchy(const mappi::conf::Frame& frame, 
				     const conf::InstrumentType instrType,				     
				     QList<conf::Frame>* types)
{ 
  //  var(frame.type());
  
  //проверяем приборы в самом кадре
  for (auto instr : frame.instr()) {
    if (instr == instrType) {
      (*types) << frame;
      return true;
    }    
  }

  QList<conf::Frame> curtypes;
  
  //проверяем подкадры
  for (auto subframe : frame.frame()) {
    bool ok = createInstrHierarchy(subframe, instrType, &curtypes);
    if (ok) {
      types->append(curtypes);
      types->prepend(frame);
      return true;
    }
  }

  return false;
}

const mappi::conf::PretrSat* SatFormat::pretreatment(const QString& name, conf::RateMode mode)
{
  QMap<QString, mappi::conf::PretrSat>::const_iterator it = _satConf.constFind(name);
  while (it != _satConf.constEnd() && it.key() == name) {
    if (it.value().mode() == mode) {
      return &it.value();
    }
    ++it;
  }
  
  return nullptr;
}

// const mappi::conf::PretrInstr* SatFormat::instrumentRT(const QString& name, conf::RateMode mode,
// 						       mappi::conf::DataLevel* level,
// 						       mappi::conf::FrameType* frame_type)
// {
//   QMap<QString, mappi::conf::PretrSat>::const_iterator it = _satConf.constFind(name);
//   while (it != _satConf.constEnd() && it.key() == name) {
//     if (it.value().mode() == mode) {
//       *level = it.value().level();
//       *frame_type = it.value().frame();
//       for (int idx = 0; idx < it.value().instr_size(); idx++) {
// 	if (true == it.value().instr(idx).realtime()) {
// 	  return &it.value().instr(idx);
// 	}
//       }
//       break;
//     }
    
//     ++it;
//   }
  
//   return nullptr;
// }
