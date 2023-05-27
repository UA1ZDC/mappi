#include "treprgrib.h"
#include "trepresent.pb.h"

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include <cross-commons/debug/tlog.h>

#include "tgribparser.h"

using namespace google::protobuf;
using namespace TRepresent;
using namespace grib;

//----- debug
// void printRepresent(uchar* data, uint64_t size)
// {
//   unsigned startIdx = 12;

//   printf("Print Represent\n");
//   for (uint64_t i=0; i< size; i++) {
//     printf("%2llu: %2x %3d\n", i+startIdx, data[i], data[i]);
//   }
// }

//-----

TReprGrib::TReprGrib():_repr(0)
{
  _repr = new(std::nothrow) TReprDefinition;
}

TReprGrib::~TReprGrib()
{
  if (_repr) {
    delete _repr;
  }
  _repr = 0;
}

//! Создание структуры для определения продукта
/*! 
  \param templ номер шаблона
  \param version номер версии GRIB
  \return созданная структура
*/
google::protobuf::Message* TReprGrib::createDefinition(uint16_t templ, int version)
{
  if (version > 2) {
    error_log<<"Grib version "<<version<<"doesn't supported";
    return 0;
  }

  if (_repr->definition) delete _repr->definition;
  _repr->definition = 0;

  if (version == 2) {
    switch (templ) {
    case 0: case 41: case 61: {
      SimplePoint* def = new SimplePoint;
      _repr->definition = def;
    }
      break;
    case 1: {
      MatrixPoint* def = new MatrixPoint;
      _repr->definition = def;
    }
      break;
    case 2: case 3: {
      ComplexPoint* def = new ComplexPoint;
      _repr->definition = def;
    }
      break;

    case 4: {
      IEEEPoint* def = new IEEEPoint;
      _repr->definition = def;
    }
      break;

    case 40: {
      JpegPoint* def = new JpegPoint;
      _repr->definition = def;
    }
      break;

    case 50: case 51: {
      Spectral* def = new Spectral;
      _repr->definition = def;
    }
      break;

    case 200: {
      LevelValues* def = new LevelValues;
      _repr->definition = def;
    }
      break;

    default:
      error_log<<"Representation with template number"<<templ<<"doesn't supported";
    }
  } else if (version == 1) {
    switch (templ) {
    case 0: {
      SimplePoint* def = new SimplePoint;
      _repr->definition = def;
    }
      break;
    default:
      error_log<<"Representation with template number"<<templ<<"doesn't supported";
    }
  }

  _repr->type = templ;
  return _repr->definition;
}
