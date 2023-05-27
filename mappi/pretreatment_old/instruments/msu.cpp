#include "msu.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/debug/tmap.h>

#include <mappi/pretreatment/formats/frame.h>
#include <mappi/pretreatment/formats/satformat.h>
#include <mappi/pretreatment/formats/channel.h>

#include <qimage.h>


//Стандартная таблица квантования согласно ISO/IEC 10918-1
// uint8_t standardQuantizationTable[64] {
// 	16,  11,  10,  16,  24,  40,  51,  61,
// 	12,  12,  14,  19,  26,  58,  60,  55,
// 	14,  13,  16,  24,  40,  57,  69,  56,
// 	14,  17,  22,  29,  51,  87,  80,  62,
// 	18,  22,  37,  56,  68, 109, 103,  77,
// 	24,  35,  55,  64,  81, 104, 113,  92,
// 	49,  64,  78,  87, 103, 121, 120, 101,
// 	72,  92,  95,  98, 112, 100, 103,  99
// };

//Стандартная таблица квантования согласно ISO/IEC 10918-1, заполненная в порядке зиг-заг
const uint8_t kStandardQuantizationTable[64] = {
  16, 11,  12,  14,  12,  10, 16,  14, 13, 14,  18,  17,  16,  19, 24,  40, 
  26, 24,  22,  22,  24,  49, 35,  37, 29, 40,  58,  51,  61,  60, 57,  51, 
  56, 55,  64,  72,  92,  78, 64,  68, 87, 69,  55,  56,  80, 109, 81,  87, 
  95, 98, 103, 104, 103,  62, 77, 113, 121, 112, 100, 120, 92, 101, 103, 99
};

const QByteArray kJpgHeader = QByteArray::fromHex("ffd8ffe000104a46494600010100000100010000");
const QByteArray kJpgDqt  = QByteArray::fromHex("ffdb004300");
const QByteArray kJpgSof0 = QByteArray::fromHex("ffc0000b080008007001011100");
const QByteArray kJpgDht  = QByteArray::fromHex("ffc4001f0000010501010101010100000000000000000102030405060708090A0B");
const QByteArray kJpgDht1 = QByteArray::fromHex("ffc400b5100002010303020403050504040000017d01020300041105122131410613516107227114328191a1082342b1c11552d1f02433627282090a161718191a25262728292a3435363738393a434445464748494a535455565758595a636465666768696a737475767778797a838485868788898a92939495969798999aa2a3a4a5a6a7a8a9aab2b3b4b5b6b7b8b9bac2c3c4c5c6c7c8c9cad2d3d4d5d6d7d8d9dae1e2e3e4e5e6e7e8e9eaf1f2f3f4f5f6f7f8f9fa");
const QByteArray kJpgSos  = QByteArray::fromHex("ffda0008010100003f00");
const QByteArray kJpgTail  = QByteArray::fromHex("ffd9");


static const TMap<int, int> apid2ch = TMap<int, int>() 
  << QPair<int, int>(64, 1)
  << QPair<int, int>(65, 2)
  << QPair<int, int>(66, 3)
  << QPair<int, int>(67, 4)
  << QPair<int, int>(68, 5)
  << QPair<int, int>(69, 6)
  << QPair<int, int>(70, -1);


namespace {
  mappi::po::Instrument* createMsu(const mappi::conf::Instrument& conf, const mappi::conf::InstrFormat& format, mappi::po::Frame* frame)
  {
    return new mappi::po::Msu(conf, format, frame);
  }

  static const bool res = mappi::po::singleton::SatFormat::instance()->
     registerInstrHandler(mappi::InstrFrameType(mappi::conf::kMsuMrLo, mappi::conf::kCadu), createMsu);
}

using namespace mappi;
using namespace po;


Msu::Msu(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent):
  InstrumentTempl(conf, format),
  _conf(conf),
  _frame(parent),
  _palette(256)
{ 
  _ch.resize(_conf.chan_cnt());

  for (uint i=0; i< 256; i++) {
    _palette[i] = QRgb((i<<16)+ (i<<8) + i);
  }

}

Msu::~Msu()
{
  clearChannels();
}

