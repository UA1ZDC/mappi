#include "tgridgrib.h"
#include "tgrid.pb.h"

#include <qdom.h>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/util/message_differencer.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <commons/mathtools/mnmath.h>

#include "tgribformat.h"
#include "tgribparser.h"

using namespace google::protobuf;
using namespace TGrid;
using namespace grib;

//------ debug
// void printGrid(uchar* data, uint64_t size)
// {
//   printf("Print Grid\n");
//   unsigned startIdx = 15;

//   for (uint64_t i=0; i< size; i++) {
//     printf("%2llu: %2x %3d\n", i+startIdx, data[i], data[i]);
//   }
// }
//------

TGridGrib::TGridGrib()
{
  _grid = new TGridDefinition;
  _shape = 0;
}

TGridGrib::~TGridGrib()
{
  if (_grid) {
    delete _grid;
  }
  _grid = 0;
  _shape = 0;
}

void TGridGrib::parseCustom(google::protobuf::Message* , const uint8_t* data, uint32_t size, QDomElement& node, int32_t startIdx)
{
  if (node.attribute("name") == "shape") {
    setAxis(data, size, node, startIdx);
  }
}

void TGridGrib::setAxis(const uchar* data, uint32_t /*dataSize*/, QDomElement& node, int startIdx)
{
  if (!_shape) return;

  int idx = node.attribute("idx").toUInt() + startIdx;
  if (idx < 0) return;

  uchar grib_Shape = data[idx];

  switch (grib_Shape) {
  case 0:
    setShape(6367470.);
    break;
  case 1:
    setShape(calcValueFactor(data, node.firstChildElement("radius"), startIdx));
    break;
  case 2:
    setShape(6378160., 6356775.);
    break;
  case 3:
    setShape(calcValueFactor(data, node.firstChildElement("major"), startIdx)*1000, 
	     calcValueFactor(data, node.firstChildElement("minor"), startIdx)*1000);
    break;
  case 4:
    setShape(6378137., 6356752.314);
    break;
  case 5:
    setShape(6378137., 6356752.3142);
    break;
  case 6:
    setShape(6371229.);
    break;
  case 7:
    setShape(calcValueFactor(data, node.firstChildElement("major"), startIdx),
	     calcValueFactor(data, node.firstChildElement("minor"), startIdx));
    break;
  case 8:  
    setShape(6371200.);
    //TODO horizontal datum of the resulting Latitude/Longitude field is the WGS84 reference frame
    break;
  case 9:
    //TODO OSGB 1936 Datum, using the Airy_1830 Spheroid, the Greenwich meridian as 0 Longitude, the Newlyn datum as mean sea level, 0 height.
    break;
  }
}

void TGridGrib::setShape(float r)
{
  _shape->set_type(Shape::Spherical);
  _shape->set_radius(r);
}

void TGridGrib::setShape(float major, float minor)
{
  _shape->set_type(Shape::Oblate);
  _shape->set_majoraxis(major);
  _shape->set_minoraxis(minor); 
}


//! Создание структуры для описания сетки
/*! 
  \param templ номер шаблона
  \param version номер версии GRIB
  \return созданная структура
*/
google::protobuf::Message* TGridGrib::createDefinition(uint16_t templ, int version)
{
  if (_grid->definition) delete _grid->definition;
  _grid->definition = 0;

  if (version == 2) {
    switch (templ) {
    case 0: case 1: case 2: case 3: case 4: case 5: 
    case 20: case 30: case 31: 
    case 40: case 41: case 42: case 43: 
    case 110: {
      LatLon* def = new LatLon;
      _grid->definition = def;
      _shape = def->mutable_shape();
    }
      break;
    case 10: case 12: {
      Mercator* def = new Mercator;
      _grid->definition = def;
      _shape = def->mutable_shape();
    }
      break;
    case 50: case 51: case 52: case 53: {
      SpherHarmCoef* def = new SpherHarmCoef;
      _grid->definition = def;
    }
      break;
    case 90: {
      SpaceView* def = new SpaceView;
      _grid->definition = def;
      _shape = def->mutable_shape();
    }
      break;
    case 100: {
      Icosahedron* def = new Icosahedron;
      _grid->definition = def;
    }
      break;
    case 101: {
      Unstructured* def = new Unstructured;
      _grid->definition = def;
    }
      break;
    default:
      error_log << QObject::tr("Сетка с номером шаблона %1 не поддерживается").arg(templ);
    }
  } else if (version == 1) {
    switch (templ) {
    case 0:  case 3: case 4: case 5: case 8: case 10: 
    case 13: case 14: case 20: case 24: case 30: case 34: {
      LatLon* def = new LatLon;
      _grid->definition = def;
    }
      break;
    case 1: {
      Mercator* def = new Mercator;
      _grid->definition = def;
      _shape = def->mutable_shape();
    }
      break;
    case 50: case 60: case 70: case 80: {
      SpherHarmCoef* def = new SpherHarmCoef;
      _grid->definition = def;
    }
      break;
    case 90: {
      SpaceView* def = new SpaceView;
      _grid->definition = def;
    }
      break;
    default:
      error_log << QObject::tr("Сетка с номером шаблона %1 не поддерживается").arg(templ);
    }    
  } else {
    error_log<<"Grib version "<<version<<"doesn't supported";
  }

  _grid->type = templ;
  return _grid->definition;
}

