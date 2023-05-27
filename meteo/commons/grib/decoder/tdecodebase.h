#ifndef TGRIBBASE_H
#define TGRIBBASE_H

#include <cross-commons/debug/tlog.h>
#include <sql/psql/psql.h>

#include <meteo/commons/grib/parser/tgridgrib.h>
#include <meteo/commons/grib/parser/tproductgrib.h>

#include <stdint.h>

#define METEO_APP_NAME "meteo" //!< это д.б. в настройках

namespace google {
  namespace protobuf {
    class Reflection;
    class Message;
  }
}

namespace grib {
  class TGribData;
}


class QFile;
class TReprGrib;
class TProductGrib;

typedef bool (*ReprFunc)(float* dst, uint32_t dstSize, uint8_t* src, uint32_t size, google::protobuf::Message* def);

class TDecodeGribBase
{
public:
  TDecodeGribBase();
  virtual ~TDecodeGribBase();

  virtual bool checkFormat(uint64_t fileIdx, QFile* file) = 0;
  virtual uint16_t getNextSection() = 0;

  virtual bool checkFormat(const QByteArray& ba, int64_t* idx) = 0;
  virtual uint16_t getNextSection(const QByteArray& ba, int64_t* idx) = 0;

  virtual bool parseSection1() { trc<<"Not realised\n"; return false; }
  virtual bool parseSection2() { trc<<"Not realised\n"; return false; }
  virtual bool parseSection3() { trc<<"Not realised\n"; return false; }
  virtual bool parseSection4() { trc<<"Not realised\n"; return false; }
  virtual bool parseSection5() { trc<<"Not realised\n"; return false; }
  virtual bool parseSection6() { trc<<"Not realised\n"; return false; }
  virtual bool parseSection7() { trc<<"Not realised\n"; return false; }

  virtual bool isValidGrib() = 0;  

  virtual QString gridFile() = 0;
  virtual QString representFile() = 0;
  virtual QString productFile() = 0;

  virtual uint64_t dataSize() const = 0;
  virtual const float* data() const = 0;
  virtual int edition() = 0;
  virtual const grib::TGribData* gribData() const = 0;

  // virtual uint32_t saveInDb() =0;
  // virtual bool readFromDb(uint64_t id) = 0;

  const grib::TGridDefinition* gridDefinition() const { return _grid->definition(); }
  const grib::TProductDefinition* prodDefinition() const {return _prod->definition(); }

protected:

  bool parseRepr(const char* data, uint64_t size, uint16_t templ);
  bool restoreData(float* dst, uint64_t dstSize, char* src, uint32_t size);

  bool parseGrid(std::string* res, const char* data, uint64_t size, uint16_t templ);
  bool parseGridFromStr(const std::string& src, uint32_t type, int version);

  grib::TProductDefinition* parseProd(std::string* res, const char* data, uint64_t size, uint16_t templ);
  bool parseProdFromStr(const std::string& src, uint32_t type, int version);

private:

  bool addUnpackFunc(int type, ReprFunc func);

private:
  TProductGrib* _prod;
  TGridGrib* _grid;
  TReprGrib*  _repr;
  QMap<int, ReprFunc> _reprList;
};

#endif
