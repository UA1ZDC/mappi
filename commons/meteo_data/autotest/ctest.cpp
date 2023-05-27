#include <commons/meteo_data/tmeteodescr.h>
#include <commons/meteo_data/complexmeteo.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include "ctest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTest );

//! проверка получения дескрипторов через TMeteoDescriptor
void CTest::f1()
{
  descr_t descr = TMeteoDescriptor::instance()->descriptor("T");
  CPPUNIT_ASSERT(descr == 12101);

  descr = TMeteoDescriptor::instance()->descriptor("qwe");
  CPPUNIT_ASSERT(descr == -1);

  QList<QString> names = TMeteoDescriptor::instance()->names();

  QHash<descr_t, QString> descrList;
  for (int i=0; i< names.count(); i++) {
    descr = TMeteoDescriptor::instance()->descriptor(names.at(i));
    if (descrList.contains(descr)) {
      CPPUNIT_FAIL(QString("Descriptor repeated %1 (%2, %3)").arg(descr).arg(descrList.value(descr)).arg(names.at(i)).toStdString());
    } else {
      descrList.insert(descr, names.at(i));
    }
  }
  
  CPPUNIT_ASSERT(descrList.count() == names.count());

  meteodescr::Property prop;
  TMeteoDescriptor::instance()->property("CL", &prop);
  CPPUNIT_ASSERT(prop.units == "NO");
  CPPUNIT_ASSERT(prop.description == "Облака нижнего яруса");

  CPPUNIT_ASSERT(TMeteoDescriptor::instance()->description("CM") == "Облака среднего яруса");

  TMeteoDescriptor::instance()->property("dw1", &prop);
  CPPUNIT_ASSERT(prop.unitsRu == "°");
  TMeteoDescriptor::instance()->property("F4", &prop);
  CPPUNIT_ASSERT(prop.unitsRu == QString("кДж/м²"));
  TMeteoDescriptor::instance()->property("S0S0", &prop);
  CPPUNIT_ASSERT(prop.unitsRu == QString("‰"));

  QString description = TMeteoDescriptor::instance()->description(20012, -1, 12);
  CPPUNIT_ASSERT(description == "Облака верхнего яруса");
  description = TMeteoDescriptor::instance()->description(22023, 1, 10);
  CPPUNIT_ASSERT(description == "Высота зыби 2 системы");
}

//доп имена
void CTest::f2()
{
  TMeteoData data;
  
  data.set("CL", TMeteoParam("32", 32, control::NO_CONTROL));
  data.set("CH", TMeteoParam("12", 12, control::NO_CONTROL));

  data.set("CH", TMeteoParam("15", 15, control::NO_CONTROL));//заменяем CH
  data.set("CH", TMeteoParam("33", 33, control::NO_CONTROL));//заменяем CL

  CPPUNIT_ASSERT(data.meteoParam("CL").value() == 33);
  CPPUNIT_ASSERT(data.meteoParam("CH").value() == 15);

  data.set("Hw1s", TMeteoParam("31", 31, control::NO_CONTROL));
  data.set("Hw2s", TMeteoParam("32", 32, control::NO_CONTROL));
  data.set("Hw1s", TMeteoParam("1", 1, control::NO_CONTROL));

  CPPUNIT_ASSERT(data.meteoParam("Hw1s").value() == 1);
  CPPUNIT_ASSERT(data.meteoParam("Hw2s").value() == 32);

  data.add("Hw1s", TMeteoParam("3", 3, control::NO_CONTROL));
  data.add("CL", TMeteoParam("32", 32, control::NO_CONTROL));

  int cnt = data.remove("Hw1s");
  CPPUNIT_ASSERT(cnt == 1);

  cnt = data.remove("CL");
  CPPUNIT_ASSERT(cnt == 2);

  //NOTE так нельзя добавлять после удаления. будут перезаписываться значения. 
  data.add("CL", TMeteoParam("35", 35, control::NO_CONTROL));

  CPPUNIT_ASSERT(data.countParam(TMeteoDescriptor::instance()->descriptor("C"), false) == 1);
}

