#include "wmodecoder.h"

#include <qtextcodec.h>
#include <qdom.h>

using namespace meteo;
using namespace anc;

WmoDecoder::WmoDecoder(QTextCodec* codec):
  _codec(codec)
{
}

WmoDecoder::~WmoDecoder()
{
}


//! Раскодирование сводки
/*! 
  
  \param report Сводка
  \param decoder XML для раскодирования
  
  \return false - в случае ошибки
*/
bool WmoDecoder::decode(const QByteArray &report, const QDomDocument &decoder)
{

#ifdef PARSE_DEBUG_LOG
  debug_log << "decodeWmo";
  debug_log << report;
  debug_log << "CATEGORY" << decoder.documentElement().attribute("type");
#endif
 
  if (! decodeNode(decoder.documentElement(), _codec->toUnicode(report))) {
    return false;
  }

#ifdef PARSE_DEBUG_LOG
  debug_log << "report" << report;
#endif

  return true;
}
