#include "virr.h"

#include <cross-commons/debug/tlog.h>

#include <mappi/pretreatment/formats/channel.h>
#include <mappi/pretreatment/formats/frame.h>
#include <mappi/pretreatment/formats/satformat.h>
#include <mappi/pretreatment/formats/stream_utility.h>
#include <mappi/pretreatment/frames/frametypes.h>

#include <qimage.h>

#define DATA_START  349*2
#define DATA_OFFSET 20840*2

const uchar synchro_fy3[7] = {0xA1, 0x16, 0xFD, 0x71, 0x9D, 0x83, 0xC9};
const int synchro_len_fy3 = 7;


// Channel Wavelength Primary Use
// 1       0.58-0.68  Daytime cloud, ice and snow, vegetation
// 2       0.84-0.89  Daytime cloud, vegetation, water vapor
// 3       3.55-3.95  Heat source, night cloud
// 4       10.3-11.3  SST, day/night cloud
// 5       11.5-12.5  SST, day/night cloud
// 6       1.58-1.64  Soil moisture, ice/snow distinguishing
// 7       0.43-0.48  Ocean color
// 8       0.48-0.53  Ocean color
// 9       0.53-0.58  Ocean color
// 10      0.90-0.965 Water vapor


namespace {
  mappi::po::Instrument* createVirr(const mappi::conf::Instrument& conf, const mappi::conf::InstrFormat& format, mappi::po::Frame* frame)
  {
    return new mappi::po::Virr(conf, format, frame);
  }

  static const bool res = mappi::po::singleton::SatFormat::instance()->
    registerInstrHandler(mappi::InstrFrameType(mappi::conf::kVirr, mappi::conf::kCadu), createVirr);
  static const bool res1 = mappi::po::singleton::SatFormat::instance()->
    registerInstrHandler(mappi::InstrFrameType(mappi::conf::kVirr, mappi::conf::kCaduClipped), createVirr);
}

using namespace meteo;
using namespace mappi;
using namespace po;

Virr::Virr(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent):
  InstrumentTempl(conf, format),
  _conf(conf),
  _frame(parent)
{ 
  _ch.resize(_conf.chan_cnt());
}

Virr::~Virr()
{
  clearChannels();
}

bool Virr::parse(const QByteArray& dataAll, QDateTime* dtStart, QDateTime* /*dtEnd*/)
{
  QDateTime dt = *dtStart;
  int shift_bit=0, shift_byte=0;
  
  if (!findSynchro(dataAll, synchro_fy3, synchro_len_fy3, 
		   &shift_bit, &shift_byte, 1, 0xFF, false)) {
    error_log << QObject::tr("Ошибка поиска синхропоследовательности");
    return false;
  }
  
  // debug_log << shift_byte << shift_bit;
  int remain;

  int rawSize = ((dataAll.size() - shift_byte) * 8 - shift_bit) / 10 * 2;
  
  QByteArray rawBuf;
  
  cif2raw(dataAll.right(dataAll.size() - shift_byte), shift_bit, 
	  &rawBuf, &remain, mappi::conf::kChrpt, false);

  uint length = rawSize / _conf.chan_cnt() / 2 / _conf.samples();
  for (int ch = 0; ch < _ch.size(); ch++) {
    _ch[ch].setup(length, _conf.samples(), ch + 1);
    _ch[ch].setLimits(0, 0x3FF);
  }

  readDt(dt, dataAll);

  int idx = DATA_START;
  int frameSize = DATA_OFFSET;
  int chIdx = 0;
  while (idx - DATA_OFFSET + frameSize < rawSize) {
    for (int sample = 0; sample < _conf.samples(); sample++) {
      for (int ch = 0; ch < _conf.chan_cnt(); ch++) {
       	ushort val = (ushort((uchar)rawBuf[idx + (_conf.chan_cnt()*sample + ch)*2]) << 8) + uchar(rawBuf[idx + 2*( _conf.chan_cnt()*sample + ch) + 1]);
	// debug_log << "idx=" << idx << "sample=" << sample << "ch=" << ch << "val=" <<
	//   (idx + ( _conf.chan_cnt()*sample + ch)*2) << (idx + 2*( _conf.chan_cnt()*sample + ch) + 1);
	//	rdata[ch].append((uchar)(val >> 2));
	_ch[ch].set(chIdx, val);
      }
      ++chIdx;
    }
    idx += DATA_OFFSET;

    //idx += frameSize;
  }
  
  return true;
}


