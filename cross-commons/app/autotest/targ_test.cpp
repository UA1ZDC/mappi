#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

class TArg_Test : public CppUnit::TestCase
{
  CPPUNIT_TEST_SUITE( TArg_Test );
  CPPUNIT_TEST( oneSymbolKey );
  CPPUNIT_TEST( multySymbolKey );
  CPPUNIT_TEST( checkAndGetValue );
  CPPUNIT_TEST_SUITE_END();

public:
  void oneSymbolKey();
  void multySymbolKey();
  void checkAndGetValue();
};

#include <cross-commons/app/targ.h>
#include <cross-commons/debug/tlog.h>

#include <cstring>
#include <iostream>


CPPUNIT_TEST_SUITE_REGISTRATION( TArg_Test );


std::ostream & operator <<(std::ostream & os, const QString & s)
{
  os << s.toLocal8Bit().constData();
  return os;
}


void TArg_Test::oneSymbolKey()
{
  //! TEST: разбор односимвольных ключей и групп односимвольных ключей
  const QStringList list = QStringList()
                           << "app-name"
                           << "-s"
                           << "-H"
                           << "-xyz"
                           << "-a" << "abc"
                           << "-B" << "'B B B'"
                           << "-c" << "\"cd\""
                           << "-9"
                              ;

  char* input[list.size()];
  for( int i=0,sz=list.size(); i<sz; ++i ){
    input[i] = new char[list.at(i).length()+1];
    std::strcpy( input[i], list.at(i).toLocal8Bit().constData() );
  }

  TArg arg( list.size(), &input[0] );
  CPPUNIT_ASSERT_EQUAL( 9, arg.keyCount() );
  CPPUNIT_ASSERT( arg.contains("s") );
  CPPUNIT_ASSERT( arg.contains("H") );
  CPPUNIT_ASSERT( arg.contains("x") );
  CPPUNIT_ASSERT( arg.contains("y") );
  CPPUNIT_ASSERT( arg.contains("z") );
  CPPUNIT_ASSERT( arg.contains("a") );
  CPPUNIT_ASSERT( arg.contains("B") );
  CPPUNIT_ASSERT( arg.contains("c") );
  CPPUNIT_ASSERT( arg.contains("9") );

  CPPUNIT_ASSERT_EQUAL( QString("abc"), arg.value("a") );
  CPPUNIT_ASSERT_EQUAL( QString("B B B"), arg.value("B") );
  CPPUNIT_ASSERT_EQUAL( QString("cd"), arg.value("c") );
}

void TArg_Test::multySymbolKey()
{
  //! TEST: разбор многосимвольных ключей
  const QStringList list = QStringList()
                           << "app-name"
                           << "--abc"
                           << "--cpp-out"
                           << "--text='my text'"
                           << "--title=\"title\""
                           << "--date=12.01.2015"
                              ;

  char* input[list.size()];
  for( int i=0,sz=list.size(); i<sz; ++i ){
    input[i] = new char[list.at(i).length()+1];
    std::strcpy( input[i], list.at(i).toLocal8Bit().constData() );
  }

  TArg arg( list.size(), &input[0] );
  CPPUNIT_ASSERT_EQUAL( 5, arg.keyCount() );
  CPPUNIT_ASSERT( arg.contains("abc") );
  CPPUNIT_ASSERT( arg.contains("cpp-out") );
  CPPUNIT_ASSERT( arg.contains("text") );
  CPPUNIT_ASSERT( arg.contains("title") );
  CPPUNIT_ASSERT( arg.contains("date") );

  CPPUNIT_ASSERT_EQUAL( QString("my text"), arg.value("text") );
  CPPUNIT_ASSERT_EQUAL( QString("title"), arg.value("title") );
  CPPUNIT_ASSERT_EQUAL( QString("12.01.2015"), arg.value("date") );
}

void TArg_Test::checkAndGetValue()
{
  const QStringList list = QStringList()
                           << "app-name"
                           << "-d" << "'10.10.1980'"
                           << "--date=12.01.2015"
                              ;

  char* input[list.size()];
  for( int i=0,sz=list.size(); i<sz; ++i ){
    input[i] = new char[list.at(i).length()+1];
    std::strcpy( input[i], list.at(i).toLocal8Bit().constData() );
  }

  TArg arg( list.size(), &input[0] );
  CPPUNIT_ASSERT_EQUAL( QString("10.10.1980"), arg.value(QStringList() << "d" << "date") );
}