bool Msu::parse(const QList<QByteArray>& dataAll, QDateTime* dtS, QDateTime* dtE)
{
  QMap<int, QMap<int, QByteArray> > chan; //канал, номер сегмента, данные
  QMap<int, QByteArray> rdata; //канал

  QList<int> apids;
  apids << 64 << 65 << 66 << 70; //TODO

  int cntch = 0; //номер msu строки (от 0 до 13)
  //  int oldchannel = -1;
 
 //  bool fl = false;

  int idx = 0;
  int cnt = 0; //номер пакета
  QDateTime dt;
  dt = *dtS;

  bool ok = findStartPackNum(&cnt, &idx, &dt, dataAll);
  if (!ok) {
    debug_log << QObject::tr("Не найден первый достоверный пакет");
    return false; 
  }

  // if (idx != 0) {
  //   debug_log << "start" << idx;
  // }

  QDateTime startDt(dt.date(), dt.time(), Qt::TimeZone);
  QDateTime endDt(dt.date(), dt.time(), Qt::TimeZone);

  var(startDt.toString());

  while (idx < dataAll.size()) {
    const QByteArray& data = dataAll.at(idx);
    uint16_t apid = (((uint8_t)data[0]&0x3) << 8) + (uint8_t)data[1];
    int channel = apid2ch.value(apid);

    cntch = (uint8_t)data[14] / 14;

    //    uint16_t length = ((uint8_t)data[4] << 8) + (uint8_t)data[5] + 1;
    //uint8_t seq = ((uint8_t)data[2] >> 6);
    cnt = (((uint8_t)data[2] << 8) + (uint8_t)data[3]) & 0x7ff;
    dt.setDate(QDate::currentDate());
    dt.setTime(QTime(0,0));
    dt = dt.addMSecs(((uint8_t)data[8] << 24) + ((uint8_t)data[9] << 16) + 
		     ((uint8_t)data[10] << 8) +	 (uint8_t)data[11]);
    

    // debug_log << ((uint8_t)data[0] >> 5) << (((uint8_t)data[0] >> 4)&0x1) << (((uint8_t)data[0] >> 3)&0x1)
    // 	      << length
    // 	      << apid
    // 	      << seq 
    // 	      << cnt
    //   // << length
    // 	      << dt.toString("hh:mm:ss.zzz") 
    // 	      << cntch;
    //<< (uint8_t)data[15] << (uint8_t)data[16] << ((((uint8_t)data[17]) << 8) + (uint8_t)data[18]) << (uint8_t)data[19];
    
    if (channel != -1) {
      //oldchannel = channel;
      //   ++idx;
      //   continue;
      // }
      
      QImage img = QImage::fromData(createMcu(data));
      chan[channel].insert(cntch, QByteArray((char*)(img.bits()), img.byteCount()));
    }

    int curApid = apids.lastIndexOf(apid);
    int missed = getMissedCount(&idx, dataAll);
    //debug_log << "missed" << missed << idx;

    if (missed == 0) {
      if (cntch == 13) {
	endDt.setDate(dt.date());
	endDt.setTime(dt.time());
	setStripe(chan, channel, &rdata);
	chan[channel].clear();
	cntch = -1;
      }
    } else {

    while (--missed >= 0) {
      // debug_log << "channel" << channel << cntch << missed;
      if (channel == -1) {
	++curApid;
	if (curApid == apids.length()) {
	  curApid = 0;
	}
	channel = apid2ch.value(apids.at(curApid));
	cntch = -1;
      }

      if (cntch == 13) {

	setStripe(chan, channel, &rdata);
	chan[channel].clear();
	cntch = -1;

	++curApid;
	if (curApid == apids.length()) {
	  curApid = 0;
	}

	channel = apid2ch.value(apids.at(curApid));
      }
      //debug_log << "miss" << cntch << missed << (++cnt) << channel << curApid;
      if (cnt == 2047) {
	cnt = -1;
      }
      ++cntch;
    }
    }
  } 
  
  // var(rdata.keys());
  // var(chan.keys());
  int chIdx = 0;
  QMapIterator<int, QByteArray> it(rdata);
  while (it.hasNext()) {
    it.next();
    int h = it.value().size() / _conf.samples() ;
    if (chIdx >= _ch.size()) break;
    _ch[chIdx].setup(h, _conf.samples(), it.key());
    _ch[chIdx].setLimits(0, 0xFF);
    for (int pix = 0; pix < it.value().size(); pix++) {
      _ch[chIdx].set(pix, (uchar)it.value().data()[pix]);
    }
    ++chIdx;
    // debug_log << it.key() << it.value().size();
    // QImage imqt((const uchar*)(it.value().data()), w, h, w, QImage::Format_Indexed8);
    // imqt.setColorCount(_palette.size());
    // imqt.setColorTable(_palette);
    // imqt.save(QString("/tmp/e") + QString::number(it.key()) + ".bmp");
  }
  
  if (0 != _frame) {
    startDt = startDt.addSecs(-3*3600);
    startDt.setTimeSpec(Qt::UTC);
    endDt   = endDt.addSecs(-3*3600);
    endDt.setTimeSpec(Qt::UTC);
    _frame->setDateTime(startDt, endDt);
  }
  *dtS = startDt;
  *dtE = endDt;

  return true;
}

