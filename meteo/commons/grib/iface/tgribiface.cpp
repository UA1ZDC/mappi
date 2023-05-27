#include "tgribiface.h"
#include <meteo/commons/grib/parser/tgridgrib.h>
#include <meteo/commons/grib/parser/tproductgrib.h>
#include <meteo/commons/proto/tgrid.pb.h>
#include <meteo/commons/proto/tproduct.pb.h>


#include <cross-commons/debug/tlog.h>

#include <qvector.h>

using namespace grib;

TGribIface::TGribIface(const grib::TGribData* grib) :
  _grib(0), _grid(0), _prod(0), _vers(-1), _isDummy(false)
{
  setGrib(grib);
}

TGribIface::~TGribIface()
{
  clear();
}

void TGribIface::clear()
{
  _vers = -1;
  if (_grid) delete _grid;
  _grid = 0;
  if (_prod) delete _prod;  
  _prod = 0;
  if (_isDummy) delete _grib;
  _isDummy = false;
}

void TGribIface::setGrib(const grib::TGribData* grib)
{
  clear(); 

  _grib = grib; 
  if (_grib == 0) {
    _grib = new grib::TGribData;
    _isDummy = true;
  }

  if (_grib->has_product2()) {
    _vers = 2;    
  } else {
    _vers = 1;
  }
}

//! Получить описание сетки
const grib::TGridDefinition& TGribIface::grid()
{
  if (!_grid) {
    createGrid();
  }

  return *(_grid->definition());
}

//! Получить описание продукта
const grib::TProductDefinition& TGribIface::product()
{
  if (!_prod) {
    createProduct();
  }

  return *(_prod->definition());
}

//! Заполнение массива количества точек в каждом ряду
/*! 
  \param points массив
  \return false в случае ошибки, иначе - true
*/
bool TGribIface::fillPoints(QVector<uint16_t>* points) const
{
  if (!points || isRegular()) return false;

  points->resize(_grib->grid().points().size()/2);

  const char* dataPoints = _grib->grid().points().data();
  for (int i=0; i < points->size(); i++) {
    (*points)[i] = (dataPoints[2*i]<< 8) + dataPoints[2*i + 1];
  }
  
  return true;
}

//! Заполнение массива количества точек в каждом ряду
/*! 
 *  \param points массив
 *  \return false в случае ошибки, иначе - true
 */
bool TGribIface::fillPoints(::google::protobuf::RepeatedField< ::google::protobuf::int32 >* points) const
{
  if (!points || isRegular()) return false;
  int kol = _grib->grid().points().size()/2;
  points->Reserve(kol);
  
  const char* dataPoints = _grib->grid().points().data();
  for (int i=0; i < kol; i++) {
    points->Add((dataPoints[2*i]<< 8) + dataPoints[2*i + 1]);
  }
  
  return true;
}


//! Заполнение массива данными измерений
/*! 
 *  \param points массив
 *  \return false в случае ошибки, иначе - true
 */
bool TGribIface::fillData(QVector<float>* data) const
{
  if (!data || _vers > 2) return false;
  
  bool ok = fillData(data, _grib->data());
  
  return ok;
}

//! Заполнение массива данными измерений
/*! 
 *  \param points массив
 *  \return false в случае ошибки, иначе - true
 */
bool TGribIface::fillData(::google::protobuf::RepeatedField< double >* data) const
{
  if (!data || _vers > 2) return false;
  
  int64_t size = _grib->data().size()/ sizeof(float);
  if (size == 0) return false;
  
  const float* val =  reinterpret_cast<const float*>(_grib->data().data());
  if(0 == val) return false;  
  data->Reserve(size );
  
  for (int i=0; i < size; i++) {
    data->Add(val[i]);
  }
  
  return true;
}

//! Заполнение массива данными измерений из строки (массив байт)
/*! 
  \param points массив для измерений
  \return false в случае ошибки, иначе - true
*/
bool TGribIface::fillData(QVector<float>* data, const std::string& str) const
{
  uint64_t size = str.size();
  if (size == 0) return false;

  const float* val =  reinterpret_cast<const float*>(str.data());
  
  data->resize(size / sizeof(float));
//  qMemCopy(data->data(), val, size);
  ::memcpy(data->data(), val, size);
  return true;
}


//! Создание структуры для хранения сетки и её заполнение
bool TGribIface::createGrid()
{
  if (_grid) delete _grid;
  _grid = new TGridGrib;

  if (!_grib->has_grid()) return false;

  _grid->createFromString(_grib->grid().type(), _vers, _grib->grid().data());

  return true;
}

//! Создание структуры для хранения описание продукта и её заполнение
bool TGribIface::createProduct()
{
  if (!_prod) delete _prod;
  _prod = new TProductGrib;
  
  if (!_grib->has_product2()) return false;

  _prod->createFromString(_grib->product2().deftype(), _vers, _grib->product2().definition());
  
  return true;
}
