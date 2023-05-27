#include "tgribcalc.h"

#include "tgribiface.h"


#include <commons/mathtools/mnmath.h>

#include <cross-commons/debug/tlog.h>


#define GRIB_DEW_POINT_DEFICIT 18
#define GRIB_WIND_SPEED 32
#define GRIB_WIND_DIRECTION 31

typedef void (*CreateFunc)(float v1, float v2, float* res);
namespace grib {

  void copyCommon(const grib::TGribData& src, grib::TGribData* dst);
  bool copyWithoutData(const grib::TGribData& src, grib::TGribData* dst);
  bool convertGrib(const grib::TGribData& src1, const grib::TGribData& src2, 
		   int parametr, CreateFunc func, grib::TGribData* dst);
  bool convertGrib(const grib::TGribData& u, const grib::TGribData& v, grib::TGribData* ff, grib::TGribData* dd);
}

//! Преобразование данных ветра из векторного представления в скорость и направление
/*! 
  \param u компонент ветра u
  \param v компонент ветра v
  \param ff скорость ветра
  \param dd направление ветра
  \return false - в случе ошибки, иначе true
*/
bool grib::uv2ff(const grib::TGribData& u, const grib::TGribData& v, grib::TGribData* ff, grib::TGribData* dd)
{
  return convertGrib(u, v, ff, dd);
}


//! Преобразование данных температуры точки росы в дефицит точки росы
/*! 
  \param T  температура
  \param Td температура точки росы
  \param D  дефицит точки росы
  \return false - в случе ошибки, иначе true
*/
bool grib::Td2D(const grib::TGribData& T, const grib::TGribData& Td, grib::TGribData* D)
{
  return convertGrib(T, Td, GRIB_DEW_POINT_DEFICIT, MnMath::Td2D, D);
}

//! Преобразование данных влажности в дефицит точки росы
/*! 
  \param T температура
  \param U влажность
  \param D дефицит
  \return false - в случе ошибки, иначе true
*/
bool grib::U2D(const grib::TGribData& T, const grib::TGribData& U, grib::TGribData* D)
{
  return convertGrib(T, U, GRIB_DEW_POINT_DEFICIT, MnMath::U2D_kelvin, D);
}


//! Преобразование данных GRIB для указанного параметра с помощью указанной функции
/*! 
  \param dst заполняемая структура
  \param src1 источник данных
  \param src2 источник данных
  \param parametr номер параметра
  \param func функция для преобразования, принимающая на вход 
  \return  false - в случе ошибки, иначе true
*/
bool grib::convertGrib(const grib::TGribData& src1, const grib::TGribData& src2, 
		       int parametr, CreateFunc func, grib::TGribData* dst)
{
  if (!dst) return false;
  if (!copyWithoutData(src1, dst)) return false;

  dst->mutable_product1()->set_number(parametr);

  if (src1.data().size() != src2.data().size()) {
    return false;
  }

  uint64_t size = src1.data().size() / sizeof(float);
  if (size == 0) return false;

  const float* val1 = reinterpret_cast<const float*>(src1.data().data());
  const float* val2 = reinterpret_cast<const float*>(src2.data().data());
  float* val = new(std::nothrow) float[size];
  if (!val) return false;

  for (uint64_t idx = 0; idx < size; idx++) {
    func(val1[idx], val2[idx], &(val[idx]));
  }

  dst->set_data(val, size*sizeof(float));
  uint32_t crc = 0; //MnMath::makeCrc(val, size*sizeof(float));
  dst->set_datacrc(crc);

  if (val) delete[] val;
  return true;
}



//! Преобразование данных ветра из векторного представления в скорость и направление
/*! 
  \param u компонент ветра u
  \param v компонент ветра v
  \param ff скорость ветра
  \param dd направление ветра
  \return  false - в случе ошибки, иначе true
*/
bool grib::convertGrib(const grib::TGribData& u, const grib::TGribData& v, 
		       grib::TGribData* ff, grib::TGribData* dd)
{
  if (!ff || !dd) return false;
  if (!copyWithoutData(u, ff)) return false;
  if (!copyWithoutData(u, dd)) return false;

  ff->mutable_product1()->set_number(GRIB_WIND_SPEED);
  dd->mutable_product1()->set_number(GRIB_WIND_DIRECTION);

  if (u.data().size() != v.data().size()) {
    return false;
  }

  uint64_t size = u.data().size() / sizeof(float);
  if (size == 0) return false;

  const float* u_val = reinterpret_cast<const float*>(u.data().data());
  const float* v_val = reinterpret_cast<const float*>(v.data().data());
  float* dd_val = new(std::nothrow) float[size];
  float* ff_val = new(std::nothrow) float[size];
  if (!dd_val  || !ff_val) return false;

  for (uint64_t idx = 0; idx < size; idx++) {
    MnMath::preobrUVtoDF(u_val[idx]*.01, v_val[idx]*.01, &(dd_val[idx]), &(ff_val[idx]));
  }

  dd->set_data(dd_val, size*sizeof(float));
  uint32_t crc = 0; //MnMath::makeCrc(dd_val, size*sizeof(float));
  dd->set_datacrc(crc);

  ff->set_data(ff_val, size*sizeof(float));
  crc = 0; //MnMath::makeCrc(ff_val, size*sizeof(float));
  ff->set_datacrc(crc);


  if (dd_val) delete[] dd_val;
  if (ff_val) delete[] ff_val;
  return true;
}

