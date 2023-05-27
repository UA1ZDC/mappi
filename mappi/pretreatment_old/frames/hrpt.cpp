#include "hrpt.h"


#include <mappi/pretreatment/formats/instrument.h>
#include <mappi/pretreatment/formats/satformat.h>

#include <cross-commons/debug/tlog.h>

#include <qbytearray.h>
#include <qdatetime.h>

using namespace mappi;
using namespace po;

const QByteArray kHrptSynhro   = QByteArray::fromHex("0284016f035c019d020f0095");
const QByteArray kHrptSynhroLe = QByteArray::fromHex("84026f015c039d010f029500");

#define NEED_VALID_READ_CNT 5  //!< Количество совпадений данных, по которому можно сделать вывод о их верности
#define HRPT_MIN_LINE_MSECS 166
#define HRPT_MAX_LINE_MSECS 167

namespace {
mappi::po::Frame* createHrpt(const mappi::conf::Frame& conf, Frame* parent)
{
  return new mappi::po::Hrpt(conf, parent);
}

static const bool res = mappi::po::singleton::SatFormat::instance()->registerFrameHandler(mappi::conf::kHrpt, createHrpt);
}


Hrpt::Hrpt(const conf::Frame& conf, Frame* parent): 
  Frame(conf, true, parent),
  _format(conf)
{
  //  debug_log << QObject::tr("NOTE: Дата берется из заголовка"); //см. строка 397
}

Hrpt::~Hrpt()
{
  clear();
}

void Hrpt::clear()
{
  _numMinor = -1;
  _ch3x = -1;
  _lineChan3x = -1;
  _startNum = -1;
  _endNum = -1;
  Frame::clear();
}


bool Hrpt::isBigEndian(const QByteArray& data) 
{
  int pos = data.indexOf(kHrptSynhro, 0);
  if (pos != -1) {
    return true;
  }

  pos = data.indexOf(kHrptSynhroLe, 0);
  if (pos != -1) {
    return false;
  }

  return true;
}

//! парсинг кадра
/*! 
  \param dt дата/время начала приёма
  \param data поток данных (массив принятых байт)
  \return
*/
bool Hrpt::parseFormatCustom(conf::DataLevel level, const QDateTime& dt, QByteArray* adata)
{
  if (level == conf::kFrameLevel) {
    if (isBigEndian(*adata)) { //TODO пока так. наша железка отдаёт в обратном порядке данные
      debug_log << "BE";
    } else {
      debug_log << "LE";
      for (int idx = 0; idx + 1 < adata->size(); idx += 2) {
        char ch = (*adata)[idx];
        (*adata)[idx] = (*adata)[idx + 1];
        (*adata)[idx + 1] = ch;
      }
    }
  }

  if (!readId(*adata)) {
    return false;
  }
  if (false == dt.isValid() ) {
    readDt(dt, *adata);
  }

  readChan3x(*adata);
  
  return true;
}

bool Hrpt::parseFormatCustomRT(conf::DataLevel level, const QDateTime& dt, QByteArray* adata)
{
  Q_UNUSED(dt)

  return true; //TODO то надо, то не надо) см. функцию выше

  if (level == conf::kFrameLevel) {
    for (int idx = 0; idx + 1 < adata->size(); idx += 2) {
      QByteRef ch = (*adata)[idx];
      (*adata)[idx] = (*adata)[idx + 1];
      (*adata)[idx + 1] = ch;
    }
  }
  
  return true;
}

//! Определение даты/времени начала и окончания приёма в потоке
/*! 
  \param dt дата/время начала приёма
  \return
*/
bool Hrpt::readDt(const QDateTime& dt, const QByteArray& data)
{
  QDateTime dtStart;
  if (!readStartDt(dt.date(), data, &dtStart, &_startNum) && isDtValid()) {
    dtStart = dt;
    _startNum = 0;
  }

  QDateTime dtEnd;
  //--
  // uint maxLines = data.size() / _format.size();
  // dtEnd = dtStart.addMSecs( (int)((maxLines/_format.velocity())*1000) );
  // var(dtEnd);
  // var(maxLines);

  
  if (!readEndDt(dtStart.date(), data, &dtEnd, &_endNum)) {
    uint maxLines = data.size() / _format.size();
    _endNum = maxLines - 1;
    if (dtStart.isValid()) {
      dtEnd = dtStart.addMSecs( (int)((maxLines/_format.velocity())*1000) );
    } else {
      dtEnd = QDateTime::currentDateTimeUtc();
      //debug_log << "addMsecs" << -(maxLines/_format.velocity());
      dtStart = dtEnd.addMSecs( (int)(-(maxLines/_format.velocity())*1000) );
    }
  }

  debug_log << "dur" << dtStart.msecsTo(dtEnd) << (float)dtStart.msecsTo(dtEnd) / 1000. * 6 + 1 << data.size() / _format.size();

  setDateTime(dtStart, dtEnd);

  return true;
}

