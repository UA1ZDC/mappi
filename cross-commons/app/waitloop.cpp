#include "waitloop.h"

#include <qtimer.h>

namespace meteo {

WaitLoop::WaitLoop( QObject* p )
  : QObject(p),
  loop_(this)
{
}

WaitLoop::~WaitLoop()
{
}

void WaitLoop::wait( int32_t msecs )
{
  QTimer tm(this);
  tm.setInterval(msecs);
  QObject::connect( &tm, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
  tm.start();
  loop_.exec();
}

void WaitLoop::slotQuit()
{
  loop_.quit();
}

void WaitLoop::slotTimeout()
{
  loop_.quit();
}

}
