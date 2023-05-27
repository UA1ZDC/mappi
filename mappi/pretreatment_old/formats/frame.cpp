#include "frame.h"
#include "instrument.h"
#include "stream_utility.h"

#include <commons/mathtools/mnmath.h>

#include <cross-commons/debug/tlog.h>

#include <qstring.h>

using namespace meteo;
using namespace mappi;
using namespace po;

/*! 
 * \brief Расчёт угла между меридианом,проходящим через начальную точку и линией, соединяющей начальную и конечную точку, относительно начальной точки (угол поворота изображения относительно начальной/верхней точки)
 * \param geoB начальная точка в географических координатах
 * \param geoE конечная точка  в географических координатах
 * \return Угол поворота, радиан
 */
float po::rotateAngle(Coords::GeoCoord geoB, Coords::GeoCoord geoE)
{
  // printf("beg: lat=%f lon=%f\nEnd: lat=%f lon=%f\n", rad2deg(geoB.lat), rad2deg(geoB.lon),
  // 	 rad2deg(geoE.lat), rad2deg(geoE.lon)
  // 	 );
  float d =  Coords::geoDistance(geoB, geoE);
  return MnMath::spAcos( (cos(geoE.lat)*cos(geoB.lon-geoE.lon) - cos(d)*cos(geoB.lat) ) / sin(d)/ sin(geoB.lat));
}


Frame::Frame(const conf::Frame& conf, bool setSat, Frame* parent) : 
  _parent(parent),
  _format(conf)
{
  if (nullptr != parent) {
    parent->addFrame(conf.type(), this);
  }
  
  if (setSat) {
    _sat = new SatViewPoint;
  }
}

Frame::~Frame()
{
  Frame::clear();

  delete _sat;
  _sat = nullptr;
}

void Frame::clear() 
{
  delete _decoder;
  _decoder = 0;

  for (int idx = 0; idx < _instr.size(); idx++) {
    Instrument* instr = _instr[idx];
    delete instr;
  }
  _instr.clear();

  QMapIterator<conf::FrameType, Frame*> it(_frames);
  while (it.hasNext()) {
    it.next();
    delete it.value();
  }
  _frames.clear();

  _data.clear();
}


QString Frame::satName()
{
  if (!_satName.isEmpty()) return _satName;
  if (0 != _parent) return _parent->satName();

  return QString();
}

conf::RateMode Frame::mode()
{
  if (conf::kUnkRate != _mode) return _mode;
  if (0 != _parent) return _parent->mode();
  
  return conf::kUnkRate;
}


//! Установка даты/времения первой и последней строк сканирования
/*! 
  \param dtBeg  дата/время первой строки сканирования
  \param dtEnd  дата/время последней строки сканирования
*/
void Frame::setDateTime(const QDateTime& dtBeg, const QDateTime& dtEnd)
{
  if (nullptr != _parent) {
    _parent->setDateTime(dtBeg, dtEnd);
    return;
  }

  if (nullptr == _sat) {
    error_log << QObject::tr("Ошибка установки даты/времени. Кадр сформирован без создания спутника");
    return;
  }
  
  debug_log << "dtBeg" << dtBeg.toString("dd.MM.yy hh:mm:ss.zzz") << "dtEnd" << dtEnd.toString("dd.MM.yy hh:mm:ss.zzz");

  _sat->setDateTime(dtBeg, dtEnd);

  //определение положения начальной и конечной точек сканирования
  Coords::GeoCoord geoB;
  Coords::GeoCoord geoE;
  if (! _sat->getPosition( _sat->timeFromTLE(dtBeg), &geoB) ||
      ! _sat->getPosition( _sat->timeFromTLE(dtEnd), &geoE) ) {
    error_log << QObject::tr("Ошибка определения положения спутника");
  }

  var(geoB.lat*180/3.14);
  var(geoB.lon*180/3.14);
  var(geoE.lat*180/3.14);
  var(geoE.lon*180/3.14);
  
  _rAngle = mappi::po::rotateAngle(geoB, geoE); //TODO
  debug_log<<"rangle=" << _rAngle * 180/3.14;
  // bool rotate = false;
  // if (fabs(rAngle) > M_PI_2) {
  //   rotate= true;
  // }
}

