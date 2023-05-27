#include <cross-commons/debug/tlog.h>
#include "../thematiccalc.h"

template <class T>
static QString join(const QList<T> &list, const QString &separator = "")
{
  QString out;
  for(int i = 0; i<list.size(); i++)
    out += (i ? separator : "") + QString::number(list[i]);
  return out;
}

void test_0(){
  mappi::to::Channel A;
  for(int i=0; i<10; i++){
    A.push_back(i);
  }
  mappi::thematic::ThematicCalc calc;
  QString test = "A";
  calc.parsingExpression(test);
  calc.bindArray("A", A);
  calc.dataProcessing();
  auto result = calc.getResult();

  debug_log << "A = " << join(A.toList(), ", ");
  debug_log << "R = " << join(result.toList(), ", ");
}

void test_1()
{
  mappi::thematic::ThematicCalc calc;

  mappi::to::Channel A, B, C, D;
  for(int i = 0; i < 10; i++) {
    if(i == 0)
      A.push_back(i);
    else
      A.push_back(i);
    B.push_back(33.16 / (7.1 * (i + 1)));
    C.push_back(12.17 * i / 21.3);
    D.push_back(i * i);
  }
  calc.bindArray("A", A);
  calc.bindArray("B", B);

  QString test;

  //test = "-(-cos(-A))+(-sin(A))^|-2|!*5E2";
  test = "(A+50)";
  calc.parsingExpression(test);
  debug_log << QObject::tr("%1 = %2").arg(test).arg(calc.dataProcessing().last());

  calc.bindArray("C", C);
  calc.bindArray("D", D);

  test = "-A+50";
  calc.parsingExpression(test);
  debug_log << QObject::tr("%1 = %2").arg(test).arg(calc.dataProcessing().last());

  test = "-C*(-5!)";
  calc.parsingExpression(test);
  debug_log << QObject::tr("%1 = %2").arg(test).arg(calc.dataProcessing().first());

  test = "|-2.25|*(-A!)";
  calc.parsingExpression(test);
  debug_log << QObject::tr("%1 = %2").arg(test).arg(calc.dataProcessing().first());

  test = "|-2.25*B!|";
  calc.parsingExpression(test);
  debug_log << QObject::tr("%1 = %2").arg(test).arg(calc.dataProcessing().first());

  test = "D ^ (1/3)";
  calc.parsingExpression(test);
  debug_log << QObject::tr("%1 = %2").arg(test).arg(calc.dataProcessing().first());
}

void test_2()
{
  mappi::to::Channel A, B, C, D;
  for(int i = 0; i < 10; i++) {
    if(i == 0)
      A.push_back(0.5);
    else
      A.push_back(15.5 / (2.2 * (i + 1)));
    B.push_back(33.16 / (7.1 * (i + 1)));
    C.push_back(12.17 * i / 21.3);
    D.push_back(i * i);
  }

  mappi::thematic::ThematicCalc calc;
  QString test;

  //test = "5 % 2 ^ 2 - 5! + ((1 * B - (9)) / A + 2) ^ 8";
  test = "100 * ((B-A)/(B+A) + 1)";
  calc.parsingExpression(test);
  calc.bindArray("A", A);
  calc.bindArray("B", B);
  debug_log << QObject::tr("%1 = %2").arg(test).arg(calc.dataProcessing().first());

  test = "((1 * 2 + 9) / (-3!) + 2) ^ 8 + ((5 % 2) ^ 2) - 5!";
  calc.parsingExpression(test);
  debug_log << QObject::tr("%1 = %2").arg(test).arg(calc.dataProcessing().first());
}

void test_3(){
  mappi::to::Channel A, B, C;
  for(int i=0; i<10; i++){
    A.push_back(i);
    B.push_back(i);
    C.push_back(i);
  }
  mappi::thematic::ThematicCalc calc;
  QString test = "(100 * A) + (10 * B) + C";
  calc.parsingExpression(test);

  calc.bindArray("A", A);
  calc.bindArray("B", B);
  calc.bindArray("C", B);
  calc.dataProcessing();
  auto result = calc.getResult();
  debug_log << "A = " << join(A.toList(), ", ");
  debug_log << "B = " << join(B.toList(), ", ");
  debug_log << "C = " << join(C.toList(), ", ");
  debug_log << "R = " << join(result.toList(), ", ");
}

void test_4(){
  mappi::to::Channel A;
  for(int i=0; i<10; i++){
    A.push_back(250 + i);
  }
  mappi::thematic::ThematicCalc calc;
  QString test = "A";
  calc.parsingExpression(test);
  calc.bindArray("A", A);
  auto color = calc.dataProcessing();
  auto result = calc.getResult();

  debug_log << "A = " << join(A.toList(), ", ");
  debug_log << "C = " << join(color.toList(), ", ");
  debug_log << "R = " << join(result.toList(), ", ");
}

void test_5(){
  mappi::to::Channel A;
  for(int i=0; i<10; i++){
    A.push_back(1 + i);
  }
  A._header.lines = 2;
  A._header.samples = 5;

  mappi::thematic::ThematicCalc calc;
  QString test = "A";
  calc.parsingExpression(test);
  calc.bindArray("A", A);
  auto color = calc.dataProcessing();
  auto result = calc.getResult();

  debug_log << "A = " << join(A.toList(), ", ");
  debug_log << "C = " << join(color.toList(), ", ");
  debug_log << "R = " << join(result.toList(), ", ");
}

int main(int argv, char** argc)
{
  Q_UNUSED(argv);
  Q_UNUSED(argc);

  debug_log << "Test 0";
  test_0();
  debug_log << "\n\n\n" << "Test 1";
  test_1();
  debug_log << "\n\n\n" << "Test 2";
  test_2();
  debug_log << "\n\n\n" << "Test 3";
  test_3();
  debug_log << "\n\n\n" << "Test 4";
  test_4();
  debug_log << "\n\n\n" << "Test 5";
  test_5();

  return 0;
}