//! Распределение по каналам распакованных кусочков (МЦУ, 8x14) полосы данных (8 строк, 14 кусочков по 112 байт = 1568)
void Msu::setStripe(const QMap<int, QMap<int, QByteArray> >& chan, int channel, QMap<int, QByteArray>* rdata)
{
  //debug_log << "rdata prev" << channel << rdata[channel].size();
  for (int line = 0; line < 8; line++) {
    for (int pp = 0; pp < 14; pp++) {
      if (! chan.value(channel).contains(pp)) {
	(*rdata)[channel].append(QByteArray(112, 0));
      } else {
	QByteArray cba = chan.value(channel).value(pp).mid(line*112, 112);
	cba.resize(112);
	(*rdata)[channel].append(cba);
      }
      //debug_log << "rdata part" << channel << line << pp  << rdata[channel].size();
    }
  }
}

//! Сборка кусочка изображения (МЦУ), запакованного в формате jpg, создание из него картинки в формате jpg
QByteArray Msu::createMcu(const QByteArray& data)
{
  int qFactor = (uint8_t)data[19];
  float F = 0;
  if (qFactor >= 20 && qFactor <= 100) {
    F = (qFactor >= 20 && qFactor <= 50) ? (5000 / qFactor) & 0xFF : (200 - 2 * qFactor) & 0xFF;
  }
  
  if (qFactor < 20 || qFactor > 100) {
    debug_log << "err qfactor" << qFactor; 
  }
  
  // if (apid == 65) {
  //   debug_log << "stripe" << seq << qFactor << F << ((qFactor >= 20 && qFactor <= 50) ? (5000.0 / qFactor)/100 : (200 - 2. * qFactor)/100);
  // }
  uint8_t quantizationTable[64];
  for (int i=0; i<64; i++) {
    if (((int)round(F /100.0 * kStandardQuantizationTable[i])) > 255) {
      debug_log << "max limit";
    }
    quantizationTable[i] = ((int)round(F /100.0 * kStandardQuantizationTable[i])) & 0xff;
    if (quantizationTable[i] == 0) {
      quantizationTable[i] = 1;
    }
  }
  
  QByteArray jpghead = kJpgHeader + kJpgDqt + QByteArray::fromRawData((char*)(quantizationTable), 64) + 
    kJpgSof0 + kJpgDht + kJpgDht1 + kJpgSos;
  
  uint16_t length = ((uint8_t)data[4] << 8) + (uint8_t)data[5] + 1;
  QByteArray body = data.mid(20, length - 14).replace(QByteArray(1, 0xff),  QByteArray::fromHex("ff00"));
  if (body.endsWith(QByteArray::fromHex("ff00"))) {
    body = body.remove(body.size()-1, 1);
  }
  
  return jpghead + body  + kJpgTail;
}


//! Нахождение первого корректного пакета
bool Msu::findStartPackNum(int* cnt, int* idx, QDateTime* adt, const QList<QByteArray>& dataAll)
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

