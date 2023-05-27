#include <QString>
#include <QtTest>

#include "tfield.h"

using namespace obanal;

class Test_PointValue : public QObject
{
  Q_OBJECT

public:
  Test_PointValue();

private Q_SLOTS:
  void initTestCase();
  void cleanupTestCase();
  void testCase();
  void testCase_data();

private:
  TField field_;

};

Test_PointValue::Test_PointValue()
{
}

void Test_PointValue::initTestCase()
{
  RegionParam rp;
  rp.start = meteo::GeoPoint::fromDegree(30.0, -20.0);
  rp.end = meteo::GeoPoint::fromDegree(60.0, 20.0);

  const double latStep = meteo::DEG2RAD * 5.0;
  const double lonStep = latStep;

  const double fieldValues[7][9] = {{29.8,31.8,33.6,34.9,35.9,36.5,36.9,37.0,36.9},
                                    {23.5,24.7,25.7,26.5,27.0,27.4,27.5,27.5,27.4},
                                    {19.5,20.2,20.8,21.3,21.6,21.8,21.9,21.9,21.9},
                                    {16.9,17.4,17.8,18.2,18.4,18.6,18.7,18.8,18.8},
                                    {15.0,15.4,15.8,16.1,16.3,16.5,16.7,16.8,16.8},
                                    {13.6,14.0,14.3,14.6,14.9,15.1,15.2,15.4,15.5},
                                    {12.4,12.9,13.2,13.5,13.7,13.8,14.0,14.2,14.3}};

  field_.setNet(rp, latStep, lonStep);

  for (int fiIndex = 0; fiIndex < field_.kolFi(); ++fiIndex) {
    for (int laIndex = 0; laIndex < field_.kolLa(); ++laIndex) {
      field_.setData(fiIndex, laIndex, fieldValues[fiIndex][laIndex], true);
    }
  }
}

void Test_PointValue::cleanupTestCase()
{
  field_.clear();
}

void Test_PointValue::testCase_data()
{
  QTest::addColumn<double>("lat");
  QTest::addColumn<double>("lon");

  QTest::newRow("value in node")  << 45.00 << 0.0;
  QTest::newRow("value not in node") << 52.32 << 13.45;
}

void Test_PointValue::testCase()
{
  QFETCH(double, lat);
  QFETCH(double, lon);

  meteo::GeoPoint gp = meteo::GeoPoint::fromDegree(lat, lon);

  bool value_ok, valueF_ok;
  value_ok = valueF_ok = false;
  double testValue  = field_.pointValue(gp, &value_ok);
  double testValueF = field_.pointValueF(gp, &valueF_ok);

  QVERIFY2(value_ok,  "Failure TField::pointValue");
  QVERIFY2(valueF_ok, "Failure TField::pointValueF");
  QCOMPARE(testValue, testValueF);
}

QTEST_APPLESS_MAIN(Test_PointValue)

#include "test_pointvalue.moc"
