#include "ctest.h"

#include <commons/meteo_data/tmeteodescr.h>
#include <commons/meteo_data/meteo_data.h>
#include <meteo/commons/control/tmeteocontrol.h>

#include <cross-commons/debug/tlog.h>

#include <qdebug.h>


CPPUNIT_TEST_SUITE_REGISTRATION( CTest );


void CTest::control_limitsurface()
{
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
  TMeteoData data1, data2;
  bool ok;

  TMeteoParam param=TMeteoParam("1", 1, control::NO_CONTROL);
  data1.add(d->descriptor("level_type"), param);

  TMeteoParam param0=TMeteoParam("0", 0, control::NO_CONTROL);
  data1.add(d->descriptor("category"), param0);
  
  TMeteoParam param1=TMeteoParam("12", 12., control::NO_CONTROL);//day
  data1.add(d->descriptor("YY"),param1);
  
  TMeteoParam param2=TMeteoParam("18", 18., control::NO_CONTROL);//hour
  data1.add(d->descriptor("GG"),param2);
  
  TMeteoParam param3=TMeteoParam("1", 1., control::NO_CONTROL);//type of instrumentation for wind measurement
  data1.add(d->descriptor("iw"),param3);
  
  TMeteoParam param4=TMeteoParam("4", 300., control::RIGHT);//height of base of cloud, in m
  data1.add(d->descriptor("h"),param4);
  
  TMeteoParam param5=TMeteoParam("37", 37, control::NO_CONTROL);//horizontal visibility, in km
  data1.add(d->descriptor("Td"),param5);
  
  TMeteoParam param11=TMeteoParam("1", 1, control::NO_CONTROL);
  data1.add(d->descriptor("ix"),param11);

  TMeteoParam param12=TMeteoParam("10", 10, control::NO_CONTROL);
  data1.add(d->descriptor("T"),param12);

  TMeteoParam param13=TMeteoParam("40", 40, control::NO_CONTROL);
  data1.add(d->descriptor("La"),param13);

  TMeteoParam param14=TMeteoParam("4", 4, control::NO_CONTROL);
  data1.add(d->descriptor("MM"),param14);

  TMeteoParam param15=TMeteoParam("100", 100, control::NO_CONTROL);
  data1.add(d->descriptor("ff"),param15);

  ok = TMeteoControl::instance()->control(control::LIMIT_CTRL, &data1);
  CPPUNIT_ASSERT(ok = true);

  CPPUNIT_ASSERT(data1.getParam(d->descriptor("iw")).quality() == control::NO_CONTROL);
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("h")).quality() == control::RIGHT);
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("Td")).quality() == control::DOUBTFUL);
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("T")).quality() == control::RIGHT);
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("ff")).quality() == control::DOUBTFUL);

  // ---

  data2.add(d->descriptor("level_type"),param);
  data2.add(d->descriptor("category"),param0);
  
  TMeteoParam param6=TMeteoParam("32", 32., control::NO_CONTROL);//day
  data2.add(d->descriptor("YY"),param6);
  
  TMeteoParam param7=TMeteoParam("45", 45., control::NO_CONTROL);//hour
  data2.add(d->descriptor("GG"),param7);
  
  TMeteoParam param8=TMeteoParam("7", 7., control::NO_CONTROL);//type of instrumentation for wind measurement
  data2.add(d->descriptor("iw"),param8);
  
  TMeteoParam param9=TMeteoParam("10", 4600., control::MISTAKEN);//height of base of cloud, in m
  data2.add(d->descriptor("h"),param9);
  
  TMeteoParam param10=TMeteoParam("-90", -90, control::MISTAKEN);//horizontal visibility, in km
  data2.add(d->descriptor("Td"),param10);

  data2.add(d->descriptor("ix"),param11);
  data2.add(d->descriptor("La"),param13);
  data2.add(d->descriptor("MM"),param14);

  TMeteoParam param16=TMeteoParam("190", 190, control::NO_CONTROL);
  data2.add(d->descriptor("ff"),param16);

  ok = TMeteoControl::instance()->control(control::LIMIT_CTRL, &data2);
  CPPUNIT_ASSERT(ok = true);

  CPPUNIT_ASSERT(data2.getParam(d->descriptor("iw")).quality() == control::NO_CONTROL);
  CPPUNIT_ASSERT(data2.getParam(d->descriptor("h")).quality() == control::MISTAKEN);
  CPPUNIT_ASSERT(data2.getParam(d->descriptor("Td")).quality() == control::MISTAKEN);
  CPPUNIT_ASSERT(data2.getParam(d->descriptor("ff")).quality() == control::MISTAKEN);

}

