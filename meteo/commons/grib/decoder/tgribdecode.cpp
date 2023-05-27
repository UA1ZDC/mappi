#include "tgribdecode.h"

#include <meteo/commons/grib/parser/tgribformat.h>
#include <meteo/commons/proto/tgribformat.pb.h>

#include "tgrib2_decode.h"
#include "tgrib1_decode.h"
#include "tdecodebase.h"
//#include <meteo/commons/grib/iface/tgribcalc.h>

//#include <trequest.h>
#include <meteo/commons/grib/parser/tproductgrib.h>
#include <meteo/commons/grib/parser/treprgrib.h>
#include <meteo/commons/grib/parser/tgridgrib.h>

#include <sql/psql/psql.h>
#include <cross-commons/debug/tlog.h>

#include <qbytearray.h>
#include <qfile.h>

#include <meteo/commons/proto/state.pb.h>

#define READ_BLOCK 0xffff //чтение по блокам при поиске ключевого слова "GRIB"
#define VERSION_OFFSET 7 //сдвиг до номера версии


TDecodeGrib::TDecodeGrib(meteo::Psql *db /*= 0*/):_db(db)
{
  _size = 0;
  _fileIdx = 0;
  _grib = 0;

  _stat.set(grib::kGrib1, grib::kReceived, meteo::app::OperationState_WARN, 
	    QObject::tr("Принято сводок GRIB"));
  _stat.set(grib::kGrib1, grib::kDecoded, meteo::app::OperationState_WARN, 
	    QObject::tr("Раскодировано сводок GRIB"));

  _stat.set(grib::kGrib2, grib::kReceived, meteo::app::OperationState_WARN, 
	    QObject::tr("Принято сводок GRIB2"));
  _stat.set(grib::kGrib2, grib::kDecoded, meteo::app::OperationState_WARN, 
	    QObject::tr("Раскодировано сводок GRIB2"));
}

TDecodeGrib::~TDecodeGrib()
{
  if (_grib) delete _grib;
  _grib = 0;
}

//! -1 - not GRIB
int TDecodeGrib::edition(const QByteArray & data)
{
  int idx = data.indexOf("GRIB");
  if (idx == -1) {
    return -1;
  }

  return data.at(idx + 7); //edition number
}

int TDecodeGrib::decode(const QMap<QString, QString>& type, const QByteArray& ba)
{
  Q_UNUSED(type);
  int cnt = decode(ba);
  
  return cnt;
}

// int TDecodeGrib::decode(const QByteArray& ba, QList<QByteArray>* result, QString* error, const QDateTime&)
// {
//   uchar version = 0;
//   int64_t current = 0;
//   int64_t idx = 0;

//   if (0 == (version = detectGrib(ba, &idx))) {
//     *error = "Ошибка определения версии GRIB";  
//     return 1;
//   }  

//   if (_grib) delete _grib;
//   _grib = 0;

//   if (version == 1) {
//     _grib = new TDecodeGrib1(_db);
//   } else if (version == 2) {
//     _grib = new TDecodeGrib2(_db);
//   } else {
//     error_log << QObject::tr("Версия GRIB %1 не поддерживается").arg(version);
//     *error = QObject::tr("Версия GRIB %1 не поддерживается").arg(version);
//     return -1;
//   }

//   if (!_grib->checkFormat(ba.mid(idx), &current)) {
//     error_log << QObject::tr("Ошибка формата GRIB. idPtkpp=%1").arg(sourceId());
//     *error = QObject::tr("Ошибка формата GRIB");
//     return -1;
//   }

//   bool ok = true;
//   ushort section=0;
//   while( 0 != (section = _grib->getNextSection(ba.mid(idx), &current)) ) {
//     ok = parseSection(section);

//     if (!ok) {
//       if (section == 0 && version == 2) {
// 	break;
//       } else {
// 	error_log << QObject::tr("Ошибка раскодирования секции %1. idPtkpp=%2").arg(section).arg(sourceId());
// 	*error = QObject::tr("Ошибка раскодирования секции %1").arg(section);
// 	return -1;
//       }
//     }

//     if (_grib->isValidGrib()) {
//       grib::TGribData simple;
//       grib::copyIdentificataion(*grib(), &simple);
//       std::string dataStr;
//       simple.SerializeToString(&dataStr);
//       QByteArray msg(dataStr.data(), dataStr.size());
//       result->append(msg);
//     }

//   }
  
//   //  debug_log<<"end grib";

//   return 0;
// }