//! Загрузка орбитальных параметров
/*! 
  \param satName название спутника
  \param tleFile файл с орбитальными параметрами
*/
void Frame::setTLE(const QString& satName, const QString& tleFile) 
{
  if (nullptr == _sat) {
    error_log << QObject::tr("Ошибка загрузки орбитальных параметров. Кадр сформирован без создания спутника");
    return;
  }
  
  if (!_sat->readTLE(satName, tleFile)) {
    error_log << QObject::tr("Ошибка чтения TLE %1. Спутник %2").arg(tleFile).arg(satName);
  }
}

//! 
/*! Установка орбитальных параметров
  \param tle орбитальные параметры
*/
void Frame::setTLE(const MnSat::TLEParams& tle)
{
  if (nullptr == _sat) {
    error_log << QObject::tr("Ошибка загрузки орбитальных параметров. Кадр сформирован без создания спутника");
    return;
  }

  _sat->setTLEParams(tle);
}

//! Установка приборов, данные которых будут обрабатываться для текущего формата
/*! 
  \param instrs список приборов (название, настройка обработки)
*/
// void Frame::setInstruments(const google::protobuf::RepeatedPtrField<mappi::conf::PretrInstr>& instrs)
// {
//   for (int idx = 0; idx < instrs.size(); idx++) {
//     _instrPretr.insert(instrs.Get(idx).type(), instrs.Get(idx));
//   }
// }

void Frame::setDataLevel(conf::DataLevel level)
{
  _level = level;

  if (_level == conf::kManchesterLevel || _level == conf::kManchesterInvertLevel) {
    QByteArray synchroba = QByteArray::fromHex(QByteArray(_format.sync().data(), _format.sync().size()));
    
    if (_format.wordsize() <= 8) {
      _decoder = new meteo::ManchesterDecoder<uint8_t>(synchroba, _format.syncraw(), _format.syncmask(), _format.size(), _format.wordsize());
    } else  {
      _decoder = new meteo::ManchesterDecoder<uint16_t>(synchroba, _format.syncraw(), _format.syncmask(), _format.size() / 2, _format.wordsize());
    }
    
    _decoder->setThresholds(_format.sync_thr(), _format.resync_thr());
  }
}

bool Frame::addFrame(conf::FrameType type, Frame* frame)
{
  if (!_frames.contains(type)) {
    _frames.insert(type, frame);
    return true;
  }

  return false;
}

bool Frame::setData(const QByteArray& data)
{
  var(_level);
  bool ok = false;
  
  switch (_level) {
    case conf::kManchesterLevel:
      if (nullptr != _decoder) {
        QByteArray cur;
        ok = _decoder->decode(data, &cur);
        _data.append(cur);
      }
    break;
    case conf::kManchesterInvertLevel:
      if (nullptr != _decoder) {
        QByteArray cur;
        _decoder->decodeInvert(data, &cur);
        _data.append(cur);
      }
    break;
    case conf::kDeframerLevel:
    case conf::kDeframerIvertLevel:
      ok = createFrames(data);
    break;
    case conf::kFrameLevel:
      _data.append(data);
      ok = true;
    break;
    default: {
      return false;
    }
  }

  return ok;
}