void CTest::fillair(TMeteoData* data1, TMeteoData* data2)
{
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
  
  TMeteoParam param=TMeteoParam("61", 61, control::NO_CONTROL);
  data1->add(d->descriptor("category"), param);

  TMeteoParam param0=TMeteoParam("100", 100, control::NO_CONTROL);
  data1->add(d->descriptor("level_type"), param0);

  TMeteoParam param13=TMeteoParam("40", 40, control::NO_CONTROL);
  data1->add(d->descriptor("La"),param13);

  TMeteoParam param14=TMeteoParam("4", 4, control::NO_CONTROL);
  data1->add(d->descriptor("MM"),param14);


  TMeteoData& child = data1->addChild();
  TMeteoParam param1 = TMeteoParam("-20", -20, control::NO_CONTROL);
  child.add(d->descriptor("T"),param1);
  TMeteoParam param3 = TMeteoParam("1000", 1000, control::NO_CONTROL);
  child.add(d->descriptor("P1"),param3);
  TMeteoParam param5 = TMeteoParam("300", 300, control::NO_CONTROL);
  child.add(d->descriptor("hh"),param5);
  child.add(d->descriptor("level_type"),param0);

  TMeteoData& child1 = data1->addChild();
  TMeteoParam param2 = TMeteoParam("-57", -27, control::NO_CONTROL);
  child1.add(d->descriptor("T"),param2);
  TMeteoParam param4 = TMeteoParam("850", 850, control::NO_CONTROL);
  child1.add(d->descriptor("P1"),param4);
  TMeteoParam param51 = TMeteoParam("100", 100, control::NO_CONTROL);
  child1.add(d->descriptor("hh"),param51);
  child1.add(d->descriptor("level_type"),param0);

  TMeteoData& child2 = data1->addChild();
  TMeteoParam param22 = TMeteoParam("-30", -30, control::NO_CONTROL);
  child2.add(d->descriptor("T"),param22);
  TMeteoParam param41 = TMeteoParam("700", 700, control::NO_CONTROL);
  child2.add(d->descriptor("P1"),param41);
  TMeteoParam param52 = TMeteoParam("2350", 2350, control::NO_CONTROL);
  child2.add(d->descriptor("hh"),param52);
  child2.add(d->descriptor("level_type"),param0);


  //---

  data2->add(d->descriptor("level_type"),param);
  data2->add(d->descriptor("category"),param0);
  data2->add(d->descriptor("La"),param13);
  data2->add(d->descriptor("MM"),param14);

  TMeteoData& child3 = data2->addChild();
  child3.add(d->descriptor("P1"),param3);
  TMeteoParam param24 = TMeteoParam("55", 55, control::NO_CONTROL);
  child3.add(d->descriptor("T"),param24);
  TMeteoParam param53 = TMeteoParam("100", 100, control::NO_CONTROL);
  child3.add(d->descriptor("hh"),param53);
  child3.add(d->descriptor("level_type"),param0);


  TMeteoData& child4 = data2->addChild();
  child4.add(d->descriptor("P1"),param4);
  TMeteoParam param6 = TMeteoParam("-100", -100, control::NO_CONTROL);
  child4.add(d->descriptor("T"),param6);
  TMeteoParam param54 = TMeteoParam("200", 200, control::NO_CONTROL);
  child4.add(d->descriptor("hh"),param54);
  child4.add(d->descriptor("level_type"),param0);


  TMeteoData& child5 = data2->addChild();
  child5.add(d->descriptor("P1"),param41);
  TMeteoParam param23 = TMeteoParam("20", 20, control::NO_CONTROL);
  child5.add(d->descriptor("T"),param23);
  TMeteoParam param55 = TMeteoParam("2300", 2300, control::NO_CONTROL);
  child5.add(d->descriptor("hh"),param55); 
  child5.add(d->descriptor("level_type"),param0);
}

