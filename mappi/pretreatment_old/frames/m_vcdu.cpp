#include "m_vcdu.h"
#include "m_ccsds.h"


#include <mappi/pretreatment/formats/instrument.h>
#include <mappi/pretreatment/formats/satformat.h>

#include <cross-commons/debug/tlog.h>

#include <qbytearray.h>
#include <qdatetime.h>
#include <qfile.h>

using namespace mappi;
using namespace po;

#define NEED_VALID_READ_CNT 6  //!< Количество совпадений данных, по которому можно сделать вывод о их верности

#define CCSDS_HEADER_LENGTH 6

namespace {
mappi::po::Frame* createVcdu(const mappi::conf::Frame& conf, Frame* parent)
{
  return new mappi::po::Vcdu(conf, parent);
}

static const bool res = mappi::po::singleton::SatFormat::instance()->registerFrameHandler(mappi::conf::kVcdu, createVcdu);
}

bool VcduHeader::parse(const QByteArray& vcdu, uint pos)
{
  if ((uint)vcdu.size() < pos + 6) {
    return false;
  }
  
  version = (uint8_t)vcdu[pos + 0] >> 6;
  scid = (((uint8_t)vcdu[pos + 0]&0x3f) << 2) + ((uint8_t)vcdu[pos + 1] >> 6);
  vcid = (uint8_t)vcdu[pos + 1] & 0x3F;
  counter = ((uint8_t)vcdu[pos + 2] << 16) + ((uint8_t)vcdu[pos + 3] << 8) + ((uint8_t)vcdu[pos + 4]);
  replayFlag  = ((uint8_t)vcdu[pos + 5]) >> 7;

  uint8_t offset = 6;
  if (hasMpdu) {
    if (hasInsertZone) {
      offset += 2;
    }
    if ((uint)vcdu.size() < pos + offset + 2) {
      return false;
    }
    
    packOffset = ((((uint8_t)vcdu[pos + offset] << 8) + (uint8_t)vcdu[pos + offset+1]) & 0x7ff); //сдвиг до начала заголовка;
  }

  return true;
}

TLog& ::mappi::po::operator<<(TLog& log, const VcduHeader& header)
{
  log << header.version
      << header.scid
      << header.vcid
      << header.counter
      << header.replayFlag;

  if (header.hasMpdu) {
    log << header.packOffset;
  }

  return log;
}

Vcdu::Vcdu(const conf::Frame& conf, Frame* parent): 
  Frame(conf, false, parent)
{
}

Vcdu::~Vcdu()
{
}


//! парсинг подкадров
/*! 
  \param instr прибор, отвечающий за обработку заданного типа подкадра
  \param instrFormat описание подкадра
  \return номер последнего считанного байта
*/
int Vcdu::parseFrame(Instrument* instr, const conf::InstrFormat& instrFormat, const QByteArray& data)
{
  if (instrFormat.sect_size() != 1) {
    error_log << QObject::tr("Ошибка описания VCDU пакета");
    return 0;
  }

  uint32_t pos = 0;//findStartCCSDS(data); //обязательно для китайца
  int cnt = 0;

  if (pos >= (uint32_t)data.size()) {
    return 0;
  }


  bool hasIZ = true;
  if (instr->type().first == mappi::conf::kModisTerra || instr->type().first == mappi::conf::kModisAqua ) {
    hasIZ = false;
  }

  VcduHeader header(hasIZ);
  uint32_t nextOffset = instrFormat.sect(0).size() + instrFormat.sect(0).start();
  uint oldcounter = 0;
  
  while ((pos + format().size()) <=  (uint32_t)data.size()) {

    header.parse(data, pos);
    
    if (header.vcid == instr->format().id()) {
      if ((uint)oldcounter+1 != header.counter) {
        debug_log << "cnt" << oldcounter << header.counter << header.packOffset
                  << ((((uint8_t)data[pos+ instrFormat.sect(0).start() + header.packOffset + 2] << 8) + (uint8_t)data[pos +instrFormat.sect(0).start() + header.packOffset+ 3]) & 0x3fff);
      }


      if (header.packOffset == 0x7ff) {
        if (instr->type().first == mappi::conf::kMsuMrLo ||
            instr->type().first == mappi::conf::kVirr) {
          instr->addNewFrame(header.counter, data.mid(pos + instrFormat.sect(0).start(), instrFormat.sect(0).size()));
        } else {
          if (oldcounter+1 == header.counter) {
            instr->addFrame(header.counter, data.mid(pos + instrFormat.sect(0).start(), instrFormat.sect(0).size()));
          }
        }
      } else {
        if (oldcounter+1 == header.counter) {
          instr->addFrame(header.counter, data.mid(pos + instrFormat.sect(0).start(), header.packOffset));
        } else {
          debug_log << "lost tail";
        }

        uint32_t pp = pos + instrFormat.sect(0).start() + header.packOffset;
        while (pp < pos + nextOffset) {
          if (pp + CCSDS_HEADER_LENGTH >= pos + nextOffset) {
            //остался маленький кусочек, меньше заголовка. считываем, что есть
            //debug_log << "start" << (instrFormat.sect(0).size() + pos + instrFormat.sect(0).start() ) - pp;
            instr->addNewFrame(header.counter, data.mid(pp, (pos + nextOffset) - pp));
            pp += (pos + nextOffset) - pp;
          } else {
            uint16_t length;
            bool ok = getCCSDSLength(data, pp, &length);
            if (!ok) {
              //TODO мал размер
            }
            length += 1; //в поле хранится число байт минус 1

            //debug_log << "length = " << length;
            if (pp + length + CCSDS_HEADER_LENGTH >= pos + nextOffset) {
              //размер ccsds больше куска, считываем до конца
              //debug_log << "part" << (instrFormat.sect(0).size() + pos + instrFormat.sect(0).start() ) - pp;
              instr->addNewFrame(header.counter, data.mid(pp, (pos + nextOffset) - pp));
              pp += (pos + nextOffset) - pp;
            } else {
              //несколько ccsds, считываем текущий, сдвигаемся на начало следующего
              //debug_log << "all  " << length  + CCSDS_HEADER_LENGTH;
              instr->addNewFrame(header.counter, data.mid(pp, length + CCSDS_HEADER_LENGTH));
              pp += length + CCSDS_HEADER_LENGTH;
            }

          }
          cnt++;
        }

      }

      oldcounter = header.counter;
    }
    
    pos += format().size();
    //debug_log << "pos" << pos;
  }
  return pos;
}


