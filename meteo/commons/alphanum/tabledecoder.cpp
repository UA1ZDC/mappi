#include "tabledecoder.h"

#include <cross-commons/debug/tlog.h>

#include <qtextstream.h>
#include <qdom.h>

using namespace meteo;
using namespace anc;


TableDecoder::TableDecoder(QTextCodec* codec):
  _codec(codec)
{
}


TableDecoder::~TableDecoder()
{
}
  
//! Раскодирование сводки
/*! 
  
  \param report Сводка
  \param decoder XML для раскодирования
  
  \return false - в случае ошибки
*/
bool TableDecoder::decode(const QByteArray &report, const QDomDocument &decoder)
{
#ifdef PARSE_DEBUG_LOG
  debug_log << "decodeInstrument";
  debug_log << report;
  debug_log << "CATEGORY" << m_caption;//decoder.documentElement().attribute("type");
#endif
 
  QString line;
  int idx = -1;
  int prev = idx;

  while (idx + 1 < report.size()) {
    prev = idx + 1;
    idx = report.indexOf('\n', prev);

    if (idx == -1) {
      idx = report.size();
    }

    if (idx - prev > 0) {
      
      line = _codec->toUnicode(report.mid(prev, idx - prev).data(), idx-prev);
      //line = QString::fromLatin1(report.mid(prev, idx - prev).data(), idx-prev);
      line = line.simplified().remove(0x03);
      
      if (! line.isEmpty()) {
	debug_log << "Строка :" << line << line.size();

#ifdef PARSE_DEBUG_LOG
	debug_log << "Строка :" << line;
	debug_log << reg.indexIn(line);
#endif
	
	if (!decodeNode(decoder.documentElement(), line)) { 
	  //debug_log << QObject::tr("Ошибка раскодирования строки"); 
	}
	
      }
    }

    prev = idx + 1;

  }

  return true;
}
