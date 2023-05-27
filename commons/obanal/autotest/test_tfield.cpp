#include "autotest.h"

class Test_TField : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE( Test_TField );

  CPPUNIT_TEST( metainfo );

  CPPUNIT_TEST_SUITE_END();

public:
  void metainfo();
};


T_REGISTER_TEST( Test_TField, "tfield" );

//
//

#include <qcoreapplication.h>

#include <commons/obanal/tfield.h>

//struct TestData
//{
//  TestData(const QString& d, double v, bool a)
//    : descr(d), value(v), alert(a)
//  {}

//  QString descr;
//  double value;
//  bool alert;
//};

struct TestMetaInfoItem
{
  TestMetaInfoItem(const QString& aname, const QString& avalue)
    : name(aname), value(avalue)
  {}

  QString name;
  QString value;
};

//! TEST:
void Test_TField::metainfo()
{
//  meteo::gGlobalObj(new meteo::UkazGlobal);
//  CPPUNIT_ASSERT( meteo::ukaz::Settings::instance()->load() );

//  QList<TestData> in = QList<TestData>()
//      << TestData("ff", 0, false)
//      << TestData("ff", 15, true)
//      << TestData("ff", 25, true)
//      << TestData("precip", 0, false)
//      << TestData("precip", 50, true)
//      << TestData("precip", 75, true)
//      << TestData("T", 0, false)
//      << TestData("T", -40, true)
//      << TestData("T", -50, true)
//      << TestData("T", 35, true)
//      << TestData("T", 45, true)
//      << TestData("V", 0, true)
//      << TestData("V", 1000, true)
//      << TestData("V", 2000, false)
//         ;

//  foreach ( const TestData& i, in ) {
//    bool ok = false;
//    bool alert = meteo::ukaz::isHazard(i.descr, i.value, &ok);
//    QString msg = QObject::tr("descr: %1, value: %2, alert: %3").arg(i.descr).arg(i.value).arg(i.alert);
//    CPPUNIT_ASSERT_MESSAGE( msg.toStdString(), ok );
//    CPPUNIT_ASSERT_EQUAL_MESSAGE( msg.toStdString(), i.alert, alert );
//  }

  const QString appPath = qApp->applicationDirPath();
  const QStringList inputFiles = { "12101_0_7_34_4_1_302400.obn" };

  QMap<QString,QString> map;
  map["12101_0_7_34_4_1_302400.obn.kolFi"] = "361";
  map["12101_0_7_34_4_1_302400.obn.kolLa"] = "721";
  map["12101_0_7_34_4_1_302400.obn.kolData"] = "260281";
  map["12101_0_7_34_4_1_302400.obn.typeNet"] = "7";
  map["12101_0_7_34_4_1_302400.obn.getDescr"] = "12101";
  map["12101_0_7_34_4_1_302400.obn.getLevelType"] = "1";
  map["12101_0_7_34_4_1_302400.obn.getDate"] = "2018-02-06T18:00:00";
  map["12101_0_7_34_4_1_302400.obn.getHour"] = "302400";
  map["12101_0_7_34_4_1_302400.obn.getModel"] = "4";
  map["12101_0_7_34_4_1_302400.obn.getCenter"] = "34";
  map["12101_0_7_34_4_1_302400.obn.getCenterName"] = "";
  map["12101_0_7_34_4_1_302400.obn.getLevelTypeName"] = "";
  map["12101_0_7_34_4_1_302400.obn.getSmootch"] = "0";
  map["12101_0_7_34_4_1_302400.obn.max_val"] = "37.301";
  map["12101_0_7_34_4_1_302400.obn.min_val"] = "-53.4466";

  for ( const QString& n : inputFiles ) {
    obanal::TField field;
    CPPUNIT_ASSERT(field.fromFile(appPath + "/" + n));

//    none_log.nospace() << "map[\"" << n << ".kolFi\"] = \"" << QString::number(field.kolFi()) << "\";";
//    none_log.nospace() << "map[\"" << n << ".kolLa\"] = \"" << QString::number(field.kolLa()) << "\";";
//    none_log.nospace() << "map[\"" << n << ".kolData\"] = \"" << QString::number(field.kolData()) << "\";";
//    none_log.nospace() << "map[\"" << n << ".typeNet\"] = \"" << QString::number(field.typeNet()) << "\";";
//    none_log.nospace() << "map[\"" << n << ".getDescr\"] = \"" << QString::number(field.getDescr()) << "\";";
//    none_log.nospace() << "map[\"" << n << ".getLevelType\"] = \"" << QString::number(field.getLevelType()) << "\";";
//    none_log.nospace() << "map[\"" << n << ".getDate\"] = \"" << field.getDate().toString(Qt::ISODate) << "\";";
//    none_log.nospace() << "map[\"" << n << ".getHour\"] = \"" << QString::number(field.getHour()) << "\";";
//    none_log.nospace() << "map[\"" << n << ".getModel\"] = \"" << QString::number(field.getModel()) << "\";";
//    none_log.nospace() << "map[\"" << n << ".getCenter\"] = \"" << QString::number(field.getCenter()) << "\";";
//    none_log.nospace() << "map[\"" << n << ".getCenterName\"] = \"" << field.getCenterName() << "\";";
//    none_log.nospace() << "map[\"" << n << ".getLevelTypeName\"] = \"" << field.getLevelTypeName() << "\";";
//    none_log.nospace() << "map[\"" << n << ".getSmootch\"] = \"" << QString::number(field.getSmootch()) << "\";";
//    none_log.nospace() << "map[\"" << n << ".max_val\"] = \"" << QString::number(field.max_val()) << "\";";
//    none_log.nospace() << "map[\"" << n << ".min_val\"] = \"" << QString::number(field.min_val()) << "\";";

    CPPUNIT_ASSERT_EQUAL( map[n + ".kolFi"], QString::number(field.kolFi()) );
    CPPUNIT_ASSERT_EQUAL( map[n + ".kolLa"], QString::number(field.kolLa()) );
    CPPUNIT_ASSERT_EQUAL( map[n + ".kolData"], QString::number(field.kolData()) );
    CPPUNIT_ASSERT_EQUAL( map[n + ".typeNet"], QString::number(field.typeNet()) );
    CPPUNIT_ASSERT_EQUAL( map[n + ".getDescr"], QString::number(field.getDescr()) );
    CPPUNIT_ASSERT_EQUAL( map[n + ".getLevelType"], QString::number(field.getLevelType()) );
    CPPUNIT_ASSERT_EQUAL( map[n + ".getDate"], field.getDate().toString(Qt::ISODate) );
    CPPUNIT_ASSERT_EQUAL( map[n + ".getHour"], QString::number(field.getHour()) );
    CPPUNIT_ASSERT_EQUAL( map[n + ".getModel"], QString::number(field.getModel()) );
    CPPUNIT_ASSERT_EQUAL( map[n + ".getCenter"], QString::number(field.getCenter()) );
    CPPUNIT_ASSERT_EQUAL( map[n + ".getCenterName"], field.getCenterName() );
    CPPUNIT_ASSERT_EQUAL( map[n + ".getLevelTypeName"], field.getLevelTypeName() );
    CPPUNIT_ASSERT_EQUAL( map[n + ".getSmootch"], QString::number(field.getSmootch()) );
    CPPUNIT_ASSERT_EQUAL( map[n + ".max_val"], QString::number(field.max_val()) );
    CPPUNIT_ASSERT_EQUAL( map[n + ".min_val"], QString::number(field.min_val()) );
  }
}
