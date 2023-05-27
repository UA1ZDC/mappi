#include "tgrib1_decode.h"

#include <meteo/commons/proto/tgribformat.pb.h>
#include <meteo/commons/grib/parser/tgribformat.h>
#include <meteo/commons/grib/parser/tproductgrib.h>
#include <meteo/commons/grib/parser/treprgrib.h>
#include <meteo/commons/grib/parser/tgridgrib.h>

#include <commons/mathtools/mnmath.h>
#include <cross-commons/debug/tlog.h>

#include <qfile.h>

#define LENGTH_SEC0 8

#define GRID_SECT_bm 0x80
#define BITMAP_SECT_bm 0x40

//#define GRIB1_FILENAME "grib1_db.xml"

#define GRIB1_REPR_FILE    "grib1_represent.xml"
#define GRIB1_GRID_FILE    "grib1_grid.xml"
//#define GRIB1_PRODUCT_FILE "grib1_product.xml"

using namespace grib;

TDecodeGrib1::TDecodeGrib1( meteo::Psql* ):
_grib(0)
{
  _fileIdx = 0;
  _file = 0;
  _size = 0;
  _cursect = 0;
  _sectMask = 0;
  _grib = new(std::nothrow) TGribData;
  _validGrib = false;
  _dataUnused = 0;
}

TDecodeGrib1::~TDecodeGrib1()
{
  if (_grib) delete _grib; 
  _grib = 0;
}

//--- settings

// QString TDecodeGrib1::dbRuleFile()
// {
//   return GRIB1_FILENAME;
// }

QString TDecodeGrib1::representFile()
{
  return MnCommon::etcPath(METEO_APP_NAME) + GRIB1_REPR_FILE;
}

QString TDecodeGrib1::gridFile()
{
  return MnCommon::etcPath(METEO_APP_NAME) + GRIB1_GRID_FILE;
}

QString TDecodeGrib1::productFile()
{
  return QString();
}

//--- parse

bool TDecodeGrib1::checkFormat(uint64_t fileIdx, QFile* file)
{
  _fileIdx = fileIdx;
  _file = file;

  _data = _file->read(LENGTH_SEC0);

  char2dec(_data.mid(4, 3), 3, &_size);
  //  debug_log<<"grib1 size="<<_size;

  if (_size == 0) return false;

  _file->seek(_fileIdx + _size - 4);
  if ( _file->read(4) != "7777" ) {
    return false;    
  }
  
  _fileIdx += LENGTH_SEC0;
  _file->seek(_fileIdx);
  
  //  var(_fileIdx);

  return true;
}

bool TDecodeGrib1::checkFormat(const QByteArray& arr, int64_t* idx)
{
  char2dec(arr.mid(*idx + 4, 3), 3, &_size);
  if (_size == 0) return false;

  QByteArray end = arr.mid(*idx + _size - 4, 4);
  if (end != "7777") {
    return false;
  }

  *idx += LENGTH_SEC0;

  //  var(*idx);

  return true;
}

uint16_t TDecodeGrib1::getNextSection()
{
  //debug_log<<"pos="<<_file->pos()<<" size="<<_size<<" fsize="<<_file->size();

  if (_file->pos() - (_fileIdx-LENGTH_SEC0) + 5 >= (uint64_t)_size) {
    _file->seek(_fileIdx - LENGTH_SEC0 + _size - 4);//to the end of grib2 data
    return 0;
  }
  
  _data = _file->read(3);
  uint len;
  char2dec(_data.data(), 3, &len);
  
  _data.append(_file->read(len - 3));

  switch (_cursect) {
  case 0: case 3:
    ++_cursect;
    break;
  case 1:
    if (_sectMask & GRID_SECT_bm) {
      _cursect = 2;
      break;
    } 
  case 2:
    if (_sectMask & BITMAP_SECT_bm) {
      _cursect = 3;
    } else {
      _cursect = 4;
    }
    break;
  case 4:
    _cursect = 0;
  };

  //  debug_log<<"\nsect="<<_cursect<<" len="<<len;
  return _cursect;
}

