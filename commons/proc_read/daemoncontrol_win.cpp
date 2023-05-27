#include "daemoncontrol.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/funcs/mn_errdefs.h>

namespace ProcControl {

int daemonizeSock(const QString& name, bool daemon)
{
  Q_UNUSED( daemon );

  return isAlreadyRun(name) ? -1 : 0;
}

int singleInstanceSock(const QString& )
{
  not_impl;
  return -1;
}

int checkSockInUse( const QString&  )
{
  not_impl;
  return -1;
}

} // ProcControl