//! Поиск идентификационных данных в потоке
/*! 
  \return false - в случае ошибки, дальнейшая обработка не возможна
*/
bool Hrpt::readId(const QByteArray& data)
{
  HrptId id, idOld;
  int num = -1;  //количество строк, потребовавшееся для нахождения id
  int check=0;   //количество совпадений ожидаемых и фактических значений данных id
  uint pos = _format.header_start();

  while (check < NEED_VALID_READ_CNT) {

    if (pos >= (uint)data.size()) {
      error_log << QObject::tr("Ошибка данных. Невозможно определить ID.");
      return false;
    }
    
    idOld.M[0] = (uchar(data[pos])<<8) + (uchar)(data[pos + 1]);
    idOld.M[1] = (uchar(data[pos + 2])<<8) + uchar(data[pos + 3]);

    // printf("%x\n",(int)data[pos]);
    // printf("%x\n",(int)data[pos+1]);
    // printf("%x\n",(int)data[pos+2]);
    // printf("%x\n",(int)data[pos+3]);

    // printvar(idOld.M[0]);
    // printvar(idOld.M[1]);
    // printvar(idOld.S.frame);
    // printvar(idOld.S.addr);

    if ( idOld.S.frame == 0) {
      check = 0;
    } else if (((id.S.frame + 1) == idOld.S.frame ||
                (idOld.S.frame == 1 && id.S.frame == 3) )
               && (idOld.S.addr == id.S.addr)) {
      ++check;
    } else {
      check = 0;
    }

    ++num;
    id.S =  idOld.S;
    
    pos += _format.size();
  }
  
  if (!setSatellite(id.S.addr) ) {
    error_log << QObject::tr("Ошибка определения спутника");
    return false;
  }

  //вычисляем номер minor frame для первой строки
  _numMinor = id.S.frame;
  _numMinor -= num % 3;
  if (_numMinor <= 0 ) {
    _numMinor += 3;
  }

  return true;
}

/*! 
 * \brief Определение 3A или 3B канал и номера строки, если он изменился в HRPT
 * \param file  Файл с данными HRPT
 * \return  0 в случае успеха, иначе код ошибки
 */
bool Hrpt::readChan3x(const QByteArray& data)
{
  HrptId id;
  int chan3x = -1;   // 0=AVHRR Ch3B, 1=AVHRR Ch3A
  int check = 0;     // количество совпадений при поиске номера канала
  int line = 0;      // номер строки сканирования, когда канал изменился
  int checkChange=0; // количество совпадений, после изменения канала
  uint pos = _format.header_start();

  while (pos < (uint)data.size()) {
    
    id.M[0] = (uchar(data[pos]) << 8) + (uchar)(data[pos + 1]);
    id.M[1] = (uchar(data[pos + 2]) << 8) + uchar(data[pos + 3]);
    
    ++line;
    
    if (check < NEED_VALID_READ_CNT) { //определяем значение 3 канала
      if (chan3x == id.S.ch3x ) {
        ++check;
      } else {
        chan3x = id.S.ch3x;
        check = 0;
      }
    } else { //проверяем не изменялся ли канал в процессе сканирования
      if (checkChange < NEED_VALID_READ_CNT) {
        if (chan3x != id.S.ch3x) {
          ++checkChange;
        } else {
          checkChange = 0;
        }
      } else {
        break; //за один HRPT дважды канал поменяться не должен
      }
    }
    
    pos += _format.size();
  }

  if (check < NEED_VALID_READ_CNT) {
    error_log << QObject::tr("Ошибка данных. Невозможно определить использование 3A/3B канала.");
    return false;
  }

  _ch3x = chan3x;
  if (checkChange >=  NEED_VALID_READ_CNT) {
    _lineChan3x = line - NEED_VALID_READ_CNT + 1;
  } else {
    _lineChan3x = -1;
  }
  
  return true;
}