bool Virr::readDt(const QDateTime& dt, const QByteArray& data)
{
  QDateTime dtStart = dt;

  //так было в старом китайце, тут другой формат, неизвестный
  //  if (!isDtValid()) { //TODO
  //  bool ok = readStartDt(dt.date(), dtStart, data);
  // } else {
  //   dtStart = dt;
  // }  

  uint maxLines = data.size() / DATA_OFFSET;//_format.size();
  var(maxLines);
  var(_conf.velocity());
  var((int)((maxLines/_conf.velocity())*1000) );
  QDateTime dtEnd = QDateTime(dtStart.date(), dtStart.time().addMSecs( (int)((maxLines/_conf.velocity())*1000) ), Qt::UTC);
  _frame->setDateTime(dtStart, dtEnd);  

  return true;
}

//! Определение даты/времени начала приёма в потоке
/*! 
  \param date дата начала приёма
  \param dtStream возвращаемые дата/время, установленные в соответствии с информацией в потоке
  \return 
*/
// bool Virr::readStartDt(const QDate& date, QDateTime& dtStream, const QByteArray& data)
// {
//   trc;
//   int num = -1;    //количество строк, потребовавшееся для нахождения даты/времени
//   int check = 0; //количество совпадений даты/времени с ожидаемым в строках HRPT файла
//   uint pos = _frame->format().header_start() + 16;
 
//   while (check < NEED_VALID_READ_CNT) {
//     if (pos >= (uint)data.size()) {
//       error_log << QObject::tr("Ошибка данных. Невозможно определить Time Code.");
//       return false;
//     }
    
//     QDateTime dt;
//     readDt(data.mid(pos, 20), date, dt);
//     //если следующая строка сканирования больше, чем через 1 сек, значит с датой что-то не так
//     if ( dtStream.isNull() || (dtStream.secsTo(dt) > 1) || (dtStream.secsTo(dt) < 0)) {
//       check = 0;
//     } else {
//       ++check;
//     }

//     ++num;
//     dtStream = dt;

//     pos += DATA_OFFSET; //_format.size();
//   }

//   var(check);

//   determDt(dtStream, num, _frame->format().velocity());
//   return true;
// }

// void Virr::readDt(const QByteArray& buf, const QDate& d, QDateTime& dtStream)
// {
//   uint idx = 2*2;
//   unsigned long ms = (( buf[3+idx] & 0x7f) << 20) + 
//     (((buf[4+idx]<<8) + buf[5+idx]) << 10) + 
//     (( buf[6+idx]<<8) + buf[7+idx]);
//   QTime time( 0, 0 );
//   dtStream.setDate(d);
//   dtStream.setTime(time.addMSecs( ms ) );
//   dtStream.setTimeSpec(Qt::UTC);
//   //var(ms);
//   //var(dtStream.time().toString());
// }

/*! 
 * \brief Определение даты/времени первой строки сканирования по указанной
 * \param dtB дата/время, передаётся для указанной строки, возвращается для первой
 * \param dtE возвращаемая дата/время для последней строки
 * \param numDt количество строк, потребовавшееся для нахождения даты/времени
 * \param velocity скороксть передачи строк (строк/сек)
 */
// void Virr::determDt(QDateTime& dtB, int numDt, float velocity)
// {
//   //если полночь и смена дня
//   if (dtB.time().msecsTo(QTime(0,0,0)) > (int)(-(numDt/velocity)*1000) ) {
//     dtB.addDays(-1);
//   }
//   dtB.setTime( dtB.time().addMSecs( (int)(-(numDt/velocity)*1000) ));
//   var(dtB);
// }