//! Определение количества пропущенных строк
int Msu::getMissedCount(int* idx, const QList<QByteArray>& dataAll)
{
  const int kChecklimit = 3;
  int cnt = (((uint8_t)dataAll.at(*idx)[2] << 8) + (uint8_t)dataAll.at(*idx)[3]) & 0x7ff;
  QDateTime dt;
  dt.setDate(QDate::currentDate());
  dt = dt.addMSecs(((uint8_t)dataAll.at(*idx)[8] << 24) + ((uint8_t)dataAll.at(*idx)[9] << 16) + 
		   ((uint8_t)dataAll.at(*idx)[10] << 8) +  (uint8_t)dataAll.at(*idx)[11]);

  //  debug_log << "start miss" << *idx;

  ++(*idx);
  int start = cnt;
  QDateTime startdt = dt;
  int check=0;   //количество совпадений ожидаемых и фактических значений данных
  bool isNext = true;

  int ncnt = cnt;
  while (check < kChecklimit) {
    if (*idx >= dataAll.size()) {
      break;
    }

    const QByteArray& current = dataAll.at(*idx);
    if (current.size() < 20) {
      check = 0;
      ++(*idx);
      isNext = false;
      continue;
    }
    
    uint16_t length = ((uint8_t)current[4] << 8) + (uint8_t)current[5] + 1;
    if (current.size() < length + 6 || length < 14) { //в массиве д.б. данные + заголовок (6 байт)
      //debug_log << "lengthh" << current.size() << length + 6;
      check = 0;
      ++(*idx);
      isNext = false;
      continue;
    }
 
    ncnt = (((uint8_t)current[2] << 8) + (uint8_t)current[3]) & 0x7ff;
    QDateTime ndt;
    ndt.setDate(QDate::currentDate());
    ndt = ndt.addMSecs(((uint8_t)current[8] << 24) + ((uint8_t)current[9] << 16) + 
		       ((uint8_t)current[10] << 8) +  (uint8_t)current[11]);
    int diffdt =  dt.msecsTo(ndt);
    
    // debug_log << (*idx) << (cnt) << ncnt << dt.isValid() << ndt.isValid() << diffdt 
    // 	      << dt.toString("hh:mm:ss.zzz") << ndt.toString("hh:mm:ss.zzz") << "check=" << check;

    if ((cnt + 1 == ncnt || (cnt > ncnt && cnt >= 1500 && ncnt <= 500)) 
	&& dt.isValid() && ndt.isValid() && ndt >= dt && 
	(diffdt == 0 || diffdt == 1228 || diffdt == 1232) &&
	ndt >= startdt && (start + 1 <= ncnt || (start > ncnt && start >= 1500 && ncnt <= 500))) {
      ++check;
      if (isNext) {
	check = kChecklimit - 1;
	break;
      } 
    }
    cnt = ncnt;
    dt = ndt;

    ++(*idx);

    isNext = false;
  }

  if (isNext) {
    //debug_log << "end miss" << *idx << 0 << check;
    return 0;
  }

  if (check < kChecklimit) {
    //debug_log << "end miss" << *idx << 0 << check;
    return 0;
  }

  *idx = *idx - check - 1;  
  ncnt = (((uint8_t)dataAll.at(*idx)[2] << 8) + (uint8_t)dataAll.at(*idx)[3]) & 0x7ff;
  

  if (0 != ncnt - 1 - start) {
    if (ncnt - 1 - start < 0) {
      //debug_log << "end miss" << *idx << ncnt - 1 + (2047 - start + 1);
      return ncnt - 1 + (2047 - start + 1);
    } else {
      //debug_log << "end miss" << *idx << ncnt - 1 - start;
    }
  }

  return ncnt - 1 - start;
}


// bool Msu::parseRT(const QList<QByteArray>& dataAll, const QDateTime& adt, int* remain)
// {
//   var(dataAll.size());
//   if (0 == remain) return false;

//   if (dataAll.size() < 13) {
//     *remain = dataAll.size();
//     return false;
//   }


