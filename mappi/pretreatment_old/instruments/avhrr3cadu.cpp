#include "avhrr3cadu.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <mappi/pretreatment/formats/frame.h>
#include <mappi/pretreatment/formats/satformat.h>
#include <mappi/pretreatment/frames/hrpt.h>
#include <mappi/pretreatment/frames/m_ccsds.h>
#include <mappi/pretreatment/formats/channel.h>
#include <mappi/pretreatment/formats/stream_utility.h>

#include <QFile>

#define CCSDS_PRIMARY_HEADER_SIZE 6
#define CCSDS_SECONDARY_HEADER_SIZE 8 //время
#define ANCILLARY_DATA_SIZE 6
#define PACKET_LENGTH 12960


//HRPT / LRPT DIRECT BROADCAST SERVICES SPECIFICATION  (hrpt_LRPT_spec.pdf)


//~/build_dir/bin/mappi.pretreatment -s "METOP-A" -y 2017 -m 03 -d 01 ~/Download/Download/datameteo/sat/2021/METOP/MetOp-A/2017-03-26/cadu.pds  > tmp 2>&1

namespace {
  mappi::po::Instrument* createAvhrr3Cadu(const mappi::conf::Instrument& conf, const mappi::conf::InstrFormat& format, mappi::po::Frame* frame)
  {
    return new mappi::po::Avhrr3Cadu(conf, format, frame);
  }

  static const bool res = mappi::po::singleton::SatFormat::instance()->
    registerInstrHandler(mappi::InstrFrameType(mappi::conf::kAvhrr3, mappi::conf::kCadu), createAvhrr3Cadu);
  static const bool res1 = mappi::po::singleton::SatFormat::instance()->
    registerInstrHandler(mappi::InstrFrameType(mappi::conf::kAvhrr3, mappi::conf::kCaduClipped), createAvhrr3Cadu);
}

using namespace mappi;
using namespace po;


Avhrr3Cadu::Avhrr3Cadu(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* frame):
  Avhrr3(conf, format, frame),
  _conf(conf)
{
  _ch.resize(_conf.chan_cnt());
  
  _frame = frame;
}

Avhrr3Cadu::~Avhrr3Cadu()
{
}

