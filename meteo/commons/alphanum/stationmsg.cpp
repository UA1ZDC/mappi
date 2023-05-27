#include "stationmsg.h"
#include "wmodecoder.h"
#include "tabledecoder.h"
#include "metadata.h"

#include <cross-commons/debug/tlog.h>
#include <commons/meteo_data/meteo_data.h>

#include <qdatetime.h>
#include <qregexp.h>

using namespace meteo;
using namespace anc;

StationMsg::StationMsg():
  _typeNum(-1),
  _stationIdx(-1),
  _data(0),
  _wmoDecoder(0),
  _tableDecoder(0)
{
  _codec = QTextCodec::codecForLocale();
}

StationMsg::~StationMsg()
{
  if (0 != _wmoDecoder)   { delete _wmoDecoder;   _wmoDecoder = 0;   }
  if (0 != _tableDecoder) { delete _tableDecoder; _tableDecoder = 0; }
}

//! Раскодирование сообщения
bool StationMsg::decode(const QMap<QString, QString>& type, const QByteArray& ba)
{
  Q_UNUSED(type);

  //var(ba);
  
  bool ok = false;
  int headerSize;

  if (! parseHeader(ba, &headerSize)) {
    error_log << QObject::tr("Ошибка чтения заголовка сообщения");
    return false;
  }
  
  switch (MetaData::instance()->format(_typeNum)) {
  case kWmoFormat:
    ok = decodeWmo(ba.right(ba.size() - headerSize));
    break;
  case kTableFormat:
    ok = decodeTable(ba.right(ba.size() - headerSize));
    break;
  default: 
    ok = false;
  }
  
  if (ok) {
    _data->setDateTime(_dt);
    dataReady();
  }

  return ok;  
}

//! Раскодирование сводки в Буквенно-цифровом коде ВМО
bool StationMsg::decodeWmo(const QByteArray& ba)
{
  _data = 0;
  _code = "";

  if (0 == _wmoDecoder) {
    _wmoDecoder = new WmoDecoder(_codec);
  }
  
  _wmoDecoder->setCaption(_dt.toString(Qt::ISODate) + " " + _station);
  _wmoDecoder->setStationName(_station);
  _wmoDecoder->setStationIdx(_stationIdx);
  _wmoDecoder->setDateTime(_dt, false);
  _wmoDecoder->setDiffUtc(_diffUtc);

  bool ok = _wmoDecoder->decode(_typeNum, ba);
  if (ok) {
    _code = _wmoDecoder->code();
    _data = _wmoDecoder->data();
  }

  return ok;
}

//! Раскодирование табличноориентированной сводки
bool StationMsg::decodeTable(const QByteArray& ba)
{
  _data = 0;
  _code = "";

  if (0 == _tableDecoder) {
    _tableDecoder = new TableDecoder(_codec);
  }
  
  _tableDecoder->setCaption(_dt.toString(Qt::ISODate) + " " + _station);
  _tableDecoder->setStationName(_station);
  _tableDecoder->setStationIdx(_stationIdx);
  _tableDecoder->setDateTime(_dt, false);
  _tableDecoder->setDiffUtc(_diffUtc);
  bool ok = _tableDecoder->decode(_typeNum, ba);
  if (ok) {
    _code = _tableDecoder->code();
    _data = _tableDecoder->data();
  }

  return ok;
}


//! Чтение заголовка сообщения
/*! 
  \param ba Сообщение

  \return true в случае успешного извлечения
*/
bool StationMsg::parseHeader(const QByteArray& ba, int* headerSize)
{
  int pos = ba.indexOf("999");
  pos = ba.indexOf('\n', pos + 1);
  pos = ba.indexOf('\n', pos + 1);
  
  //  QRegExp rx("([\\w-:]+)\\s+([-+]?[\\d]+)\\s+([\\d]+)\\s+([\\w\\s.-_]+)\\s+([\\w]+)\\s*");
  QRegExp rx("([\\w-:]+)\\s+([-+]?[\\d]+)\\s+([\\w-_]+)\\s+([\\d]+)");
  *headerSize = 0;

  *headerSize = ba.indexOf('\n', pos + 1);
  if (*headerSize == -1) return false;

  QString capStr(ba.mid(pos + 1, *headerSize));
  pos = 0;
  // var(capStr);

  pos = rx.indexIn(capStr, pos);
  if (pos == -1) return false;

  QStringList caption = rx.capturedTexts();
  if ( 5 != caption.size()) {
    return false;
  }

  //  *headerSize = rx.matchedLength();

  _dt = QDateTime::fromString(caption.at(1), Qt::ISODate);
  _diffUtc = caption.at(2).toInt();

  QString group;
  group.append(caption.at(3));
  _typeNum = MetaData::instance()->forms().value(group, -1);

  _stationIdx = caption.at(4).toInt();


  // var(_dt);
  //var(_diffUtc);
  // var(_stationIdx);
  // var(_station);
  // var(_typeNum);
  return true;
}
