#include "tgrib2_decode.h"

#include <meteo/commons/proto/tproduct.pb.h>
#include <meteo/commons/proto/tgribformat.pb.h>
#include <meteo/commons/grib/parser/tgribformat.h>
#include <meteo/commons/grib/parser/tproductgrib.h>
#include <meteo/commons/grib/parser/treprgrib.h>
#include <meteo/commons/grib/parser/tgridgrib.h>

#include <commons/mathtools/mnmath.h>
#include <cross-commons/debug/tlog.h>

#include <qfile.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qmap.h>

#include <math.h>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#define LENGTH_SEC0 16
#define READ_BLOCK 0xffff
#define ONLY_LOCAL 255

//#define GRIB2_FILENAME "grib2_db.xml"

#define GRIB2_REPR_FILE    "grib2_represent.xml"
#define GRIB2_GRID_FILE    "grib2_grid.xml"
#define GRIB2_PRODUCT_FILE "grib2_product.xml"

using namespace google::protobuf;
using namespace grib;

//grid - 0,4,5,20

TDecodeGrib2::TDecodeGrib2( meteo::Psql* ):
  _fileIdx(0), _validGrib(false),_grib(0),_resNumber(0)
{
  _grib = new TGribData;
  _reprDataSize = 0;
}

TDecodeGrib2::~TDecodeGrib2()
{
  if (_grib) delete _grib;
  _grib = 0;
}

//--- settings

// QString TDecodeGrib2::dbRuleFile()
// {
//   return GRIB2_FILENAME;
// }

QString TDecodeGrib2::representFile()
{
  return MnCommon::etcPath(METEO_APP_NAME) + GRIB2_REPR_FILE;
}

QString TDecodeGrib2::gridFile()
{
  return MnCommon::etcPath(METEO_APP_NAME) + GRIB2_GRID_FILE;
}

QString TDecodeGrib2::productFile()
{
  return MnCommon::etcPath(METEO_APP_NAME) + GRIB2_PRODUCT_FILE;
}

//

void TDecodeGrib2::clear()
{
  if (_grib) {
    _grib->Clear();
  }
  
}

bool TDecodeGrib2::checkFormat(uint64_t fileIdx, QFile* file)
{
  _fileIdx = fileIdx;
  _file = file;

  _data = _file->read(LENGTH_SEC0);

  char2dec(_data.mid(8, 8), 8, &_size);
  //  debug_log<<"grib2 size="<<_size;

  if (_size == 0) return false;

  _file->seek(_fileIdx + _size - 4);
  if ( _file->read(4) != "7777" ) {
    return false;    
  }
  
  _fileIdx += LENGTH_SEC0;
  _file->seek(_fileIdx);

  _grib->set_discipline(_data.at(6));

  return true;
}

ushort TDecodeGrib2::getNextSection()
{
  //  debug_log<<"pos="<<_file->pos()<<" size="<<_size<<" fsize="<<_file->size();

  if (_file->pos() - (_fileIdx-LENGTH_SEC0) + 5 >= (uint64_t)_size) {
     _file->seek(_fileIdx - LENGTH_SEC0 + _size - 4);//to the end of grib2 data
    return 0;
  }

  _data = _file->read(5);
  uint len;
  char2dec(_data.data(), 4, &len);
  ushort sect = _data[4];
  
  _data.append(_file->read(len - 5));

  //  debug_log<<"sect="<<sect<<" len="<<len;

  return sect;
}

bool TDecodeGrib2::checkFormat(const QByteArray& arr, int64_t* idx)
{
  char2dec(arr.mid(*idx + 8, 8), 8, &_size);
  if (_size == 0) return false;

  QByteArray end = arr.mid(*idx + _size - 4, 4);
  if (end != "7777") {
    return false;
  }
  
  _grib->set_discipline(arr.at(*idx + 6));

  *idx += LENGTH_SEC0;
  return true;
}