//! проверка сохранения, изменения и чтения в TMeteoData
void CTest::f3()
{
  TMeteoData data;
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();

  data.add(d->descriptor("YY"), "3", 3, control::NO_CONTROL);

  data.add(d->descriptor("GG"), "5", 5, control::NO_CONTROL);
  data.set(d->descriptor("GG"), "6", 6, control::NO_CONTROL);

  data.add(d->descriptor("T"), "10", 10, control::NO_CONTROL);
  data.add(d->descriptor("T"), "20", 20, control::NO_CONTROL);
  data.add(d->descriptor("T"), "30", 30, control::NO_CONTROL);
  data.set(d->descriptor("T"), "40", 40, control::NO_CONTROL);
  data.add(d->descriptor("T"), "50", 50, control::NO_CONTROL);
  data.add(d->descriptor("T"), "60", 60, control::NO_CONTROL);

  data.add("CL", TMeteoParam("12", 12, control::NO_CONTROL));

  CPPUNIT_ASSERT(data.countParam(d->descriptor("T"), true) == 5);
  CPPUNIT_ASSERT(data.hasParam(d->descriptor("GG")) == true);
  CPPUNIT_ASSERT(data.hasParam(-1) == false);
  
  

  float v1 = 555;
  v1 = data.getValue(-1, 333);

  CPPUNIT_ASSERT(v1 == 333);
  v1 = data.getValue(d->descriptor("GG"), 333);
  CPPUNIT_ASSERT(v1 == 6);

  QDateTime dt = d->dateTime(data);
  QDateTime dtRev = QDateTime::currentDateTime();
  dtRev.setDate(QDate(dtRev.date().year(), dtRev.date().month(), 3));
  dtRev.setTime(QTime(6, 0, 0));
  dtRev.setTimeSpec(Qt::UTC);

  CPPUNIT_ASSERT(dt == dtRev);

  const TMeteoParam& p1 = data.getParam(d->descriptor("YY"));
  CPPUNIT_ASSERT(p1.value() == 3);
  CPPUNIT_ASSERT(p1.quality() == control::NO_CONTROL);

  CPPUNIT_ASSERT(data.countParam(d->descriptor("GG"), true) == 1);
  
  CPPUNIT_ASSERT(data.getParam(d->descriptor("T"), 0).value() == 10);
  CPPUNIT_ASSERT(data.getParam(d->descriptor("T"), 100).value() == 20);
  CPPUNIT_ASSERT(data.getParam(d->descriptor("T"), 200).value() == 40);

  QStringList names = data.allNames();
  CPPUNIT_ASSERT(names.size() == 4);
  QMap<QString, TMeteoParam> all = data.allByNames();
  CPPUNIT_ASSERT(all.value("YY").value() == 3);

  //--

  TMeteoData* child = &data.addChild();
  child->add(d->descriptor("level_type"), "1", 1, control::NO_CONTROL);
  child->add(d->descriptor("T"), "10", 10, control::NO_CONTROL);
  child->add(d->descriptor("P1"), "1", 1, control::NO_CONTROL);

  int cnt = data.remove("T");
  CPPUNIT_ASSERT(cnt == 6);
  
  // int idx = data.findIndex(d->descriptor("T"), 20);
  // CPPUNIT_ASSERT(idx == 100);
  // TMeteoParam* Tptr = data.paramPtr(d->descriptor("T"), idx);
  // Tptr->setQuality(control::DOUBTFUL);
  
  // CPPUNIT_ASSERT(data.getParam(d->descriptor("T"), 100).quality() == control::DOUBTFUL);

  // QList<int> indexes;
  // indexes << 100 << 200 << 400;
  // QList<TMeteoParam*> tList = data.getParamList(d->descriptor("T"), indexes);
  // tList[1]->setQuality(control::RIGHT);

  //  data.printData();

  //  CPPUNIT_ASSERT(data.getParam(d->descriptor("T"), 200).quality() == control::RIGHT);
}