//! Поиск начала пакета CCSDS
int64_t Vcdu::findStartCCSDS(const QByteArray& data)
{
  int check=0;   //количество совпадений ожидаемых и фактических значений данных

  int32_t pos = 0;
  uint8_t vers = (uint8_t)data[pos + 0] >> 6;
  uint8_t scid = ((((uint8_t)data[pos + 0]&0x3f) << 2) + ((uint8_t)data[pos + 1] >> 6));
  uint8_t vcid = (uint8_t)data[pos + 1] & 0x3F;
  uint32_t counter = ((uint8_t)data[pos + 2] << 16) + ((uint8_t)data[pos + 3] << 8) + ((uint8_t)data[pos + 4]);
  uint8_t flag = ((uint8_t)data[pos + 5]);
  pos += format().size();

  QMap<uint8_t, uint32_t> vcidcnt;
  // if (vcid != 0) {
  vcidcnt.insert(vcid, counter);
  //}

  while (check < NEED_VALID_READ_CNT) {
    if (pos + 5 >=  data.size()) {
      return data.size();
    }

    vcid = (uint8_t)data[pos + 1] & 0x3F;
    counter = ((uint32_t)(uint8_t)data[pos + 2] << 16) + ((uint8_t)data[pos + 3] << 8) + ((uint8_t)data[pos + 4]);

    //debug_log << scid << vcid << counter << vers << flag;

    if ((vers == (uint8_t)data[pos + 0] >> 6) &&
        (scid == (((uint8_t)data[pos + 0]&0x3f) << 2) + ((uint8_t)data[pos + 1] >> 6)) &&
        //(counter + 1 == ((uint32_t)(uint8_t)data[pos + 2] << 16) + ((uint8_t)data[pos + 3] << 8) + ((uint8_t)data[pos + 4])) &&
        (!vcidcnt.contains(vcid) || vcidcnt.value(vcid) + 1 == counter) &&
        (flag == ((uint8_t)data[pos + 5]))
        ) {
      check++;
    } else {
      check = 0;
    }

    vers = (uint8_t)data[pos + 0] >> 6;
    scid = ((((uint8_t)data[pos + 0]&0x3f) << 2) + ((uint8_t)data[pos + 1] >> 6));

    flag = ((uint8_t)data[pos + 5]);
    
    //  if (vcid != 0) {
    vcidcnt[vcid] = counter;
    //}

    pos += format().size();
  }

  // var(vcidcnt);

  if (check < NEED_VALID_READ_CNT) return data.size();

  return pos - (NEED_VALID_READ_CNT + 1) * format().size();
}

//! Проверка валидности пакета
bool Vcdu::packIsValid(QByteArray cur, QByteArray prev)
{
  uint8_t vers = (uint8_t)cur[0] >> 6;
  uint8_t scid = ((((uint8_t)cur[0]&0x3f) << 2) + ((uint8_t)cur[1] >> 6));
  uint32_t counter = ((uint8_t)cur[2] << 16) + ((uint8_t)cur[3] << 8) + ((uint8_t)cur[4]);
  uint8_t flag = ((uint8_t)cur[5]);

  if (((vers == (uint8_t)prev[0] >> 6) ||  (scid == (((uint8_t)prev[0]&0x3f) << 2) + ((uint8_t)prev[1] >> 6)) ||
       (flag == ((uint8_t)prev[5]))) &&
      (counter + 1 >= ((uint32_t)(uint8_t)prev[2] << 16) + ((uint8_t)prev[3] << 8) + ((uint8_t)prev[4]) || counter == 0)
      ) {
    return true;
  }

  return false;
}