uint16_t TDecodeGrib2::getNextSection(const QByteArray& arr, int64_t* idx)
{
  //debug_log<<"pos="<<*idx<<" size="<<_size<<" asize="<<arr.size();

  if (*idx + 5 >= (int64_t)_size) {
    return 0;
  }

  _data = arr.mid(*idx, 5);
  uint len;
  char2dec(_data.data(), 4, &len);
  ushort sect = _data[4];
  
  if (*idx + len >= (int64_t)_size) {
    error_log << QObject::tr("Ошибка размера файла %1 >= %2").arg(*idx+len).arg(_size);
    return 0;
  }
  
  _data.append(arr.mid(*idx + 5, len - 5));
  *idx += len;

  //debug_log<<"sect="<<sect<<" len="<<len;

  return sect;
}

//! Identifivcation section
bool TDecodeGrib2::parseSection1()
{
  //  debug_log<<"Parse 1 size="<<_data.size();

  if (_data.size() < 21) {
    error_log << QObject::tr("Ошибка размера 1 секции = %1").arg(_data.size());
    return false;
  }
  
  uint32_t val;
  char2dec(_data.data() + 5, 2, &val);
  _grib->set_center(val);
  char2dec(_data.data() + 7, 2, &val);
  _grib->set_subcenter(val);

  _grib->set_mastertable(_data[9]);
  //  debug_log<<"master vers="<<vers;
  if (uchar(_data[9]) == ONLY_LOCAL) {
    error_log << QObject::tr("Данные содержат только локальные таблицы");
    return false;
  }

  _grib->set_localtable(_data[10]);

  _grib->set_signdt(uchar(_data[11]));

  char2dec(_data.data()+ 12, 2, &val);
  QDateTime dt(QDate(val, (uint)_data.at(14), (uint)_data.at(15)),
	       QTime(_data.at(16), (uint)_data.at(17), (uint)_data.at(18)),
	       Qt::UTC);
  _grib->set_dt(dt.toString(Qt::ISODate).toStdString());
  
  _grib->set_status(uchar(_data[19]));
  _grib->set_datatype(uchar(_data[20]));

  return true;
}

//! Local section use
bool TDecodeGrib2::parseSection2()
{
  if (_data.size() != 0) {
    info_log << QObject::tr("Есть локальные данные. Центр=%1. Подцентр=%2. Таблицы=%3 Локальные=%4")
      .arg(_grib->center()).arg(_grib->subcenter()).arg(_grib->mastertable()).arg(_grib->localtable());
  }
  return true;
}

//! Grid definition
bool TDecodeGrib2::parseSection3()
{
  //  debug_log<<"Parse 3"<<_data.size();

  if (_data.size() < 15) {
    error_log << QObject::tr("Ошибка размера 3 секции %1").arg(_data.size());
    return false;
  }
  
  bool ok = false;

  TGridData* g = _grib->mutable_grid();

  g->Clear();
  _resNumber = 0;

  // for (int i=0; i< _data.size(); i++) {
  //   printf("%d = 0x%x %u\n", i+6, _data.at(i), (uint)_data.at(i));
  // }

  uint8_t grid_source = uchar(_data[5]);
  if (grid_source != 0 && grid_source != 255) {
    warning_log << QObject::tr("Неизвестный источник сетки = %1").arg(grid_source);
  }

  char2dec(_data.data()+6, 4, &_resNumber);
  //  debug_log<<"_resNumber"<<_resNumber;
 
  uint32_t pointsSize = uchar(_data[10]);
  g->set_pointstype(uchar(_data[11]));

  uint32_t val;
  char2dec(_data.data() + 12, 2, &val);
  g->set_type(val);

  if (grid_source != 0) {
    return true;
  }

  uint32_t size = _data.size() - 14 - pointsSize;
  uint16_t crc = 0;
  std::string gridStr;

  if (size != 0) {
    ok = parseGrid(&gridStr, _data.data() + 14, size, g->type());
    if (ok) {
      g->set_data(gridStr.data(), gridStr.size());
      crc = 0; //MnMath::makeCrc(gridStr.data(), gridStr.size());
      g->set_datacrc(crc);
    }
  }

  if (pointsSize != 0) {
    g->set_points(_data.data() + 14 + size, pointsSize);
    crc = 0; //MnMath::makeCrc(_data.data() + 14 + size, pointsSize);
    g->set_pointscrc(crc);
  }

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
  
  return ok;
}

