#ifndef METEO_ALPHANUM_WMODECODER_h
#define METEO_ALPHANUM_WMODECODER_h

#include <meteo/commons/alphanum/regexpdecode.h>

namespace meteo {
namespace anc {

//! раскодировщик сводок в формате международных буквенно-цифровых кодов
class WmoDecoder : public RegExpDecode {
public:

  WmoDecoder(QTextCodec* codec);
  ~WmoDecoder();

  // bool decode(const QString& code, const QByteArray& report) {
  //   return RegExpDecode::decode(code, report);
  // }

  bool decode(int code, const QByteArray& report) {
    return RegExpDecode::decode(code, report);
  }

private:
  
  bool decode(const QByteArray &report, const QDomDocument &decoder);
  
private:
  QTextCodec* _codec;
};
  


} //anc
} //meteo

#endif