//поток не выровненный по кадрам, необходимость побитового поиска синхры
bool Frame::createFrames(const QByteArray& data)
{
  bool invert = (_level == conf::kDeframerIvertLevel);

  QVector<uint8_t> syncraw;
  for (uint idx = 0; idx < _format.sync().size(); idx += 2) {
    syncraw.append(std::stoul(_format.sync().substr(idx, 2), nullptr, 16));
  }
  int synclen = syncraw.size();
  
  if (data.size() <= synclen) {
    return false;
  }

  int shift_bit = 0, shift_byte = 0;
  int pos = 0, next = 0;
  
  bool ok = meteo::findSynchro(data, syncraw.constData(), synclen, &shift_bit, &shift_byte, 0, 0xFF, invert);
  if (!ok) {
    return false;
  }
  var(ok);
  //debug_log << "shift_byte" << shift_byte;

  pos = shift_byte;
  int bit = shift_bit;

  int synconst = 3;
  int cnt = 1;
  
  int synCheckCnt = synconst;

  while ((pos + _format.size()) <= data.size()) {
    //если synconst раз ошибка в синхре, то ищем её заново

    if (synCheckCnt <= 0) {
      ok = meteo::findSynchro(data.right(data.size() - pos - synclen), syncraw.constData(), synclen, &shift_bit, &shift_byte, 0, 0xFF, invert);
      next = pos + synclen + shift_byte + 1; //+1 - кусочек с битами
      synCheckCnt = synconst;
      //debug_log << synCheckCnt << pos << next << next - pos;
    } else {
      next = pos + _format.size() + 1;
    }
    ++cnt;
    if (!ok) {
      synCheckCnt = 0;
      var(cnt);
      return (_data.size() > 0);
    }
    
    QByteArray syndata;
    int remain = 0;
    meteo::cif2raw(data.mid(pos, next - pos), bit, &syndata, &remain, 8, invert);
    if (!meteo::checkSynchro(syndata.left(synclen), syncraw.constData(), synclen, 0)) {
      --synCheckCnt;
      //  debug_log << syndata.left(synchro_len_meteor_32).toHex() << QByteArray((const char*)synchro_meteor_32, synchro_len_meteor_32).toHex() << _synCheckCnt << pos << next << next - pos;
    } else {
      synCheckCnt = synconst;
      //      debug_log << "ok" << syndata.left(synchro_len_meteor_32).toHex() << QByteArray((const char*)synchro_meteor_32, synchro_len_meteor_32).toHex() << _synCheckCnt << pos << next << next - pos;
    }

    // if (syndata.size() < _format.size()) {
    //   debug_log << syndata.size() << _format.size();
    // }
    // syndata.resize(_format.size());

    _data.append(syndata);

    pos = next - 1;
    bit = shift_bit;
  }
  
  var(cnt);
  //NOTE: если часть потока, то от pos до конца дынные будут потеряны

  return true;
}


bool Frame::parse(const QDateTime& dt)
{ 
  if (!parseFormatCustom(_level, dt, &_data)) {
    return false;
  }
  
  return processFrames(dt);
}


//! обработка подкадров
bool Frame::processFrames(const QDateTime& dt)
{
  bool ok = true;

  //анализ данных
  int pos = 0;
  QMapIterator<conf::FrameType, Frame*> it(_frames);
  while (it.hasNext()) {
    it.next();
    Frame* subframe = it.value();
    if (nullptr == subframe) continue;

    const conf::Frame& frameFormat = subframe->format();
    //subframe->clear();
    debug_log<<_format.size()<<frameFormat.header_start()<<_data.size();
    while ((pos + _format.size()) <= _data.size()) {
      subframe->addFrame(_data.mid(pos + frameFormat.header_start(), frameFormat.size()));
      pos +=  _format.size();
    }

    ok &= subframe->processFrames(dt);
  }


  
  for (int idx = 0; idx < _instr.size(); idx++) {
    Instrument* instr = _instr.at(idx);
    if (nullptr == instr) continue;
    pos = parseFrame(instr, instr->format(), _data);
    QDateTime dtStart = dt;
    QDateTime dtEnd = dt;

    ok &= instr->process(satellite(), instrPretr(instr->type().first), &dtStart, &dtEnd);
  }

  _data.remove(0, pos); //TODO подкадры и приборы одновременно
  
  return ok;
}


bool Frame::save(const global::StreamHeader& header)
{
  if (nullptr == _sat) {
    error_log << QObject::tr("Ошибка сохранения. Кадр сформирован без создания спутника");
    return false;
  }

  QString startStr = header.start.toString("yyyyMMddhhmm");
  QString endStr = header.stop.toString("yyyyMMddhhmm");
  if (_sat->dtStart().isValid()) {
    startStr = _sat->dtStart().toString("yyyyMMddhhmm");
  }

  if (_sat->dtEnd().isValid()) {
    endStr = _sat->dtEnd().toString("yyyyMMddhhmm");
  }
  
  QString baseName = startStr + "_"  + endStr  + "_" +   satName().remove(' ') ;

  debug_log << _sat->dtStart() << header.start;

  // if (fabs(_rAngle) > M_PI_2) {
  //   baseName += 's';
  // } else {
  //   baseName += 'n';
  // }
  
  baseName += _suffix;
  
  return save(header, baseName, _sat->dtStart(), _sat->dtEnd(), _sat);
}