//   QMap<int, QMap<int, QByteArray> > chan; //канал, номер сегмента, данные
//   QMap<int, QByteArray> rdata; //канал

//   QList<int> apids;
//   apids << 64 << 65 << 66 << 70;

//   int cntch = 0; //номер msu строки (от 0 до 13)

//   int idx = 0;
//   //  int cnt = 0; //номер пакета
//   QDateTime dt;
//   dt = adt;
  
//   // bool ok = findStartPackNum(&cnt, &idx, &dt, dataAll);
//   // if (!ok) {
//   //   debug_log << QObject::tr("Не найден первый достоверный пакет");
//   //   return false; 
//   // }
//   //  cnt = (((uint8_t)dataAll.at(idx)[2] << 8) + (uint8_t)dataAll.at(idx)[3]) & 0x7ff;

//   // QDateTime startDt(dt.date(), dt.time(), Qt::LocalTime);
//   // QDateTime endDt(dt.date(), dt.time(), Qt::LocalTime);

//   while (idx < dataAll.size()) {
//     const QByteArray& data = dataAll.at(idx);
//     if (data.size() < 20 || data.size() < (((uint8_t)data[4] << 8) + (uint8_t)data[5] + 1) + 6) {
//       ++idx;
//       continue;
//     }

//     uint16_t apid = (((uint8_t)data[0]&0x3) << 8) + (uint8_t)data[1];
//     int channel = apid2ch.value(apid);

//     cntch = (uint8_t)data[14] / 14;

//     //    uint16_t length = ((uint8_t)data[4] << 8) + (uint8_t)data[5] + 1;
//     //uint8_t seq = ((uint8_t)data[2] >> 6);
//     //cnt = (((uint8_t)data[2] << 8) + (uint8_t)data[3]) & 0x7ff;
//     // dt.setDate(QDate::currentDate());
//     // dt.setTime(QTime(0,0));
//     // dt = dt.addMSecs(((uint8_t)data[8] << 24) + ((uint8_t)data[9] << 16) + 
//     // 		     ((uint8_t)data[10] << 8) +	 (uint8_t)data[11]);
    
//     // debug_log << ((uint8_t)data[0] >> 5) << (((uint8_t)data[0] >> 4)&0x1) << (((uint8_t)data[0] >> 3)&0x1)
//     // 	      << length
//     // 	      << apid
//     // 	      << seq 
//     // 	      << cnt
//     //   // << length
//     // 	      << dt.toString("hh:mm:ss.zzz") 
//     // 	      << cntch;
//     //<< (uint8_t)data[15] << (uint8_t)data[16] << ((((uint8_t)data[17]) << 8) + (uint8_t)data[18]) << (uint8_t)data[19];
    
//     if (channel != -1) {
//       //oldchannel = channel;
//       //   ++idx;
//       //   continue;
//       // }
      
//       QImage img = QImage::fromData(createMcu(data));
//       chan[channel].insert(cntch, QByteArray((char*)(img.bits()), img.byteCount()));
//     }

//     int curApid = apids.lastIndexOf(apid);
//     int missed = 0;//getMissedCount(&idx, dataAll);
//     //debug_log << "missed" << missed << idx;

//     if (missed == 0) {
//       if (cntch == 13) {
// 	// endDt.setDate(dt.date());
// 	// endDt.setTime(dt.time());
// 	setStripe(chan, channel, &rdata);
// 	chan[channel].clear();
// 	cntch = -1;
	
// 	if (idx + 13 < dataAll.size()) {
// 	  *remain = dataAll.size() - idx - 1;
// 	  break;
// 	}

//       }
//     } else {

//     while (--missed >= 0) {
//       // debug_log << "channel" << channel << cntch << missed;
//       if (channel == -1) {
// 	++curApid;
// 	if (curApid == apids.length()) {
// 	  curApid = 0;
// 	}
// 	channel = apid2ch.value(apids.at(curApid));
// 	cntch = -1;
//       }

//       if (cntch == 13) {

// 	setStripe(chan, channel, &rdata);
// 	chan[channel].clear();
// 	cntch = -1;

// 	++curApid;
// 	if (curApid == apids.length()) {
// 	  curApid = 0;
// 	}

