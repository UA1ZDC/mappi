#include "tproductgrib.h"
#include "tproduct.pb.h"

#include <qdom.h>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include <cross-commons/debug/tlog.h>

#include "tgribparser.h"
#include "tgribformat.h"

using namespace google::protobuf;
using namespace TProduct;
using namespace grib;

//-----
// void printProduct(TProductData* src)
// {
//   printf("Print Product\n");
//   uchar* data = src->definition;
//   uint startIdx = 10;
//   printf("def=%p\n", src->definition);
//   debug_log<<"template="<<src->defType;
//   debug_log<<"category="<<data[10-startIdx];
//   uint32_t val;
//   grib::char2dec(data + 15 - startIdx, 2, &val);
//   debug_log<<"hour="<<val;
//   debug_log<<"minutes="<<data[17 - startIdx];
//   debug_log<<"first val fact="<<data[24];
//   grib::char2dec(data + 25 - startIdx, 2, &val);
//   debug_log<<"first val="<<val;
//   grib::char2dec(data + 19 - startIdx, 4, &val);
//   debug_log<<"forecast time="<<val;
//   debug_log<<"forecast time unit="<<data[18 - startIdx];

//   for (uint i=0; i<src->defSize; i++) {
//     printf("%2d: %2x %3d\n", i+startIdx, data[i], data[i]);
//   }
// }

//-----

TProductGrib::TProductGrib():_prod(0)
{
  _prod = new TProductDefinition;
}

TProductGrib::~TProductGrib()
{
  if (_prod) {
    delete _prod;
  }
  _prod = 0;
}

//! Создание структуры для описания продукта
/*! 
  \param templ номер шаблона
  \param version номер версии GRIB
  \return созданная структура
*/
google::protobuf::Message* TProductGrib::createDefinition(uint16_t templ, int version)
{
  if (version != 2) {
    error_log<<"Grib version "<<version<<"doesn't supported";
    return 0;
  }

  if (_prod->definition) delete _prod->definition;
  _prod->definition = 0;

  switch (templ) {
  case  0: case  1: case  2: case  6: case  7: case  8: 
  case 10: case 11: case 12: case 15: case 40: case 41: 
  case 42: case 43: case 44: case 45: case 46: case 47: 
  case 48: case 1000: case 1001: {
    Product0* def = new Product0;
    _prod->definition = def;
  }
    break;
  case 3: case 4: case 13: case 14: {
    Product1* def = new Product1;
    _prod->definition = def;
  }
    break;
  case 5: case 9: {
    Product2* def = new Product2;
    _prod->definition = def;
  }
    break;
  case 20: {
    Product3* def = new Product3;
    _prod->definition = def;
  }
    break;
  case 31: case 32: case 33: case 34: {
    Product4* def = new Product4;
    _prod->definition = def;
  }
    break;
  case 51: case 91: {
    Product5* def = new Product5;
    _prod->definition = def;
  }
    break;     
 
 case 53: case 54: {
    Product6* def = new Product6;
    _prod->definition = def;
  }
    break;    

  case 60: case 61: {
    Product7* def = new Product7;
    _prod->definition = def;
  }
    break;   

 case 254: {
    Product7* def = new Product7;
    _prod->definition = def;
  }
    break;   
 
  default:
    error_log<<"Grid with template number"<<templ<<"doesn't supported";
  }

  _prod->type = templ;
  return _prod->definition;
}

void TProductGrib::parseCustom(google::protobuf::Message* msg, const uchar* data, uint32_t /*size*/, QDomElement& el, int32_t startIdx)
{
  if (el.attribute("subtype") == "time") {
    int idx = el.attribute("idx").toInt() + startIdx;
    int size = el.attribute("size").toUInt();
    int number = el.attribute("pbnum").toUInt();

    uint64_t factor = getFactor(data, el.firstChildElement("unit"), startIdx);
    uint32_t val=0;
    grib::char2dec(data + idx, size, &val);
    msg->GetReflection()->SetUInt64(msg, msg->GetDescriptor()->FindFieldByNumber(number), val*factor);
  }
}

uint64_t TProductGrib::getFactor(const uchar* data, const QDomElement& el, int32_t startIdx)
{
  int idx = el.attribute("idx").toInt() + startIdx;
  if (idx < 0) return 1;
  uint8_t pos = data[idx];
  
  int cnt = el.attribute("factorcnt").toUInt();
  if (pos >= cnt) return 1;

  QString factor = el.attribute("factor");
  return factor.section(',', pos, pos).toULongLong();
}
