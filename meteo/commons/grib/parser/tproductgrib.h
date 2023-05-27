#ifndef TPRODUCT_GRIB_H
#define TPRODUCT_GRIB_H

#include <stdint.h>

#include <meteo/commons/grib/tgribdefinition.h>

#include "tgribparser.h"

//! парсинг описания продукта GRIB2 из БД, описание в xml и proto
class TProductGrib : public TGribParser {
public:
  TProductGrib();
  ~TProductGrib();

  grib::TProductDefinition* definition() {return _prod;}

protected:
  google::protobuf::Message* createDefinition(uint16_t templ, int version);
  void parseCustom(google::protobuf::Message* msg, const uint8_t* data, uint32_t dataSize, QDomElement& el, int32_t startIdx);

private:
  uint64_t getFactor(const uint8_t* data, const QDomElement& child, int32_t startIdx);

private:
  grib::TProductDefinition* _prod;
};



#endif