//! Сравнение двух сеток
bool TGridGrib::isEqualGrid(const meteo::surf::GribData& grib1, const meteo::surf::GribData& grib2)
{
  google::protobuf::util::MessageDifferencer diff;
  const FieldDescriptor * valueField = grib1.GetDescriptor()->FindFieldByName("value");
  const FieldDescriptor * descrField = grib1.GetDescriptor()->FindFieldByName("descr");
  
  diff.IgnoreField(valueField);
  diff.IgnoreField(descrField);
  return diff.Compare(grib1, grib2);
}

//нижняя широта, верхняя широта, левая долгота, правая долгота
void TGridGrib::getLatLon(const TGrid::LatLon& msg, float* latMin, float* latMax, float* lonMin, float* lonMax)
{
  double subdivision = 1e-3;
  double subdiv_max  = 1e6;
  if (msg.lo1() > subdiv_max ||
      msg.lo2() > subdiv_max ||
      msg.la1() > subdiv_max ||
      msg.la2() > subdiv_max) {
    subdivision = 1e-6;   
  }

  if (msg.la1() < msg.la2()) {
    *latMin = msg.la1() * subdivision;
    *latMax = msg.la2() * subdivision;    
  } else {
    *latMin = msg.la2() * subdivision;
    *latMax = msg.la1() * subdivision;    
  }

  bool isLonAdd = (msg.scanmode() & 0x80) == 0;
  if(isLonAdd) {
    *lonMin = msg.lo1() * subdivision;
    *lonMax = msg.lo2() * subdivision;
  } else {
    *lonMin = msg.lo2() * subdivision;
    *lonMax = msg.lo1() * subdivision;
  }

  *lonMin = MnMath::M0To360(*lonMin);
  if (*lonMax < 0 || *lonMax > 360) { // =360 может быть крайней правой точкой
    *lonMax = MnMath::M0To360(*lonMax);
  }
}

//! Проверка на пересечение
bool TGridGrib::isIntersectGrid(const meteo::surf::GribData& grib1, const meteo::surf::GribData& grib2)
{
  float lat1Min, lat1Max;
  float lon1Min, lon1Max;

  float lat2Min, lat2Max;
  float lon2Min, lon2Max;

  getLatLon(grib1.grid_def(), &lat1Min, &lat1Max, &lon1Min, &lon1Max);
  getLatLon(grib2.grid_def(), &lat2Min, &lat2Max, &lon2Min, &lon2Max);

  //сетки одна над другой
  if (lat2Max < lat1Min || MnMath::isEqual(lat2Max, lat1Min) ||
      lat1Max < lat2Min || MnMath::isEqual(lat1Max, lat2Min)) {
    return false;
  }

  //нет перехода через 0
  if (lon1Min < lon1Max && lon2Min < lon2Max) {
    if (lon2Max < lon1Min || MnMath::isEqual(lon2Max, lon1Min) ||
	lon1Max < lon2Min || MnMath::isEqual(lon1Max, lon2Min)) {
      //сетки сбоку друг от друга
      return false;
    } else {
      return true;
    }
  }

  //обе пересекают 0, т.е. как минимум общая точка есть
  if (lon1Min > lon1Max && lon2Min > lon2Max) {
    return true;
  }

  //одна из сеток имеет переход через 0
  if (lon2Max < lon2Min) { //2я
    if (lon2Max >= lon1Min || lon2Min <= lon1Max) {
      return true;
    }
  }
  
  if (lon1Max < lon1Min) { //1я
    if (lon1Max >= lon2Min || lon1Min <= lon2Max) {
      return true;
    }
  }
  
  return false;
}

//! return - общая грань у grib1 с grib2
TGrid::GridEdge TGridGrib::getCommonEdge(const meteo::surf::GribData& grib1,
					 const meteo::surf::GribData& grib2)
{
  float lat1Min, lat1Max;
  float lon1Min, lon1Max;

  float lat2Min, lat2Max;
  float lon2Min, lon2Max;

  getLatLon(grib1.grid_def(), &lat1Min, &lat1Max, &lon1Min, &lon1Max);
  getLatLon(grib2.grid_def(), &lat2Min, &lat2Max, &lon2Min, &lon2Max);
  
  //сетки одна над другой
  if (MnMath::isEqual(lat2Max, lat1Min)) {
    return TGrid::BottomEdge;
  }
  
  if (MnMath::isEqual(lat1Max, lat2Min)) {
    return TGrid::TopEdge;
  }

  if (MnMath::isEqual(lon2Max, lon1Min)) {
    return TGrid::LeftEdge;
  }
  
  if (MnMath::isEqual(lon1Max, lon2Min)) {
    return TGrid::RightEdge;
  }

  return TGrid::NoEdge;
}
