#include "tdecodebase.h"
//#include "tgribxml.h"

#include <meteo/commons/grib/parser/treprgrib.h>
#include <meteo/commons/grib/parser/tgridgrib.h>
#include <meteo/commons/grib/parser/tunpack.h>

#include <qlist.h>
#include <cross-commons/debug/tlog.h>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>


using namespace google::protobuf;
using namespace grib;

TDecodeGribBase::TDecodeGribBase()
{
  _repr = new TReprGrib;
  _grid = new TGridGrib;
  _prod = new TProductGrib;

  addUnpackFunc(0, grib::SimplePoint);
  addUnpackFunc(1, grib::MatrixPoint);
  addUnpackFunc(2, grib::ComplexPoint);
  addUnpackFunc(3, grib::ComplexSpDiffPoint);
  addUnpackFunc(4, grib::IeeePoint);
  addUnpackFunc(40, grib::JpegPoint);
  addUnpackFunc(41, grib::PngPoint);
  addUnpackFunc(50, grib::SimpleSpectral);
  addUnpackFunc(51, grib::ComplexSpectral);
  addUnpackFunc(61, grib::SimpleLogPoint);
  addUnpackFunc(200, grib::LevelValues);
}

TDecodeGribBase::~TDecodeGribBase()
{
  if (_repr) delete _repr;
  _repr = 0;
  if (_grid) delete _grid;
  _grid = 0;
  if (_prod) delete _prod;
  _prod = 0;
}


bool TDecodeGribBase::addUnpackFunc(int type, ReprFunc func)
{
  if (_reprList.contains(type)) {
    error_log<<"Function for template"<<type<<"already registered";
    return false;
  }

  _reprList.insert(type, func);
  return true;
}

bool TDecodeGribBase::parseRepr(const char* data, uint64_t size, uint16_t templ)
{
  _repr->setup(reinterpret_cast<const uchar*>(data), size, representFile(), edition(), templ);
  return _repr->parse();
}

bool TDecodeGribBase::restoreData(float* dst, uint64_t dstSize, char* src, uint32_t size)
{
  bool ok = false;

  if (size == 0) {
    warning_log << QObject::tr("Нет данных");
    return true;
  }

  if (_reprList.contains(_repr->type())) {
    ok = _reprList[_repr->type()](dst, dstSize, reinterpret_cast<uchar*>(src), size, _repr->definition());
  }

  return ok;
}

bool TDecodeGribBase::parseGrid(std::string* res, const char* data, uint64_t size, uint16_t templ)
{
  bool ok = false;
  _grid->setup(reinterpret_cast<const uchar*>(data), size, gridFile(), edition(), templ);
  if (_grid->parse() && _grid->definition()->definition->IsInitialized()) {
    ok = _grid->definition()->definition->SerializeToString(res);
  } else {
    error_log << QObject::tr("Ошибка формирования данных сетки. Номер шаблона %1").arg(templ);
  }
  return ok;
}

bool TDecodeGribBase::parseGridFromStr(const std::string& src, uint32_t type, int version)
{
  return _grid->createFromString(type, version, src);
}

grib::TProductDefinition* TDecodeGribBase::parseProd(std::string* res, const char* data, uint64_t size, uint16_t templ)
{
  _prod->setup(reinterpret_cast<const uchar*>(data), size, productFile(), edition(), templ);
  if (_prod->parse() && _prod->definition()->definition->IsInitialized()) {
    _prod->definition()->definition->SerializeToString(res);
  } else {
    error_log << QObject::tr("Ошибка формирования данных продукта. Номер шаблона %1").arg(templ);
  }
  return _prod->definition();
}

bool TDecodeGribBase::parseProdFromStr(const std::string& src, uint32_t type, int version)
{
  return _prod->createFromString(type, version, src);
}