bool TDecodeGrib2::parseSection4()
{
  //  debug_log<<"parse 4";

  if (_data.size() < 11) {
    error_log << QObject::tr("Ошибка размера 4 секции %1").arg(_data.size());
    return false;
  }
  
  bool ok = false;

  TProductData2* p = _grib->mutable_product2();
  TGridData* g = _grib->mutable_grid();

  p->Clear();

  uint32_t val;
  uint16_t verticSize = 0;
  char2dec(_data.data()+5, 2, &verticSize);
  char2dec(_data.data() + 7, 2, &val);
  p->set_deftype(val);
  uint32_t defSize = _data.size() - 9 -verticSize;

  //  debug_log<<"defType="<<p->deftype();
  uint32_t crc;
  std::string prodStr;

  if (defSize != 0) {
    grib::TProductDefinition* prod = parseProd(&prodStr, _data.data() + 9, defSize, p->deftype());
    p->set_definition(prodStr.data(), prodStr.size());
    crc = 0; //MnMath::makeCrc(prodStr.data(), prodStr.size());
    p->set_defcrc(crc);

    ok = setProductData(prod);
    
  }

  if (verticSize != 0) {
    g->set_verticcoords(_data.data() + 9 + defSize, verticSize);
    crc = 0;//MnMath::makeCrc(_data.data() + 9 + defSize, verticSize);
    g->set_verticcrc(crc);
  }

  p->set_category(uchar(_data[9])); //уже внутри template
  p->set_number(uchar(_data[10]));
  //  p.processType =  TODO зависит от шаблона defType
  //forecast time TODO если надо для таблицы

  return ok;
}

bool TDecodeGrib2::parseSection5()
{
  if (_data.size() < 12) {
    error_log << QObject::tr("Ошибка размера 5 секции %1").arg(_data.size());
    return false;
  }
  
  bool ok = false;
  //  debug_log<<"parse 5";

  char2dec(_data.data()+5, 4, &_reprDataSize);

  uint32_t reprType;
  char2dec(_data.data() + 9, 2, &reprType);
  //  var(reprType);

  if (_data.size() > 11) {
    ok = parseRepr(_data.data() + 11, _data.size() - 11, reprType);
  }

  return ok;
}

bool TDecodeGrib2::parseSection6()
{
  //  debug_log<<"parse 6";

  if (_data.size() < 5) {
    error_log << QObject::tr("Ошибка размера 6 секции %1").arg(_data.size());
    return false;
  }
  
  TBitmapData* p = _grib->mutable_bitmap();  

  p->set_type(uchar(_data[5]));

  //  debug_log<<"type"<<p->type()<<"data="<<(uchar)(_data[5]);
  uint16_t bitmapSize = _data.size() - 6;

  if (p->type() == 0) {
    if (bitmapSize <= 0) {
      error_log << QObject::tr("Ошибка размера 6 секции %1").arg(_data.size());
      _grib->clear_bitmap();
    } else {
      p->set_bitmap( _data.data() + 6, bitmapSize);
      uint32_t crc = 0;//MnMath::makeCrc(_data.data() + 6, bitmapSize);
      p->set_crc(crc);
    }
  }  else if (p->type() != 254 && bitmapSize == 0) {
    _grib->clear_bitmap();
  }
  return true;
}

bool TDecodeGrib2::parseSection7()
{
  //  printvar(QString::fromStdString(_grib->DebugString()));

  //debug_log<<"parse 7" << "resSize" << _reprDataSize << "data.size()" << _data.size();
  if (_data.size() < 5) {
    error_log << QObject::tr("Ошибка размера 7 секции %1").arg(_data.size());
    return false;
  }
  
  if (_reprDataSize == 0) {
    error_log << QObject::tr("Размер данных = 0");
    return false;
  }
  float* resData = new(std::nothrow) float[_reprDataSize];
  if (!resData) return false;
  
  _validGrib = restoreData(resData, _reprDataSize, _data.data() + 5, _data.size() - 5);

  // for (uint i=30; i<60; i++) {
  //   printf("%d %f\n", i, resData[i]);
  // }

  if (_validGrib) {
    _grib->set_data(resData, _reprDataSize*sizeof(float));
    
    uint16_t crc = 0; //MnMath::makeCrc(resData, _reprDataSize*sizeof(float));
    _grib->set_datacrc(crc);
  }

  delete[] resData;

  return _validGrib;
}

