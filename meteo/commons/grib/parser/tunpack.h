#ifndef TUNPACK_H
#define TUNPACK_H

#include <stdint.h>

namespace google {
  namespace protobuf {
     class Message;
  }
}

namespace TRepresent {
  class PointData;
  class ComplexPoint;
}

//функции для распаковки
namespace grib {
  bool unpackBits(float* dst, uint32_t dstSize, uint8_t* src, uint32_t cnt, TRepresent::PointData* base);
  bool unpackBits(uint64_t* dst, uint8_t* src, uint32_t bits, uint32_t cnt, uint32_t ref = 0, uint16_t scale = 1);
  uint64_t unpackVal(uint8_t* src, uint32_t bits, uint8_t* shiftBit, uint32_t* shiftByte);

 bool SimplePoint(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def);
 bool MatrixPoint(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def);
 bool ComplexPoint(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def);
 bool ComplexSpDiffPoint(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def);
 bool IeeePoint(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def);
 bool JpegPoint(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def);
 bool PngPoint(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def);
 bool SimpleSpectral(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def);
 bool ComplexSpectral(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def);
 bool SimpleLogPoint(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def);
 bool LevelValues(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def);

  bool fillMissing(uint32_t gWidth, uint32_t gRefs, uint64_t* val, const TRepresent::ComplexPoint* cp, uint16_t bits);
}

#endif
