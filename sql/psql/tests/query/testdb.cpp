#include "testdb.h"

#include <cross-commons/debug/tlog.h>
#include <sql/psql/psql.h>


TestDb::TestDb( QObject* parent )
  : QObject(parent),
  base_(0)
{
  timer_ = new TTimer(this);
  connect( timer_, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
  timer_->setInterval(100);
}

TestDb::~TestDb() {
}

bool TestDb::connectToDb( const QString& host, const QString& name,
       const QString& login, const QString& passwd ) {
  if ( 0 != base_ ) {
    delete base_;
    base_ = 0;
  }

  base_ = new meteo::Psql( host, name, login, passwd );
  connect( base_, SIGNAL( connectionNonBlockFailed() ), this, SLOT( slotFailed() ) );
  connect( base_, SIGNAL( connectionNonBlockSucces() ), this, SLOT( slotConnected() ) );
  connect( base_, SIGNAL( connectionStatusChanged(ConnStatusType) ), this, SLOT( slotStatusChanged(ConnStatusType) ) );
  bool res = base_->connectNonBlock();
//  bool res = base_->Connect();
  if ( false != res ) {
    timer_->start();
  }
  return true;
}

void TestDb::slotFailed() {
  debug_log << "Connection failed!";
  timer_->stop();
}

void TestDb::slotConnected() {
  debug_log << "Connection succes!";
  timer_->stop();
}

void TestDb::slotStatusChanged( ConnStatusType type ) {
  debug_log << "connstatus changed. type =" << type;
}

void TestDb::slotTimeout() {
  debug_log << "slotTimeout() worked";
}
