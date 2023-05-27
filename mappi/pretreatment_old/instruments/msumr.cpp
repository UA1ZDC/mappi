#include "msumr.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/funcs/mn_funcs.h>

#include <mappi/pretreatment/formats/channel.h>
#include <mappi/pretreatment/formats/frame.h>
#include <mappi/pretreatment/formats/satformat.h>

#include "hrpt_const.h"

//--

#define MSU_START_EARTH_DATA 50  //сдвиг в пакете до начала данных измерений
#define MSU2_FRAME_SIZE 11850
#define MSU2_ID_BEGIN 8 //сдвиг до идентификационных данных

//измерения передаются по 4 пиксела на каждый канал, по 10 бит на пиксел => 5 байт
#define MSU_MR_ONE_CHANNEL_ 5
#define MSU_MR_ONE_CHANNEL_PIX_COUNT 4
#define MSU_WORD_SIZE 10

#define NEED_VALID_READ_CNT 5

#define MSU_AVG_LINE_MSECS 154 //разница времени между строками через одну 152 и 156
#define MSU_MIN_LINE_MSECS 152
#define MSU_MAX_LINE_MSECS 156

//--

const QByteArray kMsuMrSynhro = QByteArray::fromHex("0218A7A392DD9ABF");

namespace {
  mappi::po::Instrument* createMsuMr(const mappi::conf::Instrument& conf, const mappi::conf::InstrFormat& format, mappi::po::Frame* frame)
  {
    return new mappi::po::MsuMr(conf, format, frame);
  }

  static const bool res = mappi::po::singleton::SatFormat::instance()->
    registerInstrHandler(mappi::InstrFrameType(mappi::conf::kMsuMr, mappi::conf::kMhrpt), createMsuMr);
}


using namespace mappi;
using namespace po;


MsuMr::MsuMr(const mappi::conf::Instrument& conf, const conf::InstrFormat& format, Frame* parent):
  InstrumentTempl(conf, format),
  _conf(conf),
  _frame(parent)
{ 
  _ch.resize(_conf.chan_cnt());

}

MsuMr::~MsuMr()
{
  clearChannels();
}


bool MsuMr::parse(const QByteArray& adata, QDateTime* dtS, QDateTime* dtE)
{
  QDateTime dt = *dtS;
  
  int64_t cur = findStartFrame(adata, 5);
  if (-1 == cur) {
    error_log << QObject::tr("Не найдена синхрострока");
    return false;
  }

  QByteArray data = adata.mid(cur, adata.size() - cur);

  fillMissingParts(&data);

  QDateTime dtBeg, dtEnd;
  int startNum = findStartTime(data, MSU2_FRAME_SIZE, MSU_AVG_LINE_MSECS, dt.date(), dtBeg);
  int endNum = findEndTime(data,   MSU2_FRAME_SIZE, MSU_AVG_LINE_MSECS, dt.date(), dtEnd);
  if (-1 == startNum || -1 == endNum) {
    return false;
  }
  
  //debug_log << data.size() / MSU2_FRAME_SIZE;

  var(_frame);
  if (0 != _frame) {
    QDateTime beg = dtBeg.addSecs(-3*3600);
    beg.setTimeSpec(Qt::UTC); //из Московского времени в UTC
    QDateTime end = dtEnd.addSecs(-3*3600);
    end.setTimeSpec(Qt::UTC); //из Московского времени в UTC
    _frame->setDateTime(beg, end);
  }

  //  debug_log << "duration=" << dtBeg.msecsTo(dtEnd);

  fillMissingLines(&data, MSU2_FRAME_SIZE, dtBeg, startNum, endNum);
  //  debug_log << data.size() / MSU2_FRAME_SIZE;

  long length = data.size() / MSU2_FRAME_SIZE;
  uint idx = 0;
  for (int i=0; i<_ch.size(); i++) {
    _ch[i].setup(length, _conf.samples(), idx + 1);
    _ch[i].setLimits(0, 0x3FF);
    idx++;
  }
  
  int numLines = 0;
  long next = 0;
  int pixnum = 0;

  int kEndEarthData = _conf.samples() * _conf.chan_cnt() * MSU_WORD_SIZE / 8 + MSU_START_EARTH_DATA;

  while (next + MSU2_FRAME_SIZE <= data.size() && numLines <= length) {
    int cur_chan = 0;
    int line_num_pix =0;
    
    for (int pix = MSU_START_EARTH_DATA; pix < kEndEarthData; pix += MSU_MR_ONE_CHANNEL_) {
      QByteArray pix4 = data.mid(next + pix, MSU_MR_ONE_CHANNEL_);
      if (cur_chan + 1 > _conf.chan_cnt()) {
	cur_chan = 0;
	line_num_pix += 4;
      }
      
      for (int pi = 0; pi < MSU_MR_ONE_CHANNEL_PIX_COUNT; ++pi) {
	unsigned int ret_val;
	if (!MnCommon::getBitData(pix4.data(), MSU_MR_ONE_CHANNEL_, 
				  pi*MSU_WORD_SIZE, MSU_WORD_SIZE, ret_val)) break;
	
	_ch[cur_chan].set(pixnum + line_num_pix + pi, ret_val);
	// debug_log << cur_chan << pixnum + line_num_pix + pi;
      }
      cur_chan++;
    }
    line_num_pix += 4;
    // var(pixnum);
    // var(line_num_pix);
    pixnum += line_num_pix;
    
    numLines++;
    next += MSU2_FRAME_SIZE;

    //debug_log << numLines << next << data.size();
  }


  *dtS = dtBeg;
  *dtE = dtEnd;
  return true;
}