void CTest::fill_signair(TMeteoData* data1, TMeteoData* data2)
{
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
  
  TMeteoParam param=TMeteoParam("61", 61, control::NO_CONTROL);
  data1->add(d->descriptor("category"), param);

  TMeteoParam param0=TMeteoParam("100", 100, control::NO_CONTROL);
  data1->add(d->descriptor("level_type"), param0);

  TMeteoParam param01=TMeteoParam("40", 40, control::NO_CONTROL);
  data1->add(d->descriptor("La"),param01);

  TMeteoParam param02=TMeteoParam("4", 4, control::NO_CONTROL);
  data1->add(d->descriptor("MM"),param02);

  TMeteoParam param03=TMeteoParam("156", 156, control::NO_CONTROL);
  data1->add(d->descriptor("h0_station"),param03);


  TMeteoData& child1 = data1->addChild();
  TMeteoParam param11 = TMeteoParam("1027", 1027, control::NO_CONTROL);
  child1.add(d->descriptor("P1"),param11);
  TMeteoParam param12 = TMeteoParam("-1", -1, control::NO_CONTROL);
  child1.add(d->descriptor("T"),param12);
  TMeteoParam param13 = TMeteoParam("5", 5, control::NO_CONTROL);
  child1.add(d->descriptor("ff"),param13);
  child1.add(d->descriptor("level_type"),param0);

  TMeteoData& child2 = data1->addChild();
  TMeteoParam param21 = TMeteoParam("1000", 1000, control::NO_CONTROL);
  child2.add(d->descriptor("P1"),param21);
  TMeteoParam param22 = TMeteoParam("015", -1.5, control::NO_CONTROL);
  child2.add(d->descriptor("T"),param22);
  TMeteoParam param23 = TMeteoParam("6", 6, control::NO_CONTROL);
  child2.add(d->descriptor("ff"),param23);
  TMeteoParam param24 = TMeteoParam("00380", 380, control::NO_CONTROL);
  child2.add(d->descriptor("hh"),param24);
  child2.add(d->descriptor("level_type"),param0);

  TMeteoData& child3 = data1->addChild();
  TMeteoParam param31 = TMeteoParam("075", -7.5, control::NO_CONTROL);
  child3.add(d->descriptor("T"),param31);
  TMeteoParam param32 = TMeteoParam("873", 873, control::NO_CONTROL);
  child3.add(d->descriptor("P1"),param32);
  TMeteoParam param33 = TMeteoParam("9", 9, control::NO_CONTROL);
  child3.add(d->descriptor("ff"),param33);
  child3.add(d->descriptor("level_type"),param0);

  TMeteoData& child4 = data1->addChild();
  TMeteoParam param42 = TMeteoParam("850", 850, control::NO_CONTROL);
  child4.add(d->descriptor("P1"),param42);
  TMeteoParam param41 = TMeteoParam("073", -7.3, control::NO_CONTROL);
  child4.add(d->descriptor("T"),param41);
  TMeteoParam param43 = TMeteoParam("013", 13, control::NO_CONTROL);
  child4.add(d->descriptor("ff"),param43);
  TMeteoParam param44 = TMeteoParam("0085660", 1660, control::NO_CONTROL);
  child4.add(d->descriptor("hh"),param44);
  child4.add(d->descriptor("level_type"),param0);

  TMeteoData& child5 = data1->addChild();
  TMeteoParam param52 = TMeteoParam("711", 711, control::NO_CONTROL);
  child5.add(d->descriptor("P1"),param52);
  TMeteoParam param51 = TMeteoParam("103", -10.3, control::NO_CONTROL);
  child5.add(d->descriptor("T"),param51);
  TMeteoParam param53 = TMeteoParam("10", 10, control::NO_CONTROL);
  child5.add(d->descriptor("ff"),param53);
  child5.add(d->descriptor("level_type"),param0);

  TMeteoData& child6 = data1->addChild();
  TMeteoParam param62 = TMeteoParam("700", 700, control::NO_CONTROL);
  child6.add(d->descriptor("P1"),param62);
  TMeteoParam param61 = TMeteoParam("123", -12.3, control::NO_CONTROL);
  child6.add(d->descriptor("T"),param61);
  TMeteoParam param63 = TMeteoParam("9", 9, control::NO_CONTROL);
  child6.add(d->descriptor("ff"),param63);
  TMeteoParam param64 = TMeteoParam("70171", 3171, control::NO_CONTROL);
  child6.add(d->descriptor("hh"),param64);
  child6.add(d->descriptor("level_type"),param0);

  TMeteoData& child7 = data1->addChild();
  TMeteoParam param71 = TMeteoParam("550", 550, control::NO_CONTROL);
  child7.add(d->descriptor("P1"),param71);
  TMeteoParam param72 = TMeteoParam("247", -24.7, control::NO_CONTROL);
  child7.add(d->descriptor("T"),param72);
  TMeteoParam param73 = TMeteoParam("14", 14, control::NO_CONTROL);
  child7.add(d->descriptor("ff"),param73);
  child7.add(d->descriptor("level_type"),param0);

  //---

  data2->add(d->descriptor("category"),param);
  data2->add(d->descriptor("level_type"),param0);
  data2->add(d->descriptor("La"),param01);
  data2->add(d->descriptor("MM"),param02);
  data2->add(d->descriptor("h0_station"),param03);

  TMeteoData& child1_ = data2->addChild();
  TMeteoParam param11_ = TMeteoParam("1027", 1027, control::NO_CONTROL);
  child1_.add(d->descriptor("P1"),param11_);
  TMeteoParam param12_ = TMeteoParam("-1", -1, control::NO_CONTROL);
  child1_.add(d->descriptor("T"),param12_);
  TMeteoParam param13_ = TMeteoParam("5", 5, control::NO_CONTROL);
  child1_.add(d->descriptor("ff"),param13_);
  child1_.add(d->descriptor("level_type"),param0);

  TMeteoData& child2_ = data2->addChild();
  TMeteoParam param21_ = TMeteoParam("1000", 1000, control::NO_CONTROL);
  child2_.add(d->descriptor("P1"),param21_);
  TMeteoParam param22_ = TMeteoParam("105", -30.5, control::NO_CONTROL);
  child2_.add(d->descriptor("T"),param22_);
  TMeteoParam param23_ = TMeteoParam("6", 6, control::NO_CONTROL);
  child2_.add(d->descriptor("ff"),param23_);
  TMeteoParam param24_ = TMeteoParam("00380", 380, control::NO_CONTROL);
  child2_.add(d->descriptor("hh"),param24_);
  child2_.add(d->descriptor("level_type"),param0);

  TMeteoData& child3_ = data2->addChild();
  TMeteoParam param31_ = TMeteoParam("075", -7.5, control::NO_CONTROL);
  child3_.add(d->descriptor("T"),param31_);
  TMeteoParam param32_ = TMeteoParam("873", 873, control::NO_CONTROL);
  child3_.add(d->descriptor("P1"),param32_);
  TMeteoParam param33_ = TMeteoParam("9", 9, control::NO_CONTROL);
  child3_.add(d->descriptor("ff"),param33_);
  child3_.add(d->descriptor("level_type"),param0);

  TMeteoData& child4_ = data2->addChild();
  TMeteoParam param42_ = TMeteoParam("850", 850, control::NO_CONTROL);
  child4_.add(d->descriptor("P1"),param42_);
  TMeteoParam param41_ = TMeteoParam("073", -7.3, control::NO_CONTROL);
  child4_.add(d->descriptor("T"),param41_);
  TMeteoParam param43_ = TMeteoParam("003", 13, control::NO_CONTROL);
  child4_.add(d->descriptor("ff"),param43_);
  TMeteoParam param44_ = TMeteoParam("0085660", 1660, control::NO_CONTROL);
  child4_.add(d->descriptor("hh"),param44_);
  child4_.add(d->descriptor("level_type"),param0);

  TMeteoData& child5_ = data2->addChild();
  TMeteoParam param52_ = TMeteoParam("711", 711, control::NO_CONTROL);
  child5_.add(d->descriptor("P1"),param52_);
  TMeteoParam param51_ = TMeteoParam("103", -10.3, control::NO_CONTROL);
  child5_.add(d->descriptor("T"),param51_);
  TMeteoParam param53_ = TMeteoParam("10", 10, control::NO_CONTROL);
  child5_.add(d->descriptor("ff"),param53_);
  child5_.add(d->descriptor("level_type"),param0);

  TMeteoData& child6_ = data2->addChild();
  TMeteoParam param62_ = TMeteoParam("700", 700, control::NO_CONTROL);
  child6_.add(d->descriptor("P1"),param62_);
  TMeteoParam param61_ = TMeteoParam("013", -1.3, control::NO_CONTROL);
  child6_.add(d->descriptor("T"),param61_);
  TMeteoParam param63_ = TMeteoParam("1", 1, control::NO_CONTROL);
  child6_.add(d->descriptor("ff"),param63_);
  TMeteoParam param64_ = TMeteoParam("70171", 3171, control::NO_CONTROL);
  child6_.add(d->descriptor("hh"),param64_);
  child6_.add(d->descriptor("level_type"),param0);

  TMeteoData& child7_ = data2->addChild();
  TMeteoParam param71_ = TMeteoParam("550", 550, control::NO_CONTROL);
  child7_.add(d->descriptor("P1"),param71_);
  TMeteoParam param72_ = TMeteoParam("247", -24.7, control::NO_CONTROL);
  child7_.add(d->descriptor("T"),param72_);
  TMeteoParam param73_ = TMeteoParam("14", 14, control::NO_CONTROL);
  child7_.add(d->descriptor("ff"),param73_);
  child7_.add(d->descriptor("level_type"),param0);
  
}


