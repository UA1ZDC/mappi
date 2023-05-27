#ifndef MAPPI_GLOBAL_STREAM_HEADER_H
#define MAPPI_GLOBAL_STREAM_HEADER_H

#include <sat-commons/satellite/satellite.h>
#include <mappi/proto/satellite.pb.h>

#include <stdint.h>

class QDateTime;
class QByteArray;

namespace meteo {
namespace global {

enum MappiFileType {
  kMappiUnkFile   = 0,
  kRawFile   = 1,
  kPretrFile = 2,
  kThemFile  = 3,
};

//! Общий заголовок для всех форматов (сырые, после ПО, ТО)
struct PreHeader {
  MappiFileType type = kMappiUnkFile; //!< тип файла, 1 байт
  uint16_t year = 0;   //!< дата создания
  uint8_t  month = 0;  //!< дата создания
  uint8_t  day = 0;    //!< дата создания
  uint8_t  res = 0;
  uint16_t offset = 0; //!< смещение до данных (= размер следующего заголовка для конкретного формата)

  PreHeader() {}
  PreHeader(MappiFileType atype, uint16_t aoffset) {
    type = atype;
    offset = aoffset;
    QDateTime cur = QDateTime::currentDateTimeUtc();
    year = cur.date().year();
    month = cur.date().month();
    day = cur.date().day();
  }

  static int size() { return 8; }
};

QDataStream& operator<<(QDataStream &out, const PreHeader& header);
QDataStream& operator>>(QDataStream &in, PreHeader& header);

//---------

//! заголовок потока (добавляется в приёмнике в начало)
struct StreamHeader {
  QString site;                     //!< название приёмного пункта
  meteo::GeoPoint siteCoord;        //!< координаты приёмного пункта
  QDateTime start;                  //!< время начала приёма
  QDateTime stop;                   //!< время окончания приёма
  QString satellite;                //!< название спутника
  int32_t direction = -1;           //!< направление (с севера, с юга)
  mappi::conf::RateMode mode = mappi::conf::kUnkRate;       //!< режим приёма
  MnSat::TLEParams tle;             //!< орбитальные параметры
};

void toBuffer(const StreamHeader& header, QByteArray& buf);
void fromBuffer(const QByteArray& buf, StreamHeader& header);

QDataStream& operator<<(QDataStream &out, const StreamHeader& header);
QDataStream& operator>>(QDataStream &in, StreamHeader& header);

//---- Формат заголовка виндового сюжета
union NoradElem {
  double M[21];
  struct {
    double  epoch;    // epoch time (YYDDD.DDDDDDD)
    double  d_lv;     // correction longitude of ascending node (rad)
    double  eo;       // eccentricity
    double  xincl;    // inclination    (deg)
    double  xnodeo;   // longitude of ascending node (deg)
    double  omegao;   // argument of perigee  (deg)
    double  thetg;    // angle of the spring equilibrium point  (rad)
    double  xmo;      // mean anomaly  (deg)
    double  xno;      // mean motion (calculated)
    double  d_epoch;  // correction time of ascending node (epoch)
    double  OrbNum;   // Revolution number (must be DWORD)
    double  EphemerisType;  // type of data (TBUS or NORAD)
    double  period;
    double  xndt2o;
    double  xndd6o;
    double  bstar;
    double  iexp;
    double  ibexp;
    double  spare1;
    double  spare2;
    double  spare3;
  } S;
};

union DateTime {
  uint16_t M[8];
  struct {
    uint16_t wYear;
    uint16_t wMonth;
    uint16_t wDayOfWeek;
    uint16_t wDay;
    uint16_t wHour;
    uint16_t wMinute;
    uint16_t wSecond;
    uint16_t wMilliseconds;
  } S;
};

struct StreamHeaderOld {
  char site[20]; //!< название приёмного пункта
  double lat;    //!< широта приёмного пункта, рад
  double lon;    //!< долгота приёмно пункта, рад
  double h;      //!< высота приёмного пункта
  char satName[20];    //!< название спутника
  NoradElem mNore;
  int32_t direct;
  DateTime timeStart;
  int32_t flagTimeCor;

  QDateTime dateTime() const;
  MnSat::TLEParams tle() const;
  void fill(StreamHeader* header) const;

  static int size() { return 256; }
};

QDataStream& operator>>(QDataStream& stream, StreamHeaderOld& header);

//----

//! Заголовок, формируемый после предварительной обработки, для данных каналов
struct PoHeader {
  QString site;		//!< название приёмного пункта
  meteo::GeoPoint siteCoord;//!< координаты приёмного пункта
  QDateTime start;		//!< время начала приёма
  QDateTime stop;		//!< время окончания приёма
  QString satellite;	//!< название спутника
  int32_t direction = -1;	//!< направление (с севера, с юга)
  MnSat::TLEParams tle;	//!< орбитальные параметры
  mappi::conf::InstrumentType instr = mappi::conf::kInstrUnk; //!< прибор
  int32_t channel = -1;	//!< номер канала
  int32_t lines   = -1;	//!< число строк сканирования
  int32_t samples = -1;	//!< число сканирований в строке
  quint64 type_hash = 0;     //!< type_info::hash_code для типа данных
  float min = -9999;        //!< минимальное значение
  float max = 0;            //!< максимальное значение
  int undef = -9999;        //!< отсутствующее значение
  uint16_t procflag = 0;    //!< для ПО: 0x1 - калиброван, для ТО: тип обработки ThemType (thematic.proto)
  QString name;	//!< название обработки
  QString short_name;	//!< название обработки
  QString unit_name;	//!< ед измерения
};

const PoHeader& operator>>(const PoHeader& head, QByteArray& out);
PoHeader& operator<<(PoHeader& head, const QByteArray& ba);

QDataStream& operator<<(QDataStream &out, const PoHeader& header);
QDataStream& operator>>(QDataStream &in, PoHeader& header);

PoHeader readPoHeader(const QString& fileName);
}
}

#endif