bool Frame::save(const global::StreamHeader& header, const QString& baseName, const QDateTime& dtStart, const QDateTime& dtEnd,
                 SatViewPoint* sat)
{
  bool ok = true;

  QMapIterator<conf::FrameType, Frame*> it(_frames);
  while (it.hasNext()) {
    it.next();
    Frame* subframe = it.value();
    if (nullptr == subframe) continue;
    subframe->save(header, baseName, dtStart, dtEnd, sat);
  }

  for (int idx = 0; idx < _instr.size(); idx++) {
    Instrument* instr = _instr.at(idx);
    if (nullptr == instr) continue;
    instr->save(header, path(), baseName, dtStart, dtEnd, sat);
  }
  
  return ok;
}

//--------------------------------------------------------------------

bool Frame::parseL(const QDateTime& dt)
{ 
  if (!parseFormatCustom(_level, dt, &_data)) {
    return false;
  }
  
  return parseFramesL(dt);
}

bool Frame::parseFramesL(const QDateTime& dt)
{
  bool ok = true;

  int posLast = ceil(_data.size() / _format.size()) * _format.size();
  var(posLast);
  debug_log << _frames.size() << _instr.size();
  
  //анализ данных
  QMapIterator<conf::FrameType, Frame*> it(_frames);
  while (it.hasNext()) {
    it.next();

    Frame* subframe = it.value();
    if (nullptr == subframe) continue;

    const conf::Frame& frameFormat = subframe->format();
    int pos = 0;
    while ((pos + _format.size()) <= posLast) {
      uint64_t sync = ((uchar)_data[pos] << 24) + ((uchar)_data[pos+1] << 16) +
          ((uchar)_data[pos+2] << 8) + ((uchar)_data[pos+3]);
      if (!_format.has_syncraw() || sync == _format.syncraw()) {
        subframe->addFrame(_data.mid(pos + frameFormat.header_start(), frameFormat.size()));
      }

      pos += _format.size();
    }
    
    int subpos = subframe->processLPart(dt); //сохр в файл, поиск синхры, cif to raw
    if (!frameFormat.interim()) {
      ok &= subframe->parseFramesL(dt); //для тех, кто разбивает дальше данные
    } else {
      subframe->_data.remove(0, subpos);
    }
  }

  if (!_format.interim()) { //данные в инструменты, для фреймов не сохраняющих их у себя
    for (int idx = 0; idx < _instr.size(); idx++) {
      Instrument* instr = _instr.at(idx);
      if (nullptr == instr) continue;
      parseFrameL(instr, instr->format(), _data.right(posLast));
    }
  }
  
  _data.remove(0, posLast);
  return ok;
}

//после обработки всего файла
bool Frame::processFramesL(const QDateTime& dt, const global::StreamHeader& header, SatViewPoint* sat)
{
  // if (nullptr == _sat) {
  //   error_log << QObject::tr("Ошибка постобработки. Кадр сформирован без создания спутника");
  //   return false;
  // }

  
  bool ok = true;
  debug_log << _frames.size() << _instr.size();
  
  QMapIterator<conf::FrameType, Frame*> it(_frames);

  while (it.hasNext()) {
    it.next();
    Frame* subframe = it.value();
    if (nullptr == subframe) continue;

    const conf::Frame& frameFormat = subframe->format();
    if (!_format.interim()) {
      ok &= subframe->processFramesL(dt, header, sat);
    } else {
      
      ok &= setFrameDataL(frameFormat, &_data);

      int pos = 0;
      while ((pos + _format.size()) <= _data.size()) {
        subframe->addFrame(_data.mid(pos + frameFormat.header_start(), frameFormat.size()));
        pos += _format.size();
      }
      
      subframe->processLPart(dt); //сохр в файл, поиск синхры, cif to raw
      if (!frameFormat.interim()) {
        ok &= subframe->parseFramesL(dt); //для тех, кто разбивает дальше данные
      }
      _data.clear();
    }
  }

  for (int idx = 0; idx < _instr.size(); idx++) {
    Instrument* instr = _instr.at(idx);
    if (nullptr == instr) continue;
    if (_format.interim()) {
      setFrameDataL(instr, instr->format());
    }
    QDateTime dtStart = dt;
    QDateTime dtEnd = dt;
    ok &= instr->process(satellite(), instrPretr(instr->type().first), &dtStart, &dtEnd);

    
    QString baseName = //_sat->dtStart().toString("yyyyMMddhhmm")  + "_" + satName().remove(' ');
                       dtStart.toString("yyyyMMddhhmm")  + "_" + satName().remove(' ');

    ok &= instr->save(header, path(), baseName, dtStart, dtEnd, sat);
    
  }
  
  return ok;
}