void CTest::control_limitair()
{
  TMeteoData data1, data2;
  bool ok;
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();

  fillair(&data1, &data2);
  
  ok = TMeteoControl::instance()->control(control::LIMIT_CTRL, &data1);
  CPPUNIT_ASSERT(ok = true);

  const TMeteoData* md = data1.findChild(meteodescr::kIsobarLevel, 1000);
  CPPUNIT_ASSERT(md != 0);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("T")).quality() == control::RIGHT);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("hh")).quality() == control::RIGHT);

  md = data1.findChild(meteodescr::kIsobarLevel, 850);
  CPPUNIT_ASSERT(md != 0);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("T")).quality() == control::RIGHT);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("hh")).quality() == control::DOUBTFUL); 
 
  md = data1.findChild(meteodescr::kIsobarLevel, 700);
  CPPUNIT_ASSERT(md != 0);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("T")).quality() == control::RIGHT);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("hh")).quality() == control::RIGHT);
 
 
  //---
  ok = TMeteoControl::instance()->control(control::LIMIT_CTRL, &data2);
  CPPUNIT_ASSERT(ok = true);

  md = data2.findChild(meteodescr::kIsobarLevel, 1000);
  CPPUNIT_ASSERT(md != 0);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("T")).quality() == control::DOUBTFUL);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("hh")).quality() == control::RIGHT);

  md = data2.findChild(meteodescr::kIsobarLevel, 850);
  CPPUNIT_ASSERT(md != 0);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("T")).quality() == control::MISTAKEN);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("hh")).quality() == control::RIGHT);

  md = data2.findChild(meteodescr::kIsobarLevel, 700);
  CPPUNIT_ASSERT(md != 0);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("T")).quality() == control::RIGHT);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("hh")).quality() == control::DOUBTFUL);}


