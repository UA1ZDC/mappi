#include "autotest.h"
#include <meteo/commons/ui/graph/labellayout.h>

//
class Test_LabelLayout : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE( Test_LabelLayout );

//  CPPUNIT_TEST( optimPos );
//  CPPUNIT_TEST( tooLittleSpace );
  CPPUNIT_TEST( evenLabels );

  CPPUNIT_TEST_SUITE_END();

public:
  void optimPos();
  void tooLittleSpace();
  void evenLabels();
};


REGISTER_TEST( Test_LabelLayout, "label" );

//
//

class TestData
{
public:
  TestData() {}
  TestData(double b, double e)
  {
    beg = b;
    end = e;
  }

  double beg; // начало рамки
  double end; // конец рамки

  QVector<double> pos;  // позиция привязки подписи
  QVector<double> size; // высота подписи

  QVector<double> exp;  // ожидаемый редультат (смещение относительно позиции)

  TestData& add(double p, double sz, double e)
  {
    pos.append(p);
    size.append(sz);
    exp.append(e);
    return *this;
  }
};

//
//

using namespace meteo;

//! TEST: Подписи не пересекаются и могут быть расположены в оптимальных позициях.
void Test_LabelLayout::optimPos()
{
  // INPUT
  LabelLayout l;
  l.setRamkaPos(0,100);

  l.addLabel(10, 10);
  l.addLabel(25, 20);
  l.addLabel(50, 30);
  l.addLabel(70, 40);

  // OUTPUT
  QVector<double> exp(4, 0);
  exp[0] = 0;
  exp[1] = 0;
  exp[2] = 0;
  exp[3] = 0;

  // CHECK
  QVector<double> r = l.optimPosition();

  CPPUNIT_ASSERT( r.size() == exp.size() );

  for ( int i=0,isz=r.size(); i<isz; ++i ) {
    CPPUNIT_ASSERT_DOUBLES_EQUAL( exp[i], r[i], 0.0001 );
  }
}

//! TEST: Слишком мало места, чтобы разместить все подписи.
//! CASE: Подписи будут выходить за рамку на одинаковое расстояние с обоих концов.
void Test_LabelLayout::tooLittleSpace()
{
  // INPUT
  LabelLayout l;
  l.setRamkaPos(0,20);

  l.addLabel(5, 10);
  l.addLabel(5, 10);
  l.addLabel(10, 20);
  l.addLabel(15, 10);

  // OUTPUT
  QVector<double> exp(4, 0);
  exp[0] = -10;
  exp[1] = 0;
  exp[2] = 15;
  exp[3] = 20;

  // CHECK
  QVector<double> r = l.optimPosition();

  CPPUNIT_ASSERT( r.size() == exp.size() );

  for ( int i=0,isz=r.size(); i<isz; ++i ) {
    CPPUNIT_ASSERT_DOUBLES_EQUAL( exp[i], r[i], 0.0001 );
  }
}

//! TEST: Чётное количество подписей для размещения.
void Test_LabelLayout::evenLabels()
{
  // INPUT
  QList<TestData> input;
  input << TestData(0,100)
           .add(50, 10, 0);
  input << TestData(0,100)
           .add(50, 10, 5)
           .add(50, 10, -5);
  // симметрично относительно одной точки
  input << TestData(0,100)
           .add(50, 10, 15)
           .add(50, 10, 5)
           .add(50, 10, -5)
           .add(50, 10, -15)
           ;
  input << TestData(0,100)
           .add(50, 10, 10)
           .add(50, 10, 0)
           .add(50, 10, -10);
  // 2 подписи симметрично относительно одной точки и 2 по отдельности
  input << TestData(0,100)
           .add(100, 10, 55)
           .add(0, 10, -55)
           ;


  // CHECK
  foreach ( const TestData& td, input ) {
    LabelLayout l;
    l.setRamkaPos(td.beg, td.end);

    for ( int i=0,isz=td.pos.size(); i<isz; ++i ) {
      l.addLabel(td.pos[i], td.size[i]);
    }

    QVector<double> r = l.optimPosition();

    CPPUNIT_ASSERT( r.size() == td.exp.size() );

    for ( int i=0,isz=r.size(); i<isz; ++i ) {
      CPPUNIT_ASSERT_DOUBLES_EQUAL( td.exp[i], r[i], 0.0001 );
    }
  }
}
