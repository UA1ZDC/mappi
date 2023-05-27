#ifndef TGRIB_REPR_H
#define TGRIB_REPR_H

#include <stdint.h>

#include <google/protobuf/message.h>

#include "tgribparser.h"

namespace grib {
  struct TReprDefinition {
    uint16_t type;
    google::protobuf::Message* definition;
    TReprDefinition():type(0xffff),definition(0) {}
    ~TReprDefinition() { if (definition) delete definition; definition = 0; }
  };
}

//! парсинг определения продукта GRIB2 из БД, описание в xml и proto
class TReprGrib : public TGribParser {
public:
  TReprGrib();
  ~TReprGrib();

  uint16_t type() {if (!_repr) return 0; return _repr->type;}
  google::protobuf::Message* definition() { if (!_repr) return 0; return _repr->definition;}
  
protected:
  google::protobuf::Message* createDefinition(uint16_t templ, int version);

private:
  grib::TReprDefinition* _repr;
};


#endif 
