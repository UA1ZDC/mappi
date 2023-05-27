#ifndef TGRIB2_GRID_H
#define TGRIB2_GRID_H

#include <stdint.h>

#include <meteo/commons/grib/parser/tgribparser.h>
#include <meteo/commons/grib/tgribdefinition.h>
#include <meteo/commons/proto/surface.pb.h>

namespace TGrid {
  class Shape;

  //! Для определения общей грани сеток
  enum GridEdge {
    NoEdge    = 0,
    LeftEdge  = 1,
    TopEdge   = 2,
    RightEdge = 3,
    BottomEdge = 4
  };
}

//! парсинг сетки GRIB из БД, описание в xml и proto
class TGridGrib : public TGribParser {
public:

  TGridGrib();
  ~TGridGrib();

  grib::TGridDefinition* definition() {return _grid;}

  static void getLatLon(const TGrid::LatLon& msg, float* latMin, float* latMax, float* lonMin, float* lonMax);
  static bool isEqualGrid(const meteo::surf::GribData& grib1, const meteo::surf::GribData& grib2);
  static bool isIntersectGrid(const meteo::surf::GribData& grib1, const meteo::surf::GribData& grib2);
  static TGrid::GridEdge getCommonEdge(const meteo::surf::GribData& grib1, const meteo::surf::GribData& grib2);
  
  
protected:
  google::protobuf::Message* createDefinition(uint16_t templ, int version);
  void parseCustom(google::protobuf::Message* msg, const uint8_t* data, uint32_t dataSize, QDomElement& node, int32_t startIdx);

private:
  void setAxis(const uint8_t* data, uint32_t dataSize, QDomElement& node, int32_t startIdx);
  
  void setShape(float r);
  void setShape(float minor, float major);
 
private:
  grib::TGridDefinition* _grid;
  TGrid::Shape* _shape;
};

#endif