//! Поиск начала кадра
int64_t MsuMr::findStartFrame(const QByteArray& data, int goodCnt)
{
  int good = 0;
  int pos = data.indexOf(kMsuMrSynhro, 0);
  int oldpos = pos;
  int64_t startpos = -1; 
  //  var(pos);

  while(pos != -1) {
    pos = data.indexOf(kMsuMrSynhro, pos + kMsuMrSynhro.size());
    //debug_log << pos << oldpos << (pos - oldpos);
    if (pos == -1) break;

    if (pos - oldpos == MSU2_FRAME_SIZE) {
      ++good;
      if (startpos == -1) {
	startpos = oldpos;
      }
    } else {
      good = 0;
      startpos = -1;
    }

    oldpos = pos;
    
    if (good >= goodCnt) break;

  }

  return startpos;
}

//! Заполнение неполных строк (потерянных пакетов) до MSU2_FRAME_SIZE. Возвращает количество лишний байт в конце последовательности
int MsuMr::fillMissingParts(QByteArray* data)
{
  long offset = 0;
  long next = offset;

  while (next != -1 && offset < data->size()) {
    next = data->indexOf(kMsuMrSynhro, next + kMsuMrSynhro.size());
    if (-1 == next) break;
    
    if (next - offset < MSU2_FRAME_SIZE) {
      data->insert(next, QByteArray(MSU2_FRAME_SIZE - (next - offset), char(0)));
      //debug_log << "add" << MSU2_FRAME_SIZE - (next - offset); 
      next += MSU2_FRAME_SIZE - (next - offset);
    } else if (next - offset > MSU2_FRAME_SIZE) {
      //lost = (next - offset - MSU2_FRAME_SIZE) / MSU2_FRAME_SIZE;
      if ((next - offset) % MSU2_FRAME_SIZE != 0) {
	int rest = MSU2_FRAME_SIZE - ((next - offset) % MSU2_FRAME_SIZE);
	//var(rest);
	if (rest != 0) { 
	  data->insert(next, QByteArray(rest, char(0)));
	  next += rest;
	  //debug_log << "add" << rest; 
	}
      }
    }

    offset = next;
  }

  int remain = data->size() - offset;
  data->remove(offset, remain);

  //var(remain);

  return remain;
}


/*! 
 * \brief Заполнение пропущенных строк 
 * \param data  исходный буфер
 * \param lineSize размер строки
 */
