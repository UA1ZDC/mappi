#include "m_ccsds.h"

#include <QByteArray>


bool mappi::po::CCSDSHeader::parse(const QByteArray& data, const QDate& date)
{
  if (data.size() < size) {
    return false;;
  }
  for(unsigned int i=0; i<size; i++) raw[i]=data[i];
  version = raw[0] >> 5;
  indicator = (raw[0] >> 4) % 2;
  hasSecondary = (raw[0] >> 3) % 2;
  apid = (raw[0] % (int)pow(2, 3)) << 8 | raw[1];
  seqFlag = raw[2] >> 6;
  cnt = (raw[2] % (int)pow(2, 6)) << 8 | raw[3];
  length = raw[4] << 8 | raw[5];
  
  if (hasSecondary) {
    size += 10;
    if (data.size() < size) {
      return false;
    }
    
    dts.setDate(date); //QDate(1958, 1, 1));
    dts = dts.addDays(((uint8_t)data[6] << 8) + (uint8_t)data[7]);
    dts.setTime(QTime(0,0));
    dts = dts.addMSecs(((uint8_t)data[8] << 24) + ((uint8_t)data[9] << 16) +
		       ((uint8_t)data[10] << 8) + (uint8_t)data[11]);
    //12,13 - 2 байта микросекунды времени
    
    pckCnt = (uint8_t)data[14];
  }
  return true;
}

TLog& mappi::po::operator<<(TLog& log, const CCSDSHeader& header)
{
  log << header.version
      << header.indicator    
      << header.apid
      << header.seqFlag 
      << header.cnt
      << header.length;
    //      << ((uint8_t)data[size + 4] << 8) +  (uint8_t)data[size + 5]

  if (header.hasSecondary) {
    log << "secondary:"  
	<< header.dts.toString("yyyy-MM-ddThh:mm:ss.zzz")
	<< header.pckCnt;
    
    // << ((uint8_t)data[header.size + 16] << 24) + ((uint8_t)data[header.size + 17] << 16) +
    //    ((uint8_t)data[header.size + 18] <<  8) +  (uint8_t)data[header.size + 19];
  }
  
  return log;
}


bool mappi::po::getCCSDSLength(const QByteArray& data, uint16_t* length)
{
  if (data.size() < 6) {
    return false;
  }
  *length = ((uint8_t)data[4] << 8) + (uint8_t)data[5];
  return true;
}

bool mappi::po::getCCSDSLength(const QByteArray& data, uint32_t pos, uint16_t* length)
{
  if ((uint32_t)data.size() < pos + 6) {
    return false;
  }
  *length = ((uint8_t)data[pos + 4] << 8) + (uint8_t)data[pos + 5];
  return true;
}

int32_t mappi::po::getCCSDSCnt(const QByteArray& data)
{
  if (data.size() < 4) {
    return -1;
  }
 
  return (((uint8_t)data[2] << 8) + (uint8_t)data[3]) & 0x3fff;
}