//! Определение даты/времени начала приёма в потоке
/*! 
  \param date дата начала приёма
  \param dtStream возвращаемые дата/время, установленные в соответствии с информацией в потоке
  \param anum номер кадра с достоверным временем
*/
bool Hrpt::readStartDt(const QDate& date, const QByteArray& data, QDateTime* dtStream, int* anum)
{
  int num = -1;    //количество строк, потребовавшееся для нахождения даты/времени
  int check = 0; //количество совпадений даты/времени с ожидаемым в строках HRPT файла
  uint pos = _format.header_start() + sizeof(HrptId);

  while (check < NEED_VALID_READ_CNT) {
    if (pos >= (uint)data.size()) {
      error_log << QObject::tr("Ошибка данных. Невозможно определить Time Code.");
      return false;
    }
    
    QDateTime dt;
    bool ok = readDt(data.mid(pos, 8), date, dt);
    if (!ok || dtStream->isNull() ||
        dtStream->msecsTo(dt) > HRPT_MAX_LINE_MSECS ||
        dtStream->msecsTo(dt) < HRPT_MIN_LINE_MSECS) {
      check = 0;
    } else {
      ++check;
    }

    ++num;

    //debug_log << num << "check=" << check << "dt=" << dt.toString("dd.MM.yyyy hh:mm:ss:zzz") << dtStream->msecsTo(dt);

    *dtStream = dt;

    pos += _format.size();
  }

  *dtStream = dtStream->addMSecs( (int)(-num*1000/_format.velocity()) );
  *anum = num - NEED_VALID_READ_CNT;

  //debug_log << "dtStart=" << dtStream->toString("hh:mm:ss:zzz") << num;

  return true;
}

//! Определение даты/времени окончания приёма в потоке
/*! 
  \param date дата начала приёма
  \param dtEnd возвращаемые дата/время, установленные в соответствии с информацией в потоке
  \param anum номер кадра с достоверным временем
*/
bool Hrpt::readEndDt(const QDate& date, const QByteArray& data, QDateTime* dtEnd, int* anum)
{
  int check = 0;
  int numDt = -1;
  uint maxLines = data.size() / _format.size();
  int offset = (maxLines - 1) * _format.size() + _format.header_start() + sizeof(HrptId);

  while (check < NEED_VALID_READ_CNT) {
    if (offset < 0) {
      error_log << QObject::tr("Ошибка данных. Невозможно определить Time Code.");
      return false;
    }

    QDateTime dt;
    bool ok = readDt(data.mid(offset, 8), date, dt);

    if (!ok || dtEnd->isNull() ||
        dt.msecsTo(*dtEnd) > HRPT_MAX_LINE_MSECS ||
        dt.msecsTo(*dtEnd) < HRPT_MIN_LINE_MSECS) {
      check = 0;
    } else {
      ++check;
    }

    ++numDt;
    
    //debug_log << "end" << numDt << check << dt.toString("hh:mm:ss:zzz") << dt.msecsTo(*dtEnd);

    *dtEnd = dt;

    offset -= _format.size();
  }
  
  if (check < NEED_VALID_READ_CNT) return false;

  *dtEnd = dtEnd->addMSecs( (int)((numDt*1000/_format.velocity())) );

  //debug_log << "dtEnd=" << dtEnd->toString("hh:mm:ss:zzz") << numDt;

  *anum = data.size() / _format.size() - (numDt - NEED_VALID_READ_CNT);

  return true;
}

//! Определение даты/времени начала приёма в потоке
/*! 
  \param buf данные
  \param d дата начала приёма
  \param dtStream возвращаемые дата/время, установленные в соответствии с информацией в потоке
  \return
*/
bool Hrpt::readDt(const QByteArray& buf, const QDate& d, QDateTime& dtStream)
{
  unsigned long ms = (uchar(buf[3] & 0x7f) << 20) +
      (((uchar(buf[4])<<8) + uchar(buf[5])) << 10) +
      ((uchar(buf[6])<<8) + uchar(buf[7]));
  unsigned days = (uchar(buf[0])<<7) + ((uchar(buf[1])>>1) & 0x7f);

  //debug_log << "readDt" << d.year() << days << ms << db(uchar(buf[0])<<7) << db(uchar(buf[1])>>1);
  if (ms > 86400000) {
    return false;
  }

  QDate date( d.year(), 1, 1 );

  QTime time( 0, 0 );

  //TODO коммент, чтоб взялась текущая дата, для старых данных
  // Q_UNUSED(days);
  // dtStream.setDate(d);
  //------
  if (days > 366) {
    dtStream.setDate(d);
  } else {
    dtStream.setDate(date.addDays( days - 1 ));
  }
  //------
  
  dtStream.setTime(time.addMSecs( ms ));
  dtStream.setTimeSpec(Qt::UTC);
  return true;
}