void MsuMr::fillMissingLines(QByteArray* data, int lineSize, const QDateTime& dtBeg, int startNum, int endNum)
{
  QDateTime nextDt;
  float lineMSecs = MSU_AVG_LINE_MSECS;

  QDateTime curDt = dtBeg;
  long offset = MSU2_FRAME_SIZE*startNum;

  readDt(data->mid(offset + MSU2_ID_BEGIN, 4), curDt.date(), curDt);
  if (curDt < dtBeg) {
    curDt = curDt.addDays(1);
  }

  int numLines = startNum;
  int sum = 0;

  while (offset + lineSize < data->size() && numLines < endNum) {
    int numDt=0;
    int trustOffset = getNextTrustDt(*data, offset, lineSize, 
				     curDt, &nextDt, &numDt);

    if (-1 == trustOffset) {
      break;
    }
    
    // debug_log << "cur" << curDt.toString("hh:mm:ss:zzz") 
    // 	      << "next" << nextDt.toString("hh:mm:ss:zzz")
    // 	      << "msecs" << curDt.time().msecsTo(nextDt.time())
    // 	      << "numDt" << numDt << numLines;

    if (curDt.time().msecsTo(nextDt.time()) > numDt * lineMSecs) {
      int numAdded = MnMath::ftoi_norm(curDt.time().msecsTo(nextDt.time()) / (float)lineMSecs) - numDt - 1;
      //var(numAdded);
      if (numAdded < 360) {
	for (int i=0; i< numAdded; i++) {
	  data->insert(offset, kMsuMrSynhro);
	  data->insert(offset, QByteArray(lineSize - kMsuMrSynhro.size(), char(0)));
	  offset += lineSize;
	}
	sum += numAdded;
      }
    }

    offset += trustOffset;
    curDt = nextDt;
    numLines += 1 + numDt;
  }

  var(sum);
}

//! Определение следующего после смещения offset достоверного значения даты/времени
int MsuMr::getNextTrustDt(const QByteArray& data, long offset, int lineSize, 
			  const QDateTime& curDt, QDateTime* nextDt, int* numDt)
{
  long startOffset = offset;

  offset += lineSize;
  readDt(data.mid(offset + MSU2_ID_BEGIN, 4), curDt.date(), *nextDt);

  int diffMsecs = curDt.msecsTo(*nextDt);
  //корректное значение
  if (diffMsecs == MSU_MIN_LINE_MSECS || diffMsecs == MSU_MAX_LINE_MSECS) {
    return offset - startOffset;
  }

  //  debug_log << "trust" << curDt.toString("hh:mm:ss.zzz") << nextDt->toString("hh:mm:ss.zzz") << diffMsecs;

  //проверяем несколько следующих на случай ошибочной даты в строке
  if (checkNearDt(data, offset, lineSize, curDt, nextDt, numDt)) {
    return offset  - startOffset + lineSize * (*numDt);
  }
  
  //ищем новое корректное значение (возможен пропуск строки)
  if (findNextTrustTime(data, &offset, lineSize, curDt.date(), nextDt, numDt)) {
    return offset - startOffset;
  }

  return -1;
}

bool MsuMr::findNextTrustTime(const QByteArray& data, long* aoffset, int lineSize, const QDate& date, 
			      QDateTime* nextDt, int* numDt)
{
  int check = 0;
  long offset = *aoffset + lineSize;
  
  while (check < NEED_VALID_READ_CNT && (offset + lineSize) <= data.size()) {
    QDateTime dt;
    readDt(data.mid(offset + MSU2_ID_BEGIN, 4), date, dt);
    
    //debug_log << "diff_t=" << dtBeg.msecsTo(dt);
    if (nextDt->isNull() || !dt.isValid() ||
	(nextDt->msecsTo(dt) != MSU_MIN_LINE_MSECS && nextDt->msecsTo(dt) != MSU_MAX_LINE_MSECS)) {
      check = 0;
    } else {
      ++(check);
    }
    ++(*numDt);

    // debug_log << "check=" << check << "dt=" <<  nextDt->toString("hh:mm:ss:zzz") << dt.toString("hh:mm:ss.zzz") 
    // 	      << nextDt->msecsTo(dt)<< *numDt;

    *nextDt = dt;
    
    offset += lineSize;
    
  }
  
  if (check < NEED_VALID_READ_CNT) {
    return false;
  }

  *numDt -= NEED_VALID_READ_CNT;
  *aoffset += *numDt*lineSize;
  readDt(data.mid(*aoffset + MSU2_ID_BEGIN, 4), date, *nextDt);


  //  debug_log << "dtNxt=" << nextDt->toString("hh:mm:ss:zzz") << *numDt;
  return true;
}

