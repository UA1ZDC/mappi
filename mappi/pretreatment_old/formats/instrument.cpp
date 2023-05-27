#include "instrument.h"
#include "channel.h"

#include <commons/textproto/pbtools.h>
#include <sat-commons/satellite/satviewpoint.h>
#include <cross-commons/debug/tlog.h>
#include <mappi/pretreatment/savenotify/savenotify.h>

using namespace meteo;
using namespace mappi;
using namespace po;

QString db(unsigned value)
{
  QString value_binary("\0");
  
  if ((value)!=1 && value != 0) { value_binary=db(value/2);}
  char a=value%2+48;
  return value_binary+=a;
}


Instrument::Instrument(const mappi::conf::Instrument& conf, const conf::InstrFormat& format):
  _conf(conf),
  _format(format)
{
}

Instrument::~Instrument()
{
}

//! Обработка данных после получения всех данных
bool Instrument::process(SatViewPoint* sat,  const mappi::conf::PretrInstr& settings, QDateTime* dtStart, QDateTime* dtEnd) 
{ 
  bool ok;
  _isCalibrate = false;
  _pretr = settings;
  
  //  setPretrSettings(settings);

  //  debug_log<< "Parse size =" << _data.size();
  if (_data.size() == 1) {
    ok = parse(_data.first(), dtStart, dtEnd);
  } else if (needUnion()) {
    ok = unionData();
    if (ok) {
      ok = parse(_data.first(), dtStart, dtEnd);
    }
  } else {
    ok = parse(_data, dtStart, dtEnd);
  }

  _data.clear();
  _counter.clear();
  if (!ok) return false;

  if (settings.calibrate()) {
    debug_log << "Calibrate";
    _isCalibrate = calibrate();
  }

  Q_UNUSED(sat)
  // if (settings.geom()) {
  // debug_log<< "Correction";
  // ok = correction(sat);
  // }

  return ok;
}    

//! Обработка данных после получения всех данных
// bool Instrument::processRT(const mappi::conf::PretrInstr& /*settings*/, const QDateTime& dt) 
// { 
//   bool ok;
//   _isCalibrate = false;

//   if (_data.size() == 0) return true;

//   //setPretrSettings(settings);

//   int remain = 0;

//   // debug_log<< "Parse size =" << _data.size();
//   if (_data.size() == 1) {
//     ok = parseRT(_data.first(), dt, &remain);
//   } else if (needUnion()) {
//     ok = unionData();
//     if (ok) {
//       ok = parseRT(_data.first(), dt, &remain);
//     }
//   } else {
//     ok = parseRT(_data, dt, &remain);     
//   }

//   if (_data.size() == 1 || needUnion()) {
//     //    debug_log << "remove" << _data.at(0).size() - remain;
//     _data[0].remove(0, _data.at(0).size() - remain);
//   } else {
//     //    debug_log << "remove ar" << remain << _data.size();
//     auto itEnd = _data.end();
//     itEnd -= remain;
//     _data.erase(_data.begin(), itEnd);
//     _counter.clear();
//     //    var(_data.size());
//   }

//   if (!ok) return false;

//    return ok;
// }

//! Объединение данных из кадров с добавлением пустых при пропуске кадра
bool Instrument::unionData()
{
  if (_data.size() != _counter.size() || _counter.size() == 0) {
    debug_log << QObject::tr("Невозможно объединить данные data= %1 counter=%2").arg(_data.size()).arg(_counter.size());
    return false;
  }

  // QByteArray data = _data[0];
  // _data[0].clear();

  int counter = _counter.at(0);
  for (int idx = 1; idx < _counter.size(); idx++) {
    //debug_log << counter << _data.at(idx).size();
    if (counter + 1 == _counter.at(idx)) {
      _data[0].append(_data.at(idx));
      _data[idx].clear();
      counter = _counter.at(idx);
    } else {
      int missed = missedFrames(counter, idx);
      //debug_log << "counter" << counter << _counter.at(idx) <<_counter.at(idx + 1) << missed;
      
      if (missed != -1) {
        QByteArray dummy(_format.sect(0).size(), 0);
        for (int kk = 0; kk < missed; kk++) {
          _data[0].append(dummy);
          ++counter;
        }
        _data[0].append(_data.at(idx));
        _data[idx].clear();
        ++counter;
      }
    }
  }

  _counter[0] = counter;
  _data.erase(_data.begin() + 1, _data.end());
  _counter.erase(_counter.begin() + 1, _counter.end());
  
  //  var(_data.at(0).size());

  return true;
}