void CTest::control_consistair()
{
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
  TMeteoData data1, data2;
  bool ok;

  fillair(&data1, &data2);

  ok = TMeteoControl::instance()->control(control::CONSISTENCY_CTRL, &data1);
  CPPUNIT_ASSERT(ok = true);

  const TMeteoData* md = data1.findChild(meteodescr::kIsobarLevel, 1000);
  CPPUNIT_ASSERT(md != 0);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("T")).quality() == control::RIGHT);

  md = data1.findChild(meteodescr::kIsobarLevel, 850);
  CPPUNIT_ASSERT(md != 0);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("T")).quality() == control::RIGHT);

  md = data1.findChild(meteodescr::kIsobarLevel, 700);
  CPPUNIT_ASSERT(md != 0);
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("T")).quality() == control::RIGHT);

  //---

  ok = TMeteoControl::instance()->control(control::CONSISTENCY_CTRL, &data2);
  CPPUNIT_ASSERT(ok = true);

  md = data2.findChild(meteodescr::kIsobarLevel, 1000);
  CPPUNIT_ASSERT(md != 0);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("T")).quality() == control::MISTAKEN);

  md = data2.findChild(meteodescr::kIsobarLevel, 850);
  CPPUNIT_ASSERT(md != 0);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("T")).quality() == control::MISTAKEN);

  md = data2.findChild(meteodescr::kIsobarLevel, 700);
  CPPUNIT_ASSERT(md != 0);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("T")).quality() == control::MISTAKEN);

}


void CTest::control_consistsurf()
{
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
  TMeteoData data1, data2;
  bool ok;

  TMeteoParam param = TMeteoParam("1", 1, control::NO_CONTROL);
  data1.add(d->descriptor("level_type"), param);

  TMeteoParam param0=TMeteoParam("0", 0, control::NO_CONTROL);
  data1.add(d->descriptor("category"), param0);

  TMeteoParam param00=TMeteoParam("2", 1, control::NO_CONTROL);
  data1.add(d->descriptor("ix"), param00);


  TMeteoParam param1=TMeteoParam("0", 0, control::NO_CONTROL);
  data1.add(d->descriptor("dd"), param1);

  TMeteoParam param2=TMeteoParam("0", 0, control::NO_CONTROL);
  data1.add(d->descriptor("ff"), param2);

  TMeteoParam param3=TMeteoParam("10", 10, control::NO_CONTROL);
  data1.add(d->descriptor("T"), param3);

  TMeteoParam param4=TMeteoParam("80", 80, control::NO_CONTROL);
  data1.add(d->descriptor("w"), param4);

  TMeteoParam param5=TMeteoParam("1", 1, control::NO_CONTROL);
  data1.add(d->descriptor("Td"), param5);

  TMeteoParam param6=TMeteoParam("-1", -1, control::NO_CONTROL);
  data1.add(d->descriptor("sss"), param6);
  
  ok = TMeteoControl::instance()->control(control::CONSISTENCY_CTRL, &data1);
  CPPUNIT_ASSERT(ok = true);

  CPPUNIT_ASSERT(data1.getParam(d->descriptor("dd")).quality() == control::SPECIAL_VALUE);
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("ff")).quality() == control::RIGHT);
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("T")).quality()  == control::RIGHT);
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("w")).quality()  == control::RIGHT);
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("Td")).quality() == control::RIGHT);
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("sss")).quality() == control::SPECIAL_VALUE);
  //----


  data2.add(d->descriptor("level_type"), param);  
  data2.add(d->descriptor("category"), param0);
  data2.add(d->descriptor("ix"), param00);

  TMeteoParam param11=TMeteoParam("0", 0, control::NO_CONTROL);
  data2.add(d->descriptor("dd"), param11);

  TMeteoParam param22=TMeteoParam("3", 3, control::NO_CONTROL);
  data2.add(d->descriptor("ff"), param22);

  TMeteoParam param33=TMeteoParam("10", 10, control::NO_CONTROL);
  data2.add(d->descriptor("T"), param33);

  TMeteoParam param44=TMeteoParam("75", 75, control::NO_CONTROL);
  data2.add(d->descriptor("w"), param44);

  TMeteoParam param55=TMeteoParam("78", 78, control::NO_CONTROL);
  data2.add(d->descriptor("Td"), param55);



  ok = TMeteoControl::instance()->control(control::CONSISTENCY_CTRL, &data2);
  CPPUNIT_ASSERT(ok = true);

  
  CPPUNIT_ASSERT(data2.getParam(d->descriptor("dd")).quality() == control::MISTAKEN);
  CPPUNIT_ASSERT(data2.getParam(d->descriptor("ff")).quality() == control::MISTAKEN);
  CPPUNIT_ASSERT(data2.getParam(d->descriptor("T")).quality() == control::DOUBTFUL);
  CPPUNIT_ASSERT(data2.getParam(d->descriptor("w")).quality() == control::DOUBTFUL);
  CPPUNIT_ASSERT(data2.getParam(d->descriptor("Td")).quality() == control::DOUBTFUL);

}