bool TDecodeGrib2::setProductData(grib::TProductDefinition* prod)
{
  if (prod == 0 || prod->definition == 0) return false;

  TProductData2* p = _grib->mutable_product2();

  const FieldDescriptor * field = prod->definition->GetDescriptor()->FindFieldByName("pc");
  if (field != 0) {
    const TProduct::ProductCommon& pc = static_cast<const TProduct::ProductCommon&>(prod->definition->GetReflection()->GetMessage(*(prod->definition), field));
    //printvar(QString::fromStdString(pc.DebugString()));
    p->set_forecasttime(pc.forecasttime());
    p->set_surf1_type(pc.surf1_type());
    if (pc.surf1_type() == 100 || pc.surf1_type() == 108) {
      p->set_surf1(pc.surf1() / 100.);
    } else {
      p->set_surf1(pc.surf1());      
    }    
    p->set_surf2_type(pc.surf2_type());
    if (pc.surf2_type() == 100 || pc.surf2_type() == 108) {
      p->set_surf2(pc.surf2() / 100.);
    } else {
      p->set_surf2(pc.surf2());
    }
    p->set_processtype(pc.id());
  } else {
    p->set_forecasttime(0);
    p->set_surf1_type(255);
    p->set_surf1(0);
    p->set_surf2_type(255);
    p->set_surf2(0);
    p->set_processtype(255);
  }

  QDateTime dt = QDateTime::fromString(QString::fromStdString(_grib->dt()), Qt::ISODate);
  p->set_dt1(dt.addSecs(p->forecasttime()).toString(Qt::ISODate).toStdString());
  QDateTime dt2 = dt.addSecs(p->forecasttime());
  p->set_timerange(255);

  field = prod->definition->GetDescriptor()->FindFieldByName("ti");
  bool hasFiled = prod->definition->GetReflection()->HasField(*prod->definition, field);
  if (hasFiled) {
    const TProduct::TimeInterval& ti = static_cast<const TProduct::TimeInterval&>(prod->definition->GetReflection()->GetMessage(*(prod->definition), field));
    dt2 = QDateTime(QDate(ti.endyear(), ti.endmonth(), ti.endday()), 
		    QTime(ti.endhour(), ti.endminute(), ti.endsecond()),
		    Qt::UTC);

    if (ti.tr_size() != 0) {
      p->set_timerange(ti.tr(0).timerange());      
    }
  }

  p->set_dt2(dt2.toString(Qt::ISODate).toStdString());

  return true;
}

//------------ save

bool TDecodeGrib2::existBitmap()
{
  return _grib->has_bitmap();
}

// uint32_t TDecodeGrib2::saveInDb()
// {
//   if (!_validGrib) {
//     error_log<<QObject::tr("Ошибка сохранения: данные не сформированы");
//     return 0;
//   }

//   return TGribDb::saveInDb(GRIB2_FILENAME);
// }

// bool TDecodeGrib2::readFromDb(uint64_t id)
// {
//   bool ok = TGribDb::readFromDb(id);
//   if (!ok) return false;
  
//   parseGridFromStr(_grib->grid().data(), _grib->grid().type(), 2);
//   parseProdFromStr(_grib->product2().definition(), _grib->product2().deftype(), 2);
  
//   return true;
// }

google::protobuf::Message* TDecodeGrib2::getGrid()
{
  return _grib->mutable_grid(); 
}

google::protobuf::Message* TDecodeGrib2::getBitmap()
{
  return _grib->mutable_bitmap(); 
}

google::protobuf::Message* TDecodeGrib2::getProduct()
{
  return _grib->mutable_product2(); 
}

google::protobuf::Message* TDecodeGrib2::getCommonData()
{
  return _grib;
}

//----

uint64_t TDecodeGrib2::dataSize() const
{
  if (!_validGrib) return 0;
  return _grib->data().size();
}

const float* TDecodeGrib2::data()  const
{
  if (!_validGrib) return 0;
  return reinterpret_cast<const float*>(_grib->data().data());
}

// uint64_t TDecodeGrib2::resultSize() const
// {
//   if (!_validGrib) return 0;
//   return _grib->mutable_product2()->resnumber();
// }