//!< парсинг
bool Avhrr3Cadu::parse(const QList<QByteArray>& dataAll, QDateTime* dtS, QDateTime* dtE)
{
  trc;
  QByteArray cif;
  
  QList<int> apids;
  apids << 103 << 104; //TODO

  int idx = -1;
  //  int cnt = 0; //номер пакета
  QDateTime dt;
  dt = *dtS;

  bool ok = true;//findStartPackNum(&cnt, &idx, &dt, dataAll);
  if (!ok) {
    debug_log << QObject::tr("Не найден первый достоверный пакет");
    return false; 
  }

  QDateTime startDt; //(dt.date(), dt.time(), Qt::TimeZone);
  QDateTime endDt; //(dt.date(), dt.time(), Qt::TimeZone);
  //  var(startDt.toString());

  //NOTE: 6 в начале пакета и в конце - выравнивание до адекватное картинки, что за данные не понятно

  int endOffset = 6;
  int cifWidth = PACKET_LENGTH - CCSDS_SECONDARY_HEADER_SIZE - ANCILLARY_DATA_SIZE - endOffset; //12940

  int prevCnt = -1;
  
  while (++idx < dataAll.size()) {
      
    const QByteArray& data = dataAll.at(idx);

       
    CCSDSHeader header;
    bool ok = header.parse(data, QDate(2000,1,1));
    if (!ok) {
      continue;
    }
    //debug_log << header;
    
    uint16_t apid = header.apid; //(((uint8_t)data[0]&0x3) << 8) + (uint8_t)data[1];

    uint16_t length = header.length + 1; //((uint8_t)data[4] << 8) + (uint8_t)data[5] + 1;
    //uint8_t seq = ((uint8_t)data[2] >> 6);
    //cnt = (((uint8_t)data[2] << 8) + (uint8_t)data[3]) & 0x7ff;

    // dt.setDate(QDate(2000,1,1));
    // dt.setTime(QTime(0,0));
    // dt = dt.addDays(((uint8_t)data[6] << 8) + ((uint8_t)data[7]));
    // dt = dt.addMSecs(((uint8_t)data[8] << 24) + ((uint8_t)data[9] << 16) + 
    // 		     ((uint8_t)data[10] << 8) +	 (uint8_t)data[11]);
    dt = header.dts;
    //+2 байта микросекунд

    //var(dt);
    

    
    // debug_log << ((uint8_t)data[0] >> 5) << (((uint8_t)data[0] >> 4)&0x1) << (((uint8_t)data[0] >> 3)&0x1)
    // 	      << length
    // 	      << apid
    // 	      << seq 
    // 	      << cnt	    
    // 	      << dt.toString("dd.MM.yy hh:mm:ss.zzz") 
    // 	      << (uint8_t)data[15] << (uint8_t)data[16] << ((((uint8_t)data[17]) << 8) + (uint8_t)data[18]) << (uint8_t)data[19];
        
    //debug_log << idx << dataAll.at(idx).size() << apid << cnt << length;

    if (length + 6 > data.size() && prevCnt == -1) {
      debug_log << "err length" << (length + 6) << data.size();
      continue;
    }
    
    if (apid != 103 && apid != 104) {
      continue;
    }


    //    var(dt);

    if (dt.isValid() && !startDt.isValid()) {
      startDt = dt;
      startDt.setTimeSpec(Qt::UTC);
    }
    if (dt.isValid()) {
      endDt = dt;
      endDt.setTimeSpec(Qt::UTC);
    }
  

    
    int missed = missedFrames(prevCnt, idx, dataAll);
    if (missed != 0) {
      // var(missed);
      // var(dataAll.size() - idx);
    }
    if (missed >= dataAll.size() - idx || (missed > 10 && 2*missed >= dataAll.size() - idx)) {
      break;
    }
    
    cif.append(QByteArray( missed*cifWidth, char(0)));
    

    QByteArray cifba = dataAll.at(idx).mid(CCSDS_PRIMARY_HEADER_SIZE + CCSDS_SECONDARY_HEADER_SIZE + ANCILLARY_DATA_SIZE, cifWidth);
    cif.append(cifba);
    //cif.append(dataAll.at(idx).mid(CCSDS_PRIMARY_HEADER_SIZE + CCSDS_SECONDARY_HEADER_SIZE + ANCILLARY_DATA_SIZE, cifWidth));
    
    if (cifba.size() < cifWidth) {
      cif.append(cifWidth - cifba.size(), char(0));
    }
    
        
    prevCnt = header.cnt;
  }

  var(_frame);
  if (nullptr != _frame) {
    //TODO чтоб текущая дата, а не из кадра
    // startDt.setDate(dtS->date());
    // endDt.setDate(dtS->date());
    _frame->setDateTime(startDt, endDt);
  }

  var(cif.size());
  
  
  int shift_bit = 0;
  int remain;
  int wordSize = 10;
  // QVector<ushort> raw;
  // ok = meteo::cif2raw(cif, shift_bit, &raw, &remain, false);
  // var(remain);
  QByteArray raw;
  ok = meteo::cif2raw(cif, shift_bit, &raw, &remain, wordSize, false);
  var(remain);

  cif.clear();  
 
  // var(ok);
  var(raw.size());

  
  uint64_t rawWidth = (cifWidth*8)/10 * 2;
  int imgWidth = (rawWidth)/2;

  int length = raw.size() / imgWidth / 2;
  var(length);
  ok = readChannelData(raw, length);
   
  // {
  //   var(raw.size());
  //   QFile file("/tmp/user_raw.dat");
  //   file.open(QIODevice::WriteOnly);
  //   QDataStream ds(&file);
  //   ds << raw;
  //   file.close();
  // }

  // {
  //   uint cifWidth = 12960 - 20; 
  //   uint64_t rawWidth = (cifWidth*8)/10 * 2;
  //   int imgWidth = (rawWidth)/2;
  //   var(imgWidth);
  //   debug_log << "set channel";
  //   mappi::po::Channel ch(raw.size() / imgWidth / 2, imgWidth, 0);
  //   ch.setLimits(0, 0x3ff);
  //   for (int idx = 0, num = 0; idx + 1 < raw.size(); idx += 2, num++) {
  //     ch.set(num, ((uchar)raw.at(idx) << 8) + (uchar)raw.at(idx+1));
  //   }
  //   ch.saveImage("/tmp/all.bmp");
  // }
  

  *dtS = startDt;
  *dtE = endDt;

  
  
  return ok;
}


