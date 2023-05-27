#ifndef METEO_ALPHANUM_TABLEDECODER_H
#define METEO_ALPHANUM_TABLEDECODER_H

#include <meteo/commons/alphanum/regexpdecode.h>

namespace meteo {
namespace anc {

//! раскодировщик сводок, имеющих табличное представление
class TableDecoder : public RegExpDecode {
public:

  TableDecoder(QTextCodec* codec);
  ~TableDecoder();

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
