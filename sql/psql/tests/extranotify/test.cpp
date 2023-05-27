#include <stdlib.h>

#include <qapplication.h>
#include <qtextcodec.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <sql/nspgbase/ns_pgbase.h>

#include "test.h"


void Test::slotNotify( const char* name, const char* extra )
{
  debug_log << "notify name =" << name;
  debug_log << "extra value =" << extra;
}

void Test::slotNotify( const char* name )
{
  debug_log << "notify name =" << name;
}

int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("test");
  QApplication app( argc, argv );

  NS_PGBase* base = new NS_PGBase( "10.10.11.201", "db_ptkpp", "postgres", "" );
  if ( false == base->Connect() ) {
    error_log << "can't connect to db";
    return EXIT_FAILURE;
  }

  Test* test = new Test;
  base->addNotify("new_message");
  QObject::connect( base, SIGNAL( notifyActivated( const char*, const char* ) ),
      test, SLOT( slotNotify( const char*, const char* ) ) );
  QObject::connect( base, SIGNAL( notifyActivated( const char* ) ),
      test, SLOT( slotNotify( const char* ) ) );

  app.exec();
  return EXIT_SUCCESS;
}