//! Количество пропущенных кадров. -1 если номер кадра некорректен
int Avhrr3Cadu::missedFrames(int counter, int idx, const QList<QByteArray>& data)
{
  if (idx + 1 >= data.size()) return -1;

  //ничего не пропущено
  if (counter == -1 || getCCSDSCnt(data.at(idx)) == counter + 1) {
    return 0;
  }

  //случайная ошибка номера кадра, ничего не пропущено
  if (getCCSDSCnt(data.at(idx+1)) == counter + 2) {
    return 0;
  }

  
  const int needValidCnt = 7;

  //проверяем корректность значения номера кадра
  int check = 0;  //количество совпадений ожидаемых и фактических значений данных
  int index = idx;
  int nextcnt = getCCSDSCnt(data.at(index));

  //debug_log << idx << counter << data.size() << nextcnt; 
  
  for (; check < needValidCnt; check++, index++) {
    
    if (index + 1 >= data.size()) {
      break;
    }

    int benchmark = getCCSDSCnt(data.at(index + 1));
    if (nextcnt + 1 == benchmark) {
      ++check;
    } 
    if (nextcnt + 1 > benchmark || counter > benchmark) {
      break;
    }
    
    nextcnt = benchmark;
  }
 
  if (check >= needValidCnt || check == data.size() - 1 - idx) {
    return  getCCSDSCnt(data.at(idx)) - counter - 1;
  }

  return -1;
}


//! Извлечение данных, разбор по каналам
bool Avhrr3Cadu::readChannelData(const QByteArray& data, int length)
{
  uint idx = 0;
  for (int i = 0; i < _ch.size(); i++) {
    _ch[i].setup(length, _conf.samples(), idx + 1);
    _ch[i].setLimits(0, 0x3FF);
    idx++;
  }

  uint pos = 55*2; //8*2;// ;
  idx = 0;

  //offset 63 len = 10240 end_offset = 57

  debug_log << length << _conf.samples() << _ch.size();
  for (int row = 0; row < length; row++) {
    for (int col = 0; col < _conf.samples(); col++) {
      for (int ci = 0; ci < _ch.size(); ci++) {

	if (pos + 1 >= (uint)data.size()) {
	  debug_log << "err pos=" << pos << data.size() << row << col << ci;
	  return false;
	}
	
	if (ci == 2 || ci == 3) {
	  if (_lineChan3x == -1 || row < _lineChan3x) { //канал не меняется или ещё первый
	    if ((_chan3x == 1 && ci == 3) || //пропускаем 3 канал
		(_chan3x == 0 && ci == 2)) { //пропускаем 2 канал
	      continue; 
	    }
	  } else { //канал поменялся
	    if ((_chan3x == 1 && ci == 2) || //пропускаем 2 канал
		(_chan3x == 0 && ci == 3)) { //пропускаем 3 канал
	      continue; 
	    }
	  }
	}
	
	//	_ch[ci]->set(idx, data.at(pos));
	_ch[ci].set(idx, ((uchar)data.at(pos) << 8) + (uchar)data.at(pos+1));
	pos+=2;
      }
      idx++;
    }
    // var(pos);
    pos += 55*2 + 55*2 + 2*2; //57 + 63; //смещение на заголовок
  }

  return true;
}