// 	channel = apid2ch.value(apids.at(curApid));
//       }
//       //debug_log << "miss" << cntch << missed << (++cnt) << channel << curApid;
//       // if (cnt == 2047) {
//       // 	cnt = -1;
//       // }
//       ++cntch;
//     }
//     }
//     ++idx;
//   } 
  
//   var(rdata.keys());
//   var(chan.keys());
//   int chIdx = 0;
//   QMapIterator<int, QByteArray> it(rdata);
//   while (it.hasNext()) {
//     it.next();
//     int h = it.value().size() / _conf.samples() ;
//     if (chIdx >= _ch.size()) break;
//     if (0 == _ch[chIdx]) {
//       _ch[chIdx] = new Channel(h, _conf.samples(), it.key());
//       _ch[chIdx]->setLimits(0, 0xFF);
//     } else {
//       _ch[chIdx]->resetRT(h);
//     }
//     for (int pix = 0; pix < it.value().size(); pix++) {
//       _ch[chIdx]->set(pix, (uchar)it.value().data()[pix]);
//     }
//     ++chIdx;
//     // debug_log << it.key() << it.value().size();
//     // QImage imqt((const uchar*)(it.value().data()), w, h, w, QImage::Format_Indexed8);
//     // imqt.setColorCount(_palette.size());
//     // imqt.setColorTable(_palette);
//     // imqt.save(QString("/tmp/e") + QString::number(it.key()) + ".bmp");
//   }
  
//   // if (0 != _frame) {
//   //   _frame->setDateTime(startDt.toUTC(), endDt.toUTC());
//   // }

//   return true;
// }


// bool Msu::parseRT(const QList<QByteArray>& dataAll, const QDateTime& adt, int* remain)
// {
//   if (0 == remain) {
//     return false;
//   }

//   if (dataAll.size() < 13) {
//     *remain = dataAll.size();
//     return false;
//   }

//   *remain = dataAll.size();

//   QMap<int, QMap<int, QByteArray> > chan; //канал, номер сегмента, данные
//   QMap<int, QByteArray> rdata; //канал

//   QList<int> apids;
//   apids << 64 << 65 << 66 << 70;

//   int cntch = -1; //номер msu строки (от 0 до 13)

//   int idx = 0;
//   int cnt = 0; //номер пакета
//   QDateTime dt;
//   dt = adt;

//   bool ok = findStartPackNum(&cnt, &idx, &dt, dataAll);
//   if (!ok) {
//     debug_log << QObject::tr("Не найден первый достоверный пакет");
//     return false; 
//   }

//   //var(idx);
//   int channel = -1;
//   int prevValid = idx;

//   int lastIdx = -1;
//   uint8_t seq = 0;

//   while (idx < dataAll.size()) {
//     const QByteArray& data = dataAll.at(idx);

//     //    debug_log << idx << dataAll.size() << *remain << cntch;

//     if (idx != 0 && idx != prevValid) {
//       if (!isValidPacket(data, dataAll.at(prevValid))) {
//     	++idx;
// 	//	--(*remain);
//     	continue;
//       }
//     }

//     int nextch = apid2ch.value((((uint8_t)data[0]&0x3) << 8) + (uint8_t)data[1]); 
//     if (cntch == 13 || cntch > (uint8_t)data[14] / 14 || 
// 	(channel != -1 && channel != nextch && 
// 	  seq + 1 != ((uint8_t)data[2] >> 6))) {
//       //debug_log << "setStripe" << channel;
//       setStripe(chan, channel, &rdata);
//       chan[channel].clear();
//       lastIdx = prevValid;
//       if (idx + 14 >= dataAll.size()) {
//       	break;
//       }
//       cntch = -1;
//      }

//     prevValid = idx;

//     uint16_t apid = (((uint8_t)data[0]&0x3) << 8) + (uint8_t)data[1];
//     channel = apid2ch.value(apid);

//     cntch = (uint8_t)data[14] / 14;