void CTest::control_sign()
{ 
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
  TMeteoData data1, data2;

  fill_signair(&data1, &data2);
  
  uint mask = control::CONSISTENCY_SIGN;
  bool ok;

  ok = TMeteoControl::instance()->control(mask, &data1);
  CPPUNIT_ASSERT(ok = true);

  //data1.printData();

  QList<TMeteoData*> childs = data1.findChilds(d->descriptor("P1"), meteodescr::kIsobarLevel);
  for (int idx = 0; idx < childs.size(); idx++) {
    CPPUNIT_ASSERT(childs.at(idx)->getParam(d->descriptor("T")).quality() <= control::NO_CONTROL);
    CPPUNIT_ASSERT(childs.at(idx)->getParam(d->descriptor("ff")).quality() <= control::NO_CONTROL);
    if (childs.at(idx)->hasParam(d->descriptor("hh"))) {
      CPPUNIT_ASSERT(childs.at(idx)->getParam(d->descriptor("hh")).quality() <= control::NO_CONTROL);
    }
  }

  ok = TMeteoControl::instance()->control(mask, &data2);
  CPPUNIT_ASSERT(ok = true);

  //data2.printData();

  QList<TMeteoData*> childs2 = data2.findChilds(d->descriptor("P1"), meteodescr::kIsobarLevel);
  for (int idx = 0; idx < childs.size(); idx++) {
    if (childs2.at(idx)->getValue(d->descriptor("P1"), -1) == 850) {
      CPPUNIT_ASSERT(childs2.at(idx)->getParam(d->descriptor("T")).quality() <= control::NO_CONTROL);
    } else {
      CPPUNIT_ASSERT(childs2.at(idx)->getParam(d->descriptor("T")).quality() == control::DOUBTFUL);
    }
    if (childs2.at(idx)->getValue(d->descriptor("P1"), -1) >= 1000) {
      CPPUNIT_ASSERT(childs2.at(idx)->getParam(d->descriptor("ff")).quality() <= control::NO_CONTROL);
    } else {
      CPPUNIT_ASSERT(childs2.at(idx)->getParam(d->descriptor("ff")).quality() <= control::DOUBTFUL);
    }
  }
 
}


