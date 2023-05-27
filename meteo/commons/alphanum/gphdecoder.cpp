#include "gphdecoder.h"
#include "metadata.h"

#include <commons/meteo_data/tmeteodescr.h>
#include <cross-commons/debug/tlog.h>

#include <qtextcodec.h>
#include <qregexp.h>
#include <qdebug.h>
#include <qdatastream.h>

#include <math.h>

using namespace meteo;
using namespace anc;

GphDecoder::GphDecoder(QTextCodec* codec):
  _codec(codec)
{
}

GphDecoder::~GphDecoder()
{
}


int GphDecoder::parse(const QMap<QString, QString>& type, const QByteArray& ba, const QDateTime& dt)
{ 
  _maps.clear();
  _epochs.clear();

  _idPtkpp = type.value("id");

  setCaption(_idPtkpp);
  if (dt.isValid()) {
    setDateTime(dt, true);
  }

  int start = 0;
  int ttype = findType(ba.left(300), &start);
  if (-1 != ttype && -1 != start) {
    bool ok = decode(ttype, ba.mid(start));
    if (ok) {
      return _epochs.size();
    }
  }

  return 0;
}

int GphDecoder::findType(const QByteArray &report, int* start)
{ 
  QList<QRegExp> groups;//code groups TTAA, TTBB, ...
  int number = 0;
  int type = -1;
  *start = -1;
  
  QList<int> keys;
  keys << MetaData::instance()->uniqHeaderKeys();
  
  QList<int>::iterator it;
  for(it = keys.begin(); it != keys.end(); ++it) { //перебор по кодовым формам
    groups = MetaData::instance()->headers(*it);
    // debug_log << *it << "groups:"<< groups;
    for (auto MiMiMjMj : groups) { //перебор по разделам (A,B,C,D) каждой кодовой формы
      //  QRegExp MiMiMjMj(groups.at(i));
      number = 0;
      if(-1 < (number = MiMiMjMj.indexIn(report, number))) {
	*start = number;
	type = *it;
	break;
      }
    }
    if (number != -1) break;
  }

  return type;
}

bool GphDecoder::decode(const QByteArray &report, const QDomDocument &decoder)
{
#ifdef PARSE_DEBUG_LOG
  debug_log << "decodeGeophysic";
  debug_log << "CATEGORY" << decoder.documentElement().attribute("type");
#endif

  int endHeader = report.indexOf(endWith());
  if (endHeader == -1) return false;

  QByteArray header(report.left(endHeader));

  QString line;
  int idx = -1;
  int prev = idx;

  while (idx + 1 < header.size()) {
    prev = idx + 1;
    idx = header.indexOf('\x0A', prev);
   
    if (idx == -1) {
      idx = header.size();
    }

    if (idx - prev > 0) {
      
      line = _codec->toUnicode(header.mid(prev, idx - prev).data(), idx-prev);
      
      if (! line.trimmed().isEmpty()) {
#ifdef PARSE_DEBUG_LOG
	debug_log << "Строка :" << line;
#endif
	
	if (!decodeNode(decoder.documentElement(), line)) { 
	  debug_log << QObject::tr("Ошибка раскодирования строки"); 
	  return false;
	}
      }
    }

    prev = idx + 1;
  }

  bool ok = false;
  
  TMeteoData* md = RegExpDecode::data();
  if (0 != md) {
    float expon = md->getValue(TMeteoDescriptor::instance()->descriptor("exp"), BAD_METEO_ELEMENT_VAL);
    if (expon != BAD_METEO_ELEMENT_VAL) {
      ok = readMaps(report.right(report.size() - endHeader), expon);
    }
  }
  
  return ok;
}