// bool Virr::parseRT(const QByteArray& dataAll, const QDateTime& dt, int* remainByte)
// {
//   Q_UNUSED(dt);
//   if (0 == remainByte) return false;

//   int cur_shift_bit=0, cur_shift_byte=0;
//   *remainByte = 0;

//   if (!findSynchro(dataAll, synchro_fy3, synchro_len_fy3, 
// 		   &cur_shift_bit, &cur_shift_byte, 1, 0xFF, false)) {
//     *remainByte = dataAll.size();
//     //error_log << QObject::tr("Ошибка поиска синхропоследовательности");
//     return false;
//   }

//   int start_shift_bit  = cur_shift_bit;
//   int start_shift_byte = cur_shift_byte;
  
//   //debug_log << "shift" << cur_shift_byte << cur_shift_bit;

//   int end_shift_byte = cur_shift_byte;

//   QByteArray cur = dataAll.mid(start_shift_byte + synchro_len_fy3, dataAll.size() - start_shift_byte - synchro_len_fy3);

//   while (cur.size() > end_shift_byte + synchro_len_fy3 && 
// 	 findSynchro(cur, synchro_fy3, synchro_len_fy3, 
// 		     &cur_shift_bit, &cur_shift_byte, 1, 0xFF, false)) {
//     end_shift_byte += synchro_len_fy3 + cur_shift_byte;
//     // var(cur_shift_byte);
//     // var(end_shift_byte);
//     //debug_log << dataAll.size() - end_shift_byte - synchro_len_fy3;
//     cur = dataAll.mid(end_shift_byte + synchro_len_fy3, dataAll.size() - end_shift_byte - synchro_len_fy3);
//   }
  
//   if (end_shift_byte == start_shift_byte) {
//     *remainByte = dataAll.size() - start_shift_byte;
//     //error_log << QObject::tr("Ошибка поиска синхропоследовательности 2");
//     return false;
//   }

//   //debug_log << "end_shift" << end_shift_byte << end_shift_bit;
//   *remainByte = dataAll.size() - end_shift_byte;
//   //var(*remainByte);

//   cur = dataAll.mid(start_shift_byte, end_shift_byte - start_shift_byte);

//   int remain;
//   int rawSize = (cur.size() * 8 - start_shift_bit) / 10 * 2;
//   //var(rawSize);

//   QByteArray rawBuf;
  
//   cif2raw(cur, start_shift_bit,
// 	  &rawBuf, &remain, mappi::conf::kChrpt, false);

//   uint length = rawSize / _conf.chan_cnt() / 2 / _conf.samples();
//   // var(length);
//   for (int ch = 0; ch < _ch.size(); ch++) {
//     if (_ch[ch].property().num == -1) {
//       _ch[ch].setup(length, _conf.samples(), ch + 1);
//       _ch[ch].setLimits(0, 0x3FF);
//     } else {
//       _ch[ch].resetRT(length);
//     }
//   }

//   int idx = DATA_START;
//   int frameSize = DATA_OFFSET;
//   int chIdx = 0;
//   while (idx - DATA_OFFSET + frameSize < rawSize) {
//     for (int sample = 0; sample < _conf.samples(); sample++) {
//       for (int ch = 0; ch < _conf.chan_cnt(); ch++) {
//        	ushort val = (ushort((uchar)rawBuf[idx + (_conf.chan_cnt()*sample + ch)*2]) << 8) + uchar(rawBuf[idx + 2*( _conf.chan_cnt()*sample + ch) + 1]);
// 	// debug_log << "idx=" << idx << "sample=" << sample << "ch=" << ch << "val=" <<
// 	//   (idx + ( _conf.chan_cnt()*sample + ch)*2) << (idx + 2*( _conf.chan_cnt()*sample + ch) + 1);
// 	//	rdata[ch].append((uchar)(val >> 2));
// 	_ch[ch].set(chIdx, val);
//       }
//       ++chIdx;
//     }
//     idx += DATA_OFFSET;

//     //idx += frameSize;
//   }

//   return true;
// }
