#include "ctest.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/mathtools/mnmath.h>
#include <meteo/commons/grib/iface/tgribcalc.h>
#include <tgribformat.pb.h>

#define PRECISION 0.1

using grib::TGribData;

CPPUNIT_TEST_SUITE_REGISTRATION( CTest );



//! проверка формул преобразования
void CTest::f1()
{
  float T = 21 + 273.15;
  float U = 70;
  float Td = 15.31 + 273.15;
  float D = 5.69;
  
  float D_r = 0;
  MnMath::Td2D(T, Td, &D_r);

  CPPUNIT_ASSERT(fabs(D-D_r) < PRECISION);

  MnMath::U2D_kelvin(T, U, &D_r);
  var(D-D_r);
  CPPUNIT_ASSERT(fabs(D-D_r) < PRECISION);
}

//! проверка преобразования GRIB
void CTest::f2()
{
  TGribData T;
  TGribData U;
  TGribData D;
  TGribData D_res;

  T.set_center(10);
  T.set_subcenter(2);
  T.set_dt("2015-01-01");
  T.set_mastertable(8);
  T.set_localtable(0);
  T.set_discipline(3);
  T.set_signdt(4);
  T.set_status(5);
  T.set_datatype(1);
  grib::TGridData* grid = T.mutable_grid();
  grid->set_type(1);
  std::string data("1234567890");
  grid->set_data(data);
  uint32_t crc = MnMath::makeCrc(data.data(), data.size());
  grid->set_datacrc(crc);
  grib::TProductData1* prod = T.mutable_product1();
  prod->set_number(11);
  prod->set_leveltype(1);
  prod->set_levelvalue(0);

  float temp[3] = {294.15, 297.15, 289.15}; //21, 24, 16
  T.set_data(temp, sizeof(temp));
  crc = MnMath::makeCrc(temp, sizeof(temp));
  T.set_datacrc(crc);
  
  U.set_center(10);
  U.set_subcenter(2);
  U.set_dt("2015-01-01");
  U.set_mastertable(8);
  U.set_localtable(0);
  U.set_discipline(3);
  U.set_signdt(4);
  U.set_status(5);
  U.set_datatype(1);
  grid = U.mutable_grid();
  grid->set_type(1);
  grid->set_data(data);
  crc = MnMath::makeCrc(data.data(), data.size());
  grid->set_datacrc(crc);
  prod = U.mutable_product1();
  prod->set_number(52);
  prod->set_leveltype(1);
  prod->set_levelvalue(0);

  float hum[3] = {70, 75, 80};
  U.set_data(hum, sizeof(hum));
  crc = MnMath::makeCrc(hum, sizeof(hum));
  U.set_datacrc(crc);

  D.set_center(10);
  D.set_subcenter(2);
  D.set_dt("2015-01-01");
  D.set_mastertable(8);
  D.set_localtable(0);
  D.set_discipline(3);
  D.set_signdt(4);
  D.set_status(5);
  D.set_datatype(1);
  grid = D.mutable_grid();
  grid->set_type(1);
  grid->set_data(data);
  crc = MnMath::makeCrc(data.data(), data.size());
  grid->set_datacrc(crc);
  prod = D.mutable_product1();
  prod->set_number(18);
  prod->set_leveltype(1);
  prod->set_levelvalue(0);

  float deficit[3] = {5.69, 4.71, 3.45};
  D.set_data(deficit, sizeof(deficit));
  crc = MnMath::makeCrc(deficit, sizeof(deficit));
  D.set_datacrc(crc);
  
  grib::U2D(T, U, &D_res);

  CPPUNIT_ASSERT(D.center() == D_res.center());
  CPPUNIT_ASSERT(D.subcenter() == D_res.subcenter());
  CPPUNIT_ASSERT(D.dt() == D_res.dt());
  CPPUNIT_ASSERT(D.mastertable() == D_res.mastertable());
  CPPUNIT_ASSERT(D.localtable() == D_res.localtable());
  CPPUNIT_ASSERT(D.discipline() == D_res.discipline());
  CPPUNIT_ASSERT(D.signdt() == D_res.signdt());
  CPPUNIT_ASSERT(D.status() == D_res.status());
  CPPUNIT_ASSERT(D.datatype() == D_res.datatype());
  CPPUNIT_ASSERT(D.grid().type() == D_res.grid().type());
  CPPUNIT_ASSERT(D.grid().datacrc() == D_res.grid().datacrc());
  CPPUNIT_ASSERT(D.product1().number() == D_res.product1().number());
  CPPUNIT_ASSERT(D.product1().leveltype() == D_res.product1().leveltype());
  CPPUNIT_ASSERT(D.product1().levelvalue() == D_res.product1().levelvalue());

  CPPUNIT_ASSERT(D.data().size() == 3*sizeof(float));

  const float* deficit_res = reinterpret_cast<const float*>(D_res.data().data());
  for (int i=0; i < 3; i++) {
    CPPUNIT_ASSERT(fabs(deficit_res[i] - deficit[i]) < PRECISION);
  }

}