uint16_t TDecodeGrib1::getNextSection(const QByteArray& arr, int64_t* idx)
{
  //debug_log<<"pos="<<*idx<<" size="<<_size<<" asize="<<arr.size();

  if (*idx + 5 >= (int64_t)_size) return 0;

  _data = arr.mid(*idx, 3);
  uint len;
  char2dec(_data.data(), 3, &len);

  //var(len);

  if (*idx + len >= (int64_t)_size) {
    return 0;
  }

  _data.append(arr.mid(*idx + 3, len - 3));
  *idx += len;

  switch (_cursect) {
  case 0: case 3:
    ++_cursect;
    break;
  case 1:
    if (_sectMask & GRID_SECT_bm) {
      _cursect = 2;
      break;
    }
  case 2:
    if (_sectMask & BITMAP_SECT_bm) {
      _cursect = 3;
    } else {
      _cursect = 4;
    }
    break;
  case 4:
    _cursect = 0;
  };

  //  debug_log<<"\nsect="<<_cursect<<" len="<<len;
  return _cursect;

}

bool TDecodeGrib1::parseSection1() 
{ 
  if (!_grib) return false;
  //  debug_log<<"Parse 1 size="<<_data.size();
  _validGrib = false;

  TProductData1* p = _grib->mutable_product1();
  
  _grib->set_mastertable(_data[3]);
  _grib->set_center(uchar(_data[4]));
  _grib->mutable_product1()->set_processtype(uchar(_data[5]));

  _grib->mutable_grid()->set_type(uchar(_data[6]));

  _sectMask = uchar(_data[7]);
  p->set_number(uchar(_data[8]));
  uchar leveltype = uchar(_data[9]);
  switch (leveltype) {
  case 102:
    leveltype = 101;
    break;
  case 105:
    leveltype = 103;
    break;
  case 111:
    leveltype = 106;
    break;
  default: {}
  }
  p->set_leveltype(leveltype);
  uint32_t val;
  char2dec(_data.data() + 10, 2, &(val));
  p->set_levelvalue(val);
  QDateTime dt(QDate((uchar(_data[24]) - 1)*100 + uchar(_data[12]), uchar(_data[13]), 
		     uchar(_data[14])), QTime(uchar(_data[15]), uchar(_data[16])), Qt::UTC);
  _grib->set_dt(dt.toString(Qt::ISODate).toStdString());

  uint32_t unitsIdx = uchar(_data[17]);
  const uint64_t factor[9] = {60, 3600, 86400, 2592000, 31536000, 315360000, 946080000,  Q_UINT64_C(31536000000), 1};
  if (unitsIdx >= sizeof(factor)/sizeof(factor[0])) unitsIdx = sizeof(factor)/sizeof(factor[0]) - 1;

  int32_t time_range = uchar(_data[20]);
  if (time_range == 10) { 
    uint16_t p1 = (uchar(_data[18]) << 8) + uchar(_data[19]);
    p->set_p1(factor[unitsIdx]*p1);
    p->set_p2(0);
  } else {
    p->set_p1(factor[unitsIdx]*uchar(_data[18]));
    p->set_p2(factor[unitsIdx]*uchar(_data[19]));
  }
  switch (time_range) {
  case 0: case 1: case 2: case 10: //анализ либо прогноз
    p->set_timerange(255);
    break;
  case 3: case 6: case 7: //среднее время
    p->set_timerange(0);
    break;
  case 4: //аккумуляция
    p->set_timerange(1);
    break;
  default:
    p->set_timerange(-time_range);
  }

  //dt1
  if (time_range != 6 && time_range != 7) {
    p->set_dt1(dt.addSecs(p->p1()).toString(Qt::ISODate).toStdString());
    if (time_range == 4) {
      p->set_forecasttime(p->p2());
    } else {
      p->set_forecasttime(p->p1());
    }
  } else {
    p->set_dt1(dt.addSecs(-p->p1()).toString(Qt::ISODate).toStdString());
    p->set_forecasttime(-p->p1());
  }
  
  //dt2
  if (time_range == 0 || time_range == 1 || time_range == 10) {
    p->set_dt2(p->dt1());
  } else {
    if (time_range == 6) {
      p->set_dt2(dt.addSecs(-p->p2()).toString(Qt::ISODate).toStdString());
    } else {
      p->set_dt2(dt.addSecs(p->p2()).toString(Qt::ISODate).toStdString());
    }   
  }

  char2dec(_data.data() + 21, 2, &val);
  p->set_numavg(val);
  p->set_absentavg(uchar(_data[23]));
  _grib->set_subcenter(uchar(_data[25]));
  
  _repr.append(_data.data() + 26, 2);

  //  debug_log<<"_sectMask="<<_sectMask<<uchar(_data[6]);
  return true; 
}