//!< парсинг, realtime
bool Avhrr3Cadu::parseRT(const QList<QByteArray>& dataAll, const QDateTime& adt, int* remainByte)
{
  if (nullptr == remainByte) return false;

  *remainByte = dataAll.size();
  
  QByteArray cif;
  
  QList<int> apids;
  apids << 103 << 104; //TODO
  
  int idx = 0;
  //  int cnt = 0; //номер пакета
  QDateTime dt;
  dt = adt;

  bool ok = true;//findStartPackNum(&cnt, &idx, &dt, dataAll);
  if (!ok) {
    debug_log << QObject::tr("Не найден первый достоверный пакет");
    return false; 
  }

    int endOffset = 6;
  int cifWidth = PACKET_LENGTH - CCSDS_SECONDARY_HEADER_SIZE - ANCILLARY_DATA_SIZE - endOffset; //12940

  while (idx < dataAll.size()) {
    const QByteArray& data = dataAll.at(idx);

    if (data.size() < 11) {
      idx++;
      continue;
    }
    
    uint16_t apid = (((uint8_t)data[0]&0x3) << 8) + (uint8_t)data[1];

    uint16_t length = ((uint8_t)data[4] << 8) + (uint8_t)data[5] + 1;
    //uint8_t seq = ((uint8_t)data[2] >> 6);
    //cnt = (((uint8_t)data[2] << 8) + (uint8_t)data[3]) & 0x7ff;

    dt.setDate(QDate(2001,1,1));
    dt.setTime(QTime(0,0));
    dt = dt.addDays(((uint8_t)data[6] << 8) + ((uint8_t)data[7]));
    dt = dt.addMSecs(((uint8_t)data[8] << 24) + ((uint8_t)data[9] << 16) + 
		     ((uint8_t)data[10] << 8) +	 (uint8_t)data[11]);
    //+2 байта микросекунд
    
    // debug_log << ((uint8_t)data[0] >> 5) << (((uint8_t)data[0] >> 4)&0x1) << (((uint8_t)data[0] >> 3)&0x1)
    // 	      << length
    // 	      << apid
    // 	      << seq 
    // 	      << cnt	    
    // 	      << dt.toString("dd.MM.yy hh:mm:ss.zzz") 
    // 	      << (uint8_t)data[15] << (uint8_t)data[16] << ((((uint8_t)data[17]) << 8) + (uint8_t)data[18]) << (uint8_t)data[19];
        
    //debug_log << idx << dataAll.at(idx).size() << apid << cnt << length;

    if (apid != 103 && apid != 104) {
      ++idx;
      continue;
    }

    if (length < 10) {
      idx++;
      continue;
    }
    
    int missed = 0; //getMissedCount(&idx, dataAll);

    if (missed != 0) {
      //TODO
    }


    cif.append(dataAll.at(idx).mid(CCSDS_PRIMARY_HEADER_SIZE + CCSDS_SECONDARY_HEADER_SIZE + ANCILLARY_DATA_SIZE, cifWidth));
        
    ++idx;
  }


  //  *remainByte = dataAll.size() - lastIdx - 1; //TODO
  *remainByte = 1; //надо бы посчитать сколько осталось от последней не полной строки, пока так TODO
  //var(cif.size());
  
  
  int shift_bit = 0;
  int remain;
  int wordSize = 10;
  // QVector<ushort> raw;
  // ok = meteo::cif2raw(cif, shift_bit, &raw, &remain, false);
  // var(remain);
  QByteArray raw;
  ok = meteo::cif2raw(cif, shift_bit, &raw, &remain, wordSize, false);
  //var(remain);

  cif.clear();  
 
  // var(ok);
  //var(raw.size());

  
  uint64_t rawWidth = (cifWidth*8)/10 * 2;
  int imgWidth = (rawWidth)/2;

  int length = raw.size() / imgWidth / 2;
  //var(length);
  ok = readChannelData(raw, length);
   
  // {
  //   var(raw.size());
  //   QFile file("/tmp/user_raw.dat");
  //   file.open(QIODevice::WriteOnly);
  //   QDataStream ds(&file);
  //   ds << raw;
  //   file.close();
  // }

  // {
  //   uint cifWidth = 12960 - 20; 
  //   uint64_t rawWidth = (cifWidth*8)/10 * 2;
  //   int imgWidth = (rawWidth)/2;
  //   var(imgWidth);
  //   debug_log << "set channel";
  //   mappi::po::Channel ch(raw.size() / imgWidth / 2, imgWidth, 0);
  //   ch.setLimits(0, 0x3ff);
  //   for (int idx = 0, num = 0; idx + 1 < raw.size(); idx += 2, num++) {
  //     ch.set(num, ((uchar)raw.at(idx) << 8) + (uchar)raw.at(idx+1));
  //   }
  //   ch.saveImage("/tmp/all.bmp");
  // }
  
  
  return ok;
}


