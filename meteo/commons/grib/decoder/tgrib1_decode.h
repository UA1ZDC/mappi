#ifndef TRGIB1_DECODE
#define TRGIB1_DECODE

#include "tdecodebase.h"

#include <stdint.h>

namespace grib {
  class TGribData;
}

class TDecodeGrib1 : public TDecodeGribBase {
public:
  TDecodeGrib1(meteo::Psql* db);
  ~TDecodeGrib1();

  bool checkFormat(uint64_t fileIdx, QFile* file);
  uint16_t getNextSection();

  bool checkFormat(const QByteArray& ba, int64_t* idx);
  uint16_t getNextSection(const QByteArray& ba, int64_t* idx);


  // uint32_t saveInDb();
  // bool readFromDb(uint64_t id);

  bool parseSection1();
  bool parseSection2();
  bool parseSection3();
  bool parseSection4();

  virtual uint64_t dataSize() const;
  const float* data()  const;
  int edition() { return 1; };
  bool isValidGrib() {return _validGrib;}
  const grib::TGribData* gribData() const { if (_validGrib) return _grib; return 0;}

protected:
  bool isValidData();
  google::protobuf::Message* getGrid();
  bool existBitmap();
  google::protobuf::Message* getBitmap();
  google::protobuf::Message* getProduct();
  google::protobuf::Message* getCommonData();

  //  bool setUpGridParser(TGridGrib2*  gridParser);  
  //  bool setUpProdParser(TProductGrib* prodParser);

  void setValidGrib(bool valid) { _validGrib = valid; }
  //  QString dbRuleFile();
  QString gridFile();
  QString representFile();
  QString productFile();

private:
  QByteArray _data;
  uint _dataUnused;
  grib::TGribData* _grib;
  bool _validGrib;
  QByteArray _repr; //0,1 - d, 2-тип (float/int) , 3-.. - с 5-го бита раздела 4

  QFile* _file;
  uint64_t _fileIdx;
  uint64_t _size; //!< grib msg size
  uchar _sectMask;
  uchar _cursect;
};

#endif