//проверка сериализации и дессериализации
void CTest::f4()
{
  TMeteoData data;
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();

  data.add(d->descriptor("YY"), "3", 3, control::RIGHT);
  data.add(d->descriptor("GG"), "5", 5, control::DOUBTFUL);

  TMeteoData* child = &data.addChild();
  child->add(d->descriptor("level_type"), "100", 100, control::MISTAKEN);
  child->add(d->descriptor("T"), "10", 10, control::MISTAKEN);
  child->add(d->descriptor("P1"), "1", 1, control::NO_CONTROL);

  child = &child->addChild();
  child->add(d->descriptor("level_type"), "7", 7, control::MISTAKEN);
  child->add(d->descriptor("T"), "20", 20, control::RIGHT);
  child->add(d->descriptor("P1"), "2", 2, control::RIGHT);

  child = &data.addChild();
  child->add(d->descriptor("level_type"), TMeteoParam("100", 100, control::NO_CONTROL, "ups"));
  child->add(d->descriptor("T"), "30", 30, control::NO_OBSERVE);
  child->add(d->descriptor("P1"), "3", 3, control::DOUBTFUL);
  
  child = &data.addChild();
  child->add(d->descriptor("level_type"), "100", 100, control::RIGHT);
  child->add(d->descriptor("T"), "40", 40, control::RIGHT);
  child->add(d->descriptor("P1"), "4", 4, control::RIGHT);

  
  QByteArray ba;
  data >> ba;

  TMeteoData data1;
  data1 << ba;

  CPPUNIT_ASSERT(data1.hasParam(d->descriptor("YY"), false) == true);
  CPPUNIT_ASSERT(data1.hasParam(d->descriptor("T"), false) == false);
  CPPUNIT_ASSERT(data1.hasParam(d->descriptor("T")) == true);

  CPPUNIT_ASSERT(data1.count() == 2);
  CPPUNIT_ASSERT(data1.getValue(d->descriptor("P1"), 2) == 1);


  data1.add(d->descriptor("Hw1"), "3", 3, control::RIGHT);
  data1.add(d->descriptor("Hw1"), "5", 5, control::RIGHT);
  CPPUNIT_ASSERT(data1.meteoParam("Hw1s").value() == 3);
  CPPUNIT_ASSERT(data1.meteoParam("Hw2s").value() == 5);

  const TMeteoData* md = data1.findChild(meteodescr::kIsobarLevel, 3);
  CPPUNIT_ASSERT(md->meteoParam("T").value() == 30);
  CPPUNIT_ASSERT(md->meteoParam("level_type").description() == "ups");

  // QList<QPair<descr_t, TMeteoParam> > mp = data.allParams();  
  // qDebug() << mp;

  //data.printData();
  
  ComplexMeteo cm;
  cm.setLevel(1, 0);
  cm.fill(data);
  cm.setDtType(1);
  cm.setDt1(QDateTime(QDate(2018, 12, 12), QTime(0,0)));
  cm.setDt2(QDateTime(QDate(2018, 12, 12), QTime(0,0)));
  cm.add(d->descriptor("Hw1"), "3", 3, control::RIGHT);
  cm.add(d->descriptor("Hw1"), "5", 5, control::RIGHT);
  //cm.printData();
  
  QByteArray bacm;
  cm >> bacm;
  ComplexMeteo cm1;
  cm1 << bacm;
  //cm1.printData();

  CPPUNIT_ASSERT(cm1.dtBeg() == QDateTime(QDate(2018, 12, 12), QTime(0,0)));
  CPPUNIT_ASSERT(cm1.hasParam(d->descriptor("T"), false) == false);
  CPPUNIT_ASSERT(cm1.hasParam(d->descriptor("T")) == true);


  CPPUNIT_ASSERT(cm1.meteoParam("Hw1s").value() == 3);
  CPPUNIT_ASSERT(cm1.meteoParam("Hw2s").value() == 5);

  ComplexMeteo* cmd = static_cast<ComplexMeteo*>(cm1.child(0));
  CPPUNIT_ASSERT(cmd->meteoParam("T").value() == 20);
}