bool GphDecoder::readMaps(const QByteArray& ba, float expon)
{
  QRegExp rx("\\s*(\\d{1,2})\\s*START\\s*OF\\s*(\\w+)\\s*MAP\\s*\n\\s*"
	     "(?:\\s*(\\d{4})\\s*(\\d{1,2})\\s*(\\d{1,2})\\s*(\\d{1,2})\\s*(\\d{1,2})\\s*(\\d{1,2})\\s*EPOCH)?");
  
  QDateTime currentEpoch;

  int pos = rx.indexIn(ba);
  QStringList strList = rx.capturedTexts();
  
  while (ba.size() - 1 != pos && -1 != pos) {
    //next = rx.indexIn(ba, pos + 60);
    int next = ba.indexOf("END", pos);
    if (next == -1) {
      next = ba.size() - 1;
    }
    
    if (strList.count() != 9) {
      //pos = next;
      pos = rx.indexIn(ba, next);
      strList = rx.capturedTexts();
      //debug_log << "error regexp" << strList.count();
      continue;
    }
    
    bool ok;
    int num = strList.at(1).toInt(&ok);
    if (!ok) {
      error_log << QObject::tr("Ошибка раскодирования номера карты");
      //pos = next;
      pos = rx.indexIn(ba, next);
      strList = rx.capturedTexts();
      continue;
    }

    if (!strList.at(3).isEmpty()) {
      bool dok;
      int y  = strList.at(3).toInt(&dok);
      ok &= dok;
      int mm = strList.at(4).toInt(&dok);
      ok &= dok;
      int d  = strList.at(5).toInt(&dok);
      ok &= dok;
      int h  = strList.at(6).toInt(&dok);
      ok &= dok;
      int m  = strList.at(7).toInt(&dok);
      ok &= dok;
      int s  = strList.at(8).toInt(&dok);
      ok &= dok;
      if (!ok) {
	error_log <<  QObject::tr("Ошибка раскодирования эпохи");
	//pos = next;
	pos = rx.indexIn(ba, next);
	strList = rx.capturedTexts();
	continue;
      }
      currentEpoch.setDate(QDate(y, mm, d));
      currentEpoch.setTime(QTime(h,  m, s));
      _epochs.insert(num, currentEpoch);
    }

    GphMapType type;
    if (strList.at(2).contains("TEC")) {
      type = kTec;      
    } else if (strList.at(2).contains("RMS")) {
      type = kRms;      
    } else if (strList.at(2).contains("HEIGHT")) {
      type = kHeight;
    } else {
      error_log <<  QObject::tr("Неизвестный тип карты");
      //pos = next;
      pos = rx.indexIn(ba, next);
      strList = rx.capturedTexts();
      continue;
    }
    
    if (_maps.contains(num)) {
      _maps[num] += parseMapData(type, expon, ba.mid(pos, next - pos + 3));
      _maps[num][0] = _maps[num][0] + 1;
    } else {
      _maps.insert(num, QByteArray(1, 1) + parseMapData(type, expon, ba.mid(pos, next - pos + 3)));
    }

    //pos = next;
    pos = rx.indexIn(ba, next);
    if ("" == rx.cap()) break;  
    strList = rx.capturedTexts();
  }

  return (_maps.size() != 0);
}

QByteArray GphDecoder::parseMapData(int type, float exp, const QByteArray& ba)
{
  QList<float> vals;

  QRegExp rx("(?:(-?\\d[1,2])\\s*EXPONENT\\s*\n)?\\s*[-\\d.\\s]+\\s*LAT/LON1/LON2/DLON/H\\s*\n\\s*");
  QRegExp end("\\s*\\d{1,3}\\s*END|\n\\s*(?:(-?\\d[1,2])\\s*EXPONENT\\s*\n)?\\s*(?:[-\\d.\\s]{,60})\\s*LAT/LON1/LON2/DLON/H\\s*\n\\s*");
  
  int pos = rx.indexIn(ba);
  QStringList strList = rx.capturedTexts();

  while (-1 != pos || strList.size() < 1) {
    //next = rx.indexIn(ba, pos + strList.at(0).size());
    int next = end.indexIn(ba, pos + strList.at(0).size());
    if (next == -1) {
      next = ba.size() - 1;
    }
    
    strList = rx.capturedTexts();
    //debug_log << "error regexp" << strList.count();
    
    if (strList.size() == 2 && !strList.at(1).isEmpty()) {
      bool ok;
      exp = strList.at(1).toFloat(&ok);
      if (!ok) {
	error_log << QObject::tr("Ошибка раскодирования экспоненты");
	//pos = next;
	pos = rx.indexIn(ba, next);
	strList = rx.capturedTexts();
	continue;
      }
    }

    QList<QByteArray> bvals = ba.mid(pos + strList.at(0).size(), 
				     next - pos - strList.at(0).size()).simplified().split(' ');
    //  var(bvals);
    for (int idx = 0; idx < bvals.size(); idx++) {
      float v = bvals.at(idx).toUInt();
      if (v != 9999) {
	vals << (v * pow(10, exp));
      } else {
	vals << v;
      }
    }
    
    //pos = next;
    pos = rx.indexIn(ba, next);
    strList = rx.capturedTexts();
  }

  QByteArray res;
  QDataStream ds(&res, QIODevice::WriteOnly);
  ds << type << vals.size();
  for (int idx = 0; idx < vals.size(); idx++) {
    ds << vals.at(idx);
  }

  return res;
}