//! Количество пропущенных кадров. -1 если номер кадра некорректен
int Instrument::missedFrames(int counter, int idx)
{
  if (idx + 1 >= _counter.size()) return -1;

  //случайная ошибка номера кадра, ничего не пропущено
  if (_counter.at(idx+1) == counter + 2) {
    return 0;
  }

  const int needValidCnt = 7;

  //проверяем корректность значения номера кадра
  int check = 0;  //количество совпадений ожидаемых и фактических значений данных
  int index = idx;
  int nextcnt = _counter.at(index);

  for (; check < needValidCnt; check++, index++) {
    if (index + 1 >= _counter.size()) {
      break;
    }

    if (nextcnt + 1 == _counter.at(index + 1)) {
      ++check;
    }
    if (nextcnt + 1 > _counter.at(index + 1) || counter > _counter.at(index + 1)) {
      break;
    }
    
    nextcnt = _counter.at(index + 1);
  }

  if (check >= needValidCnt || check == _counter.size() - 1 - idx) {
    return _counter.at(idx) - counter - 1;
  }

  
  return -1;
}

//! Сохранение данных
bool Instrument::save(const global::StreamHeader& header, const QString& path,
                      const QString& baseName, const QDateTime& dtStart,
                      const QDateTime& dtEnd, SatViewPoint* sat)
{
  global::PoHeader pohead;
  pohead.site = header.site;
  pohead.siteCoord = header.siteCoord;
  pohead.start = dtStart; //header.start;
  pohead.stop  = dtEnd; //header.stop;
  pohead.satellite = header.satellite;
  pohead.direction = header.direction;
  pohead.tle = header.tle;
  pohead.instr = type().first;
  pohead.procflag = isCalibrate();
  
  if (!pohead.start.isValid()) {
    pohead.start = QDateTime::currentDateTimeUtc();
  }

  //  QString instrPath = path + pohead.start.toString("/yyyyMMdd_hhmm/") + pbtools::toQString(_conf.dir()) + baseName + "_";
  QString instrPath = path + pohead.start.toString("/yyyyMMdd_hhmm/") + baseName + "_" + QString::number(type().first) + "_";

  
  //saveData
  for (int idx = 0; idx < channelsCount(); idx++)
  {
    QString fileName = saveData(idx, instrPath, pohead);
    var(fileName);
    
    if (nullptr != _notify && !fileName.isEmpty()) {
      _notify->dataNotify(pohead, fileName);
    }
    
    // if (!fileName.isEmpty()) {
    //   meta->append(ChannelMeta(_conf.type(), pohead.channel, isCalibrate(), fileName));
    // }


    saveImage(idx, pohead, instrPath, sat);



    //saveImagePseudo(idx, instrPath);
    //ch[idx]->saveGeoTif(instrPath, pohead, _isCalibrate);
  }



  clear();
  return true;
}

//! Сериализация исходных (некалиброванных) данных для передачи другим модулям в виде изображения
// void Instrument::serializedImgDataRT(QByteArray* ba)
// {
//   QDataStream ds(ba, QIODevice::WriteOnly);
//   ds << _conf.rt_thin();
//   ds << channelsCount();
//   for (int idx = 0; idx < channelsCount(); idx++) {
//     int rows, cols;
//     if (channelProperty(idx).num == -1) {
//       ds << int(-1);
//       ds << QByteArray();
//       ds << int(0);
//       ds << int(0);
//     } else {
//       ds << channelProperty(idx).num;
//       ds << imgRawData(idx, _conf.rt_thin(), &rows, &cols);
//       ds << rows;
//       ds << cols;
//     }
//   }
// }

void Instrument::clear()
{ 
  _data.clear();
  _counter.clear();
  _isCalibrate = false;
  
  clearChannels();
  
  // QVector<Channel*>& ch = channels();
  // for (int idx = 0; idx < ch.size(); idx++) {
  //   if (0 != ch.at(idx)) {
  //     ch[idx]->clear();
  //     delete ch[idx];
  //     ch[idx] = 0;
  //   }
  // }
} 



// void Instrument::resetRT()
// {
//   QVector<Channel*>& ch = channels();

//   for (int idx = 0; idx < ch.size(); idx++) {
//     if (0 != ch.at(idx)) {
//       ch[idx]->resetRT(0);
//     }
//   }  
// }