//! Установка спутника по его идентификатору в потоке
/*! 
  \param addr идентификатор спутника в потокеs
  \return
*/
bool Hrpt::setSatellite(uint addr) //TODO 
{
  Q_UNUSED(addr);
  //  setSatName("NOAA 18");
  return true;
}

//! парсинг подкадров
/*! 
  \param frame прибор, отвечающий за обработку заданного типа подкадра
  \param frameFormat описание подкадра
  \return номер последнего считанного байта
*/
int Hrpt::parseFrame(Instrument* instr, const conf::InstrFormat& instrFormat, const QByteArray& data)
{
  trc;
  int pos = 0;
  //int idx = 0;
  //pos = data.indexOf(kHrptSynhro, 0); не надо, это делается на уровень выше

  // debug_log << pos << _startNum << _endNum;

  // debug_log << _format.size() << data.size();

  QDateTime curDt = dtStart();
  if (_startNum != -1) {
    readDt(data.mid(pos + _format.size()*_startNum + _format.header_start() + sizeof(HrptId), 8), curDt.date(), curDt);
  }
  QDateTime nextDt = curDt;

  int stripe = 0;
  int sum = 0;

  while ((pos + _format.size()) <=  data.size()) {
    int missed = 0; // число потерянных строк
    int cntToTrust = 0; // число строк до достоверно определённой даты/времени
    if (stripe > _startNum  && stripe < _endNum) {
      if (missingLinesCount(data, pos, curDt, &missed, &cntToTrust, &nextDt)) {
        //debug_log << "missed=" << missed << cntToTrust << curDt.msecsTo(nextDt) << sum;
        curDt = nextDt;
      }
    }
    sum += missed;
    //заполняем пустые
    for (int midx = 0; midx < missed; midx++) {
      for (int idx = 0; idx < instrFormat.sect_size(); idx++) {
        //debug_log << "add" << instrFormat.sect(idx).size();
        instr->addFrame(QByteArray(instrFormat.sect(idx).size(), char(0xff))); //TODO возможно нужны флаги качества для строк (и/или значений)
      }
    }
    //заполняем до строки с достоверными датой/временем включительно
    for (int midx = 0; midx < cntToTrust + 1; midx++) {
      for (int idx = 0; idx < instrFormat.sect_size(); idx++) {
        instr->addFrame(data.mid(pos + instrFormat.sect(idx).start(), instrFormat.sect(idx).size()));
        //debug_log << stripe << instrFormat.sect(idx).start() <<  instrFormat.sect(idx).size();
      }
      pos += _format.size();
      stripe += 1;
    }
  }

  // var(sum);
  // var(stripe);

  return pos;
}

//! Поиск пропущенных строк
/*! 
  \param data данные
  \param pos  текущее положение
  \param curDt текущая dt
  \param missed число пропущенных строк
  \param numDt  число строк до достоверной строки
  \param nextDt следующая достоверная дата/время
*/
bool Hrpt::missingLinesCount(const QByteArray& data, int pos, const QDateTime& curDt, 
                             int* missed, int* numDt, QDateTime* nextDt)
{
  *missed = 0;
  *numDt = -1;

  int check = 0; //количество совпадений даты/времени с ожидаемым в строках HRPT файла
  pos += _format.header_start() + sizeof(HrptId);

  while (check < NEED_VALID_READ_CNT) {
    if (pos >= data.size()) {
      return false;
    }
    
    QDateTime dt;
    bool ok = readDt(data.mid(pos, 8), curDt.date(), dt);
    //если следующая строка сканирования больше, чем через 166.7 мс, значит с датой что-то не так
    if (!ok || nextDt->isNull() ||
        nextDt->msecsTo(dt) > HRPT_MAX_LINE_MSECS ||
        nextDt->msecsTo(dt) < HRPT_MIN_LINE_MSECS) {
      //debug_log << "check" << nextDt->msecsTo(dt) << (1000. / _format.velocity() + 0.5);
      check = 0;
    } else {
      //debug_log << "check" << nextDt->msecsTo(dt) << (1000. / _format.velocity() + 0.5) << curDt << *nextDt;
      ++check;
    }

    ++(*numDt);
    *nextDt = dt;

    pos += _format.size();
  }

  //debug_log << "check" << curDt.msecsTo(*nextDt) << *numDt << curDt << *nextDt;
  *missed = curDt.msecsTo(*nextDt)*_format.velocity() / 1000 - *numDt - 0.5;

  return true;
}