void CTest::control_wind()
{
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
  TMeteoData data1, data2;
  bool ok;

  TMeteoParam param0=TMeteoParam("61", 61, control::NO_CONTROL);
  data1.add(d->descriptor("category"), param0);
  TMeteoParam param00=TMeteoParam("100", 100, control::NO_CONTROL);
  data1.add(d->descriptor("level_type"), param00);

  TMeteoData& child = data1.addChild();
  TMeteoParam param1=TMeteoParam("150", 150, control::NO_CONTROL);
  child.add(d->descriptor("P1"), param1);
  TMeteoParam param4=TMeteoParam("10", 10, control::NO_CONTROL);
  child.add(d->descriptor("dd"), param4);
  TMeteoParam param8=TMeteoParam("20", 20, control::NO_CONTROL);
  child.add(d->descriptor("ff"), param8);
  child.add(d->descriptor("level_type"),param00);

  TMeteoData& child1 = data1.addChild();
  TMeteoParam param2=TMeteoParam("200", 200, control::NO_CONTROL);
  child1.add(d->descriptor("P1"), param2);
  TMeteoParam param5=TMeteoParam("50", 50, control::NO_CONTROL);
  child1.add(d->descriptor("dd"), param5);
  TMeteoParam param7=TMeteoParam("10", 10, control::NO_CONTROL);
  child1.add(d->descriptor("ff"), param7);
  child1.add(d->descriptor("level_type"),param00);
  
  TMeteoData& child2 = data1.addChild();
  TMeteoParam param3=TMeteoParam("300", 300, control::NO_CONTROL);
  child2.add(d->descriptor("P1"), param3);
  TMeteoParam param6=TMeteoParam("100", 100, control::NO_CONTROL);
  child2.add(d->descriptor("dd"), param6);
  TMeteoParam param9=TMeteoParam("100", 100, control::NO_CONTROL);
  child2.add(d->descriptor("ff"), param9);
  child2.add(d->descriptor("level_type"),param00);

  ok = TMeteoControl::instance()->control(control::CONSISTENCY_CTRL, &data1);
  CPPUNIT_ASSERT(ok = true);

  const TMeteoData* md = data1.findChild(meteodescr::kIsobarLevel, 150);
  CPPUNIT_ASSERT(md != 0);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("dd")).quality() == control::RIGHT);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("ff")).quality() == control::RIGHT);

  md = data1.findChild(meteodescr::kIsobarLevel, 200);
  CPPUNIT_ASSERT(md != 0);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("dd")).quality() == control::DOUBTFUL);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("ff")).quality() == control::DOUBTFUL);
  
  md = data1.findChild(meteodescr::kIsobarLevel, 300);
  CPPUNIT_ASSERT(md != 0);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("dd")).quality() == control::DOUBTFUL);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("ff")).quality() == control::DOUBTFUL);

  //-----

  data2.add(d->descriptor("category"), param0);
  data2.add(d->descriptor("level_type"), param00);

  TMeteoData& child3 = data2.addChild();
  child3.add(d->descriptor("P1"), param1);
  child3.add(d->descriptor("dd"), param4);
  TMeteoParam param91=TMeteoParam("10", 10, control::NO_CONTROL);
  child3.add(d->descriptor("ff"), param91);
  child3.add(d->descriptor("level_type"),param00);

  TMeteoData& child4 = data2.addChild();
  child4.add(d->descriptor("P1"), param2);
  child4.add(d->descriptor("dd"), param5);
  TMeteoParam param81=TMeteoParam("-10.6", -10.5, control::NO_CONTROL);
  child4.add(d->descriptor("ff"), param81);
  child4.add(d->descriptor("level_type"),param00);

  TMeteoData& child5 = data2.addChild();
  child5.add(d->descriptor("P1"), param3);
  child5.add(d->descriptor("dd"), param6);
  TMeteoParam param71=TMeteoParam("-10.6", -10.6, control::NO_CONTROL); //для формального попадания в формулу
  child5.add(d->descriptor("ff"), param71);
  child5.add(d->descriptor("level_type"),param00);

  ok = TMeteoControl::instance()->control(control::CONSISTENCY_CTRL, &data2);
  CPPUNIT_ASSERT(ok = true);

  md = data2.findChild(meteodescr::kIsobarLevel, 150);
  CPPUNIT_ASSERT(md != 0);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("dd")).quality() == control::RIGHT);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("ff")).quality() == control::DOUBTFUL);

  md = data2.findChild(meteodescr::kIsobarLevel, 200);
  CPPUNIT_ASSERT(md != 0);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("dd")).quality() == control::RIGHT);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("ff")).quality() == control::DOUBTFUL);
  
  md = data2.findChild(meteodescr::kIsobarLevel, 300);
  CPPUNIT_ASSERT(md != 0);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("dd")).quality() == control::RIGHT);
  CPPUNIT_ASSERT(md->getParam(d->descriptor("ff")).quality() == control::RIGHT);
}