//! Нахождение первого корректного пакета
bool Avhrr3Cadu::findStartPackNum(int* cnt, int* idx, QDateTime* adt, const QList<QByteArray>& dataAll)
{  
  if (dataAll.size() == 0) return false;

  *idx = 0;
  const int kChecklimit = 3;

  while (dataAll.at(*idx).size() < 20) {
    if (*idx >= dataAll.size()) {
      return false;
    }
    ++(*idx);
  }

  *cnt = (((uint8_t)dataAll.at(*idx)[2] << 8) + (uint8_t)dataAll.at(*idx)[3]) & 0x7ff;
  QDateTime dt;
  if (adt->date().isValid()) {
    dt.setDate(adt->date());
  } else {
    dt.setDate(QDateTime::currentDateTimeUtc().date());
  }
  dt.setTime(QTime(0, 0));
  dt = dt.addMSecs(((uint8_t)dataAll.at(*idx)[8] << 24) + ((uint8_t)dataAll.at(*idx)[9] << 16) + 
		   ((uint8_t)dataAll.at(*idx)[10] << 8) +  (uint8_t)dataAll.at(*idx)[11]);

  ++(*idx);
  int start = *cnt;
  QDateTime startdt = dt;
  int ncnt = *cnt;
  int check=0;   //количество совпадений ожидаемых и фактических значений данных

  while (check < kChecklimit) {
    if (*idx >= dataAll.size()) {
      break;
    }
    
    const QByteArray& current = dataAll.at(*idx);
    if (current.size() < 20) {
      check = 0;
      ++(*idx);
      continue;
    }
    
    uint16_t length = ((uint8_t)current[4] << 8) + (uint8_t)current[5] + 1;
    if (current.size() < length + 6 || length < 14) { //в массиве д.б. данные + заголовок (6 байт)
      //debug_log << "lengthh" << current.size() << length + 6;
      check = 0;
      ++(*idx);
      continue;
    }

    ncnt = (((uint8_t)current[2] << 8) + (uint8_t)current[3]) & 0x7ff;
    QDateTime ndt;
    ndt.setDate(QDate::currentDate());
    ndt.setTime(QTime(0, 0));
    ndt = ndt.addMSecs(((uint8_t)current[8] << 24) + ((uint8_t)current[9] << 16) + 
		       ((uint8_t)current[10] << 8) +  (uint8_t)current[11]);
    int diffdt =  dt.msecsTo(ndt);
    
    // debug_log << (*idx) << (cnt) << ncnt << dt.isValid() << ndt.isValid() << diffdt 
    // 	      << dt.toString("hh:mm:ss.zzz") << ndt.toString("hh:mm:ss.zzz") << "check=" << check;

    if ((*cnt + 1 == ncnt || (*cnt == 2047 && ncnt == 0)) &&
	dt.isValid() && ndt.isValid() && ndt >= dt && 
	(diffdt == 0 || diffdt == 1228 || diffdt == 1232) &&
	ndt >= startdt && (start + 1 <= ncnt || (start > ncnt && start >= 1500 && ncnt <= 500))) {
      ++check;
    }
    *cnt = ncnt;
    dt = ndt;

    ++(*idx);
  }

  if (check < kChecklimit || *idx >= dataAll.size()) {
    //debug_log << "end miss" << *idx << 0 << check;
    return false;
  }

  *idx = *idx - check - 1;
  *cnt = (((uint8_t)dataAll.at(*idx)[2] << 8) + (uint8_t)dataAll.at(*idx)[3]) & 0x7ff;
  if (!adt->date().isValid()) {
    adt->setDate(QDateTime::currentDateTimeUtc().date());
  }
  adt->setTime(QTime(0, 0));
  *adt = adt->addMSecs(((uint8_t)dataAll.at(*idx)[8] << 24) + ((uint8_t)dataAll.at(*idx)[9] << 16) + 
		       ((uint8_t)dataAll.at(*idx)[10] << 8) +  (uint8_t)dataAll.at(*idx)[11]);

  return true;
}

void Avhrr3Cadu::serializedImgDataRT(QByteArray* ba)
{
  QDataStream ds(ba, QIODevice::WriteOnly);
  ds << _conf.rt_thin();
  ds << _ch.size() - 1; //3a и 3b в одном
  for (int idx = 0; idx < _ch.size(); idx++) {
    if ((_chan3x == 1 && idx == 3) || 
  	(_chan3x == 0 && idx == 2)) {
      continue;
    }
    int rows, cols;
    ds << _ch.at(idx).number();
    ds << _ch.at(idx).imgRawData(_conf.rt_thin(), &rows, &cols);
    ds << rows;
    ds << cols;
  }
}