bool MsuMr::checkNearDt(const QByteArray& data, long offset, int lineSize, 
			const QDateTime& curDt, QDateTime* nextDt, int* numDt)
{
  int num = 0;
  QDateTime dt;

  for (int idx = 0; idx < 3; idx++) {
    if (offset + lineSize > data.size()) {
      break;
    }

    offset += lineSize;
    ++num;
    readDt(data.mid(offset + MSU2_ID_BEGIN, 4), curDt.date(), dt);
    
    int diffMsecs = curDt.msecsTo(dt);

    //    debug_log << "near=" << idx << "dt=" << dt.toString("hh:mm:ss:zzz") << diffMsecs << num;
    if (diffMsecs >= (num + 1) * MSU_MIN_LINE_MSECS && 
	diffMsecs <= (num + 1) * MSU_MAX_LINE_MSECS) {
      *numDt = num;
      *nextDt = dt;
      return true;
    }
    
  }

  return false;
}

void MsuMr::readDt(const QByteArray& buf, const QDate& d, QDateTime& dtStream)
{
  if (buf.size() < 4) return;

  QTime time( (uchar)buf[0]&0x1f, (uchar)buf[1]&0x3f, (uchar)buf[2]&0x3f, (uchar)buf[3]*4);
  dtStream.setDate(d);
  dtStream.setTime(time);
  //debug_log << "time" << (uchar)buf[0] << (uchar)buf[1] << (uchar)buf[2] << (uchar)buf[3] << time.toString("hh:mm::ss.zzz");
}


//! Дополнительная проверка достоверности значения даты/времени
bool MsuMr::checkDt(const QByteArray& buf, ulong offset, int lineSize, const QDateTime& verDt)
{
  int check = 0;
  offset += lineSize;
  QDateTime dt = verDt;
  QDateTime next;
  
  for (uint i=0; i <= 10; i++) {
    if ((offset + lineSize) > (uint)buf.size()) {
      break;
    }
    readDt(buf.mid(offset, 4), dt.date(), next);
    if ((dt.secsTo(next) > 5 ) || (dt.secsTo(next) < 0 )) {
    } else {
      ++check;
    }

    offset += lineSize;
  }

  if (check < NEED_VALID_READ_CNT*0.7) { //70% 
    return false;
  }

  return true;
}


//! Определение времени начала приёма
/*! 
  \return номер строки с которой время точно определено
*/
int MsuMr::findStartTime(const QByteArray& data, int lineSize, int linemsecs,
			 const QDate& date, QDateTime& dtBeg)
{
  int check = 0;
  long offset = 0;
  int numDt = -1;
  
  while (check < NEED_VALID_READ_CNT && (offset + lineSize) <= data.size()) {
    if (check < NEED_VALID_READ_CNT) {
      QDateTime dt;
      readDt(data.mid(offset + MSU2_ID_BEGIN, 4), date, dt);

      //debug_log << "diff_t=" << dtBeg.msecsTo(dt);
      //если следующая строка сканирования больше, чем через 1 сек, значит с датой что-то не так
      if (dtBeg.isNull() ||  !dt.isValid() ||
	  (dtBeg.msecsTo(dt) != MSU_MIN_LINE_MSECS &&  dtBeg.msecsTo(dt) != MSU_MAX_LINE_MSECS)) {
	check = 0;
      } else {
	++(check);
      }
      ++(numDt);

      //debug_log << "r&c" << check << numDt << dt.toString("dd.MM.yy hh:mm:ss.zzz") << dtBeg.toString("dd.MM.yy hh:mm:ss.zzz");

      dtBeg=dt;
      
      } 

    offset += lineSize;
    //debug_log << "check=" << check << "dt=" << dtBeg.toString("hh:mm:ss:zzz") << offset;
  }
  
  if (check < NEED_VALID_READ_CNT) {
    error_log << QObject::tr("Невозможно определить время начала приёма");
    return -1;
  }
  
  dtBeg = dtBeg.addMSecs(-numDt * linemsecs);
  //debug_log << "dtB=" << dtBeg.toString("dd.MM.yy hh:mm:ss:zzz") << numDt <<  numDt - NEED_VALID_READ_CNT;
  return numDt - NEED_VALID_READ_CNT;
}


