#include "terror_test.h"

#include "../terror.h"

CPPUNIT_TEST_SUITE_REGISTRATION( TError_Test );

namespace test {
enum Error {
  NO_ERROR = 0,
  ERROR    = 1
};
}

void TError_Test::error()
{
  TError<test::Error,test::NO_ERROR> err;

  CPPUNIT_ASSERT_EQUAL( test::NO_ERROR, err.code() );
  CPPUNIT_ASSERT( err.text().isNull() );

  err << test::ERROR
      << "some error";

  CPPUNIT_ASSERT_EQUAL( test::ERROR, err.code() );
  CPPUNIT_ASSERT_EQUAL( std::string("some error"), err.text().toStdString() );

  err.reset();

  CPPUNIT_ASSERT_EQUAL( test::NO_ERROR, err.code() );
  CPPUNIT_ASSERT( err.text().isNull() );

  CPPUNIT_ASSERT( err );
  err << test::ERROR;

  CPPUNIT_ASSERT( !err );
}
