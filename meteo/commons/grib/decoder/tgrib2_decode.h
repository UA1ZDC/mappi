#ifndef TGRIB2_DECODE_H
#define TGRIB2_DECODE_H

#include "tdecodebase.h"

#include <qbytearray.h>

#include <postgres_ext.h>

#include <stdint.h>


class QString;
namespace grib {
  class TGribData;
  class TGridData;
  class TProductData;
}

//! read grib2 and save in db
class TDecodeGrib2 : public TDecodeGribBase {
public:
  TDecodeGrib2(meteo::Psql* db);
  virtual ~TDecodeGrib2();

  bool checkFormat(uint64_t fileIdx, QFile* file);
  ushort getNextSection();

  bool checkFormat(const QByteArray& ba, int64_t* idx);
  uint16_t getNextSection(const QByteArray& ba, int64_t* idx);

  bool parseSection1();
  bool parseSection2();
  bool parseSection3();
  bool parseSection4();
  bool parseSection5();
  bool parseSection6();
  bool parseSection7();

  // bool readFromDb(uint64_t id);
  // uint32_t saveInDb();

  uint64_t dataSize() const;
  const float* data()  const;
  int edition() { return 2; };
  bool isValidGrib() {return _validGrib;}
  const grib::TGribData* gribData() const { if (_validGrib) return _grib; return 0; }

protected:

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

  void clear();

  uint64_t saveGrid();
  uint64_t findGridId();
  void deleteGrid(uint32_t gridId);
  uint64_t findBitmapId();
  uint64_t saveBitmap();
  void deleteBitmap(uint64_t id);
  uint64_t findProductId();
  uint64_t saveProduct();
  void deleteProduct(uint64_t id);
  uint64_t findGribId();
  uint64_t findGribId(uint64_t gridId, uint64_t bitmapId, uint64_t productId);
  uint64_t saveGribCommon(uint64_t gridId, uint64_t bitmapId, uint64_t productId);
  void deleteGribCommon(uint64_t id);
  uint64_t findIdInDb(const QString& query);  

  bool setProductData(grib::TProductDefinition* prod);

private:
  QFile* _file;
  QByteArray _data;
  uint64_t _size; //!< общая длина grib
  uint64_t _fileIdx;

  bool _validGrib;

  grib::TGribData* _grib;

  uint32_t _resNumber;
  uint64_t _reprDataSize;
};

#endif
