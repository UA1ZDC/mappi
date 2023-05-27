#include "handler.h"
#include <mappi/global/streamheader.h>

#include <mappi/pretreatment/formats/frame.h>
#include <mappi/pretreatment/formats/satformat.h>
#include <mappi/pretreatment/savenotify/savenotify.h>

#include <sat-commons/satellite/satellite.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <QFile>

using namespace mappi;
using namespace po;

Handler::Handler()
{
}

Handler::~Handler()
{
  delete _format;
  _format = 0;
}

void Handler::clear()
{
  delete _format;
  _format = 0;

  _mode = conf::kUnkRate;
  _fileName.clear();
  // _header;
  
  _level = mappi::conf::kUnkLevel;
  _suffix.clear();

}


//! запуск обработки данных
bool Handler::process(SaveNotify* notify)
{
  uint64_t dataSize;

  if (_fileName.isEmpty()) {
    error_log << QObject::tr("Не задан файл с потоком");
    return false;
  }
  
  
  QFile file(_fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    error_log << QObject::tr("Ошибка открытия файла %1").arg(_fileName);
    return false;
  }
  
  if(!readHeader(&file, &dataSize)) {
    file.close();
    return false;
  }

  var(dataSize);

  
  if (dataSize == 0) {
    error_log << QObject::tr("В файле нет данных");
    file.close();
    return false;
  }
  
  if (_header.satellite.isEmpty()) {
    error_log << QObject::tr("Не определено название спутника");
    file.close();
    return false;
  }

  if (!setupFormat(notify)) {
    file.close();
    return false;
  }

  var(_oneRead);
  
  notify->rawNotify(_header, _fileName);
  
  bool ok = false;
  if (_oneRead == 0) {
    ok = processFileAll(file, dataSize);
  } else {
    ok = processFileParts(file, dataSize);
  }

  
  file.close();

  if (nullptr != notify) {
    notify->finished();
  }
  
  return ok;
}

//файл целиком
bool Handler::processFileAll(QFile& file, uint32_t dataSize)
{
  QByteArray data = file.read(dataSize);
  bool ok = _format->setData(data);
  if (!ok) {
    error_log << QObject::tr("Ошибка декодирования данных");
    return ok;
  }
  data.clear();

  //_format->setDateTime(_header.start, _header.end);
  
  ok = _format->parse(_header.start);
  var(ok);

  if (ok) {
    if (_format->dtStart().isValid()) {
      _header.start = _format->dtStart();
    }
    if (_format->dtEnd().isValid()) {
      _header.stop   = _format->dtEnd();
    }
  }

  if (!_format->save(_header)) {
    error_log << QObject::tr("Ошибка сохранения данных");
    ok = false;
  }
  
  return ok;
}

//чтение файла по частям, с промежуточным сохранением и последующей обработкой
bool Handler::processFileParts(QFile& file, uint32_t dataSize)
{
  trc;
  QByteArray data;
  bool ok = false;
  
  var(dataSize);
  uint64_t curSize = _oneRead;
  if (curSize > dataSize) {
    curSize = dataSize;
  }
  
  var(curSize);

  uint64_t pos = 0;
  while (pos < dataSize) {
    
    data = file.read(curSize);
    var(data.size());
    
    ok = _format->setData(data);
    if (!ok) {
      error_log << QObject::tr("Ошибка декодирования данных");
      return ok;
    }
    data.clear();

    ok = _format->parseL(_header.start);
    var(ok);
    debug_log << dataSize - curSize;

    pos += curSize;
    
    if (dataSize - pos < _oneRead) {
      curSize = dataSize - pos;
    }

    debug_log << curSize << pos << dataSize;

    // debug_log << "TODO break";
    // break;
  }

  if (ok) {
    debug_log <<  _header.start << _format->dtStart().isValid() << _format->dtStart();
    
    if (_format->dtStart().isValid()) {
      _header.start = _format->dtStart();
    }
    if (_format->dtEnd().isValid()) {
      _header.stop   = _format->dtEnd();
    }
  }


  debug_log;
  debug_log << "process";

  _format->processFramesL(_header.start, _header, _format->satellite());

  return ok;
}