//! Определение времени окончания приёма
/*! 
  \return номер строки от конца с которой время точно определено
*/
int MsuMr::findEndTime(const QByteArray& data, int lineSize, int linemsecs,
		       const QDate& date, QDateTime& dtEnd)
{
  int check = 0;
  int offset = data.size() - lineSize;
  int numDt = -1;

  while (check < NEED_VALID_READ_CNT && offset >= 0) {
    QDateTime dt;
    readDt(data.mid(offset + MSU2_ID_BEGIN, 4), date, dt);

    if (dtEnd.isNull() ||  !dt.isValid() ||
	(dt.msecsTo(dtEnd) != MSU_MAX_LINE_MSECS && dt.msecsTo(dtEnd) != MSU_MIN_LINE_MSECS)) {
      check = 0;
    } else {
      ++(check);
    }
    ++(numDt);
    //debug_log << "check=" << check << "dt=" << dt.toString("hh:mm:ss:zzz") << dt.msecsTo(dtEnd);

    dtEnd=dt;

    offset -= lineSize;
  }
  
  if (check < NEED_VALID_READ_CNT) {
    error_log << QObject::tr("Невозможно определить время окончания приёма");
    return -1;
  }

  dtEnd = dtEnd.addMSecs(numDt * linemsecs);

  //debug_log << "dtEnd=" << dtEnd.toString("hh:mm:ss:zzz") << numDt << linemsecs << data.size() / MSU2_FRAME_SIZE - (numDt - NEED_VALID_READ_CNT);
  return  data.size() / MSU2_FRAME_SIZE - (numDt - NEED_VALID_READ_CNT);
}

//! Обработка в реальном времени
// bool MsuMr::parseRT(const QByteArray& adata, const QDateTime& dt, int* remainByte)
// {
//   Q_UNUSED(dt);
//   if (0 == remainByte) return false;
  
//   int64_t cur = adata.indexOf(kMsuMrSynhro, 0);
//   if (-1 == cur) {
//     error_log << QObject::tr("Ошибка поиска синхропоследовательности");
//     *remainByte = adata.size();
//     return false;
//   }

//   QByteArray data = adata.mid(cur, adata.size() - cur);

//   *remainByte = fillMissingParts(&data);

//   long length = data.size() / MSU2_FRAME_SIZE;
//   uint idx = 0;
//   for (int i = 0; i < _ch.size(); i++) {
//     if (_ch[i].property().num == -1) {
//       _ch[i].setup(length, _conf.samples(), idx + 1);
//       _ch[i].setLimits(0, 0x3FF);
//     } else {
//       _ch[i].resetRT(length);
//     }
//     idx++;
//   }
  
//   int numLines = 0;
//   long next = 0;
//   int pixnum = 0;

//   int kEndEarthData = _conf.samples() * _conf.chan_cnt() * MSU_WORD_SIZE / 8 + MSU_START_EARTH_DATA;

//   while (next + MSU2_FRAME_SIZE <= data.size() && numLines <= length) {
//     int cur_chan = 0;
//     int line_num_pix =0;
    
//     for (int pix = MSU_START_EARTH_DATA; pix < kEndEarthData; pix += MSU_MR_ONE_CHANNEL_) {
//       QByteArray pix4 = data.mid(next + pix, MSU_MR_ONE_CHANNEL_);
//       if (cur_chan + 1 > _conf.chan_cnt()) {
// 	cur_chan = 0;
// 	line_num_pix += 4;
//       }
      
//       for (int pi = 0; pi < MSU_MR_ONE_CHANNEL_PIX_COUNT; ++pi) {
// 	unsigned int ret_val;
// 	if (!MnCommon::getBitData(pix4.data(), MSU_MR_ONE_CHANNEL_, 
// 				  pi*MSU_WORD_SIZE, MSU_WORD_SIZE, ret_val)) break;
	
// 	_ch[cur_chan].set(pixnum + line_num_pix + pi, ret_val);
// 	// debug_log << cur_chan << pixnum + line_num_pix + pi;
//       }
//       cur_chan++;
//     }
//     line_num_pix += 4;
//     // var(pixnum);
//     // var(line_num_pix);
//     pixnum += line_num_pix;
    
//     numLines++;
//     next += MSU2_FRAME_SIZE;

//     //debug_log << numLines << next << data.size();
//   }

//   return true;
// }