//! Копирование общих параметров
/*! 
  \param dst заполняемая структура
  \param src исходная структура
*/
void grib::copyCommon(const grib::TGribData& src, grib::TGribData* dst)
{
  if (!dst) return;

  dst->set_center(src.center());
  dst->set_subcenter(src.subcenter());
  dst->set_dt(src.dt());
  dst->set_mastertable(src.mastertable());
  if (src.has_localtable()) {
    dst->set_localtable(src.localtable());
  }

  if (src.has_discipline()) {
    dst->set_discipline(src.discipline());
  }
  if (src.has_signdt()) {
    dst->set_signdt(src.signdt());
  }
  if (src.has_status()) {
    dst->set_status(src.status());
  }
  if (src.has_datatype()) {
    dst->set_datatype(src.datatype());
  }
}

//! Копирование данных кроме измеренных величин и, соотв-но, указателя (номера)  параметра
/*! 
  \param dst заполняемая структура
  \param src исходная структура
*/
bool grib::copyWithoutData(const grib::TGribData& src, grib::TGribData* dst)
{
  if (!dst) return false;

  if (src.has_product2()) {
    error_log << QObject::tr("Для GRIB2 не реализовано ");
    return false;
  }

  if (!src.has_product1()) {
    error_log << QObject::tr("Нет данных продукта");
    return false;
  }

  copyCommon(src, dst);
  dst->mutable_grid()->CopyFrom(src.grid());
  if (src.has_bitmap()) {
    dst->mutable_bitmap()->CopyFrom(src.bitmap());
  }

  grib::TProductData1* prod = dst->mutable_product1();
  prod->set_leveltype(src.product1().leveltype());
  prod->set_levelvalue(src.product1().levelvalue());
  prod->set_p1(src.product1().p1());
  prod->set_p2(src.product1().p2());
  prod->set_timerange(src.product1().timerange());
  prod->set_numavg(src.product1().numavg());
  prod->set_absentavg(src.product1().absentavg());
  prod->set_processtype(src.product1().processtype());

  return true;
}

//! Копирование заголовочных данных
/*! 
  \param dst заполняемая структура
  \param src исходная структура
*/
bool grib::copyIdentificataion(const grib::TGribData& src, grib::TGribData* dst)
{
  if (!dst) return false;

  copyCommon(src, dst);
  dst->set_data("Binary");
  dst->set_datacrc(src.datacrc());

  grib::TGridData* grid = dst->mutable_grid();
  grid->set_type(src.grid().type());
  grid->set_data("Binary");
  grid->set_datacrc(src.grid().datacrc());
  if (src.grid().has_points()) {
    grid->set_points("Binary");
  }
  if (src.grid().has_pointscrc()) {
    grid->set_pointscrc(src.grid().pointscrc());
  }
  if (src.grid().has_pointstype()) {
    grid->set_pointstype(src.grid().pointstype());
  }
  if (src.grid().has_verticcoords()) {
    grid->set_verticcoords("Binary");
  }
  if (src.grid().has_verticcrc()) {
    grid->set_verticcrc(src.grid().verticcrc());
  }

  if (src.has_product1()) {
    grib::TProductData1* prod = dst->mutable_product1();
    prod->set_number(src.product1().number());
    prod->set_leveltype(src.product1().leveltype());
    prod->set_levelvalue(src.product1().levelvalue());
    prod->set_p1(src.product1().p1());
    prod->set_p2(src.product1().p2());
    prod->set_timerange(src.product1().timerange());
    prod->set_numavg(src.product1().numavg());
    prod->set_absentavg(src.product1().absentavg());
    prod->set_processtype(src.product1().processtype());
  } else if (src.has_product2()) {
    grib::TProductData2* prod = dst->mutable_product2();
    prod->set_deftype(src.product2().deftype());
    prod->set_definition("Binary");   
    prod->set_defcrc(src.product2().defcrc());
    prod->set_category(src.product2().category());
    prod->set_number(src.product2().number());	    
    prod->set_forecasttime(src.product2().forecasttime());
    prod->set_surf1_type(src.product2().surf1_type()); 
    prod->set_surf1(src.product2().surf1()); 
    prod->set_surf2_type(src.product2().surf2_type());
    prod->set_surf2(src.product2().surf2()); 
    prod->set_processtype(src.product2().processtype());
  }
  return true;
}