bool Handler::setupFormat(SaveNotify* notify)
{
  delete _format;
  _format =  nullptr;

  if (_mode == conf::kUnkRate) {
    _mode = conf::kHiRate;
  }

  const mappi::conf::PretrSat* conf = singleton::SatFormat::instance()->pretreatment(_header.satellite, _mode);
  if (0 == conf) {
    error_log << QObject::tr("Ошибка загрузки настроек предварительной обработки для спутника") << _header.satellite;
    return false;
  }

   var(conf->Utf8DebugString());
  
  for (int idx = 0; idx < conf->instr_size(); idx++) {
    _format = singleton::SatFormat::instance()->createFrame(conf->frame(),
                                                            conf->instr(idx).type(),
                                                            conf->instr(idx),
                                                            _format,
                                                            notify);
  }

  
  // var(_format->type());
  // var(_format->framesList());
  // var(_format->frames());
  // for (auto key : _format->frames()) {
  //   Frame* fr = _format->frame(key);
  //   var(fr->framesList());
  // }

  if (nullptr == _format) {
    error_log << QObject::tr("Ошибка создания фрейма для спутника") << _header.satellite;
    return false;
  }

  if (conf->has_oneread()) {
    _oneRead = conf->oneread() * 1024 * 1024;
  }
  
  _format->setSatName(_header.satellite);
  _format->setTLE(_header.tle);
  _format->setDtValid(_header.start.isValid());
  //var(_format->isDtValid());
  //_format->setInstruments(conf->instr());
  _format->setRecvMode(_mode);
  
  if (_level == mappi::conf::kUnkLevel) {
    _level = conf->level();
  }
  _format->setDataLevel(_level);
  _format->setPath(_path);
  var(_path);
  _format->setSuffix(_suffix);//для сохранения виндовых данных с другим именем
  
  return true;
}

//! чтение заголовка
bool Handler::readHeader(QFile* file, uint64_t* dataSize)
{
  if (nullptr == dataSize) {
    return false;
  }

  *dataSize = file->size();
  
  QDataStream in(file);

  switch (_headerType) {
    case kPreOldHeader:  //заголовок в начале старого формата
    {
      meteo::global::StreamHeaderOld sHeadOld;
      in >> sHeadOld;
      sHeadOld.fill(&_header);
      *dataSize -= meteo::global::StreamHeaderOld::size();
    }
    break;

    case kPostOldHeader:  //заголовок в конце старого формата
    {
      *dataSize -= meteo::global::StreamHeaderOld::size();
      in.skipRawData(*dataSize);
      meteo::global::StreamHeaderOld sHeadOld;
      in >> sHeadOld;
      sHeadOld.fill(&_header);
      file->seek(0);
    }
    break;
    case kMappiHeader:  //заголовок в начале нового формата
    {
      meteo::global::PreHeader preHeader;
      in >> preHeader;
      if (preHeader.type == meteo::global::kRawFile) {
        QByteArray ar(preHeader.offset, 0);
        in.readRawData(ar.data(), ar.size());
        // in >> _header;
        var(ar.size());
        meteo::global::fromBuffer(ar, _header);
        *dataSize = file->size() - preHeader.size() - preHeader.offset;
        var(preHeader.offset);
        debug_log << _header.site << _header.siteCoord << _header.start << _header.stop
                  << _header.satellite << _header.direction;
      } else {
        error_log << QObject::tr("Тип файла не соответствует сырому потоку");
      }
    }
    break;
    default:
    {}
  }
  

  return true;
}




//! настройка формата в соответствии с дополниетельным заголовком
// void Handler::setupFormat(satellite::StreamHeader& sHead)
// {
//   _format->setSatName(sHead.satName);
//   _format->setDtValid(true);
//   _format->setTLE(sHead.tle());
//   _format->setInstruments(_settings->instruments(sHead.satName));
// }


//! Настройка обработки данных в реальном времени
// bool Handler::setupRT(const QString& satName, mappi::conf::RateMode mode, const MnSat::TLEParams& tle)
// {
//   mappi::conf::DataLevel level;
//   mappi::conf::FrameType frame_type;

//   delete _format;
//   _format = nullptr;

//   const mappi::conf::PretrInstr* conf = singleton::SatFormat::instance()->instrumentRT(satName, mode, &level, &frame_type);
//   if (nullptr == conf) {
//     return false;
//   }

//   _format = singleton::SatFormat::instance()->createFrame(frame_type, conf->type(), *conf, nullptr, nullptr);


//   if (nullptr == _format) return false;

//   _format->setTLE(tle);
//   _format->setSatName(satName);
//   //_format->setInstrument(conf->type(), *conf);
//   _format->setRecvMode(mode);

//   if (_level == mappi::conf::kUnkLevel) {
//     _format->setDataLevel(level);
//   }

//   return true;
// }

//! Обработка данных в реальном времени
// bool Handler::processRT(const QDateTime& dt, const QByteArray& data, QByteArray* result)
// {
//   if (nullptr == _format || nullptr == result) return false;

//   result->clear();

//   bool ok = _format->parseRT(dt, data);
//   if (ok) {
//     ok = _format->getDataRT(result);
//   }

//   return ok;
// }
