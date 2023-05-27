#include "daemoncontrol.h"

#include <qcoreapplication.h>
#include <qsystemsemaphore.h>
#include <qsharedmemory.h>

namespace ProcControl {

bool isAlreadyRun(const QString& appId)
{
  QSystemSemaphore sem(appId + ".sem", 1);
  bool isRunning;
  sem.acquire();

  { // для *nix, чтобы освободить память если приложение завершилось аварийно
    QSharedMemory mem(appId + ".mem");
    mem.attach();
  }

  QSharedMemory* mem = new QSharedMemory(appId + ".mem", qApp);
  if ( mem->attach() ) {
    isRunning = true;
  }
  else {
    mem->create(1);
    isRunning = false;
  }

  sem.release();

  return isRunning;
}

} // ProcControl