bool TDecodeGrib1::parseSection2() 
{ 
  //  debug_log<<"Parse 2 size="<<_data.size();

  TGridData* g = _grib->mutable_grid();

  uchar nv = _data[3];
  uchar idx = _data[4];
  uint32_t crc = 0;

  if (idx != 255) {
    idx -= 1;
    if (nv != 0) {
      float* vertic = new(std::nothrow) float[nv];
      if (!vertic) return false;
      for (uint i=0; i< nv; i++) {
	vertic[i] = grib::ieee2double(_data.data() + idx + i*4, 1);
      }
      g->set_verticcoords(vertic, nv*sizeof(float));
      delete[] vertic;
      idx += nv*4;
      crc = 0;//MnMath::makeCrc(_data.data() + idx, nv*sizeof(float));
      g->set_verticcrc(crc);
    } 
    g->set_points(_data.data() + idx, _data.size() - idx);
    crc = 0;//MnMath::makeCrc(_data.data() + idx, _data.size() - idx);
    g->set_pointscrc(crc);
  } else {
    idx = _data.size() - 1;
  }

  std::string gridStr;
  g->set_type(uchar(_data[5]));
  if (!parseGrid(&gridStr, _data.data() + 6, idx - 6 + 1, g->type())) {
    return false;
  }
  g->set_data(gridStr.data(), gridStr.size());
  crc = 0; //MnMath::makeCrc(gridStr.data(), gridStr.size());
  g->set_datacrc(crc);


  google::protobuf::Message* msg = gridDefinition()->definition;
  const google::protobuf::Reflection* refl = msg->GetReflection();
  const google::protobuf::FieldDescriptor* field = msg->GetDescriptor()->FindFieldByName("di");
  if (nullptr != field) {
    g->set_di(refl->GetInt32(*msg, field));
  }
  field = msg->GetDescriptor()->FindFieldByName("dj");
  if (nullptr != field) {
    g->set_dj(refl->GetInt32(*msg, field));
  }
  
  return true; 
}

bool TDecodeGrib1::parseSection3() 
{
  //  debug_log<<"Parse 3 size="<<_data.size();

  TBitmapData* p = _grib->mutable_bitmap();
  p->set_unused(uchar(_data[3]));
  uint16_t val;
  char2dec(_data.data() + 4, 2, &val);
  p->set_type(val);
  
  if (val == 0) {
    p->set_bitmap(_data.data() + 6, _data.size() - 6);    
  }

  uint32_t crc = 0; //MnMath::makeCrc(_data.data() + 6, _data.size() - 6);
  p->set_crc(crc);

  return true; 
}