int TDecodeGrib::decode(const QString &tlgfile_name)
{
  int res = 0;

  //  clear();

  _file.setFileName(tlgfile_name);

  if(!_file.open(QIODevice::ReadOnly)) {
    error_log<<"Can't open file"<<tlgfile_name;
    return -1;
  }

  while (!_file.atEnd() && res !=  -1) { //TODO uncomment for parsing all file
    res = decode();
  }

  _file.close();

  return res;
}

int TDecodeGrib::decode()
{
  uchar version = 0;
  if (0 == (version = detectGrib())) {
    return -1;
  }

  if (_grib) delete _grib;
  _grib = 0;
  int cnt = 0;
  //  debug_log << "version=" << version << "idPtkpp=" << sourceId();

  if (version == 1) {
    _grib = new TDecodeGrib1(_db);
    _stat.add(grib::kGrib1, grib::kReceived, meteo::app::OperationState_NORM);
  } else if (version == 2) {
    _grib = new TDecodeGrib2(_db);
    _stat.add(grib::kGrib2, grib::kReceived, meteo::app::OperationState_NORM);
  } else {
    error_log<<QObject::tr("Версия GRIB %1 не поддерживается").arg(version);
    return -1;
  }

  if (!_grib->checkFormat(_fileIdx, &_file)) {
    error_log << QObject::tr("Ошибка формата GRIB. idPtkpp=%1").arg(sourceId());
    return -1;
  }

  bool ok = true;
  ushort section=0;
  while( 0 != (section = _grib->getNextSection()) ) {
    ok = parseSection(section);

    if (!ok) {
      if (section == 0 && version == 2) {
	break;
      } else {
	error_log << QObject::tr("Ошибка раскодирования секции %1. idPtkpp=%2").arg(section).arg(sourceId());
	if (cnt == 0) {
	  cnt = -1;
	}
	return cnt;
      }
    }

    if (_grib->isValidGrib()) {
      if (version == 1) {
	_stat.add(grib::kGrib1, grib::kDecoded, meteo::app::OperationState_NORM);
      } else {
	_stat.add(grib::kGrib2, grib::kDecoded, meteo::app::OperationState_NORM);
      }

      dataReady();
      cnt++;
    }

  }
  
  _file.read(4); //"7777"
  //  debug_log<<"end grib";

  return cnt;
}

uint8_t TDecodeGrib::detectGrib()
{
  _fileIdx = _file.pos();

  qint64 fsize = _file.size();
  QByteArray ar;
  int idx=-1;

  while (_file.pos() + 4 < _file.size()) {
    ar = _file.read(fsize > READ_BLOCK ? READ_BLOCK: fsize);
    idx = ar.indexOf("GRIB");
    if (idx != -1) {
      break;
    }
    _file.seek(_file.pos() - 4);
    _fileIdx = _file.pos();
  }

  if (idx == -1) {
    return 0;
  }

  _fileIdx += idx;
  _file.seek(_fileIdx + VERSION_OFFSET);

  char version=0;
  _file.getChar(&version);
  _file.seek(_fileIdx);

  return (uint8_t)version;
}

bool TDecodeGrib::parseSection(ushort section)
{
  bool ok = false;;
  switch (section) {
  case 1:
    ok = _grib->parseSection1();
    break;
  case 2:
    ok = _grib->parseSection2();
    break;
  case 3:
    ok = _grib->parseSection3();
    break;
  case 4:
    ok = _grib->parseSection4();
    break;
  case 5:
    ok = _grib->parseSection5();
    break;
  case 6:
    ok = _grib->parseSection6();
    break;
  case 7:
    ok = _grib->parseSection7();
    break;
  default: {}
  }

  return ok;
}


// bool TDecodeGrib::readFromDb(uint64_t id)
// {
//   int version = TGribDb::readVersion(id, _db);

//   if (_grib) delete _grib;
//   _grib = 0;
  
//   if (version == 1) {
//     _grib = new TDecodeGrib1(_db);
//   } else if (version == 2) {
//     _grib = new TDecodeGrib2(_db);
//   } else {
//     error_log<<QObject::tr("Версия GRIB %1 не поддерживается").arg(version);
//     return false;
//   }
//   if (! _grib->readFromDb(id)) return false;

  
//   return true;
// }


//!< edition number for loaded grib data
int TDecodeGrib::edition()
{
  if (!_grib) return -1;

  return _grib->edition();
}

const grib::TGribData* TDecodeGrib::grib() const
{
  if (!_grib) return 0;
  return _grib->gribData();
}

const grib::TGridDefinition* TDecodeGrib::gridDefinition() const
{
  if (!_grib) return 0;
  return _grib->gridDefinition();
}