void CTest::control_time()
{
  trc;
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
  TMeteoData data1, data2, data3;
  bool ok;

  data1.add(d->descriptor("level_type"), TMeteoParam("", 1, control::NO_CONTROL));
  
  TMeteoParam param0 = TMeteoParam("0", 0, control::NO_CONTROL);

  data1.add(d->descriptor("category"), param0);
  TMeteoParam param1 = TMeteoParam("", 10, control::NO_CONTROL);
  data1.add(d->descriptor("T"), param1);
  TMeteoParam param2 = TMeteoParam("", 3, control::NO_CONTROL);
  data1.add(d->descriptor("Td"), param2);
  TMeteoParam param3 = TMeteoParam("", -0.6, control::NO_CONTROL);
  data1.add(d->descriptor("p"), param3);
  TMeteoParam param4 = TMeteoParam("", 12, control::NO_CONTROL);//day
  data1.add(d->descriptor("YY"), param4);
  TMeteoParam param5 = TMeteoParam("", 18, control::NO_CONTROL);//hour
  data1.add(d->descriptor("GG"), param5);

  TMeteoParam param6= TMeteoParam("", 991.7, control::NO_CONTROL);
  data1.add(d->descriptor("P0"), param6);
  TMeteoParam param8 = TMeteoParam("", 3, control::NO_CONTROL);
  data1.add(d->descriptor("a"), param8);
  TMeteoParam param9 = TMeteoParam("", 10, control::NO_CONTROL);
  data1.add(d->descriptor("P"), param9);

  //prev good
  data2.add(d->descriptor("level_type"), TMeteoParam("", 1, control::NO_CONTROL));
  data2.add(d->descriptor("category"), param0);
  TMeteoParam param21 = TMeteoParam("", 12, control::NO_CONTROL);
  data2.add(d->descriptor("T"), param21);
  TMeteoParam param22 = TMeteoParam("", 1, control::NO_CONTROL);
  data2.add(d->descriptor("Td"), param22);
  TMeteoParam param23= TMeteoParam("", 0.1, control::NO_CONTROL);
  data2.add(d->descriptor("p"), param23);
  TMeteoParam param24 = TMeteoParam("", 12., control::NO_CONTROL);//day
  data2.add(d->descriptor("YY"), param24);
  TMeteoParam param25 = TMeteoParam("", 16., control::NO_CONTROL);//hour
  data2.add(d->descriptor("GG"), param25);

  TMeteoParam param26 = TMeteoParam("", 991.7, control::NO_CONTROL);
  data2.add(d->descriptor("P0"), param26);
  TMeteoParam param28 = TMeteoParam("", 3, control::NO_CONTROL);
  data2.add(d->descriptor("a"), param28);
  TMeteoParam param29 = TMeteoParam("", 10, control::NO_CONTROL);
  data2.add(d->descriptor("P"), param29);

  //prev bad
  data3.add(d->descriptor("level_type"), TMeteoParam("", 1, control::NO_CONTROL));
  data3.add(d->descriptor("category"), param0);
  TMeteoParam param31 = TMeteoParam("", 20, control::NO_CONTROL);
  data3.add(d->descriptor("T"), param31);
  TMeteoParam param32 = TMeteoParam("", 13, control::NO_CONTROL);
  data3.add(d->descriptor("Td"), param32);
  TMeteoParam param33 = TMeteoParam("", 12, control::NO_CONTROL);
  data3.add(d->descriptor("p"), param33);
  TMeteoParam param34 = TMeteoParam("", 12., control::NO_CONTROL);//day
  data3.add(d->descriptor("YY"), param34);
  TMeteoParam param35 = TMeteoParam("", 16., control::NO_CONTROL);//hour
  data3.add(d->descriptor("GG"), param35);

  TMeteoParam param36 = TMeteoParam("", 7, control::NO_CONTROL);
  data3.add(d->descriptor("P0"), param36);
  TMeteoParam param38 = TMeteoParam("", 3, control::NO_CONTROL);
  data3.add(d->descriptor("a"), param38);
  TMeteoParam param39 = TMeteoParam("", 5, control::NO_CONTROL);
  data3.add(d->descriptor("P"), param39);

  debug_log << "first";
  ok = TMeteoControl::instance()->control(control::TIME_CTRL, &data1, &data2);
  var(ok);
  CPPUNIT_ASSERT(ok = true);
  
  data1.printData();
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("T")).quality() == control::RIGHT);
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("Td")).quality() == control::RIGHT);
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("p")).quality() == control::RIGHT);
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("P")).quality() == control::RIGHT);
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("a")).quality() == control::NO_CONTROL);

  CPPUNIT_ASSERT(data2.getParam(d->descriptor("T")).quality() == control::NO_CONTROL);
  CPPUNIT_ASSERT(data2.getParam(d->descriptor("Td")).quality() == control::NO_CONTROL);
  CPPUNIT_ASSERT(data2.getParam(d->descriptor("p")).quality() == control::NO_CONTROL);
  CPPUNIT_ASSERT(data2.getParam(d->descriptor("P")).quality() == control::NO_CONTROL);
  CPPUNIT_ASSERT(data2.getParam(d->descriptor("a")).quality() == control::NO_CONTROL);

  debug_log << "second";
  ok = TMeteoControl::instance()->control(control::TIME_CTRL, &data1, &data3);
  CPPUNIT_ASSERT(ok = true);
  var(ok);
  data1.printData();
  
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("T")).quality() == control::DOUBTFUL);
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("Td")).quality() == control::DOUBTFUL);
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("p")).quality() == control::DOUBTFUL);
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("P")).quality() == control::DOUBTFUL);
  CPPUNIT_ASSERT(data1.getParam(d->descriptor("a")).quality() == control::NO_CONTROL);


  CPPUNIT_ASSERT(data3.getParam(d->descriptor("T")).quality() == control::NO_CONTROL);
  CPPUNIT_ASSERT(data3.getParam(d->descriptor("Td")).quality() == control::NO_CONTROL);
  CPPUNIT_ASSERT(data3.getParam(d->descriptor("p")).quality() == control::NO_CONTROL);
  CPPUNIT_ASSERT(data3.getParam(d->descriptor("P")).quality() == control::NO_CONTROL);
  CPPUNIT_ASSERT(data3.getParam(d->descriptor("a")).quality() == control::NO_CONTROL);

}