bool TDecodeGrib1::parseSection4() 
{ 
  //  debug_log<<"Parse 4 size="<<_data.size();
  _dataUnused = uchar(_data[3]) & 0xf;

  uint8_t type = uchar(_data[3]) & 0xd0; //TODO типы привести к grib2
  if (type & 0x10) {
    type += (_data[13] & 0xf0) >> 4;
  }

  _repr.append(uchar(_data[3])&0x20);

  uint16_t offset = 11;
  uint16_t reprType = 0;
  if ((type & 0xC0) == 0x80) { //простая сферическая
    reprType = 50;
    offset = 15;
  } else if ((type & 0xC0) == 0x40) { //сложная
    char2dec(_data.data() + 11, 2, &offset);
    offset -= 1;
    reprType = 49152;
  } else if ((type & 0xC0) == 0xC0) { //сложная сфер
    offset = 18;
    reprType = 51;
  }

  _repr.append(_data.data() + 4, offset - 4);
  //  debug_log<<"reprType"<<reprType<<"type="<<type;


  //  setUpReprParser(_repr.data(), _repr.size(), reprType);
  if (!parseRepr(_repr.data(), _repr.size(), reprType)) {
    return false;
  }

  if (0 == _data[10]) {
    error_log << QObject::tr("Количество бит величины указано равным 0");
    return false;
  }
  
  uint64_t resSize = ((_data.size() - offset)*8 - _dataUnused) / _data[10];
  if (resSize == 0) return false;
  
  float* resData = new(std::nothrow) float[resSize];
  if (!resData) return false;

  // printvar(offset);
  // printvar(reprType);
  // printvar(type);
  // for (uint i=0; i<20; i++) {
  //   printf("%x%x\n", (uchar)(_data.data()[offset + 2*i]), (uchar)(_data.data()[offset + 2*i + 1]));
  // }

  _validGrib = restoreData(resData, resSize, _data.data() + offset, _data.size() - offset);

  //---- debug
  // for (uint i=0; i<10; i++) {
  //   printf("%3d %f\n", i, resData[i]);
  // }
  // float min = 999999999999;
  // float max = 0;
  // for (uint64_t i=0; i<resSize; i++) {
  //   if (resData[i] > max && resData[i] < 99999999) max = resData[i];
  //   if (resData[i] < min) min = resData[i];
  // }
  // debug_log<<QObject::tr("Мин. и макс. значения в данных:");
  // debug_log<<"min="<<min<<"max="<<max;
  //----

  if (_validGrib) {
    _grib->set_data(resData, resSize*sizeof(float));
    uint32_t crc = 0; //MnMath::makeCrc(resData, resSize*sizeof(float));
    _grib->set_datacrc(crc);
  }

  delete[] resData;


   return _validGrib; 
}

//---------- save


// uint32_t TDecodeGrib1::saveInDb()
// {
//   return TGribDb::saveInDb(GRIB1_FILENAME);
// }

// bool TDecodeGrib1::readFromDb(uint64_t id)
// {
//   bool ok = TGribDb::readFromDb(id);
//   if (!ok) return false;

//   parseGridFromStr(_grib->grid().data(), _grib->grid().type(), 1);
//   return true;
// }


google::protobuf::Message* TDecodeGrib1::getGrid()
{
  return _grib->mutable_grid(); 
}

bool TDecodeGrib1::existBitmap()
{
  return _grib->has_bitmap();
}

google::protobuf::Message* TDecodeGrib1::getBitmap()
{
  return _grib->mutable_bitmap(); 
}

google::protobuf::Message* TDecodeGrib1::getProduct()
{
  return _grib->mutable_product1(); 
}

google::protobuf::Message* TDecodeGrib1::getCommonData()
{
  return _grib;
}

//-----

uint64_t TDecodeGrib1::dataSize() const
{
  if (!_validGrib) return 0;
  return _grib->data().size();
}

const float* TDecodeGrib1::data()  const
{
  if (!_validGrib) return 0;
  return reinterpret_cast<const float*>(_grib->data().data());
}

// uint64_t TDecodeGrib1::resultSize() const
// {
//   if (!_validGrib) return 0;
//   return _grib->mutable_product1()->mutable_data()->size();
// }