//--------------------------------------------------------------------

//realtime
// bool Frame::parseRT(const QDateTime& dt, const QByteArray& data)
// { 
//   switch (_level) {
//   case conf::kManchesterLevel:
//     if (nullptr != _decoder) {
//       QByteArray cur;
//       _decoder->decode(data, &cur);
//       _data.append(cur);
//     }
//     break;
//   case conf::kManchesterInvertLevel:
//     if (nullptr != _decoder) {
//       QByteArray cur;
//       _decoder->decodeInvert(data, &cur);
//       _data.append(cur);
//     }
//     break;    
//   case conf::kDeframerLevel:
//   case conf::kDeframerIvertLevel:
//     error_log << QObject::tr("Not realised");
//     return false;
//   case conf::kFrameLevel:
//     _data.append(data);
//     break;
//   default: {
//     return false;
//   }
//   }  

//   if (!parseFormatCustomRT(_level, dt, &_data)) {
//     return false;
//   }

//   return processFramesRT(dt);
// }

//! обработка подкадров, realtime
// bool Frame::processFramesRT(const QDateTime& dt)
// {
//   bool ok = true;

//   //или подкадр, или прибор, смотря какие данные надо вернуть

//   if (_frames.size() != 0) {
//     int pos = 0;
//     Frame* subframe = _frames.first();

//     const conf::Frame& frameFormat = subframe->format();
//     //subframe->clear();

//     while ((pos + _format.size()) <= _data.size()) {
//       subframe->addFrame(_data.mid(pos + frameFormat.header_start(), frameFormat.size()));
//       pos +=  _format.size();
//     }  

//     ok &= subframe->processFramesRT(dt);
//     _data.remove(0, pos);

//   } else if (_instr.size() != 0) {
//     //debug_log << "format=" << _format.size() << _data.size();
//     Instrument* instr = _instr.first();
//     instr->resetRT();
//     ok = false;
//     if (_data.size() >= _format.size()) {
//       int pos = 0;
//       if (_level == conf::kDeframerIvertLevel) {
// 	pos = parseRaw(instr, instr->format(), _data, false);
//       } else {
// 	pos = parseFrame(instr, instr->format(), _data);
//       }
//       if (pos != 0) {
// 	ok &= instr->processRT(instrPretr(instr->type().first), dt);
// 	_data.remove(0, pos);
// 	ok = true;
//       }
//     } // else {
//     //   ok = false;
//     // }
//   }

//   return ok;
// }

//! Получение кусочка обработанных данных для прибора type. true - если прибор найден
// bool Frame::getDataRT(QByteArray* data)
// {
//   //если есть прибор, отдаём его данные
//   for (int idx = 0; idx < _instr.size(); idx++) {
//     Instrument* instr = _instr.at(idx);
//     if (nullptr == instr) continue;
//     instr->serializedImgDataRT(data);
//     return true;
//   }

//   //если нет прибора, обращаемся к подкадру
//   QMapIterator<conf::FrameType, Frame*> it(_frames);
//   while (it.hasNext()) {
//     it.next();
//     Frame* subframe = it.value();
//     if (nullptr == subframe) continue;

//     if (subframe->getDataRT(data)) {
//       return true;
//     }
//   }

//   return false;
// }