//     //    uint16_t length = ((uint8_t)data[4] << 8) + (uint8_t)data[5] + 1;
//     seq = ((uint8_t)data[2] >> 6);
//     cnt = (((uint8_t)data[2] << 8) + (uint8_t)data[3]) & 0x7ff;
//     dt.setDate(QDate::currentDate());
//     dt.setTime(QTime(0,0));
//     dt = dt.addMSecs(((uint8_t)data[8] << 24) + ((uint8_t)data[9] << 16) + 
//     		     ((uint8_t)data[10] << 8) +	 (uint8_t)data[11]);
    
//     // debug_log << ((uint8_t)data[0] >> 5) << (((uint8_t)data[0] >> 4)&0x1) << (((uint8_t)data[0] >> 3)&0x1)
//     // 	      << length
//     // 	      << apid
//     // 	      << seq 
//     // 	      << cnt
//     //   //<< length
//     // 	      << dt.toString("hh:mm:ss.zzz") 
//     // 	      << cntch << (uint8_t)data[14];
//     // //<< (uint8_t)data[15] << (uint8_t)data[16] << ((((uint8_t)data[17]) << 8) + (uint8_t)data[18]) << (uint8_t)data[19];

//     // if (channel == 0) {
//     //   var(channel);
//     //   var(data[14]);
//     // }

//     if (channel != -1) {
//       QImage img = QImage::fromData(createMcu(data));
//       chan[channel].insert(cntch, QByteArray((char*)(img.bits()), img.byteCount()));
//     }

//     //    --(*remain);
//     ++idx;
//   } 
  
//   //  debug_log << "remain" << *remain << (dataAll.size() - lastIdx) << lastIdx;
//   *remain = dataAll.size() - lastIdx - 1;

//   // var(rdata.keys());
//   // var(chan.keys());

//   QMapIterator<int, QByteArray> it(rdata);
//   while (it.hasNext()) {
//     it.next();
//     int h = it.value().size() / _conf.samples() ;
//     int chIdx = 0;
//     while ((int)_ch[chIdx].number() != it.key()) {
//       ++chIdx;
//     }
//     //debug_log << "img" << h << _conf.samples() << chIdx << it.value().size();
//     if (chIdx >= _ch.size()) continue;
//     if (_ch[chIdx].property().num == -1) {
//       _ch[chIdx].setup(h, _conf.samples(), it.key());
//       _ch[chIdx].setLimits(0, 0xFF);
//     } else {
//       _ch[chIdx].resetRT(h);
//     }
//     for (int pix = 0; pix < it.value().size(); pix++) {
//       _ch[chIdx].set(pix, (uchar)it.value().data()[pix]);
//     }
//   }
  

//   return true;
// }

//! Простая проверка пакета на корректность
bool Msu::isValidPacket(const QByteArray& current, const QByteArray& prev)
{
  if (current.size() < 20) {
    return false;
  }
  
  uint16_t length = ((uint8_t)current[4] << 8) + (uint8_t)current[5] + 1;
  if (current.size() < length + 6 || length < 14) { //в массиве д.б. данные + заголовок (6 байт)
    return false;
  }

  int ncnt = (((uint8_t)current[2] << 8) + (uint8_t)current[3]) & 0x7ff;
  QDateTime ndt;
  ndt.setDate(QDate::currentDate());
  ndt = ndt.addMSecs(((uint8_t)current[8] << 24) + ((uint8_t)current[9] << 16) + 
		     ((uint8_t)current[10] << 8) +  (uint8_t)current[11]);

  int cnt = (((uint8_t)prev[2] << 8) + (uint8_t)prev[3]) & 0x7ff;
  QDateTime dt;
  dt.setDate(QDate::currentDate());
  dt = dt.addMSecs(((uint8_t)prev[8] << 24) + ((uint8_t)prev[9] << 16) + 
  		   ((uint8_t)prev[10] << 8) +  (uint8_t)prev[11]);

  int diffdt =  dt.secsTo(ndt);

  if ((cnt < ncnt || (cnt > ncnt && cnt >= 1500 && ncnt <= 500)) && 
       dt.isValid() && ndt.isValid() && ndt >= dt && diffdt < 600) {
    return true;
  }

  //  var(ncnt);
  return false;  
}
