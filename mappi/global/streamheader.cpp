#include "streamheader.h"

#include <commons/mathtools/mnmath.h>
#include <cross-commons/debug/tlog.h>

#include <qdatastream.h>
#include <qdatetime.h>
#include <qbytearray.h>
#include <QFile>


namespace meteo {

namespace global {

//! Получение даты/времени из дополнительного заголовка потока
/*! 
  \return возвращаемая дата/время
*/
QDateTime StreamHeaderOld::dateTime() const 
{
  QDateTime dt;
  dt.setDate(QDate(timeStart.S.wYear, timeStart.S.wMonth, timeStart.S.wDay));
  dt.setTime(QTime(timeStart.S.wHour, timeStart.S.wMinute, 
		   timeStart.S.wSecond, timeStart.S.wMilliseconds));
  dt.setTimeSpec(Qt::UTC);
  return dt;
}

//! Зполнение орбитальных параметров из дополнительного заголовка потока
/*! 
  \return орбитальные параметры
*/
MnSat::TLEParams StreamHeaderOld::tle() const
{
  MnSat::TLEParams tle;
  
  tle.epochYear = (uint)((mNore.S.epoch)/1000); //TODO modf?
  tle.epochDay  = mNore.S.epoch - tle.epochYear*1000 +1;
  if ( tle.epochYear < 57 ) {
    tle.epochYear += 2000;
  } else {
    tle.epochYear += 1900;
  }

  tle.bstar = mNore.S.bstar*pow(10,mNore.S.ibexp);
  
  tle.inclination = MnMath::deg2rad(mNore.S.xincl);
  tle.raan    = MnMath::deg2rad(mNore.S.xnodeo);
  tle.eccentr =mNore.S.eo;
  tle.perigee = MnMath::deg2rad(mNore.S.omegao);
  tle.meanAnomaly = MnMath::deg2rad(mNore.S.xmo);
  tle.meanMotion  = mNore.S.xno * MnMath::M_2PI / MnMath::MIN_PER_DAY;
  tle.revolNumber = (ulong)(mNore.S.OrbNum);
  
  return tle;
}

void StreamHeaderOld::fill(StreamHeader* header) const
{
  if (0 == header) return;

  QTextCodec *codec = QTextCodec::codecForName("Windows-1251");

  header->site = codec->toUnicode(site);
  header->siteCoord = meteo::GeoPoint(lat, lon, h);
  header->start = dateTime();
  header->satellite = satName;
  header->direction = direct;
  header->tle = tle();
}

//! Чтение дополнительного заголовка из потока
/*! 
  \param data поток данных
  \param header возвращаемый заполненный заголовок
*/
QDataStream& operator>>(QDataStream& stream, StreamHeaderOld& header)
{
  QDataStream::ByteOrder bo = stream.byteOrder();
  
  stream.setByteOrder(QDataStream::LittleEndian);

  stream.readRawData(header.site, sizeof(header.site));
  stream >> header.lat;
  stream >> header.lon;
  stream >> header.h;
  stream.readRawData(header.satName, sizeof(header.satName));
  for (uint i=0; i< 21; i++) {
    stream >> header.mNore.M[i];
  }
  stream >> header.direct;
  for (uint i=0; i< 8; i++) {
    stream >> header.timeStart.M[i];
  }
  stream >> header.flagTimeCor;

  // for (uint i=0; i<20; i++) {
  //   debug_log<<(uchar)header.site[i]; он там, наверное, в koi8-r
  // }
  // debug_log << stream.byteOrder();
  // debug_log << (header.site)
  // 	    << (header.lat)
  // 	    << (header.lon)
  // 	    << (header.h);
  // debug_log << (header.satName)
  // 	    << (header.mNore.S.bstar*pow(10,header.mNore.S.ibexp))
  // 	    << (MnMath::deg2rad(header.mNore.S.xincl));
  // debug_log << MnMath::deg2rad(header.mNore.S.xnodeo)
  // 	    << header.mNore.S.eo
  // 	    << MnMath::deg2rad(header.mNore.S.omegao)
  // 	    << MnMath::deg2rad(header.mNore.S.xmo)
  // 	    << header.mNore.S.xno * MnMath::M_2PI / MnMath::MIN_PER_DAY
  // 	    << (ulong)(header.mNore.S.OrbNum)
  // 	    << header.direct
  // 	    << header.flagTimeCor;

  // QDateTime dt = header.dateTime();
  // debug_log<<dt.toString();

  stream.setByteOrder(bo);
  return stream;
}

QDataStream& operator<<(QDataStream &out, const PreHeader& header)
{
  out << (uint8_t)header.type;
  out << header.year;
  out << header.month;
  out << header.day;
  out << header.res;
  out << header.offset;

  return out;
}

QDataStream& operator>>(QDataStream &in, PreHeader& header)
{
  uint8_t val;
  in >> val;
  header.type = meteo::global::MappiFileType(val);
  in >> header.year;
  in >> header.month;
  in >> header.day;
  in >> header.res;
  in >> header.offset;

  return in;
}
  
void toBuffer(const StreamHeader& header, QByteArray& buf)
{
  QDataStream out(&buf, QIODevice::WriteOnly);
  out << header;
}

void fromBuffer(const QByteArray& buf, StreamHeader& header)
{
  QDataStream in(buf);
  in >> header;
}

QDataStream& operator<<(QDataStream &out, const StreamHeader& header)
{
  out << header.site;
  out << (double)header.siteCoord.lat();
  out << (double)header.siteCoord.lon();
  out << (double)header.siteCoord.alt();
  out << header.start;
  out << header.stop;
  out << header.satellite;
  out << header.direction;
  out << (qint16)header.mode;
  out << header.tle;

  return out;
}

QDataStream& operator>>(QDataStream &in, StreamHeader& header)
{
  in >> header.site;

  double v;
  in >> v; header.siteCoord.setLat(v);
  in >> v; header.siteCoord.setLon(v);
  in >> v; header.siteCoord.setAlt(v);

  in >> header.start;
  in >> header.stop;
  in >> header.satellite;
  in >> header.direction;

  qint16 vi16;
  in >> vi16;
  header.mode = mappi::conf::RateMode(vi16);
  in >> header.tle;

  return in;
}

const PoHeader& operator>>(const PoHeader& head, QByteArray& out)
{
  QDataStream ds(&out, QIODevice::WriteOnly);
  ds << head;

  return head;
}

PoHeader& operator<<(PoHeader& head, const QByteArray& ba)
{
  QDataStream ds(ba);
  ds >> head;

  return head;
}

QDataStream& operator<<(QDataStream &out, const PoHeader& header)
{
  out << header.site;
  out << (double)header.siteCoord.lat();
  out << (double)header.siteCoord.lon();
  out << (double)header.siteCoord.alt();
  out << header.start;
  out << header.stop;
  out << header.satellite;
  out << header.direction;
  out << header.tle;
  out << (int32_t)header.instr;
  out << header.channel;
  out << header.lines;
  out << header.samples;
  out << header.type_hash;
  out << header.min;
  out << header.max;
  out << header.undef;  
  out << header.procflag;
  out << header.name;
  out << header.short_name;
  out << header.unit_name;
  return out;
}

QDataStream& operator>>(QDataStream &in, PoHeader& header)
{
  in >> header.site;
  double val;
  in >> val;
  header.siteCoord.setLat(val);
  in >> val;
  header.siteCoord.setLon(val);
  in >> val;;
  header.siteCoord.setAlt(val);
  in >> header.start;
  in >> header.stop;
  in >> header.satellite;
  in >> header.direction;
  in >> header.tle;
  int32_t ival;
  in >> ival;
  header.instr = mappi::conf::InstrumentType(ival);
  in >> header.channel;
  in >> header.lines;
  in >> header.samples;
  in >> header.type_hash;
  in >> header.min;
  in >> header.max;
  in >> header.undef;
  in >> header.procflag;
  in >> header.name;
  in >> header.short_name;
  in >> header.unit_name;
  return in;
}

PoHeader readPoHeader(const QString& fileName)
{
  global::PoHeader header;
  QFile file(fileName);
  bool ok = file.open(QIODevice::ReadOnly);
  if (ok) {
    QDataStream ds(&file);
    ds >> header;
    file.close();
  }
  return header;
}

}

}