//! only grib2
const grib::TProductDefinition* TDecodeGrib::productDefinition() const
{
  if (!_grib) return 0;
  return _grib->prodDefinition();
}


const float* TDecodeGrib::data(uint64_t* size) const
{
  *size = _grib->dataSize()/sizeof(float);
  return reinterpret_cast<const float*>(_grib->data());
}

int TDecodeGrib::decode(const QByteArray& aba)
{
  uchar version = 0;
  int64_t current = 0;
  int64_t idx = 0;

  int cnt = 0;
  
  while (idx < aba.size()) {
    if (0 == (version = detectGrib(aba, &idx))) {
      return 0;
    }
    //var(version);
    
    QByteArray ba = aba.mid(idx);   
    current = 0;
    
    if (_grib) delete _grib;
    _grib = 0;
    
    //debug_log << "version=" << version << "idPtkpp=" << sourceId();
    
    if (version == 1) {
      _grib = new TDecodeGrib1(_db);
      _stat.add(grib::kGrib1, grib::kReceived, meteo::app::OperationState_NORM);
    } else if (version == 2) {
      _grib = new TDecodeGrib2(_db);
      _stat.add(grib::kGrib2, grib::kReceived, meteo::app::OperationState_NORM);
    } else {
      error_log<<QObject::tr("Версия GRIB %1 не поддерживается").arg(version);
      return -1;
    }
    
    if (!_grib->checkFormat(ba, &current)) {
      error_log << QObject::tr("Ошибка формата GRIB. idPtkpp=%1").arg(sourceId());
      return -1;
    }
    
    bool ok = true;
    ushort section=0;
    while( 0 != (section = _grib->getNextSection(ba, &current)) ) {
      ok = parseSection(section);
      if (!ok) {
	if (section == 0 && version == 2) {
	  break;
	} else {
	  error_log << QObject::tr("Ошибка раскодирования секции %1. idPtkpp=%2").arg(section).arg(sourceId());
	  if (cnt == 0) {
	    cnt = -1;
	  }
	  return cnt;
	}
      }
      
      if (_grib->isValidGrib()) {
	if (version == 1) {
	  _stat.add(grib::kGrib1, grib::kDecoded, meteo::app::OperationState_NORM);
	} else {
	  _stat.add(grib::kGrib2, grib::kDecoded, meteo::app::OperationState_NORM);
	}
	
	dataReady();
	cnt++;
      }
      
    }
    
  //  debug_log<<"end grib";
    idx += current;
    // if (cnt == 1 ) {
    //   var(cnt);
    //   break;
    // }
  }
  return cnt;
}

uint8_t TDecodeGrib::detectGrib(const QByteArray& arr, int64_t* idx)
{
  *idx = arr.indexOf("GRIB", *idx);
  if (*idx == -1) {
    return 0;
  }

  return arr.at(*idx + 7);
}


void TDecodeGrib::dataReady()
{
  // const grib::TGribData* grib = _grib->gribData();
  //   if (!grib) {
  //   error_log << QObject::tr("Ошибка. Данные не сформированы");
  //   return;
  // }

  // if (grib->has_product1()) {
  //   debug_log << "bitmap" << grib->has_bitmap() << "number" << grib->product1().number();
  // } else if (grib->has_product2()) {
  //   debug_log << "bitmap"   << grib->has_bitmap() << "discipline" << grib->discipline()
  // 	      << "category" << grib->product2().category()
  // 	      << "number"   << grib->product2().number();
  // } else {
  //   debug_log << "bitmap" << grib->has_bitmap() << "no product";
  // }

  //    debug_log<<QObject::tr("Общие данные");
  //  //var(QString::fromStdString(grib->DebugString()));
  // var(grib->center());
  // var(grib->subcenter());

  // const grib::TGridDefinition* grid = gridDefinition();
  // if (grid->definition != 0) {
  //   debug_log<<QObject::tr("Описание сетки:");
  //   var(grid->type);
  //   var(QString::fromStdString(grid->definition->DebugString()));
  //   TGrid::LatLon* latlon = static_cast<TGrid::LatLon*>(grid->definition);
  //   debug_log << "la" << latlon->la1() << latlon->la2()
  //       << "lo" << latlon->lo1() <<  latlon->lo2();
  // }

  // //  return;

  // const grib::TProductDefinition* prod = productDefinition();
  // if (prod->definition != 0) {
  //   debug_log<<QObject::tr("Описание продукта:");
  //   var(prod->type);
  //   var(QString::fromStdString(prod->definition->DebugString()));
  // } else {
  //   var(grib->product1().DebugString());
  // }
  // debug_log << "---------------";


}
